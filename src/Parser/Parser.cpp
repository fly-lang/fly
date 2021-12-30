//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTResolver.h"
#include "Basic/Debug.h"

using namespace fly;

/**
 * Parser Constructor
 * @param Input 
 * @param SourceMgr 
 * @param Diags 
 */
Parser::Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags) : Input(Input), Diags(Diags),
            Lex(Input.getFileID(), Input.getBuffer(), SourceMgr) {

}

/**
 * Start Parsing Input and compose ASTNode
 * @param Node 
 * @return true on Success or false on Error
 */
bool Parser::Parse(ASTNode *Node) {
    FLY_DEBUG("Parser", "Parse");
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
                    return false;
                }
            }

            return AST->Resolve();
        }
    }

    return false;
}

/**
 * Print Diagnostic message by SourceLocation
 * @param Loc 
 * @param DiagID 
 * @return 
 */
DiagnosticBuilder Parser::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

/**
 * Print Diagnostic message by Token
 * @param Tok 
 * @param DiagID 
 * @return 
 */
DiagnosticBuilder Parser::Diag(const Token &Tok, unsigned DiagID) {
    return Diag(Tok.getLocation(), DiagID);
}

DiagnosticBuilder Parser::Diag(unsigned DiagID) {
    return Diag(Tok, DiagID);
}

void Parser::DiagInvalidId(SourceLocation Loc) {
    if (isKeyword(Tok.getKind())) {
        Diag(Loc, diag::err_invalid_id) << tok::getKeywordSpelling(Tok.getKind());
    } else {
        Diag(Loc, diag::err_invalid_id) << tok::getPunctuatorSpelling(Tok.getKind());
    }
}

/**
 * Get String between quotes from token
 * @return string
 */
llvm::StringRef Parser::getLiteralString() {
    StringRef Name(Tok.getLiteralData(), Tok.getLength());
    StringRef Str = "";
    if (Name.startswith("\"") && Name.endswith("\"")) {
        StringRef StrRefName = Name.substr(1, Name.size()-2);
        ConsumeStringToken();
        return StrRefName;
    }
    FLY_DEBUG_MESSAGE("Parser", "getLiteralString", "String=" << Str);
    return Str;
}

/**
 * Parse package declaration
 * @param fileName
 * @return true on Success or false on Error
 */
bool Parser::ParseNameSpace() {
    // Check namespace declaration
    if (Tok.is(tok::kw_namespace)) {
        ConsumeToken();

        // NameSpace is divided by period char
        llvm::SmallVector<std::string, 4> Names;

        // Check if default namespace specified
        if (Tok.is(tok::kw_default)) {
            FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "NameSpace=default");
            ConsumeToken();
            Names.push_back(ASTNameSpace::DEFAULT);
            FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "Space=" << ASTNameSpace::DEFAULT);
        } else if (Tok.isAnyIdentifier()) { // Check if a different namespace identifier has been defined
            StringRef Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            Names.push_back(Name.str());
            FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "Space=" << Name);
        } else {
            // Invalid NameSpace defined with error
            Diag(Tok, diag::err_namespace_invalid) << Tok.getName();
            return false;
        }


        while (Tok.is(tok::period)) {
            ConsumeToken();
            if (Tok.isAnyIdentifier()) {
                StringRef SubName = Tok.getIdentifierInfo()->getName();
                ConsumeToken();
                Names.push_back(SubName.str());
                FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "Space=" << SubName);
            }
        }

        AST->setNameSpace(Names);
        return true;
    }

    // Define Default NameSpace also if it has not been defined
    FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "No namespace defined");
    AST->setDefaultNameSpace();
    return true;
}

/**
 * Parse Top Declarations
 * Classes, Functions, Global Variables
 * @return true on Success or false on Error
 */
bool Parser::ParseTopDecl() {
    FLY_DEBUG("Parser", "ParseTopDecl");
    VisibilityKind Visibility = VisibilityKind::V_DEFAULT;
    bool Constant = false;

    // Parse Public or Private and Constant
    if (ParseTopScopes(Visibility, Constant)) {

        if (Tok.is(tok::kw_class)) {
            return ParseClassDecl(Visibility, Constant);
        }

        // Parse Type
        ASTType *Type = nullptr;
        if (ParseType(Type)) {

            if (Tok.isAnyIdentifier()) {
                IdentifierInfo *Id = Tok.getIdentifierInfo();
                llvm::StringRef Name = Id->getName();
                SourceLocation IdLoc = Tok.getLocation();
                ConsumeToken();
                if (Tok.is(tok::l_paren)) {
                    return ParseFunction(Visibility, Constant, Type, Name, IdLoc);
                }
                return ParseGlobalVarDecl(Visibility, Constant, Type, Name, IdLoc);
            }
        }
    }

    // Check Error: type without identifier
    return false;
}

