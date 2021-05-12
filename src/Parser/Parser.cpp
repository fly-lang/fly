//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"

using namespace fly;

Parser::Parser(Lexer &Lex, DiagnosticsEngine &Diags) : Lex(Lex), Diags(Diags) {
}

bool Parser::Parse(ASTNode *Node) {
    AST = Node;
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse Package on first
    if (ParseNameSpace() && Tok.isNot(tok::eof)) {

        // Parse Imports
        if (ParseImportDecl() && Tok.isNot(tok::eof)) {

            // Parse All
            while (Tok.isNot(tok::eof)) {
                if (!ParseTopDecl()) {
                    return false;
                }
            }

            return AST->Finalize();
        }
    }

    return false;
}

DiagnosticBuilder Parser::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder Parser::Diag(const Token &Tok, unsigned DiagID) {
    return Diag(Tok.getLocation(), DiagID);
}

ASTNode *Parser::getAST() {
    return AST;
}

/**
 * Parse package declaration
 * @param fileName
 * @return true on Succes or false on Error
 */
bool Parser::ParseNameSpace() {
    // Check for first declaration
    if (Tok.is(tok::kw_namespace)) {
        SourceLocation StartLoc = Tok.getLocation();
        SourceLocation PackageNameLoc = ConsumeToken();
        if (Tok.isAnyIdentifier()) {
            StringRef Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            AST->setNameSpace(Name);
            return true;
        }

        Diag(Tok, diag::err_namespace_undefined);
        return false;

    }

    // Define Default NameSpace if it is not defined
    AST->setNameSpace();
    return true;
}

/**
 * Parse import declarations
 * @return
 */
bool Parser::ParseImportDecl() {
    if (Tok.is(tok::kw_import)) {
        const SourceLocation ImportLoc = ConsumeToken();
        if (Tok.is(tok::l_paren)) {
            ConsumeParen();

            // Parse import ( ... ) declarations
            return ParseImportParenDecl();

        } else if (Tok.isLiteral()) {

            // Parse import "..."
            StringRef Name = getLiteralString();

            // Syntax Error Quote
            if (Name.empty()) {
                Diag(Tok, diag::err_namespace_undefined);
                return false;
            }

            if (ParseImportAliasDecl(ImportLoc, Name)) {
                return ParseImportDecl();
            }
        }
    }

    // if parse "package" there is multiple package declarations
    if (Tok.is(tok::kw_namespace)) {

        // Multiple Package declaration is invalid, you can define only one
        Diag(Tok, diag::err_namespace_undefined);
        return false;
    }

    return true;
}

bool Parser::ParseTopDecl() {

    VisibilityKind Visibility = VisibilityKind::V_DEFAULT;
    bool Constant = false;

    // Parse Public or Private and Constant
    if (ParseTopScopes(Visibility, Constant)) {

        if (Tok.is(tok::kw_class)) {
            return ParseClassDecl(Visibility, Constant);
        }

        // Parse Type
        TypeBase *TyDecl = ParseType();

        if (TyDecl) {

            if (Tok.isAnyIdentifier()) {
                IdentifierInfo *Id = Tok.getIdentifierInfo();
                SourceLocation IdLoc = Tok.getLocation();
                ConsumeToken();
                if (Tok.is(tok::l_paren)) {
                    return ParseFunctionDecl(Visibility, Constant, TyDecl, Id, IdLoc);
                }
                return ParseGlobalVarDecl(Visibility, Constant, TyDecl, Id, IdLoc);
            }
        }
    }

    // Check Error: type without identifier
    return false;
}

StringRef Parser::getLiteralString() {
    StringRef Name(Tok.getLiteralData(), Tok.getLength());
    if (Name.startswith("\"") && Name.endswith("\"")) {
        StringRef StrRefName = Name.substr(1, Name.size()-2);
        ConsumeStringToken();
        return StrRefName;
    }

    return "";
}

