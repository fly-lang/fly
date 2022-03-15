//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/ExprParser.h"
#include "AST/ASTExpr.h"
#include "AST/ASTValue.h"
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

    // Parse NameSpace on first
    AST->setNameSpace(ParseNameSpace());

    // Parse Imports
    if (ParseImports()) {

        // Node empty
        if (Tok.is(tok::eof)) {
            Diag(Tok.getLocation(), diag::warn_empty_code);
            return true;
        }

        // Parse All
        while (Tok.isNot(tok::eof)) {

            // if parse "namespace" there is multiple package declarations
            if (Tok.is(tok::kw_namespace)) {

                // Multiple Package declaration is invalid, you can define only one and on top of declarations
                Diag(Tok, diag::err_namespace_invalid) << Tok.getName();
                return false;
            }

            // if parse "import" there is import declaration position error
            if (Tok.is(tok::kw_import)) {

                // Import can be defined after namespace and before all other declarations
                Diag(Tok, diag::err_import_invalid) << Tok.getName();
                return false;
            }

            if (!ParseTopDecl()) {
                return false;
            }
        }
    }

    return !Diags.hasErrorOccurred() && AST->Resolve();
}

bool Parser::ParseHeader(ASTNode *Node) {
    FLY_DEBUG("Parser", "ParseHeader");
    AST = Node;
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse NameSpace on first
    Node->setNameSpace(ParseNameSpace());

    while (Tok.isNot(tok::eof)) {
        if (!ParseTopDecl()) {
            return false;
        }
    }

    return !Diags.hasErrorOccurred();
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
std::string Parser::ParseNameSpace() {
    // Check namespace declaration
    if (Tok.is(tok::kw_namespace)) {
        ConsumeToken();

        StringRef Name;
        // Check if default namespace specified
        if (Tok.is(tok::kw_default)) {
            ConsumeToken();
            Name = ASTNameSpace::DEFAULT;
            FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "NameSpace=" << ASTNameSpace::DEFAULT);
        } else if (Tok.isAnyIdentifier()) { // Check if a different namespace identifier has been defined
            Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "NameSpace=" << Name);
        } else {
            // Invalid NameSpace defined with error
            Diag(Tok, diag::err_namespace_invalid) << Tok.getName();
        }

        // Push Names into namespace
        std::string NS = Name.str();
        while (Tok.is(tok::period)) {
            NS += ".";
            ConsumeToken();
            if (Tok.isAnyIdentifier()) {
                NS += Tok.getIdentifierInfo()->getName();
                ConsumeToken();
            } else {
                Diag(Tok, diag::err_namespace_invalid) << Tok.getName();
            }
        }
        FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "NameSpace=" << NS);
        return NS;
    }

    // Define Default NameSpace also if it has not been defined
    FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "No namespace defined");
    return ASTNameSpace::DEFAULT;
}

/**
 * Parse import declaration
 * @return true on Success or false on Error
 */
bool Parser::ParseImports() {
    if (Tok.is(tok::kw_import)) {
        const SourceLocation Loc = ConsumeToken();

        if (Tok.isAnyIdentifier()) {
            IdentifierInfo *ImportId = Tok.getIdentifierInfo();
            llvm::StringRef Name = ImportId->getName();
            SourceLocation ImportLoc = ConsumeToken();

            // Syntax Error Quote
            if (Name.empty()) {
                Diag(ImportLoc, diag::err_import_undefined);
                return false;
            }

            if (Tok.isAnyIdentifier()) {
                IdentifierInfo *AliasId = Tok.getIdentifierInfo();
                llvm::StringRef Alias = AliasId->getName();
                ConsumeToken();
                FLY_DEBUG_MESSAGE("Parser", "ParseImportAlias",
                                  "Import=" + Name + " , Alias=" << Alias);
                return AST->AddImport(new ASTImport(Loc, Name.str(), Alias.str())) && ParseImports();
            } else {
                FLY_DEBUG_MESSAGE("Parser", "ParseImports",
                                  "Import=" << Name);
                return AST->AddImport(new ASTImport(Loc, Name.str())) && ParseImports();
            }
        } else {
            Diag(Loc, diag::err_import_undefined);
            return false;
        }
    }

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

                // parse function
                if (Tok.is(tok::l_paren)) {
                    return ParseFunction(Visibility, Constant, Type, Name, IdLoc);
                }

                // parse global var
                return ParseGlobalVarDecl(Visibility, Constant, Type, Name, IdLoc);
            }
        }
    }

    // Check Error: type without identifier
    return false;
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
        if (!AST->isHeader()) {
            return Parser.ParseFunctionBody() && AST->AddFunction(Parser.Function);
        }
        return AST->AddFunction(Parser.Function);
    }

    return false;
}