/**
 * Parse import declaration
 * @return true on Success or false on Error
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
            FLY_DEBUG_MESSAGE("Parser", "ParseImports", "Single Import.Name=" << Name);

            // Syntax Error Quote
            if (Name.empty()) {
                Diag(Tok, diag::err_import_undefined);
                return false;
            }

            if (ParseImportAlias(ImportLoc, Name)) {
                return ParseImports();
            }
        } else {
            Diag(Tok, diag::err_import_undefined);
            return false;
        }
    }

    // if parse "namespace" there is multiple package declarations
    if (Tok.is(tok::kw_namespace)) {

        // Multiple Package declaration is invalid, you can define only one
        Diag(Tok, diag::err_namespace_invalid) << Tok.getName();
        return false;
    }

    return true;
}

/**
 * Parse Import with multiple items between parenthesis
 * @return true on Success or false on Error
 */
bool Parser::ParseImportParen() {
    if (Tok.isLiteral()) {
        SourceLocation ImportLoc = Tok.getLocation();

        // Parse Import
        StringRef Name = getLiteralString();
        FLY_DEBUG_MESSAGE("Parser", "ParseImportParen", "Multiple Import.Name=" << Name);

        if (Name.empty()) {
            Diag(Tok, diag::err_import_undefined);
            return false;
        }

        // Check if Alias exists
        if (!ParseImportAlias(ImportLoc, Name)) {
            return false;
        }

        // Parse multiple imports like (import1, import2, import3)
        if (Tok.is(tok::comma)) {
            ConsumeNext();
            return ParseImportParen();
        }

        // Right Paren ) is the end of multiple imports
        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            return true;
        }
    }

    return true;
}

/**
 * Parse Import Alias
 * @param Location 
 * @param Name 
 * @return true if no error otherwise fals
 */
bool Parser::ParseImportAlias(const SourceLocation &Location, llvm::StringRef Name) {
    if (Tok.is(tok::kw_as)) {
        ConsumeToken();

        if (Tok.isLiteral()) {
            StringRef Alias = getLiteralString();
            FLY_DEBUG_MESSAGE("Parser", "ParseImportAlias",
                              "Name=" + Name + " , Alias=" << Alias);
            return AST->AddImport(new ASTImport(Location, Name.str(), Alias.str()));
        } else {
            Diag(Tok, diag::err_alias_missing);
            return false;
        }
    }
    return AST->AddImport(new ASTImport(Location, Name.str()));
}

/**
 * Parse Top Scopes Visibility and isConst
 * @param Visibility
 * @param isConst
 * @param isParsedVisibility true when Visibility is already parsed
 * @param isParsedConstant true when isConst is already parsed
 * @return true on Success or false on Error
 */
bool Parser::ParseTopScopes(VisibilityKind &Visibility, bool &isConst,
                            bool isParsedVisibility, bool isParsedConstant) {
    if (Tok.is(tok::kw_private)) {
        Visibility = VisibilityKind::V_PRIVATE;
        ConsumeToken();
        if (isParsedVisibility) {
            Diag(Tok, diag::err_scope_visibility_duplicate << Visibility);
            return false;
        }
        FLY_DEBUG_MESSAGE("Parser", "ParseTopScopes","Visibility=" << Visibility);
        return isParsedConstant || ParseTopScopes(Visibility, isConst, true, isParsedConstant);
    } else if (Tok.is(tok::kw_public)) {
        Visibility = VisibilityKind::V_PUBLIC;
        ConsumeToken();
        if (isParsedVisibility) {
            Diag(Tok, diag::err_scope_visibility_duplicate << Visibility);
            return false;
        }
        FLY_DEBUG_MESSAGE("Parser", "ParseTopScopes","Visibility=" << Visibility);
        return isParsedConstant || ParseTopScopes(Visibility, isConst, true, isParsedConstant);
    } else if (Tok.is(tok::kw_const)) {
        isConst = true;
        ConsumeToken();
        if (isParsedConstant) {
            Diag(Tok, diag::err_scope_const_duplicate);
            return false;
        }
        FLY_DEBUG_MESSAGE("Parser", "ParseTopScopes","isConst=true");
        return isParsedVisibility || ParseTopScopes(Visibility, isConst, isParsedVisibility, true);
    }
    return true;
}