bool Parser::ParseImportParenDecl() {
    if (Tok.isLiteral()) {
        SourceLocation ImportLoc = Tok.getLocation();
        StringRef Name = getLiteralString();
        if (Name.empty()) {
            Diag(Tok, diag::err_namespace_undefined);
            return false;
        }

        if (!ParseImportAliasDecl(ImportLoc, Name)) {
            return false;
        }


        if (Tok.is(tok::comma)) {
            ConsumeNext();
            return ParseImportParenDecl();
        }

        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            return true;
        }
    }

    return true;
}

bool Parser::ParseImportAliasDecl(const SourceLocation &Location, StringRef Name) {
    if (Tok.is(tok::kw_as)) {
        ConsumeToken();
        if (Tok.isLiteral()) {
            StringRef Alias = getLiteralString();

            return AST->addImport(new ImportDecl(Location, Name, Alias));
        }
    }
    return AST->addImport(new ImportDecl(Location, Name));
}

bool Parser::ParseTopScopes(VisibilityKind &Visibility, bool &Constant) {
    if (Tok.is(tok::kw_private)) {
        Visibility = VisibilityKind::V_PRIVATE;
        ConsumeToken();
        return ParseTopScopes(Visibility, Constant);
    } else if (Tok.is(tok::kw_public)) {
        Visibility = VisibilityKind::V_PUBLIC;
        ConsumeToken();
        return ParseTopScopes(Visibility, Constant);
    } else if (Tok.is(tok::kw_const)) {
        Constant = true;
        ConsumeToken();
        return ParseTopScopes(Visibility, Constant);
    }
    return true;
}

bool Parser::ParseScopes(bool &Constant) {
    if (Tok.is(tok::kw_const)) {
        Constant = true;
        ConsumeToken();
    }
    return true;
}

bool Parser::ParseGlobalVarDecl(VisibilityKind &VisKind, bool &Constant, TypeBase *TyDecl,
                                IdentifierInfo *Id, SourceLocation &IdLoc) {
    const StringRef &IdName = Id->getName();
    GlobalVarParser Parser(this, TyDecl, IdName, IdLoc);
    if (Parser.Parse()) {
        Parser.Var->Constant = Constant;
        Parser.Var->Visibility = VisKind;
        return AST->addGlobalVar(Parser.Var);
    }

    return false;
}

bool Parser::ParseClassDecl(VisibilityKind &VisKind, bool &Constant) {
    ClassParser Parser(this);
    if (Parser.Parse()) {
        Parser.Class->Constant = Constant;
        Parser.Class->Visibility = VisKind;
        return AST->addClass(Parser.Class);
    }

    return false;
}

bool Parser::ParseFunctionDecl(VisibilityKind &VisKind, bool Constant, TypeBase *TyDecl,
                               IdentifierInfo *Id, SourceLocation &IdLoc) {
    const StringRef &IdName = Id->getName();
    FunctionParser Parser(this, IdName, IdLoc);
    if (Parser.ParseDefinition(TyDecl)) {
        Parser.Function->Constant = Constant;
        Parser.Function->Visibility = VisKind;
        return AST->addFunction(Parser.Function);
    }

    return false;
}

bool Parser::ParseSingleStmt(StmtDecl *CurrentStmt) {
    if (Tok.is(tok::kw_const) || Tok.isAnyIdentifier() || isBuiltinType()) {
        // could be a variable assign or variable definition with custom type
        // or could be a function call
        return ParseVarOrFunc(CurrentStmt);
    } else if (isOpIncrement()) {
        IncDecExpr *Exp = ParseOpIncrement();
        ConsumeToken();
        if (Tok.isAnyIdentifier()) {
            VarRefDecl *VRefD = new VarRefDecl(Tok.getLocation(), Tok.getIdentifierInfo()->getName());
            ConsumeToken();
            VRefD->Expr->Group.push_back(Exp);
            return CurrentStmt->addVar(VRefD);
        }
    } else if (Tok.is(tok::kw_return)) {
        SourceLocation RetLoc = ConsumeToken();
        GroupExpr *Exp = ParseExpr();
        if (Exp) {
            ReturnDecl *Return = new ReturnDecl(RetLoc, Exp);
            CurrentStmt->Return = Return;
            CurrentStmt->Content.push_back(Return);
            return true;
        }
    }
    return false;
}

