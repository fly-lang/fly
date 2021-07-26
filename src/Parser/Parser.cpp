//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <AST/ASTWhileBlock.h>
#include "Parser/Parser.h"
#include "AST/ASTImport.h"

using namespace fly;

Parser::Parser(InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags) : Input(Input), Diags(Diags),
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
        if (ParseImports()) {

            // If Node is empty it is not added to the Context
            if (Tok.is(tok::eof)) {
                Diag(Tok.getLocation(), diag::warn_empty_code);
                return true;
            }

            // Parse All
            while (Tok.isNot(tok::eof)) {
                if (!ParseTopDecl()) {
                    Diag(Tok.getLocation(), diag::err_top_decl);
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
bool Parser::ParseImports() {
    if (Tok.is(tok::kw_import)) {
        const SourceLocation ImportLoc = ConsumeToken();
        if (Tok.is(tok::l_paren)) {
            ConsumeParen();

            // Parse import ( ... ) declarations
            return ParseImportParen();

        } else if (Tok.isLiteral()) {

            // Parse import "..."
            StringRef Name = getLiteralString();

            // Syntax Error Quote
            if (Name.empty()) {
                Diag(Tok, diag::err_namespace_undefined);
                return false;
            }

            if (ParseImportAliasDecl(ImportLoc, Name)) {
                return ParseImports();
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
        ASTType *TyDecl = ParseType();

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

bool Parser::ParseImportParen() {
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
            return ParseImportParen();
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

            return AST->addImport(new ASTImport(Location, Name, Alias));
        }
    }
    return AST->addImport(new ASTImport(Location, Name));
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

bool Parser::ParseGlobalVarDecl(VisibilityKind &VisKind, bool &Constant, ASTType *TyDecl,
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

bool Parser::ParseFunctionDecl(VisibilityKind &VisKind, bool Constant, ASTType *TyDecl,
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

ASTType* Parser::ParseType() {
    const SourceLocation &Loc = Tok.getLocation();
    ASTType *Type = nullptr;
    switch (Tok.getKind()) {
        case tok::kw_bool:
            Type = new BoolPrimType(Loc);
            break;
        case tok::kw_int:
            Type = new IntPrimType(Loc);
            break;
        case tok::kw_float:
            Type = new FloatPrimType(Loc);
            break;
        case tok::kw_void:
            Type = new VoidRetType(Loc);
            break;
        default:
            StringRef Name = Tok.getIdentifierInfo()->getName();
            if (Name.empty()) {
                Diag(Loc, diag::err_type_undefined);
            } else {
                Type = new ClassTypeRef(Loc, Name);
            }
    }
    ConsumeToken();
    return Type;
}

/**
 * Parse statements not yet contained into braces
 * @param Block
 * @return
 */
bool Parser::ParseBlock(ASTBlock *Block) {
    if (Tok.isOneOf(tok::kw_if, tok::kw_elsif, tok::kw_else)) {
        return ParseIfStmt(Block);
    }
    if (Tok.is(tok::kw_switch)) {
        return ParseSwitchStmt(Block);
    }
    if (Tok.is(tok::kw_for)) {
        return ParseForStmt(Block);
    }
    if (Tok.is(tok::kw_while)) {
        return ParseWhileStmt(Block);
    }

    return ParseStmt(Block);
}

/**
 * Parse statements between braces
 * @param Block
 * @return
 */
bool Parser::ParseInnerBlock(ASTBlock *Block) {
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
bool Parser::ParseStmt(ASTBlock *Block) {
    bool Success = true;
    // Parse keywords
    if (Tok.is(tok::kw_return)) { // Parse return
        SourceLocation Loc = ConsumeToken();
        ASTExpr *Expr = ParseExpr(Block, Success);
        return Block->addReturn(Loc, Expr);
    } else if (Tok.is(tok::kw_break)) { // Parse break
        return Block->addBreak(ConsumeToken());
    } else if (Tok.is(tok::kw_continue)) { // Parse continue
        return Block->addContinue(ConsumeToken());
    }

    // const int a
    bool Constant = false;
    ParseConstant(Constant);

    SourceLocation Loc = Tok.getLocation();
    if (Tok.isAnyIdentifier()) {

        // T a = ... (Type)
        // a()     (Func)
        // a++
        // a = ... (Var)
        IdentifierInfo *Id = Tok.getIdentifierInfo();
        ConsumeToken();

        if (Tok.isAnyIdentifier()) { // variable declaration
            // T a = ...
            StringRef Name = Id->getName();
            ASTType *Type = new ClassTypeRef(Loc, Name);
            if (Type) {
                ASTLocalVar* Var = ParseLocalVar(Block, Constant, Type, Success);
                return Block->addVar(Var);
            }
        } else if (Tok.is(tok::l_paren)) { // function invocation
            // a()
            if (Constant) { // const a() -> Error
                Diag(Tok.getLocation(), diag::err_const_call);
                return false;
            }
            ASTFuncCall *Call = ParseFunctionCall(Block, Id, Loc, Success);
            if (Call)
                return Block->addCall(Call);
        } else if (isOpIncDec()) { // variable increment or decrement
            // a++
            ASTLocalVarStmt *S = ParseIncDec(Loc, Block, Id, Success);
            if (S) {
                return Block->addVar(S);
            }
        } else { // variable assign
            // a = ...
            ASTLocalVarStmt* Var = new ASTLocalVarStmt(Loc, Block, Id->getName());
            ASTExpr *Expr = ParseStmtExpr(Block, Var, Success);
            Var->setExpr(Expr);
            return Block->addVar(Var);
        }
    } else if (isBuiltinType()) {
        // int a = ...
        ASTLocalVar* Var = ParseLocalVar(Block, Constant, ParseType(), Success);
        return Block->addVar(Var);
    } else if (isOpIncDec()) { // variable increment or decrement
        // ++a
        ASTLocalVarStmt *S = ParseIncDec(Loc, Block, nullptr, Success);
        if (Success) {
            return Block->addVar(S);
        }
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
bool Parser::ParseIfStmt(ASTBlock *Block) {
    bool Success = true;

    ConditionBlockStmt *Stmt;
    const SourceLocation &Loc = Tok.getLocation();

    // Init the current statement parsing if, elsif or else keywords
    switch (Tok.getKind()) {
        case tok::kw_if:
            ConsumeToken();
            Stmt = new ASTIfBlock(Loc, Block);
            break;
        case tok::kw_elsif:
            ConsumeToken();
            Stmt = new ElsifBlockStmt(Loc, Block);
            ASTIfBlock::AddBranch(Block, Stmt);
            break;
        case tok::kw_else:
            ConsumeToken();
            Stmt = new ElseBlockStmt(Loc, Block);
            ASTIfBlock::AddBranch(Block, Stmt);
            break;
        default:
            assert(0 && "Unknow conditional statement");
    }

    // Check parenthesis content only for If and Elsif
    // Parse (
    bool hasParen = ParseStartParen();

    if (Stmt->getBlockKind() == BlockStmtKind::BLOCK_STMT_IF ||
            Stmt->getBlockKind() == BlockStmtKind::BLOCK_STMT_ELSIF) {

        // Parse the group of expressions into parenthesis
        ASTIfBlock *IfStmt = (ASTIfBlock *) Stmt;
        IfStmt->Condition = ParseExpr(Block, Success);
        if (IfStmt->Condition == nullptr) {
            return false;
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
        return Success;
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
 * @param Block
 * @return
 */
bool Parser::ParseSwitchStmt(ASTBlock *Block) {
    bool Success = true;

    // Parse switch keyword
    const SourceLocation &SwitchLoc = ConsumeToken();

    // Parse (
    bool hasParen = ParseStartParen();

    // Parse Var reference like (a)
    ASTVarRef *VRef = ParseVarRef(Success);
    if (Success) {

        // Consume Right Parenthesis ) if exists
        ParseEndParen(hasParen);

        // Init Switch Statement and start parse from brace
        ASTSwitchBlock *Stmt = new ASTSwitchBlock(SwitchLoc, Block, VRef);
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
                    ASTExpr * CaseExp = nullptr;
                    ASTVarRef *VRef = nullptr;
                    if (isValue()) {
                        CaseExp = ParseValueExpr(Success);
                    } else if (Tok.isAnyIdentifier()) {
                        IdentifierInfo *Id = Tok.getIdentifierInfo();
                        VRef = new ASTVarRef(Tok.getLocation(), Id->getName());
                        CaseExp = new ASTVarRefExpr(Tok.getLocation(), VRef);
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
                Block->Content.push_back(Stmt);
                return Success;
            }
        }
    }

    return false;
}

/**
 * Parse the While statement
 *
 * while (...) {
 *  ...
 * }
 *
 * or
 *
 * while ... {
 *  ...
 * }
 *
 * @param Block
 * @return
 */
bool Parser::ParseWhileStmt(ASTBlock *Block) {
    bool Success = true;
    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    ASTWhileBlock *While = new ASTWhileBlock(Loc, Block);
    While->Cond = ParseExpr(Block, Success);

    // Consume Right Parenthesis ) if exists
    ParseEndParen(hasParen);

    // Parse statement between braces
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (Success && ParseInnerBlock(While)) {
            Block->Content.push_back(While);
            return true;
        }
    } else if (Success && ParseStmt(While)) { // Only for a single Stmt without braces
        Block->Content.push_back(While);
        return true;
    }

    return false;
}

/**
 * Parse the For statement
 *
 * for ...; ...; ... {
 *  ...
 * }
 *
 * or
 *
 * for (...; ...; ...) {
 *  ...
 * }
 *
 * @param Block
 * @return
 */
bool Parser::ParseForStmt(ASTBlock *Block) {
    bool Success = true;
    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Init For Statement and start parse components
    ASTForBlock *For = new ASTForBlock(Loc, Block);

    // parse comma separated or single statements
    if (ParseForCommaStmt(For->Init)) {
        assert(Tok.isNot(tok::comma) && "Comma not parsed");

        // This is an Expression, it could be a Condition
        if (Tok.is(tok::semi)) {
            ConsumeToken();
            For->Cond = ParseExpr(For->Init, Success);

            if (Tok.is(tok::semi)) {
                ConsumeToken();
                Success &= ParseForCommaStmt(For->Post);
            }
        }
    }

    // Consume Right Parenthesis ) if exists
    ParseEndParen(hasParen);

    // Parse statement between braces
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (Success && ParseInnerBlock(For->Loop)) {
            Block->Content.push_back(For);
            return true;
        }
    } else if (Success && ParseStmt(For->Loop)) { // Only for a single Stmt without braces
        Block->Content.push_back(For);
        return true;
    }

    return false;
}

bool Parser::ParseForCommaStmt(ASTBlock *Block) {
    if (ParseStmt(Block)) {

        if (Tok.is(tok::comma)) {
            ConsumeToken();
            return ParseForCommaStmt(Block);
        }
        return true;
    }
    return false;
}

ASTFuncCall *Parser::ParseFunctionCall(ASTBlock *Block, IdentifierInfo *Id, SourceLocation &Loc, bool &Success) {
    const StringRef &IdName = Id->getName();
    FunctionParser Parser(this, IdName, Loc);
    Parser.ParseCall(Block);
    return Parser.Call;
}

ASTLocalVar *Parser::ParseLocalVar(ASTBlock *Block, bool Constant, ASTType *Type, bool &Success) {
    const StringRef Name = Tok.getIdentifierInfo()->getName();
    const SourceLocation Loc = Tok.getLocation();
    ASTLocalVar *Var = new ASTLocalVar(Loc, Block, Type, Name);
    Var->Constant = Constant;
    ConsumeToken();

    Var->setExpr(ParseStmtExpr(Block, Var, Success));
    return Var;
}

ASTLocalVarStmt *Parser::ParseIncDec(SourceLocation &Loc, ASTBlock *Block, IdentifierInfo *Id, bool &Success) {
    ASTLocalVarStmt *VStmt;
    ASTIncDecExpr* Expr;
    if (Id) {
        Expr = ParseOpIncrement(Success, true);
        ConsumeToken();
        VStmt = new ASTLocalVarStmt(Loc, Block, Id->getName());
    } else {
        Expr = ParseOpIncrement(Success, false);
        ConsumeToken();
        if (Tok.isAnyIdentifier()) {
            VStmt = new ASTLocalVarStmt(Tok.getLocation(), Block, Tok.getIdentifierInfo()->getName());
            ConsumeToken();
        } else {
            // TODO Error var not specified for inc dec
            return nullptr;
        }
    }
    VStmt->setExpr(Expr);
    return VStmt;
}

ASTVarRef* Parser::ParseVarRef(bool &Success) {
    ASTVarRef *VRef = new ASTVarRef(Tok.getLocation(), Tok.getIdentifierInfo()->getName());
    ConsumeToken();
    return VRef;
}

ASTExpr* Parser::ParseStmtExpr(ASTBlock *Block, ASTLocalVar *Var, bool &Success) {
    ASTVarRef *VarRef = new ASTVarRef(Tok.getLocation(), Var->getName());
    VarRef->Decl = Var;
    return ParseStmtExpr(Block, VarRef, Success);
}

ASTExpr* Parser::ParseStmtExpr(ASTBlock *Block, ASTVarRef *VarRef, bool &Success) {
    // Parsing =, +=, -=, ...
    if (Tok.is(tok::equal)) {
        ConsumeToken();
        return ParseExpr(Block, Success);
    } else if (Tok.isOneOf(tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal,
                           tok::percentequal, tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal,
                           tok::greatergreaterequal)) {

        SourceLocation Loc = Tok.getLocation();
        ASTGroupExpr* GroupExpr = new ASTGroupExpr;
        ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(Loc, VarRef);
        GroupExpr->Add(VarRefExpr);
        switch (Tok.getKind()) {

                // Arithmetic
            case tok::plusequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_ADD) );
                break;
            case tok::minusequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_SUB) );
                break;
            case tok::starequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_MUL) );
                break;
            case tok::slashequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_DIV) );
                break;
            case tok::percentequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_MOD) );
                break;

                // Bit
            case tok::ampequal:
                GroupExpr->Add( new ASTBitExpr(Loc, BitOpKind::BIT_AND));
                break;
            case tok::pipeequal:
                GroupExpr->Add( new ASTBitExpr(Loc, BitOpKind::BIT_OR));
                break;
            case tok::caretequal:
                GroupExpr->Add( new ASTBitExpr(Loc, BitOpKind::BIT_NOT));
                break;
            case tok::lesslessequal:
                GroupExpr->Add( new ASTBitExpr(Loc, BitOpKind::BIT_SHIFT_L));
                break;
            case tok::greatergreaterequal:
                GroupExpr->Add( new ASTBitExpr(Loc, BitOpKind::BIT_SHIFT_R));
                break;
            default:
                assert(0 && "Accept Only assignment operators");
        }
        ConsumeToken();
        return ParseExpr(Block, Success, GroupExpr);
    }
    return nullptr;
}

ASTExpr* Parser::ParseExpr(ASTBlock *Block, bool &Success, ASTGroupExpr *ParentGroup) {
    assert(Block != nullptr && "Invalid Block");

    if (Tok.is(tok::l_paren)) {
        ConsumeParen();
        ASTGroupExpr *CurrGroup = (ASTGroupExpr *) ParseExpr(Block, Success, new ASTGroupExpr());

        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            if (ParentGroup != nullptr) {
                ParentGroup->Add(CurrGroup);
            }
            if (isOperator()) {

                // Add Operator
                ASTOperatorExpr *OpExpr = ParseOperator(Success);
                ConsumeToken();
                CurrGroup->Add(OpExpr);

                // Parse with recursion
                ParseExpr(Block, Success, CurrGroup);
                return CurrGroup;
            }
            return CurrGroup;
        }

        Diag(Tok.getLocation(), diag::err_paren_unclosed);
    } else {
        ASTExpr *Expr = ParseOneExpr(Block, Success);
        if (Expr) {

            // If there is an Operator start creating a GroupExpr
            if (isOperator()) {
                ASTGroupExpr *GroupExpr = ParentGroup == nullptr ? new ASTGroupExpr : ParentGroup;

                // Continue recursively with group parsing
                // Add Expr
                GroupExpr->Add(Expr);

                // Add Operator
                ASTOperatorExpr *OpExpr = ParseOperator(Success);
                ConsumeToken();
                GroupExpr->Add(OpExpr);

                // Parse with recursion
                return ParseExpr(Block, Success, GroupExpr);
            }

            if (ParentGroup != nullptr) {
                ParentGroup->Add(Expr);
                return ParentGroup;
            }
            return Expr;
        }
    }
    return nullptr;
}