/**
 * Parse const scope of local vars and function parameters
 * @param Constant
 * @return true on Success or false on Error
 */
bool Parser::ParseConst(bool &Constant) {
    if (Tok.is(tok::kw_const)) {
        Constant = true;
        ConsumeToken();
    }
    return true;
}

/**
 * Parse GlobalVar declaration
 * @param VisKind
 * @param Constant
 * @param Type
 * @param Id
 * @param IdLoc
 * @return true on Success or false on Error
 */
bool Parser::ParseGlobalVarDecl(VisibilityKind &VisKind, bool &Constant, ASTType *Type,
                                llvm::StringRef &Name, SourceLocation &NameLoc) {
    FLY_DEBUG_MESSAGE("Parser", "ParseGlobalVarDecl", "Name=" << Name <<
    ", Type=" << Type->str());
    GlobalVarParser Parser(this, Type, Name, NameLoc);
    if (Parser.Parse()) {
        Parser.Var->Constant = Constant;
        Parser.Var->Visibility = VisKind;
        return AST->AddGlobalVar(Parser.Var);
    }

    return false;
}

/**
 * Parse Class Declaration
 * @param VisKind
 * @param Constant
 * @return true on Success or false on Error
 */
bool Parser::ParseClassDecl(VisibilityKind &VisKind, bool &Constant) {
    FLY_DEBUG("Parser", "ParseClassDecl");
    ClassParser Parser(this);
    if (Parser.Parse()) {
        Parser.Class->Constant = Constant;
        Parser.Class->Visibility = VisKind;
        return AST->AddClass(Parser.Class);
    }

    return false;
}

/**
 * Parse Function declaration
 * @param VisKind
 * @param Constant
 * @param Type
 * @param Id
 * @param IdLoc
 * @return true on Success or false on Error
 */
bool Parser::ParseFunction(VisibilityKind &VisKind, bool Constant, ASTType *Type,
                           llvm::StringRef &Name, SourceLocation &NameLoc) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunction","Name=" << Name <<
        ", Type=" << Type->str());
    FunctionParser Parser(this, Name, NameLoc);
    if (Parser.ParseFunction(Type)) {
        Parser.Function->Constant = Constant;
        Parser.Function->Visibility = VisKind;
        return AST->AddFunction(Parser.Function);
    }

    return false;
}

/**
 * Parse a data Type
 * @return true on Success or false on Error
 */
bool Parser::ParseType(ASTType *&Type) {
    const SourceLocation &Loc = Tok.getLocation();
    switch (Tok.getKind()) {
        case tok::kw_bool:
            Type = new ASTBoolType(Loc);
            break;
        case tok::kw_int:
            Type = new ASTIntType(Loc);
            break;
        case tok::kw_float:
            Type = new ASTFloatType(Loc);
            break;
        case tok::kw_void:
            Type = new ASTVoidType(Loc);
            break;
        default: {
            if (!Tok.getIdentifierInfo()) {
                DiagInvalidId(Loc);
                return false;
            }
            StringRef Name = Tok.getIdentifierInfo()->getName();
            Type = new ASTClassType(Loc, Name.str());
        }
    }
    ConsumeToken();
    FLY_DEBUG_MESSAGE("Parser", "ParseType", "Type=" << Type->str());
    return true;
}

/**
 * Parse statements not yet contained into braces
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseBlock(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseBlock");
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
 * @return true on Success or false on Error
 */