bool Parser::ParseInternalStmt(StmtDecl *CurrentStmt) {
    if (ParseSingleStmt(CurrentStmt)) {
        // Already all done into ParseSingleStmt()
        return true;
    }
    if (Tok.isOneOf(tok::kw_if, tok::kw_elsif, tok::kw_else)) {
        return ParseIfStmt(CurrentStmt);
    }
    if (Tok.is(tok::kw_switch)) {
        return ParseSwitchStmt(CurrentStmt);
    }
    if (Tok.is(tok::kw_for)) {
        return ParseSwitchStmt(CurrentStmt);
    }
    if (Tok.is(tok::kw_break)) {
        CurrentStmt->Content.push_back(new BreakDecl(ConsumeToken()));
        return true;
    }
    if (Tok.is(tok::kw_continue)) {
        CurrentStmt->Content.push_back(new ContinueDecl(ConsumeToken()));
        return true;
    }

    Diag(Tok.getLocation(), diag::err_parse_stmt);
    return false;
}

bool Parser::ParseStmt(StmtDecl *CurrentStmt) {
    // Parse until find a }
    while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {
        ParseInternalStmt(CurrentStmt);
    }

    if (Tok.is(tok::r_brace)) { // Close Brace
        ConsumeBrace();
        return true;
    }

    Diag(Tok.getLocation(), diag::err_parse_stmt);
    return false;
}

bool Parser::ParseIfStmt(StmtDecl *CurrentStmt) {
    CondStmtDecl *Stmt;
    const SourceLocation &Loc = Tok.getLocation();
    switch (Tok.getKind()) {
        case tok::kw_if:
            ConsumeToken();
            Stmt = new IfStmtDecl(Loc, CurrentStmt);
            break;
        case tok::kw_elsif:
            ConsumeToken();
            Stmt = new ElsifStmtDecl(Loc, CurrentStmt);
            IfStmtDecl::AddBranch(CurrentStmt, Stmt);
            break;
        case tok::kw_else:
            ConsumeToken();
            Stmt = new ElseStmtDecl(Loc, CurrentStmt);
            IfStmtDecl::AddBranch(CurrentStmt, Stmt);
            break;
        default:
            assert("Unknow conditional statement");
    }

    // Only for If and Elsif
    if (Tok.is(tok::l_paren)) {
        ConsumeParen();

        if (Stmt->getStmtKind() == StmtKind::D_STMT_IF || Stmt->getStmtKind() == StmtKind::D_STMT_ELSIF) {
            GroupExpr *Group = ParseExpr();
            if (Group) {
                static_cast<IfStmtDecl *>(Stmt)->Condition = Group;
            }

            if (Tok.is(tok::r_paren)) {
                ConsumeParen();
            }
        }
    }

    // If, Elsif, Else
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (ParseStmt(Stmt)) {
            CurrentStmt->Content.push_back(Stmt);
            return true;
        }
    } else if (ParseSingleStmt(Stmt)) {
        CurrentStmt->Content.push_back(Stmt);
        return true;
    }

    return false;
}