// Parse by using inheritance for return different Expr
ASTExpr* Parser::ParseOneExpr(ASTBlock *Block, bool &Success) {
    // Start Parsing
    if (isValue()) {
        return ParseValueExpr(Success);
    } else if (Tok.isAnyIdentifier()) {
        IdentifierInfo *Id = Tok.getIdentifierInfo();
        SourceLocation Loc = ConsumeToken();
        if (Tok.is(tok::l_paren)) { // function invocation
            // a()
            ASTFuncCall *Call = ParseFunctionCall(Block, Id, Loc, Success);
            if (Success) {
                Block->Top->addUnRefCall(Call);
                return new ASTFuncCallExpr(Loc, Call);
            }
        } else {
            ASTVarRef *VRef = new ASTVarRef(Loc, Id->getName());
            Block->ResolveVarRef(VRef);
            return new ASTVarRefExpr(Loc, VRef);
        }
    }
    return nullptr;
}

ASTValueExpr *Parser::ParseValueExpr(bool &Success) {
    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        const StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        ASTValue *V;
        if (Val.contains(".")) {
            // Parse Float
            float FloatVal = std::stof(Val.str());
            V = new ASTValue(Tok.getLocation(), Val, new FloatPrimType(Tok.getLocation()));
        } else {
            // Parse Int
            int IntVal = std::stoi(Val.str());
            V = new ASTValue(Tok.getLocation(), Val, new IntPrimType(Tok.getLocation()));
        }
        return new ASTValueExpr(ConsumeToken(), V);
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        ASTValue *V = new ASTValue(Tok.getLocation(), "true", new BoolPrimType(Tok.getLocation()));
        return new ASTValueExpr(ConsumeToken(), V);
    } else if (Tok.is(tok::kw_false)) {
        ASTValue *V = new ASTValue(Tok.getLocation(), "false", new BoolPrimType(Tok.getLocation()));
        return new ASTValueExpr(ConsumeToken(), V);
    }

    assert(0 && "Incorrect Value");
}