bool Parser::ParseStmt(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseStmt");
    bool Success = true;
    // Parse keywords
    if (Tok.is(tok::kw_return)) { // Parse return
        SourceLocation Loc = ConsumeToken();
        ASTExpr *Expr = ParseExpr(Block, Success);
        return Block->AddReturn(Loc, Expr);
    } else if (Tok.is(tok::kw_break)) { // Parse break
        return Block->AddBreak(ConsumeToken());
    } else if (Tok.is(tok::kw_continue)) { // Parse continue
        return Block->AddContinue(ConsumeToken());
    }

    // const int a
    bool Constant = false;
    ParseConst(Constant);

    if (Tok.isAnyIdentifier()) {

        // T a = ... (Type)
        // a()     (Func)
        // a++
        // a = ... (Var)
        IdentifierInfo *Id = Tok.getIdentifierInfo();
        SourceLocation Loc = ConsumeToken();

        if (Tok.isAnyIdentifier()) { // variable declaration
            // T a = ...
            StringRef Name = Id->getName();
            ASTType *Type = new ASTClassType(Loc, Name.str());
            if (Type) {
                ASTLocalVar* Var = ParseLocalVar(Block, Constant, Type, Success);
                return Block->AddLocalVar(Var);
            }
        } else if (Tok.is(tok::l_paren)) { // function invocation
            // a()
            if (Constant) { // const a() -> Error
                Diag(Tok.getLocation(), diag::err_const_call);
                return false;
            }
            ASTFuncCall *Call = ParseFunctionCall(Block, Id, Loc, Success);
            if (Call)
                return Block->AddCall(Call);
        } else if (isUnaryPostOperator()) { // variable increment or decrement
            // a++ or a--
            ASTOperatorExpr *Expr = ParseUnaryPostOperatorExpr(Success);
            ConsumeToken();
            if (Success) {
                ASTExprStmt *ExprStmt = new ASTExprStmt(Loc, Block);
                ASTVarRef *VarRef = new ASTVarRef(Loc, Id->getName().str());
                ExprStmt->setExpr(new ASTUnaryExpr(Loc, Expr, VarRef, UNARY_POST));
                return Block->AddExprStmt(ExprStmt);
            }
        } else { // variable assign
            // a = ...
            ASTLocalVarRef* Var = new ASTLocalVarRef(Loc, Block, Id->getName().str());
            ASTExpr *Expr = ParseStmtExpr(Block, Var, Success);
            Var->setExpr(Expr);
            return Block->AddLocalVarRef(Var);
        }
    } else if (isBuiltinType()) {
        // int a = ...
        ASTType *Type = nullptr;
        if (ParseType(Type)) {
            ASTLocalVar *Var = ParseLocalVar(Block, Constant, Type, Success);
            return Block->AddLocalVar(Var);
        }
        return false;
    } else if (isUnaryPostOperator()) { // variable increment or decrement
        // ++a or --a or !a
        ASTOperatorExpr *Expr = ParseUnaryPostOperatorExpr(Success);
        SourceLocation Loc = ConsumeToken();
        if (Success && Tok.isAnyIdentifier()) {
            ASTVarRef *VarRef = new ASTVarRef(Tok.getLocation(), Tok.getIdentifierInfo()->getName().str());
            Loc = ConsumeToken();
            ASTExprStmt *ExprStmt = new ASTExprStmt(Loc, Block);
            ExprStmt->setExpr(new ASTUnaryExpr(Loc, Expr, VarRef, UNARY_PRE));
            return Block->AddExprStmt(ExprStmt);
        }
        
        Diag(Loc, diag::err_unary_operator) << Tok.getLocation();
        return false;
    }

    Diag(diag::err_parse_stmt);
    return false;
}

/**
 * Parse open paren ( at start of cycle into condition statements
 * @return true on Success or false on Error
 */
bool Parser::ParseStartParen() {
    bool HasParen = false;
    if (Tok.is(tok::l_paren)) {
        ConsumeParen();
        HasParen = true;
    }
    FLY_DEBUG_MESSAGE("Parser", "ParseStartParen", "HasParen=" << HasParen);
    return HasParen;
}

/**
 * Parse close paren ) at end of cycle into condition statements
 * @return true on Success or false on Error
 */
bool Parser::ParseEndParen(bool HasParen) {
    if (HasParen) {
        FLY_DEBUG_MESSAGE("Parser", "ParseEndParen", "HasParen=" << HasParen);
        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            return true;
        } else {
            Diag(diag::err_right_paren);
            return false;
        }
    }
    return true;
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
 * @return true on Success or false on Error
 */
