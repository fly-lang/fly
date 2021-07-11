//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "AST/ImportDecl.h"

using namespace fly;

Parser::Parser(InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags) : Input(Input),Diags(Diags),
            Lex(Input.getFileID(), Input.getBuffer(), SourceMgr) {

}

bool Parser::Parse(ASTNode *Node) {
    AST = Node;
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse Package on first
    if (ParseNameSpace()) {

        // Parse Imports
        if (ParseImportDecl()) {

            // If Node is empty it is not added to the Context
            if (Tok.is(tok::eof)) return true;

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
        if (Tok.is(tok::kw_default)) {
            ConsumeToken();
            AST->setDefaultNameSpace();
            return true;
        }

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
    AST->setDefaultNameSpace();
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

bool Parser::ParseConstant(bool &Constant) {
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
    if (Parser.ParseDecl(TyDecl)) {
        Parser.Function->Constant = Constant;
        Parser.Function->Visibility = VisKind;
        return AST->addFunction(Parser.Function);
    }

    return false;
}

/**
 * Parse a single statement like Variable declaration, assignment or Function invocation
 *
 * cont int a
 * a = ...
 * a()
 * a v1 = ...
 * int a = ...
 *
 * @param Block
 * @return
 */
bool Parser::ParseStmt(BlockStmt *Block, GroupExpr *Group) {
    // Parse return
    if (Tok.is(tok::kw_return)) {
        SourceLocation RetLoc = ConsumeToken();
        GroupExpr *Exp = ParseExpr(Block);
        if (Exp) {
            Block->addReturn(RetLoc, Exp);
            return true;
        }
    } else if (Tok.is(tok::kw_break)) {
        ;
        Block->addBreak(ConsumeToken());
    } else if (Tok.is(tok::kw_continue)) {
        Block->addContinue(ConsumeToken());
    }

    // const int a
    bool Constant = false;
    ParseConstant(Constant);

    SourceLocation Loc = Tok.getLocation();
    IdentifierInfo *Id = NULL;
    if (Tok.isAnyIdentifier()) {
        // a = ... (Var)
        // a()     (Func)
        // T a = ... (Type)
        Id = Tok.getIdentifierInfo();
        ConsumeToken();
    } else if (isBuiltinType()) {
        // int a = ...
        VarDeclStmt* Var = ParseVarDecl(Block, Constant, ParseType());
        return Block->addVar(Var);
    }

    if (Tok.isAnyIdentifier()) { // variable definition
        // T a = ...
        StringRef Name = Id->getName();
        TypeBase *TyDecl = new ClassTypeRef(Loc, Name);
        if (TyDecl) {
            VarDeclStmt* Var = ParseVarDecl(Block, Constant, TyDecl);
            return Block->addVar(Var);
        }
    } else if (Tok.is(tok::l_paren)) { // function invocation
        // a()
        if (Constant) {
            // TODO Error cannot use const with function call
        }
        FuncCall *Call = ParseFunctionCall(Block, Id, Loc);
        if (Call)
            return Block->addCall(Call);
    } else if (isOpAssign()) {
        GroupExpr *GrExp = NULL;
        VarStmt *VStmt = new VarStmt(Loc, Block, Id->getName());
        if (Tok.isNot(tok::equal)) {
            GrExp = ParseOpAssign(VStmt);
        }
        ConsumeToken();

        GroupExpr* Ex = ParseExpr(Block, GrExp);
        if (Ex) {
            VStmt->setExpr(Ex);
            return Block->addVar(VStmt);
        }
    } else if (isOpIncDec()) {
        VarStmt *S = ParseIncDec(Loc, Block, Id);
        if (S) {
            return Block->addVar(S);
        }
    } else if (isOperator()) {

        if (Group == NULL) {
            // TODO Error this is an Expression only a Declaration is permitted
            return false;
        }

        VarRefExpr *VRef = new VarRefExpr(Loc, new VarRef(Loc, Id->getName()));
        Group->Add(VRef);
        return ParseExpr(Block, Group);
    }

    return false;
}

/**
 * Parse statements not yet contained into braces
 * @param Block
 * @return
 */
bool Parser::ParseBlock(BlockStmt *Block) {
    if (ParseStmt(Block)) {
        // Already all done into ParseSingleStmt()
        return true;
    }
    if (Tok.isOneOf(tok::kw_if, tok::kw_elsif, tok::kw_else)) {
        return ParseIfStmt(Block);
    }
    if (Tok.is(tok::kw_switch)) {
        return ParseSwitchStmt(Block);
    }
    if (Tok.is(tok::kw_for)) {
        return ParseForStmt(Block);
    }
    if (Tok.is(tok::kw_break)) {
        Block->Content.push_back(new BreakStmt(ConsumeToken(), Block));
        return true;
    }
    if (Tok.is(tok::kw_continue)) {
        Block->Content.push_back(new ContinueStmt(ConsumeToken(), Block));
        return true;
    }

    Diag(Tok.getLocation(), diag::err_parse_stmt);
    return false;
}

/**
 * Parse statements between braces
 * @param Block
 * @return
 */
bool Parser::ParseInnerBlock(BlockStmt *Block) {
    // Parse until find a }
    while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {
        if (!ParseBlock(Block)) {
            Diag(Tok.getLocation(), diag::err_parse_stmt);
            return false;
        }
    }

    if (Tok.is(tok::r_brace)) { // Close Brace
        ConsumeBrace();
        return true;
    }

    Diag(Tok.getLocation(), diag::err_parse_stmt);
    return false;
}

/**
 * Parse open paren ( at start of cycle into condition statements
 * @return
 */
bool Parser::ParseStartParen() {
    bool hasParen = false;
    if (Tok.is(tok::l_paren)) {
        ConsumeParen();
        hasParen = true;
    }
    return hasParen;
}

/**
 * Parse close paren ) at end of cycle into condition statements
 * @return
 */
bool Parser::ParseEndParen(bool hasParen) {
    if (Tok.is(tok::r_paren)) {
        if (hasParen) {
            ConsumeParen();
            return true;
        } else {
            // TODO Error missing start paren
        }
    }
    return false;
}

/**
 * Parse If, Elseif and Else statements
 *
 *  if ... {
 *    ...
 *  } elsif ... {
 *    ...
 *  } else {
 *    ...
 *  }
 *
 *  or
 *
 *  if (...) ...
 *  elsif (...) ...
 *  else ...
 *
 * @param Block
 * @return
 */
bool Parser::ParseIfStmt(BlockStmt *Block) {
    ConditionBlockStmt *Stmt;
    const SourceLocation &Loc = Tok.getLocation();

    // Init the current statement parsing if, elsif or else keywords
    switch (Tok.getKind()) {
        case tok::kw_if:
            ConsumeToken();
            Stmt = new IfBlockStmt(Loc, Block);
            break;
        case tok::kw_elsif:
            ConsumeToken();
            Stmt = new ElsifBlockStmt(Loc, Block);
            IfBlockStmt::AddBranch(Block, Stmt);
            break;
        case tok::kw_else:
            ConsumeToken();
            Stmt = new ElseBlockStmt(Loc, Block);
            IfBlockStmt::AddBranch(Block, Stmt);
            break;
        default:
            assert("Unknow conditional statement");
    }

    // Check parenthesis content only for If and Elsif
    // Parse (
    bool hasParen = ParseStartParen();

    if (Stmt->getBlockKind() == BlockStmtKind::BLOCK_STMT_IF ||
            Stmt->getBlockKind() == BlockStmtKind::BLOCK_STMT_ELSIF) {

        // Parse the group of expressions into parenthesis
        GroupExpr *Group = ParseExpr(Block);
        if (Group) {
            static_cast<IfBlockStmt *>(Stmt)->Condition = Group;
        }

        // Consume Right Parenthesis ) if exists
        ParseEndParen(hasParen);
    }

    // Parse statement between braces for If, Elsif, Else
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (ParseInnerBlock(Stmt)) {
            Block->Content.push_back(Stmt);
            return true;
        }
    } else if (ParseStmt(Stmt)) { // Only for a single Stmt without braces
        Block->Content.push_back(Stmt);
        return true;
    }

    return false;
}

/**
 * Parse the Switch statement
 *
 * switch ... {"
 *  case 1:
 *      ...
 *      break
 *  case 2:
 *      ...
 *  default:"
 *      ...
 * }"
 *
 * @param CurrStmt
 * @return
 */
bool Parser::ParseSwitchStmt(BlockStmt *CurrStmt) {
    // Parse switch keyword
    if (Tok.is(tok::kw_switch)) {
        const SourceLocation &SwitchLoc = ConsumeToken();

        // Parse (
        bool hasParen = ParseStartParen();

        // Parse Var reference like (a)
        VarRef *VRef = ParseVarRef();
        if (VRef) {

            // Consume Right Parenthesis ) if exists
            ParseEndParen(hasParen);

            // Init Switch Statement and start parse from brace
            SwitchBlockStmt *Stmt = new SwitchBlockStmt(SwitchLoc, CurrStmt, VRef);
            if (Tok.is(tok::l_brace)) {
                ConsumeBrace();

                // Exit only Statement find a closed brace or EOF
                while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {

                    // Parse case keyword
                    if (Tok.is(tok::kw_case)) {
                        const SourceLocation &CaseLoc = ConsumeToken();

                        // Parse Expression for different cases
                        // for a Value  -> case 1:
                        // or for a Var -> case a:
                        // or for a default
                        Expr * CaseExp = NULL;
                        VarRef *VRef = NULL;
                        if (isValue()) {
                            CaseExp = ParseValueExpr();
                        } else if (Tok.isAnyIdentifier()) {
                            IdentifierInfo *Id = Tok.getIdentifierInfo();
                            VRef = new VarRef(Tok.getLocation(), Id->getName());
                            CaseExp = new VarRefExpr(Tok.getLocation(), VRef);
                            ConsumeToken();
                        } else {
                            // TODO Error
                        }
                        if (Tok.is(tok::colon)) {
                            ConsumeToken();

                            // Add Case to Switch statement and parse statement not contained into braces
                            CaseBlockStmt *CaseStmt = Stmt->AddCase(CaseLoc, CaseExp);
                            while (!Tok.isOneOf(tok::r_brace, tok::kw_case, tok::kw_default, tok::eof)) {
                                ParseBlock(CaseStmt);
                            }
                        }
                    } else if (Tok.is(tok::kw_default)) {
                        ConsumeToken();

                        if (Tok.is(tok::colon)) {
                            const SourceLocation &Loc = ConsumeToken();

                            // Add Default to Switch statement and parse statement not contained into braces
                            DefaultBlockStmt *DefStmt = Stmt->AddDefault(Loc);
                            ParseBlock(DefStmt);
                        } else {
                            // TODO add error, missing :
                        }
                    }
                }

                // Switch statement is at end of it's time add current Switch to parent statement
                if (Tok.is(tok::r_brace)) {
                    ConsumeBrace();
                    CurrStmt->Content.push_back(Stmt);
                    return true;
                }
            }
        }
    }
    
    return false;
}

/**
 * Parse the For statement
 *
 * for {
 *  ...
 * }
 *
 * or
 *
 * for ... {
 *  ...
 * }
 *
 * or
 *
 * for ...; ...; ... {
 *  ...
 * }
 *
 * @param CurrStmt
 * @return
 */
bool Parser::ParseForStmt(BlockStmt *CurrStmt) {
    if (Tok.is(tok::kw_for)) {
        const SourceLocation &ForLoc = ConsumeToken();

        bool hasParen = ParseStartParen();

        // Init For Statement and start parse components
        ForBlockStmt *Stmt = new ForBlockStmt(ForLoc, CurrStmt);
        Stmt->Init = new BlockStmt(Tok.getLocation(), Stmt);
        Stmt->Cond = new GroupExpr();
        Stmt->Post = new BlockStmt(Tok.getLocation(), Stmt);
        // could be an Init component, or a Condition component or empty 
        if (Tok.is(tok::kw_const) || Tok.isAnyIdentifier() || isBuiltinType()) {
            // could be a variable assign or variable definition with custom type
            // or could be a function call
            if (ParseInitForStmt(Stmt->Init, Stmt->Cond)) {

                // This is an Expression, it could be a Condition
                if (Stmt->Init->isEmpty() && !Stmt->Cond->isEmpty() && !Tok.isOneOf(tok::comma, tok::semi)) {
                    ParseCondForStmt(CurrStmt, Stmt->Cond);
                }

                if (Tok.is(tok::semi)) {
                    ConsumeToken();

                    if (ParseCondForStmt(CurrStmt, Stmt->Cond)) {
                        if (Tok.is(tok::semi)) {
                            ConsumeToken();

                            Stmt->Post = new BlockStmt(Tok.getLocation(), Stmt);
                            ParsePostForStmt(Stmt->Post);
                        }
                    }
                }
            } else if (Stmt->Cond) {
                 // TODO
            }
        }

        // Consume Right Parenthesis ) if exists
        ParseEndParen(hasParen);

        // Parse statement between braces
        if (Tok.is(tok::l_brace)) {
            ConsumeBrace();
            if (ParseInnerBlock(Stmt)) {
                CurrStmt->Content.push_back(Stmt);
                return true;
            }
        } else if (ParseStmt(Stmt)) { // Only for a single Stmt without braces
            CurrStmt->Content.push_back(Stmt);
            return true;
        }

    }
    return false;
}

bool Parser::ParseInitForStmt(BlockStmt *InitStmt, GroupExpr *Cond) {
    if (ParseStmt(InitStmt, Cond)) {

        if (Tok.is(tok::comma)) {
            ConsumeToken();
            return ParseInitForStmt(InitStmt, NULL);
        }
        return true;
    }
    return false;
}

bool Parser::ParseCondForStmt(BlockStmt *Block, GroupExpr* Cond) {
    return ParseExpr(Block, Cond);
}

bool Parser::ParsePostForStmt(BlockStmt *PostStmt) {
    if (ParseStmt(PostStmt)) {
        if (Tok.is(tok::comma)) {
            ConsumeToken();
            return ParsePostForStmt(PostStmt);
        }
        return true;
    }
    return false;
}

FuncCall *Parser::ParseFunctionCall(BlockStmt *Block, IdentifierInfo *Id, SourceLocation &IdLoc) {
    const StringRef &IdName = Id->getName();
    FunctionParser Parser(this, IdName, IdLoc);
    Parser.ParseCall(Block);
    return Parser.Call;
}

VarDeclStmt *Parser::ParseVarDecl(BlockStmt *Block, bool Constant, TypeBase *TyDecl) {
    // Var Name
    const StringRef Name = Tok.getIdentifierInfo()->getName();
    const SourceLocation IdLoc = Tok.getLocation();
    ConsumeToken();
    VarDeclStmt *Var = new VarDeclStmt(IdLoc, Block, TyDecl, Name);
    Var->Constant = Constant;

    // Parsing =, +=, -=, ...
    if (isOpAssign()) {
        VarRef *VRef = new VarRef(Tok.getLocation(), Var->getName());
        VRef->Decl = Var;
        ParseOpAssign(VRef);
        ConsumeToken();

        GroupExpr* Ex = ParseExpr(Block);
        if (Ex) {
            Var->Expression = Ex;
        }
    }

    return Var;
}

VarRef* Parser::ParseVarRef() {
    VarRef *VRef = new VarRef(Tok.getLocation(), Tok.getIdentifierInfo()->getName());
    ConsumeToken();
    return VRef;
}

VarStmt *Parser::ParseIncDec(SourceLocation &Loc, BlockStmt *CurrStmt, IdentifierInfo *Id) {
    VarStmt *VStmt;
    IncDecExpr* Exp;
    if (Id) {
        Exp = ParseOpIncrement(true);
        ConsumeToken();
        VStmt = new VarStmt(Loc, CurrStmt, Id->getName());
    } else {
        Exp = ParseOpIncrement(false);
        ConsumeToken();
        if (Tok.isAnyIdentifier()) {
            VStmt = new VarStmt(Tok.getLocation(), CurrStmt, Tok.getIdentifierInfo()->getName());
            ConsumeToken();
        } else {
            // TODO Error var not specified for inc dec
            return NULL;
        }
    }
    VStmt->getExpr()->Add(Exp);
    return VStmt;
}

ValueExpr *Parser::ParseValueExpr() {
    if (Tok.is(tok::numeric_constant)) {
        const StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        Value *V = nullptr;
        if (Val.contains(".")) {
            // Parse Float
            float FloatVal = std::stof(Val.str());
            V = new Value(Val, new FloatPrimType(Tok.getLocation()));
        } else {
            // Parse Int
            int IntVal = std::stoi(Val.str());
            V = new Value(Val, new IntPrimType(Tok.getLocation()));
        }
        SourceLocation Loc = ConsumeToken();
        return new ValueExpr(Loc, V);
    } else if (Tok.isOneOf(tok::kw_true, tok::kw_false)) {
        Value *V = nullptr;

        switch (Tok.getKind()) {
            case tok::kw_true:
                V = new Value("true", new BoolPrimType(Tok.getLocation()));
            case tok::kw_false:
                V = new Value("false", new BoolPrimType(Tok.getLocation()));
            default:
                assert("Bool value not accepted");
        }
        SourceLocation Loc = ConsumeToken();
        return new ValueExpr(Loc, V);
    } else {
        assert("Not a value");
    }
}

// TODO return Expr*
// Parse not always as Group but use inheritance for return different Expr
GroupExpr* Parser::ParseExpr(BlockStmt *Block, GroupExpr *CurrGroup) {
//    SourceLocation Loc = Tok.getLocation();
    if (CurrGroup == NULL) {
        CurrGroup = new GroupExpr();
    }

    // Start Parsing
    if (isValue()) {
        ValueExpr *Val = ParseValueExpr();
        if (Val) {
            CurrGroup->Add(Val);
        }
    } else if (Tok.isAnyIdentifier()) {
        IdentifierInfo *Id = Tok.getIdentifierInfo();
        SourceLocation Loc = ConsumeToken();
        if (Tok.is(tok::l_paren)) { // function invocation
            // a()
            FuncCall *Call = ParseFunctionCall(Block, Id, Loc);
            if (Call) {
                Block->Top->addUnRefCall(Call);
                CurrGroup->Add(new FuncCallExpr(Loc, Call));
            }
        } else {
            VarRef *VRef = new VarRef(Loc, Id->getName());
            Block->ResolveVarRef(VRef);
            CurrGroup->Add(new VarRefExpr(Loc, VRef));
        }
    } else if (Tok.is(tok::l_paren)) {
        ConsumeToken();
        GroupExpr *GroupE = new GroupExpr();
        ParseExpr(Block, GroupE);
        if (Tok.is(tok::r_paren)) {
            ConsumeToken();
            CurrGroup->Add(GroupE);
        }
    }

    // Always check if there is an operator
    if (isOperator()) {
        Expr* E = ParseOperator();
        ConsumeToken();
        CurrGroup->Add(E);
        ParseExpr(Block, CurrGroup);
    }

    return CurrGroup;
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

bool Parser::isOpIncDec() {
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
    Gr->Add(VRefE);
    switch (Tok.getKind()) {

        // Arithmetic
        case tok::plusequal:
            Gr->Add( new ArithExpr(Loc, ArithOpKind::ARITH_ADD) );
            break;
        case tok::minusequal:
            Gr->Add( new ArithExpr(Loc, ArithOpKind::ARITH_SUB) );
            break;
        case tok::starequal:
            Gr->Add( new ArithExpr(Loc, ArithOpKind::ARITH_MUL) );
            break;
        case tok::slashequal:
            Gr->Add( new ArithExpr(Loc, ArithOpKind::ARITH_DIV) );
            break;
        case tok::percentequal:
            Gr->Add( new ArithExpr(Loc, ArithOpKind::ARITH_MOD) );
            break;

            // Bit
        case tok::ampequal:
            Gr->Add( new BitExpr(Loc, BitOpKind::BIT_AND));
            break;
        case tok::pipeequal:
            Gr->Add( new BitExpr(Loc, BitOpKind::BIT_OR));
            break;
        case tok::caretequal:
            Gr->Add( new BitExpr(Loc, BitOpKind::BIT_NOT));
            break;
        case tok::lesslessequal:
            Gr->Add( new BitExpr(Loc, BitOpKind::BIT_SHIFT_L));
            break;
        case tok::greatergreaterequal:
            Gr->Add( new BitExpr(Loc, BitOpKind::BIT_SHIFT_R));
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