ASTOperatorExpr* Parser::ParseOperator(bool &Success) {
    SourceLocation Loc = Tok.getLocation();
    switch (Tok.getKind()) {

        // Increment / Decrement
        case tok::plusplus:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_INCR);
        case tok::minusminus:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_DECR);

            // Bit
        case tok::amp:
            return new ASTBitExpr(Loc, BitOpKind::BIT_AND);
        case tok::pipe:
            return new ASTBitExpr(Loc, BitOpKind::BIT_OR);
        case tok::caret:
            return new ASTBitExpr(Loc, BitOpKind::BIT_NOT);
        case tok::lessless:
            return new ASTBitExpr(Loc, BitOpKind::BIT_SHIFT_L);
        case tok::greatergreater:
            return new ASTBitExpr(Loc, BitOpKind::BIT_SHIFT_R);

            // Boolean
        case tok::ampamp:
            return new ASTBoolExpr(Loc, BoolOpKind::BOOL_AND);
        case tok::pipepipe:
            return new ASTBoolExpr(Loc, BoolOpKind::BOOL_OR);
        case tok::exclaim:
            return new ASTBoolExpr(Loc, BoolOpKind::BOOL_NOT);

            // Arithmetic
        case tok::plus:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_ADD);
        case tok::minus:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_SUB);
        case tok::star:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_MUL);
        case tok::slash:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_DIV);
        case tok::percent:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_MOD);

            // Logic
        case tok::less:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_LT);
        case tok::lessequal:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_LTE);
        case tok::greater:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_GT);
        case tok::greaterequal:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_GTE);
        case tok::exclaimequal:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_NE);
        case tok::equalequal:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_EQ);

            // Condition
        case tok::question:
            return new ASTCondExpr(Loc, CondOpKind::COND_THAN);
        case tok::colon:
            return new ASTCondExpr(Loc, CondOpKind::COND_ELSE);
    }

    assert(0 && "Operator not accepted");
}

ASTIncDecExpr* Parser::ParseOpIncrement(bool &Success, bool Post) {
    switch (Tok.getKind()) {
        case tok::plusplus:
            return new ASTIncDecExpr(Tok.getLocation(), Post ? IncDecOpKind::POST_INCREMENT : IncDecOpKind::PRE_INCREMENT);
        case tok::minusminus:
            return new ASTIncDecExpr(Tok.getLocation(), Post ? IncDecOpKind::POST_DECREMENT : IncDecOpKind::PRE_DECREMENT);
        default:
            assert(0 && "Only Increment ++ or Decrement -- are accepted");
    }
}

bool Parser::isVoidType() {
    return Tok.is(tok::kw_void);
}

bool Parser::isBuiltinType() {
    return Tok.isOneOf(tok::kw_int, tok::kw_bool, tok::kw_float);
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

bool Parser::isValue() {
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false);
}