bool Parser::ParseIfStmt(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseIfStmt");
    bool Success = true;

    ASTIfBlock *Stmt;
    const SourceLocation &Loc = Tok.getLocation();

    // Init the current statement parsing if, elsif or else keywords
    switch (Tok.getKind()) {
        case tok::kw_if: {
            ConsumeToken();
            // Parse (
            bool hasParen = ParseStartParen();
            // Parse the group of expressions into parenthesis
            ASTExpr *Expr = ParseExpr(Block, Success);
            // Parse ) if exists
            if (!ParseEndParen(hasParen)) {
                return false;
            }
            Stmt = new ASTIfBlock(Loc, Block, Expr);
        }
            break;
        case tok::kw_elsif: {
            ConsumeToken();
            // Parse (
            bool hasParen = ParseStartParen();
            // Parse the group of expressions into parenthesis
            ASTExpr *Expr = ParseExpr(Block, Success);
            // Parse ) if exists
            if (!ParseEndParen(hasParen)) {
                return false;
            }
            Stmt = new ASTElsifBlock(Loc, Block, Expr);
            ASTIfBlock::AddBranch(Block, Stmt);
        }
            break;
        case tok::kw_else: {
            ConsumeToken();
            Stmt = new ASTElseBlock(Loc, Block);
            ASTIfBlock::AddBranch(Block, Stmt);
        }
            break;
        default:
            assert(0 && "Unknown conditional statement");
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
 * @return true on Success or false on Error
 */
bool Parser::ParseSwitchStmt(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseSwitchStmt");
    bool Success = true;

    // Parse switch keyword
    const SourceLocation &SwitchLoc = ConsumeToken();

    // Parse (
    bool hasParen = ParseStartParen();

    // Parse Var reference like (a)
    ASTExpr *Expr = ParseExpr(Block, Success);
    if (Success) {

        // Consume Right Parenthesis ) if exists
        if (!ParseEndParen(hasParen)) {
            return false;
        }

        // Init Switch Statement and start parse from brace
        ASTSwitchBlock *Stmt = new ASTSwitchBlock(SwitchLoc, Block, Expr);
        if (Tok.is(tok::l_brace)) {
            ConsumeBrace();

            // Exit only Statement find a closed brace or EOF
            while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {

                // Parse case keyword
                if (Tok.is(tok::kw_case)) {
                    const SourceLocation &CaseLoc = ConsumeToken();

                    // Parse Expression for different cases
                    // for a Value  -> case 1:
                    // for a Var -> case a:
                    // for a default
                    ASTExpr * CaseExp;
                    if (isValue()) {
                        CaseExp = ParseValueExpr(Success);
                    } else if (Tok.isAnyIdentifier()) {
                        IdentifierInfo *Id = Tok.getIdentifierInfo();
                        ASTVarRef *VarRef = new ASTVarRef(Tok.getLocation(), Id->getName().str());
                        CaseExp = new ASTVarRefExpr(Tok.getLocation(), VarRef);
                        ConsumeToken();
                    } else {
                        Diag(diag::err_syntax_error);
                        return false;
                    }
                    if (Tok.is(tok::colon)) {
                        ConsumeToken();

                        // Add Case to Switch statement and parse statement not contained into braces
                        ASTSwitchCaseBlock *CaseStmt = Stmt->AddCase(CaseLoc, CaseExp);
                        while (!Tok.isOneOf(tok::r_brace, tok::kw_case, tok::kw_default, tok::eof)) {
                            ParseBlock(CaseStmt);
                        }
                    }
                } else if (Tok.is(tok::kw_default)) {
                    ConsumeToken();

                    if (Tok.is(tok::colon)) {
                        const SourceLocation &Loc = ConsumeToken();

                        // Add Default to Switch statement and parse statement not contained into braces
                        ASTSwitchDefaultBlock *DefStmt = Stmt->setDefault(Loc);
                        ParseBlock(DefStmt);
                    } else {
                        Diag(diag::err_syntax_error);
                        return false;
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
 * @return true on Success or false on Error
 */
bool Parser::ParseWhileStmt(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseWhileStmt");
    bool Success = true;
    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    ASTExpr *Cond = ParseExpr(Block, Success);
    ASTWhileBlock *While = new ASTWhileBlock(Loc, Block, Cond);

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

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
 * @return true on Success or false on Error
 */
bool Parser::ParseForStmt(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseForStmt");
    bool Success = true;
    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Init For Statement and start parse components
    ASTForBlock *For = new ASTForBlock(Loc, Block);

    // parse comma separated or single statements
    if (ParseForCommaStmt(For)) {
        assert(Tok.isNot(tok::comma) && "Comma not parsed"); // Already parsed from ParseForCommaStmt()

        // This is an Expression, it could be a Condition
        if (Tok.is(tok::semi)) {
            ConsumeToken();

            ASTExpr *CondExpr = ParseExpr(For, Success);
            if (Success) {
                For->setCond(CondExpr);

                if (Tok.is(tok::semi)) {
                    ConsumeToken();
                    Success &= ParseForCommaStmt(For->Post);
                }
            }
        }
    }

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

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

/**
 * Parse multi statements separated by comma
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseForCommaStmt(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseForCommaStmt");
    if (ParseStmt(Block)) {

        if (Tok.is(tok::comma)) {
            ConsumeToken();
            return ParseForCommaStmt(Block);
        }
        return true;
    }
    return false;
}

/**
 * Parse a Function Call
 * @param Block
 * @param Id
 * @param Loc
 * @param Success true on Success or false on Error
 * @return true on Success or false on Error
 */
ASTFuncCall *Parser::ParseFunctionCall(ASTBlock *Block, IdentifierInfo *Id, SourceLocation &Loc, bool &Success) {
    const StringRef &Name = Id->getName();
    FLY_DEBUG_MESSAGE("Parser", "ParseFunctionCall", "Name=" << Name);
    FunctionParser Parser(this, Name, Loc);
    Parser.ParseCall(Block);
    return Parser.Call;
}

/**
 * Parse a Local Variable
 * @param Block
 * @param Constant
 * @param Type
 * @param Success true on Success or false on Error
 * @return the ASTLocalVar
 */
ASTLocalVar *Parser::ParseLocalVar(ASTBlock *Block, bool Constant, ASTType *Type, bool &Success) {
    if (!Tok.getIdentifierInfo()) {
        Diag(Tok, diag::err_var_undefined);
        Success = false;
        return nullptr;
    }
    const StringRef Name = Tok.getIdentifierInfo()->getName();
    FLY_DEBUG_MESSAGE("Parser", "ParseLocalVar",
                      "Name=" << Name << ", Constant=" << Constant << ", Type=" << Type->str());
    const SourceLocation Loc = Tok.getLocation();
    ASTLocalVar *Var = new ASTLocalVar(Loc, Block, Type, Name.str());
    Var->Constant = Constant;
    ConsumeToken();

    Var->setExpr(ParseStmtExpr(Block, Var, Success));
    return Var;
}

/**
 * Parse a Variable Reference
 * @param Success true on Success or false on Error
 * @return the ASTVarRef
 */
ASTVarRef* Parser::ParseVarRef(bool &Success) {
    if (!Tok.getIdentifierInfo()) {
        Diag(Tok, diag::err_var_undefined);
        Success = false;
        return nullptr;
    }
    ASTVarRef *VRef = new ASTVarRef(Tok.getLocation(), Tok.getIdentifierInfo()->getName().str());
    ConsumeToken();
    return VRef;
}

/**
 * Parse a Statement Expression
 * @param Block
 * @param Var
 * @param Success true on Success or false on Error
 * @return the ASTExpr
 */
ASTExpr* Parser::ParseStmtExpr(ASTBlock *Block, ASTLocalVar *Var, bool &Success) {
    ASTVarRef *VarRef = new ASTVarRef(Tok.getLocation(), Var->getName());
    VarRef->Decl = Var;
    return ParseStmtExpr(Block, VarRef, Success);
}

/**
 * Parse a Statement Expression
 * @param Block
 * @param VarRef
 * @param Success true on Success or false on Error
 * @return the ASTExpr
 */
ASTExpr* Parser::ParseStmtExpr(ASTBlock *Block, ASTVarRef *VarRef, bool &Success) {
    // Parsing =, +=, -=, ...
    if (Tok.is(tok::equal)) {
        ConsumeToken();
        return ParseExpr(Block, Success);
    } else if (Tok.isOneOf(tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal,
                           tok::percentequal, tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal,
                           tok::greatergreaterequal)) {

        SourceLocation Loc = Tok.getLocation();
        ASTGroupExpr* GroupExpr = new ASTGroupExpr(Loc);
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
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_AND));
                break;
            case tok::pipeequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_OR));
                break;
            case tok::caretequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_XOR));
                break;
            case tok::lesslessequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_SHIFT_L));
                break;
            case tok::greatergreaterequal:
                GroupExpr->Add( new ASTArithExpr(Loc, ArithOpKind::ARITH_SHIFT_R));
                break;
            default:
                assert(0 && "Accept Only assignment operators");
        }
        ConsumeToken();
        return ParseExpr(Block, Success, GroupExpr);
    }
    return nullptr;
}

/**
 * Parse all Expressions
 * @param Block
 * @param Success true on Success or false on Error
 * @param ParentGroup
 * @return the ASTExpr
 */
ASTExpr* Parser::ParseExpr(ASTBlock *Block, bool &Success, ASTGroupExpr *ParentGroup) {
    assert(Block != nullptr && "Block is nullptr");
    FLY_DEBUG("Parser", "ParseExpr");

    if (Tok.is(tok::l_paren)) {
        const SourceLocation &Loc = ConsumeParen();
        ASTGroupExpr *CurrGroup = (ASTGroupExpr *) ParseExpr(Block, Success, new ASTGroupExpr(Loc));

        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            if (ParentGroup != nullptr) {
                ParentGroup->Add(CurrGroup);
            }
            if (isOperator()) {

                // Add Operator
                ASTOperatorExpr *OpExpr = ParseOperatorExpr(Success);
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
        ASTExpr *Expr = ParseExprChunk(Block, Success);

        // If there is an Operator start creating a GroupExpr
        if (Success && isOperator()) {
            if (ParentGroup == nullptr) {
                ParentGroup = new ASTGroupExpr(Tok.getLocation());
            }
            ParentGroup->Add(Expr); // Add Expr on First

            // Continue recursively with group parsing
            // Add Operator
            ASTOperatorExpr *OpExpr = ParseOperatorExpr(Success);
            ConsumeToken();
            ParentGroup->Add(OpExpr);

            // Parse with recursion
            return ParseExpr(Block, Success, ParentGroup);
        }

        if (ParentGroup != nullptr) {
            ParentGroup->Add(Expr);
            return ParentGroup;
        }
        return Expr;

    }
    return nullptr;
}

/**
 * Parse one Expression
 * @param Block
 * @param Success true on Success or false on Error
 * @return the ASTExpr
 */
ASTExpr* Parser::ParseExprChunk(ASTBlock *Block, bool &Success) {
    FLY_DEBUG("Parser", "ParseExprChunk");
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
                Success = Block->Top->getNode()->AddUnrefCall(Call);
                return new ASTFuncCallExpr(Loc, Call);
            }
        } else {
            ASTVarRef *VarRef = new ASTVarRef(Loc, Id->getName().str());
            Success &= ASTResolver::ResolveVarRef(Block, VarRef);
            if (isUnaryPostOperator()) { // variable increment or decrement
                // a++ or a--
                ASTOperatorExpr *Expr = ParseUnaryPostOperatorExpr(Success);
                ConsumeToken();
                return new ASTUnaryExpr(Loc, Expr, VarRef, UNARY_POST);
            }
            return new ASTVarRefExpr(Loc, VarRef);
        }
    } else if (isUnaryPreOperator()) { // variable increment or decrement
        // ++a or --a or !a
        ASTOperatorExpr *Expr = ParseUnaryPreOperatorExpr(Success);
        ConsumeToken();
        if (Tok.isAnyIdentifier()) {
            IdentifierInfo *Id = Tok.getIdentifierInfo();
            SourceLocation Loc = ConsumeToken();
            ASTVarRef *VarRef = new ASTVarRef(Loc, Id->getName().str());
            ASTResolver::ResolveVarRef(Block, VarRef);
            return new ASTUnaryExpr(Loc, Expr, VarRef, UNARY_PRE);
        }

        Diag(Tok.getLocation(), diag::err_unary_operator)
                << getPunctuatorSpelling(Tok.getKind());
        Success = false;
    }
    return nullptr;
}