bool Parser::ParseSwitchStmt(StmtDecl *CurrentStmt) {
    if (Tok.is(tok::kw_switch)) {
        const SourceLocation &SwitchLoc = ConsumeToken();

        if (Tok.is(tok::l_paren)) {
            ConsumeParen();

            VarRef *Ref = ParseVarRef();
            if (Ref) {
                if (Tok.is(tok::r_paren)) {
                    ConsumeParen();

                    SwitchStmtDecl *Stmt = new SwitchStmtDecl(SwitchLoc, CurrentStmt, Ref);
                    if (Tok.is(tok::l_brace)) {
                        ConsumeBrace();

                        while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {
                            if (Tok.is(tok::kw_case)) {
                                const SourceLocation &CaseLoc = ConsumeToken();

                                Expr * CaseExp = NULL;
                                if (isValue()) {
                                    CaseExp = ParseValueExpr();
                                } else if (Tok.isAnyIdentifier()) {
                                    IdentifierInfo *Id = Tok.getIdentifierInfo();
                                    CaseExp = new VarRefExpr(
                                            Tok.getLocation(),new VarRef(Tok.getLocation(), Id->getName()));
                                    ConsumeToken();
                                } else {
                                    // TODO Error
                                }

                                if (Tok.is(tok::colon)) {
                                    ConsumeToken();

                                    CaseStmtDecl *CaseStmt = Stmt->AddCase(CaseLoc, CaseExp);
                                    while (!Tok.isOneOf(tok::r_brace, tok::kw_case, tok::kw_default, tok::eof)) {
                                        ParseInternalStmt(CaseStmt);
                                    }
                                }
                            } else if (Tok.is(tok::kw_default)) {
                                ConsumeToken();

                                if (Tok.is(tok::colon)) {
                                    const SourceLocation &Loc = ConsumeToken();
                                    DefaultStmtDecl *DefStmt = Stmt->AddDefault(Loc);
                                    ParseInternalStmt(DefStmt);
                                } else {
                                    // TODO add error, missing :
                                }
                            }
                        }

                        if (Tok.is(tok::r_brace)) {
                            ConsumeBrace();

                            CurrentStmt->Content.push_back(Stmt);
                            return true;
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

bool Parser::ParseForStmt(StmtDecl *CurrentStmt) {
    return false;
}

FuncRefDecl *Parser::ParseFunctionRefDecl(IdentifierInfo *Id, SourceLocation &IdLoc) {
    const StringRef &IdName = Id->getName();
    FunctionParser Parser(this, IdName, IdLoc);
    Parser.ParseRefDecl();
    return Parser.Invoke;
}

VarDecl *Parser::ParseVarDecl(bool Constant, TypeBase *TyDecl) {
    // Var Name
    const StringRef Name = Tok.getIdentifierInfo()->getName();
    const SourceLocation IdLoc = Tok.getLocation();
    ConsumeToken();
    VarDecl *Var = new VarDecl(IdLoc, TyDecl, Name);
    Var->Constant = Constant;

    // Parsing =, +=, -=, ...
    if (isOpAssign()) {
        VarRef *VRef = new VarRef(Tok.getLocation(), Var->getName());
        VRef->Var = Var;
        ParseOpAssign(VRef);
        ConsumeToken();

        GroupExpr* Ex = ParseExpr();
        if (Ex) {
            Var->Expression = Ex;
        }
    }

    return Var;
}

VarDecl* Parser::ParseVarDecl() {
    // Var Constant
    bool Constant = false;
    ParseScopes(Constant);

    // Var Type
    TypeBase *TyDecl = ParseType();

    return ParseVarDecl(Constant, TyDecl);
}

VarRef* Parser::ParseVarRef() {
    VarRef *VRef = new VarRef(Tok.getLocation(), Tok.getIdentifierInfo()->getName());
    ConsumeToken();
    return VRef;
}

ValueExpr *Parser::ParseValueExpr() {
    if (Tok.is(tok::numeric_constant)) {
        const StringRef V = StringRef(Tok.getLiteralData(), Tok.getLength());
        SourceLocation Loc = ConsumeToken();
        return new ValueExpr(Loc, V);
    } else if (Tok.isOneOf(tok::kw_true, tok::kw_false)) {
        StringRef B = ParseBoolValue();
        SourceLocation Loc = ConsumeToken();
        return new ValueExpr(Loc, B);
    } else {
        assert("Not a value");
    }
}

GroupExpr* Parser::ParseExpr(GroupExpr *CurrGroup) {
//    SourceLocation Loc = Tok.getLocation();
    if (CurrGroup == NULL) {
        CurrGroup = new GroupExpr();
    }

    // Start Parsing
    if (isValue()) {
        ValueExpr *Val = ParseValueExpr();
        if (Val) {
            CurrGroup->Group.push_back(Val);
        }
    } else if (Tok.isAnyIdentifier()) {
        IdentifierInfo *Id = Tok.getIdentifierInfo();
        SourceLocation Loc = ConsumeToken();
        if (Tok.is(tok::l_paren)) { // function invocation
            // a()
            FuncRefDecl *FuncRef = ParseFunctionRefDecl(Id, Loc);
            if (FuncRef) {
                CurrGroup->Group.push_back(new FuncRefExpr(Loc, FuncRef));
            }
        } else {
            CurrGroup->Group.push_back(new VarRefExpr(Loc, new VarRef(Loc, Id->getName())));
        }
    } else if (Tok.is(tok::l_paren)) {
        ConsumeToken();
        GroupExpr *GroupE = new GroupExpr();
        ParseExpr(GroupE);
        if (Tok.is(tok::r_paren)) {
            ConsumeToken();
            CurrGroup->Group.push_back(GroupE);
        }
    }

    // Always check if there is an operator
    if (isOperator()) {
        Expr* E = ParseOperator();
        ConsumeToken();
        CurrGroup->Group.push_back(E);
        ParseExpr(CurrGroup);
    }

    return CurrGroup;
}

/**
 * Parse Variable or Function invocation
 *
 * cont int a
 * a = ...
 * a()
 * a v1 = ...
 * int a = ...
 *
 * @param CurrentStmt
 * @return
 */
bool Parser::ParseVarOrFunc(StmtDecl *CurrentStmt) {
    // cont int a
    bool Constant = false;
    ParseScopes(Constant);

    SourceLocation Loc = Tok.getLocation();
    tok::TokenKind Kind = Tok.getKind();
    IdentifierInfo *VarOrFuncOrType = NULL;
    if (Tok.isAnyIdentifier()) {
        // a = ... (Var)
        // a()     (Func)
        // T a = ... (Type)
        VarOrFuncOrType = Tok.getIdentifierInfo();
        ConsumeToken();
    } else if (isBuiltinType()) {
        // int a = ...
        VarDecl* Var = ParseVarDecl(Constant, ParseType());
        return CurrentStmt->addVar(Var);
    }

    if (Tok.isAnyIdentifier()) { // variable definition
        // T a = ...
        StringRef Name = VarOrFuncOrType->getName();
        TypeBase *TyDecl = new ClassTypeRef(Loc, Name);
        if (TyDecl) {
            VarDecl* Var = ParseVarDecl(Constant, TyDecl);
            return CurrentStmt->addVar(Var);
        }
    } else if (Tok.is(tok::l_paren)) { // function invocation
        // a()
        FuncRefDecl *Invoke = ParseFunctionRefDecl(VarOrFuncOrType, Loc);
        if (Invoke)
            return CurrentStmt->addInvoke(Invoke);
    } else if (isOpAssign()) {
        GroupExpr *GrExp = NULL;
        VarRefDecl *VDecl = new VarRefDecl(Loc, VarOrFuncOrType->getName());
        if (Tok.isNot(tok::equal)) {
            GrExp = ParseOpAssign(VDecl);
        }
        ConsumeToken();

        GroupExpr* Ex = ParseExpr(GrExp);
        if (Ex) {
            VDecl->Expr = Ex;
            return CurrentStmt->addVar(VDecl);
        }
    } else if (isOpIncrement()) {
        IncDecExpr* Exp = ParseOpIncrement(true);
        ConsumeToken();
        VarRefDecl *VRefD = new VarRefDecl(Loc, VarOrFuncOrType->getName());
        VRefD->Expr->Group.push_back(Exp);
        return CurrentStmt->addVar(VRefD);
    }

    return false;
}

bool Parser::isVoidType() {
    return Tok.is(tok::kw_void);
}

bool Parser::isBuiltinType() {
    return Tok.isOneOf(tok::kw_int, tok::kw_bool, tok::kw_float);
}

bool Parser::isOpAssign() {
    return Tok.isOneOf(tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal, tok::percentequal,
                       tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal, tok::greatergreaterequal,
                       tok::equal);
}

bool Parser::isOpIncrement() {
    return Tok.isOneOf(tok::plusplus, tok::minusminus);
}

bool Parser::isOperator() {
    return Tok.isOneOf(tok::plus, tok::minus, tok::star, tok::slash, tok::percent,
                       tok::amp, tok::pipe, tok::caret,
                       tok::ampamp, tok::pipepipe, tok::exclaim,
                       tok::less, tok::lessless, tok::lessequal, tok::greater, tok::greatergreater, tok::greaterequal,
                       tok::equalequal, tok::exclaimequal,
                       tok::question, tok::colon);
}

TypeBase* Parser::ParseType() {
    TypeBase *TyDecl = ParseType(Tok.getLocation(), Tok.getKind());
    ConsumeToken();
    return TyDecl;
}

TypeBase* Parser::ParseType(SourceLocation Loc, tok::TokenKind Kind) {
    TypeBase *TyDecl = NULL;
    switch (Kind) {
        case tok::kw_bool:
            TyDecl = new BoolPrimType(Loc);
            break;
        case tok::kw_int:
            TyDecl = new IntPrimType(Loc);
            break;
        case tok::kw_float:
            TyDecl = new FloatPrimType(Loc);
            break;
        case tok::kw_void:
            TyDecl = new VoidRetType(Loc);
            break;
        default:
            StringRef Name = Tok.getIdentifierInfo()->getName();
            if (Name.empty()) {
                Diag(Loc, diag::err_type_undefined);
            } else {
                TyDecl = new ClassTypeRef(Loc, Name);
            }
    }

    return TyDecl;
}

StringRef Parser::ParseBoolValue() {
    switch (Tok.getKind()) {
        case tok::kw_true:
            return "true";
        case tok::kw_false:
            return "false";
        default:
            assert("Bool value not accepted");
    }
}

bool Parser::isValue() {
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false);
}

OperatorExpr* Parser::ParseOperator() {
    SourceLocation Loc = Tok.getLocation();
    switch (Tok.getKind()) {

        // Increment / Decrement
        case tok::plusplus:
            return new ArithExpr(Loc, ArithOpKind::ARITH_INCR);
        case tok::minusminus:
            return new ArithExpr(Loc, ArithOpKind::ARITH_DECR);

        // Bit
        case tok::amp:
            return new BitExpr(Loc, BitOpKind::BIT_AND);
        case tok::pipe:
            return new BitExpr(Loc, BitOpKind::BIT_OR);
        case tok::caret:
            return new BitExpr(Loc, BitOpKind::BIT_NOT);
        case tok::lessless:
            return new BitExpr(Loc, BitOpKind::BIT_SHIFT_L);
        case tok::greatergreater:
            return new BitExpr(Loc, BitOpKind::BIT_SHIFT_R);

        // Boolean
        case tok::ampamp:
            return new BoolExpr(Loc, BoolOpKind::BOOL_AND);
        case tok::pipepipe:
            return new BoolExpr(Loc, BoolOpKind::BOOL_OR);
        case tok::exclaim:
            return new BoolExpr(Loc, BoolOpKind::BOOL_NOT);

        // Arithmetic
        case tok::plus:
            return new ArithExpr(Loc, ArithOpKind::ARITH_ADD);
        case tok::minus:
            return new ArithExpr(Loc, ArithOpKind::ARITH_SUB);
        case tok::star:
            return new ArithExpr(Loc, ArithOpKind::ARITH_MUL);
        case tok::slash:
            return new ArithExpr(Loc, ArithOpKind::ARITH_DIV);
        case tok::percent:
            return new ArithExpr(Loc, ArithOpKind::ARITH_MOD);

        // Logic
        case tok::less:
            return new LogicExpr(Loc, LogicOpKind::LOGIC_LT);
        case tok::lessequal:
            return new LogicExpr(Loc, LogicOpKind::LOGIC_LTE);
        case tok::greater:
            return new LogicExpr(Loc, LogicOpKind::LOGIC_GT);
        case tok::greaterequal:
            return new LogicExpr(Loc, LogicOpKind::LOGIC_GTE);
        case tok::exclaimequal:
            return new LogicExpr(Loc, LogicOpKind::LOGIC_NE);
        case tok::equalequal:
            return new LogicExpr(Loc, LogicOpKind::LOGIC_EQ);

        // Condition
        case tok::question:
            return new CondExpr(Loc, CondOpKind::COND_THAN);
        case tok::colon:
            return new CondExpr(Loc, CondOpKind::COND_ELSE);

        default:
            assert("Operator not accepted");
    }
}

GroupExpr* Parser::ParseOpAssign(VarRef *Ref) {
    SourceLocation Loc = Tok.getLocation();
    GroupExpr* Gr = new GroupExpr;
    VarRefExpr *VRefE = new VarRefExpr(Loc, Ref);
    Gr->Group.push_back(VRefE);
    switch (Tok.getKind()) {

        // Arithmetic
        case tok::plusequal:
            Gr->Group.push_back( new ArithExpr(Loc, ArithOpKind::ARITH_ADD) );
            break;
        case tok::minusequal:
            Gr->Group.push_back( new ArithExpr(Loc, ArithOpKind::ARITH_SUB) );
            break;
        case tok::starequal:
            Gr->Group.push_back( new ArithExpr(Loc, ArithOpKind::ARITH_MUL) );
            break;
        case tok::slashequal:
            Gr->Group.push_back( new ArithExpr(Loc, ArithOpKind::ARITH_DIV) );
            break;
        case tok::percentequal:
            Gr->Group.push_back( new ArithExpr(Loc, ArithOpKind::ARITH_MOD) );
            break;

            // Bit
        case tok::ampequal:
            Gr->Group.push_back( new BitExpr(Loc, BitOpKind::BIT_AND));
            break;
        case tok::pipeequal:
            Gr->Group.push_back( new BitExpr(Loc, BitOpKind::BIT_OR));
            break;
        case tok::caretequal:
            Gr->Group.push_back( new BitExpr(Loc, BitOpKind::BIT_NOT));
            break;
        case tok::lesslessequal:
            Gr->Group.push_back( new BitExpr(Loc, BitOpKind::BIT_SHIFT_L));
            break;
        case tok::greatergreaterequal:
            Gr->Group.push_back( new BitExpr(Loc, BitOpKind::BIT_SHIFT_R));
            break;
        default:
            assert("Only assign operations are accepted");
    }
    return Gr;
}

IncDecExpr* Parser::ParseOpIncrement(bool post) {
    switch (Tok.getKind()) {
        case tok::plusplus:
            return new IncDecExpr(Tok.getLocation(), post ? IncDecOpKind::POST_INCREMENT : IncDecOpKind::PRE_INCREMENT);
        case tok::minusminus:
            return new IncDecExpr(Tok.getLocation(), post ? IncDecOpKind::POST_DECREMENT : IncDecOpKind::PRE_DECREMENT);
        default:
            assert("Only Increment ++ or Decrement -- are accepted");
    }
}