/**
 * Parse a data Type
 * @return true on Success or false on Error
 */
bool Parser::ParseType(ASTType *&Type, bool OnlyBuiltin) {
    const SourceLocation &Loc = Tok.getLocation();
    switch (Tok.getKind()) {
        case tok::kw_bool:
            Type = new ASTBoolType(Loc);
            break;
        case tok::kw_byte:
            Type = new ASTByteType(Loc);
            break;
        case tok::kw_ushort:
            Type = new ASTUShortType(Loc);
            break;
        case tok::kw_short:
            Type = new ASTShortType(Loc);
            break;
        case tok::kw_uint:
            Type = new ASTUIntType(Loc);
            break;
        case tok::kw_int:
            Type = new ASTIntType(Loc);
            break;
        case tok::kw_ulong:
            Type = new ASTULongType(Loc);
            break;
        case tok::kw_long:
            Type = new ASTLongType(Loc);
            break;
        case tok::kw_float:
            Type = new ASTFloatType(Loc);
            break;
        case tok::kw_double:
            Type = new ASTDoubleType(Loc);
            break;
        case tok::kw_void:
            Type = new ASTVoidType(Loc);
            break;
        default: {
            if (OnlyBuiltin) {
                assert(0 && "Unknown builtin type");
                return false;
            }
            IdentifierInfo *Id = Tok.getIdentifierInfo();
            if (!Id) {
                DiagInvalidId(Loc);
                return false;
            }
            StringRef Name = Id->getName();
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
        ASTExpr *Expr = ParseExpr(Block);
        if (Expr != nullptr)
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

        // T a = ... (Type)      ns1:T a = ... (namespace Type)
        // a = ...   (Var)       ns1:a = ...   (namespace Var)
        // a()       (Func)      ns1:a()       (namespace Func)
        // a++       (Var Incr)  ns1:a++       (namespace Var Increment)

        llvm::StringRef Name;
        llvm::StringRef NameSpace;
        SourceLocation Loc;
        if (ParseIdentifier(Name, NameSpace, Loc)) {

            if (Tok.isAnyIdentifier()) { // variable declaration
                // T a = ...
                // T a
                ASTType *Type = new ASTClassType(Loc, Name.str(), NameSpace.str());
                if (Type) {
                    ASTLocalVar *Var = ParseLocalVar(Block, Constant, Type);
                    if (Var)
                        return Block->AddLocalVar(Var);
                }

                // Error: Invalid Type format
                Diag(diag::err_parser_invalid_type);
                return false;
            } else if (ExprParser::isAssignOperator(Tok)) { // variable assignment
                // a = ...
                ASTLocalVarRef* LocalVarRef = new ASTLocalVarRef(Loc, Block, Name.str(), NameSpace.str());
                ASTExpr *Expr = ParseAssignmentExpr(Block, LocalVarRef);
                if (Expr != nullptr) {
                    LocalVarRef->setExpr(Expr);
                    return Block->AddLocalVarRef(LocalVarRef);
                }
            } else {
                // TODO need to do Tok--
                // a()
                // a++
                // a  FIXME ??
                ASTExprStmt *ExprStmt = new ASTExprStmt(Tok.getLocation(), Block);
                ASTExpr *Expr = ParseExpr(Block, Name, NameSpace, Loc);
                if (Expr != nullptr) {
                    ExprStmt->setExpr(Expr);
                    return Block->AddExprStmt(ExprStmt);
                }
            }
        }
    } else if (isBuiltinType()) { // variable declaration
        // int a = ...
        // int a
        ASTType *Type = nullptr;
        if (ParseType(Type, /* OnlyBuiltin */ true)) {
            ASTLocalVar *Var = ParseLocalVar(Block, Constant, Type);
            if (Var != nullptr)
                return Block->AddLocalVar(Var);
        }

        Diag(diag::err_parser_invalid_type);
        return false;
    }

    Diag(diag::err_parse_stmt);
    return false;
}

/**
 * Parse as Identifier with a Name and NameSpace
 * @param Name
 * @param NameSpace
 * @return
 */
bool Parser::ParseIdentifier(llvm::StringRef &Name, llvm::StringRef &NameSpace, SourceLocation &Loc) {
    Name = Tok.getIdentifierInfo()->getName();
    Loc = ConsumeToken();
    if (Tok.is(tok::colon)) {
        Loc = ConsumeToken();
        if (Tok.isAnyIdentifier()) {
            NameSpace = Name;
            Name = Tok.getIdentifierInfo()->getName();
            Loc = ConsumeToken();
            return true;
        }

        Diag(Loc, diag::err_invalid_namespace_id);
        return false;
    }
    return true;
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
            ASTExpr *Expr = ParseExpr(Block);
            // Parse ) if exists
            if (Expr == nullptr || !ParseEndParen(hasParen)) {
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
            ASTExpr *Expr = ParseExpr(Block);
            // Parse ) if exists
            if (Expr == nullptr || !ParseEndParen(hasParen)) {
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
    ASTExpr *Expr = ParseExpr(Block);
    if (Expr != nullptr) {

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
                        ASTValue *Val = ParseValue();
                        CaseExp = new ASTValueExpr(Val);
                    } else if (Tok.isAnyIdentifier()) {
                        IdentifierInfo *Id = Tok.getIdentifierInfo();
                        ASTVarRef *VarRef = new ASTVarRef(Tok.getLocation(), Id->getName().str());
                        CaseExp = new ASTVarRefExpr(VarRef);
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

    ASTExpr *Cond = ParseExpr(Block);
    if (Cond != nullptr) {
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

            ASTExpr *CondExpr = ParseExpr(For);
            if (CondExpr) {
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
ASTFuncCall * Parser::ParseFunctionCall(ASTBlock *Block, llvm::StringRef Name, llvm::StringRef NameSpace,
                                        SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunctionCall", "Name=" << Name + ", NameSpace=" << NameSpace);
    FunctionParser Parser(this, Name, Loc);
    bool Result = Parser.ParseCall(Block, NameSpace);
    return Parser.Call;
}

/**
 * Parse a Value Expression
 * @param Success true on Success or false on Error
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValue() {
    FLY_DEBUG("Parser", "ParseValue");

    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        const StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        const SourceLocation &Loc = ConsumeToken();
        ASTValue *V;
        if (Val.contains(".")) {
            // Parse Float
            float FloatVal = std::stof(Val.str()); // TODO remove?
            V = new ASTValue(Loc, Val.str(), new ASTFloatType(Loc));
        } else {
            // Parse Int
            int IntVal = std::stoi(Val.str()); // TODO remove?
            V = new ASTValue(Loc, Val.str(), new ASTIntType(Loc));
        }
        return V;
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        return new ASTValue(ConsumeToken(), "true", new ASTBoolType(Tok.getLocation()));
    }
    if (Tok.is(tok::kw_false)) {
        return new ASTValue(ConsumeToken(), "false", new ASTBoolType(Tok.getLocation()));
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return nullptr;
}

/**
 * Parse a Local Variable
 * @param Block
 * @param Constant
 * @param Type
 * @param Success true on Success or false on Error
 * @return the ASTLocalVar
 */
ASTLocalVar *Parser::ParseLocalVar(ASTBlock *Block, bool Constant, ASTType *Type) {
    IdentifierInfo *Id = Tok.getIdentifierInfo();
    if (!Id) {
        Diag(Tok, diag::err_var_undefined);
        return nullptr;
    }
    
    const StringRef Name = Id->getName();
    FLY_DEBUG_MESSAGE("Parser", "ParseLocalVar",
                      "Name=" << Name << ", Constant=" << Constant << ", Type=" << Type->str());
    const SourceLocation Loc = Tok.getLocation();
    ASTLocalVar *Result = new ASTLocalVar(Loc, Block, Type, Name.str());
    Result->Constant = Constant;
    ConsumeToken();

    ASTVarRef *VarRef = new ASTVarRef(Tok.getLocation(), Result->getName());
    VarRef->Decl = Result;

    ASTExpr *Assignment = ParseAssignmentExpr(Block, VarRef);
    if (Assignment != nullptr) {// int a or Type a is allowed
        Result->setExpr(Assignment);
    }

    return Result;
}

ASTExpr *Parser::ParseAssignmentExpr(ASTBlock *Block, ASTVarRef *VarRef) {
    ExprParser Parser(this);
    return Parser.ParseAssignmentExpr(Block, VarRef);
}

ASTExpr *Parser::ParseExpr(ASTBlock *Block) {
    ExprParser Parser(this);
    return Parser.ParseExpr(Block);
}

ASTExpr *Parser::ParseExpr(ASTBlock *Block, llvm::StringRef Name, llvm::StringRef NameSpace, SourceLocation Loc) {
    ExprParser Parser(this);
    return Parser.ParseExpr(Block, Name, NameSpace, Loc);
}

/**
 * Check if Token is one of the Builtin Types
 * @return true on Success or false on Error
 */
bool Parser::isBuiltinType() {
    return Tok.isOneOf(tok::kw_bool,
                       tok::kw_byte,
                       tok::kw_short,
                       tok::kw_ushort,
                       tok::kw_int,
                       tok::kw_uint,
                       tok::kw_long,
                       tok::kw_ulong,
                       tok::kw_float,
                       tok::kw_double);
}

/**
 * Check if Token is a Value
 * @return true on Success or false on Error
 */
bool Parser::isValue() {
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false);
}