/**
 * Parse a Value Expression
 * @param Success true on Success or false on Error
 * @return the ASTValueExpr
 */
ASTValueExpr *Parser::ParseValueExpr(bool &Success) {
    FLY_DEBUG("Parser", "ParseValueExpr");
    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        const StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        ASTValue *V;
        if (Val.contains(".")) {
            // Parse Float
            float FloatVal = std::stof(Val.str()); // TODO remove?
            V = new ASTValue(Tok.getLocation(), Val.str(), new ASTFloatType(Tok.getLocation()));
        } else {
            // Parse Int
            int IntVal = std::stoi(Val.str()); // TODO remove?
            V = new ASTValue(Tok.getLocation(), Val.str(), new ASTIntType(Tok.getLocation()));
        }
        return new ASTValueExpr(ConsumeToken(), V);
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        ASTValue *V = new ASTValue(Tok.getLocation(), "true", new ASTBoolType(Tok.getLocation()));
        return new ASTValueExpr(ConsumeToken(), V);
    } else if (Tok.is(tok::kw_false)) {
        ASTValue *V = new ASTValue(Tok.getLocation(), "false", new ASTBoolType(Tok.getLocation()));
        return new ASTValueExpr(ConsumeToken(), V);
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    Success = false;
    return nullptr;
}

/**
 * Parse an Operator Expression
 * @param Success true on Success or false on Error
 * @return the ASTOperatorExpr
 */
ASTOperatorExpr* Parser::ParseOperatorExpr(bool &Success) {
    FLY_DEBUG("Parser", "ParseOperatorExpr");
    SourceLocation Loc = Tok.getLocation();
    switch (Tok.getKind()) {

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
        case tok::amp:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_AND);
        case tok::pipe:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_OR);
        case tok::caret:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_XOR);
        case tok::lessless:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_SHIFT_L);
        case tok::greatergreater:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_SHIFT_R);
        case tok::plusplus:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_INCR);
        case tok::minusminus:
            return new ASTArithExpr(Loc, ArithOpKind::ARITH_DECR);

        // Logic
        case tok::ampamp:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_AND);
        case tok::pipepipe:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_OR);
        case tok::exclaim:
            return new ASTLogicExpr(Loc, LogicOpKind::LOGIC_NOT);

        // Comparator
        case tok::less:
            return new ASTComparisonExpr(Loc, ComparisonOpKind::COMP_LT);
        case tok::lessequal:
            return new ASTComparisonExpr(Loc, ComparisonOpKind::COMP_LTE);
        case tok::greater:
            return new ASTComparisonExpr(Loc, ComparisonOpKind::COMP_GT);
        case tok::greaterequal:
            return new ASTComparisonExpr(Loc, ComparisonOpKind::COMP_GTE);
        case tok::exclaimequal:
            return new ASTComparisonExpr(Loc, ComparisonOpKind::COMP_NE);
        case tok::equalequal:
            return new ASTComparisonExpr(Loc, ComparisonOpKind::COMP_EQ);

        // Ternary Condition
        case tok::question:
            return ParseTernaryOperatorExpr(Success);

        default:
            assert(0 && "Operator not accepted");
    }
}

ASTTernaryExpr *Parser::ParseTernaryOperatorExpr(bool &Success) {
    return nullptr; //new ASTTernaryExpr(Loc, TernaryOpKind::COND_ELSE);
}

/**
 * Parse a Unary Pre Operator Expression
 * @param Success true on Success or false on Error
 * @return the ASTOperatorExpr
 */
ASTOperatorExpr *Parser::ParseUnaryPreOperatorExpr(bool &Success) {
    // Check if ++a, --a, !a
    switch (Tok.getKind()) {
        case tok::exclaim:
            return new ASTLogicExpr(Tok.getLocation(), LOGIC_NOT);
        case tok::plusplus:
            return new ASTArithExpr(Tok.getLocation(), ARITH_INCR);
        case tok::minusminus:
            return new ASTArithExpr(Tok.getLocation(), ARITH_DECR);
    }
    assert(0 && "Only Increment '++' or Decrement '--' or Exclaim '!' are accepted");
}

/**
 * Parse a Unary Post Operator Expression
 * @param Success true on Success or false on Error
 * @return the ASTOperatorExpr
 */
ASTOperatorExpr *Parser::ParseUnaryPostOperatorExpr(bool &Success) {
    // Check if a++ or a--
    switch (Tok.getKind()) {
        case tok::plusplus:
            return new ASTArithExpr(Tok.getLocation(), ARITH_INCR);
        case tok::minusminus:
            return new ASTArithExpr(Tok.getLocation(), ARITH_DECR);
    }
    assert(0 && "Only Increment ++ or Decrement -- are accepted");
}

/**
 * Check if Token is one of the Builtin Types
 * @return true on Success or false on Error
 */
bool Parser::isBuiltinType() {
    return Tok.isOneOf(tok::kw_int, tok::kw_bool, tok::kw_float);
}

/**
 * Check if Token is one of Operators
 * @return true on Success or false on Error
 */
bool Parser::isOperator() {
    return Tok.isOneOf(tok::plus, tok::minus, tok::star, tok::slash, tok::percent,
                       tok::amp, tok::pipe, tok::caret,
                       tok::ampamp, tok::pipepipe, tok::exclaim,
                       tok::less, tok::lessless, tok::lessequal, tok::greater, tok::greatergreater, tok::greaterequal,
                       tok::equalequal, tok::exclaimequal,
                       tok::question, tok::colon);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool Parser::isUnaryPreOperator() {
    return Tok.isOneOf(tok::plusplus, tok::minusminus, tok::exclaim);
}

/**
 * Check if Token is one of the Unary Post Operators
 * @return true on Success or false on Error
 */
bool Parser::isUnaryPostOperator() {
    return Tok.isOneOf(tok::plusplus, tok::minusminus);
}

/**
 * Check if Token is a Value
 * @return true on Success or false on Error
 */
bool Parser::isValue() {
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false);
}
