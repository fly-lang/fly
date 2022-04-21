//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/NumberParser.h"
#include "Parser/ExprParser.h"
#include "AST/ASTExpr.h"
#include "AST/ASTValue.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTResolver.h"
#include "Basic/Debug.h"
#include <regex>

using namespace fly;

/**
 * Parser Constructor
 * @param Input 
 * @param SourceMgr 
 * @param Diags 
 */
Parser::Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags) : Input(Input), Diags(Diags),
            SourceMgr(SourceMgr), Lex(Input.getFileID(), Input.getBuffer(), SourceMgr) {

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
    if (ParseNameSpace()) {

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
    if (ParseNameSpace()) {

        // Parse Top declarations
        while (Tok.isNot(tok::eof)) {
            if (!ParseTopDecl()) {
                return false;
            }
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
 * ConsumeToken - Consume the current 'peek token' and lex the next one.
 * This does not work with special tokens: string literals,
 * annotation tokens and balanced tokens must be handled using the specific
 * consume methods.
 * @return the location of the consumed token
 */
SourceLocation Parser::ConsumeToken() {
    assert(!isTokenSpecial() && "Should consume special tokens with Consume*Token");
    return ConsumeNext();
}

/**
 * isTokenParen - Return true if the cur token is '(' or ')'.
 * @return
 */
bool Parser::isTokenParen() const {
    return Tok.isOneOf(tok::l_paren, tok::r_paren);
}

/**
 * isTokenBracket - Return true if the cur token is '[' or ']'.
 * @return
 */
bool Parser::isTokenBracket() const {
    return Tok.isOneOf(tok::l_square, tok::r_square);
}

/**
 * isTokenBrace - Return true if the cur token is '{' or '}'.
 * @return
 */
bool Parser::isTokenBrace() const {
    return Tok.isOneOf(tok::l_brace, tok::r_brace);
}

/**
 * isTokenConsumeParenStringLiteral - True if this token is a string-literal.
 * @return
 */
bool Parser::isTokenStringLiteral() const {
    return tok::isStringLiteral(Tok.getKind());
}

/**
 * isTokenSpecial - True if this token requires special consumption methods.
 * @return
 */
bool Parser::isTokenSpecial() const {
    return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
           isTokenBrace();
}

/**
 * ConsumeParen - This consume method keeps the paren count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeParen() {
    assert(isTokenParen() && "wrong consume method");
    if (Tok.getKind() == tok::l_paren)
        ++ParenCount;
    else if (ParenCount) {
        //AngleBrackets.clear(*this);
        --ParenCount;       // Don't let unbalanced )'s drive the count negative.
    }

    return ConsumeNext();
}

/**
 * ConsumeBracket - This consume method keeps the bracket count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeBracket() {
    assert(isTokenBracket() && "wrong consume method");
    if (Tok.getKind() == tok::l_square)
        ++BracketCount;
    else if (BracketCount) {
        //AngleBrackets.clear(*this);
        --BracketCount;     // Don't let unbalanced ]'s drive the count negative.
    }

    return ConsumeNext();
}

/**
 * ConsumeBrace - This consume method keeps the brace count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeBrace() {
    assert(isTokenBrace() && "wrong consume method");
    if (Tok.getKind() == tok::l_brace)
        ++BraceCount;
    else if (BraceCount) {
        //AngleBrackets.clear(*this);
        --BraceCount;     // Don't let unbalanced }'s drive the count negative.
    }

    return ConsumeNext();
}

bool Parser::isBraceBalanced() const {
    return BraceCount == 0;
}

/**
 * ConsumeStringToken - Consume the current 'peek token', lexing a new one
 * and returning the token kind.  This method is specific to strings, as it
 * handles string literal concatenation, as per C99 5.1.1.2, translation
 * phase #6.
 * @return
 */
SourceLocation Parser::ConsumeStringToken() {
    assert(isTokenStringLiteral() && "Should only consume string literals with this method");
    return ConsumeNext();
}

SourceLocation Parser::ConsumeNext() {
    Lex.Lex(Tok);
    while (Tok.is(tok::comment)) {
        BlockComment = Tok.getCommentData().str();
        Lex.Lex(Tok);
    }
    return Tok.getLocation();
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

void Parser::ClearBlockComment() {
    BlockComment = "";
}

/**
 * Parse package declaration
 * @return true on Success or false on Error
 */
bool Parser::ParseNameSpace() {
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
            return false;
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
                return false;
            }
        }
        FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "NameSpace=" << NS);
        AST->setNameSpace(NS);
        return true;
    }

    // Define Default NameSpace also if it has not been defined
    FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "No namespace defined");
    AST->setNameSpace(ASTNameSpace::DEFAULT);
    return true;
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
        if (ParseType(Type) && ParseArrayType(Type)) {


            if (Tok.isAnyIdentifier()) {
                const Optional<Token> &NextTok = Lex.findNextToken(Tok.getLocation(), SourceMgr);

                // parse function
                if (NextTok->is(tok::l_paren)) {
                    return ParseFunction(Visibility, Constant, Type);
                }

                // parse global var
                return ParseGlobalVarDecl(Visibility, Constant, Type);
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
 * Parse Global Var declaration
 * @param VisKind
 * @param Constant
 * @param Type
 * @param Name
 * @param NameLoc
 * @return
 */
bool Parser::ParseGlobalVarDecl(VisibilityKind &VisKind, bool &Constant, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseGlobalVarDecl", "VisKind=" << VisKind <<
    ", Constant=" << Constant << ", Type=" << Type->str());
    GlobalVarParser Parser(this, Type);
    if (Parser.Parse()) {
        Parser.AST->Constant = Constant;
        Parser.AST->Visibility = VisKind;

        return AST->AddGlobalVar(Parser.AST);
    }

    return false;
}


/**
 * Parse Function declaration
 * @param VisKind
 * @param Constant
 * @param Type
 * @param Name
 * @param NameLoc
 * @return
 */
bool Parser::ParseFunction(VisibilityKind &VisKind, bool Constant, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunction","VisKind=" << VisKind << ", Constant="
                                                           << Constant << ", Type=" << Type->str());
    FunctionParser Parser(this);
    if (Parser.ParseFunction(Type)) {
        Parser.AST->Constant = Constant;
        Parser.AST->Visibility = VisKind;

        if (AST->isHeader()) {
            return AST->AddFunction(Parser.AST);
        }

        return Parser.ParseFunctionBody() && AST->AddFunction(Parser.AST);

    }

    return false;
}

/**
 * Parse Class declaration
 * @param VisKind
 * @param Constant
 * @return
 */
bool Parser::ParseClassDecl(VisibilityKind &VisKind, bool &Constant) {
    FLY_DEBUG("Parser", "ParseClassDecl");
    ClassParser Parser(this);
    if (Parser.Parse()) {
        Parser.Class->Constant = Constant;
        Parser.Class->Visibility = VisKind;

        // Add Comment to AST
        Parser.Class->setComment(BlockComment);
        ClearBlockComment(); // Clear for next use

        return AST->AddClass(Parser.Class);
    }

    return false;
}

/**
 * Parse a data Type
 * @return true on Success or false on Error
 */
bool Parser::ParseType(ASTType *&Type, bool OnlyBuiltin) {
    const SourceLocation &TypeLoc = Tok.getLocation();
    tok::TokenKind Kind = Tok.getKind();
    ConsumeToken();
    
    switch (Kind) {
        case tok::kw_bool:
            Type = new ASTBoolType(TypeLoc);
            break;
        case tok::kw_byte:
            Type = new ASTByteType(TypeLoc);
            break;
        case tok::kw_ushort:
            Type = new ASTUShortType(TypeLoc);
            break;
        case tok::kw_short:
            Type = new ASTShortType(TypeLoc);
            break;
        case tok::kw_uint:
            Type = new ASTUIntType(TypeLoc);
            break;
        case tok::kw_int:
            Type = new ASTIntType(TypeLoc);
            break;
        case tok::kw_ulong:
            Type = new ASTULongType(TypeLoc);
            break;
        case tok::kw_long:
            Type = new ASTLongType(TypeLoc);
            break;
        case tok::kw_float:
            Type = new ASTFloatType(TypeLoc);
            break;
        case tok::kw_double:
            Type = new ASTDoubleType(TypeLoc);
            break;
        case tok::kw_void:
            Type = new ASTVoidType(TypeLoc);
            break;
        default: {
            if (OnlyBuiltin) {
                assert(0 && "Unknown builtin type");
                return false;
            }
            IdentifierInfo *Id = Tok.getIdentifierInfo();
            if (!Id) {
                DiagInvalidId(TypeLoc);
                return false;
            }
            llvm::StringRef Name = Id->getName();
            Type = new ASTClassType(TypeLoc, Name.str());
        }
    }

    FLY_DEBUG_MESSAGE("Parser", "ParseType", "Type=" << Type->str());
    return true;
}

bool Parser::ParseArrayType(ASTType *&Type, ASTBlock * Block) {
    // Check Bracket [] for Array
    if (Tok.is(tok::l_square)) {
        const SourceLocation &Loc = ConsumeBracket();

        if (Block) { // if block is true it come from a local var assign
            ASTExpr *Expr = ParseExpr(Block);
            if (Expr && !Expr->getType()->isInteger()) {
                // Error: array size must be of integer type TODO
                return false;
            }
            Type = new ASTArrayType(Loc, Type, Expr);
        } else if (Tok.is(tok::numeric_constant)) { // Parse unsigned int array size
            std::string Size = StringRef(Tok.getLiteralData(), Tok.getLength()).str();
            NumberParser *NumberP = new NumberParser(ConsumeToken(), Size);
            ASTSingleValue *SizeValue = NumberP->getValue();
            if (!SizeValue->getType()->isInteger()) {
                // Error: must be integer
            }
            Type = new ASTArrayType(Loc, Type, (ASTIntegerValue *) SizeValue);
        } else {
            // Array Type without size have default size of 0
            Type = new ASTArrayType(Loc, Type, new ASTIntegerValue(Loc, new ASTUIntType(Loc), 0));
        }

        if (!Type) {
            // Error: array type is unset TODO
            return false;
        }

        if (Tok.is(tok::r_square)) {
            ConsumeBracket();
            return true;
        }
    }

    // No Array found
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

    // Parse keywords
    if (Tok.is(tok::kw_return)) { // Parse return
        SourceLocation Loc = ConsumeToken();
        ASTExpr *Expr = ParseExprEmpty(Block);
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
                ASTLocalVarRef* LocalVarRef = new ASTLocalVarRef(Loc, Block, std::string(Name), std::string(NameSpace));
                ASTExpr *Expr = ParseAssignmentExpr(Block, LocalVarRef);
                if (Expr != nullptr) {
                    LocalVarRef->setExpr(Expr);
                    return Block->AddLocalVarRef(LocalVarRef);
                }
            } else {
                // a()
                // a++
                // a  FIXME only var ??
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
    } else if (ExprParser::isUnaryPreOperator(Tok)) {
        ASTExprStmt *ExprStmt = new ASTExprStmt(Tok.getLocation(), Block);
        ASTUnaryGroupExpr *Expr = ExprParser::ParseUnaryPreExpr(this, Block);
        if (Expr != nullptr) {
            ExprStmt->setExpr(Expr);
            return Block->AddExprStmt(ExprStmt);
        }
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
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseSwitchStmt(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseSwitchStmt");

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
                        ASTValue *Val = ParseValue(Expr->getType()); // TODO check Type
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
                return true;
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
    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Create AST While Block
    ASTExpr *Cond = ParseExprEmpty(Block);
    ASTWhileBlock *While = new ASTWhileBlock(Loc, Block, Cond);

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    // Parse statement between braces
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (ParseInnerBlock(While)) {
            Block->Content.push_back(While);
            return true;
        }
    } else if (ParseStmt(While)) { // Only for a single Stmt without braces
        Block->Content.push_back(While);
        return true;
    }

    Diag(diag::err_parse_stmt);
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
 * Parse a Function Call
 * @param Block
 * @param Id
 * @param Loc
 * @return true on Success or false on Error
 */
ASTFuncCall * Parser::ParseFunctionCall(ASTBlock *Block, llvm::StringRef Name, llvm::StringRef NameSpace,
                                        SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunctionCall", "Name=" << Name + ", NameSpace=" << NameSpace);
    FunctionParser Parser(this);
    if (Parser.ParseCall(Block, Loc, Name, NameSpace)) {
        return Parser.Call;
    }
    return nullptr;
}

ASTValue *Parser::ParseValue() {
    ASTType *Type = nullptr;
    return ParseValue(Type);
}

/**
 * Parse a Value Expression
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValue(ASTType *Type) {
    FLY_DEBUG("Parser", "ParseValue");

    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        std::string StrVal = StringRef(Tok.getLiteralData(), Tok.getLength()).str();
        const SourceLocation &Loc = ConsumeToken();
        NumberParser *Number = new NumberParser(Loc, StrVal);
        return Number->getValue(Type);
    }

    if (Tok.isCharLiteral()) {
        // empty char is like a zero byte
        if (Tok.getLiteralData() == nullptr && Tok.getLength() == 0) {
            const SourceLocation &Loc = ConsumeToken();
            return new ASTIntegerValue(Loc, new ASTByteType(Loc), 0);
        }

        const StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        char Ch = *Val.begin();

        const SourceLocation &Loc = ConsumeToken();
        return new ASTIntegerValue(Loc, new ASTByteType(Loc), Ch);
    }

    if (Tok.isStringLiteral()) {
        const char *Chars = Tok.getLiteralData();
        unsigned int StringLength = Tok.getLength();
        const SourceLocation &Loc = ConsumeStringToken();
        // TODO check Val type if need more than 1 byte of memory
        ASTArrayValue *String = new ASTArrayValue(Loc, new ASTByteType(Loc));
        for (unsigned int i = 0; i < StringLength ; i++) {
            ASTIntegerValue *StringChar = new ASTIntegerValue(Loc, new ASTByteType(Loc), Chars[i]);
            String->addValue(StringChar);
        }
        // set size to ASTArrayType on var declaration
        ((ASTArrayType *) &Type)->setSize(new ASTIntegerValue(Loc, new ASTUIntType(Loc), String->size()));
        return String;
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        return new ASTBoolValue(ConsumeToken(), true);
    }
    if (Tok.is(tok::kw_false)) {
        return new ASTBoolValue(ConsumeToken(), false);
    }

    // Parse Array values
    if (Tok.is(tok::l_brace)) {
        const SourceLocation &Loc = ConsumeBrace();

        ASTArrayValue *ArrayValues = nullptr;
        if (Type) {
            if (Type->getKind() == TYPE_ARRAY) { // Type can be null if it not came from assignation
                ArrayValues = new ASTArrayValue(Loc, ((ASTArrayType *) Type)->getType());
            }
        }
        ArrayValues = ParseValues(ArrayValues);

        // Very important for set size to ASTArrayType on var declaration
        ASTArrayType *SubType = (ASTArrayType *) Type;
        ((ASTArrayType *) Type)->setSize(new ASTIntegerValue(Type->getLocation(), SubType->getType(), ArrayValues->size()));

        return ArrayValues;
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return nullptr;
}

/**
 * Parse Array Value Expression
 * @return the ASTValueExpr
 */
ASTArrayValue *Parser::ParseValues(ASTArrayValue *ArrayValues) {
    // Parse array values Ex. {1, 2, 3}
    while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {
        ASTValue *Value = ParseValue();
        if (Value) {
            if (!ArrayValues) { // Take the ASTType from first ASTValue
                ArrayValues = new ASTArrayValue(Tok.getLocation(), Value->getType());
            }
            ArrayValues->addValue(Value);
            if (Tok.is(tok::comma)) {
                ConsumeToken();
            } else {
                break;
            }
        }
    }

    if (!ArrayValues) { // Error: cannot use empty array without declaring it
        Diag(diag::err_invalid_value) << Tok.getName();
        return nullptr;
    }

    // End of Array
    if (Tok.is(tok::r_brace)) {
        ConsumeBrace();
        return ArrayValues;
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return nullptr;
}

/**
 * Parse a Local Variable
 * This is a var declaration with assignment
 * @param Block
 * @param Constant
 * @param Type
 * @return the ASTLocalVar
 */
ASTLocalVar *Parser::ParseLocalVar(ASTBlock *Block, bool Constant, ASTType *Type) {
    IdentifierInfo *Id = Tok.getIdentifierInfo();
    if (!Id) {
        Diag(Tok, diag::err_var_undefined);
        return nullptr;
    }

    //Assign to ASTLocalVar
    const std::string Name = std::string(Id->getName());
    const SourceLocation Loc = Tok.getLocation();
    FLY_DEBUG_MESSAGE("Parser", "ParseLocalVar",
                      "Name=" << Name << ", Constant=" << Constant << ", Type=" << Type->str());
    ASTLocalVar *Result = new ASTLocalVar(Loc, Block, Type, Name);
    Result->Constant = Constant;
    ConsumeToken();

    // Need to create a reference and assign the previous Var declaration in order to be found from references
    ASTVarRef *VarRef = new ASTVarRef(Loc, Name);
    VarRef->Decl = Result;

    // Parse the assignment if exists
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

ASTExpr *Parser::ParseExprEmpty(ASTBlock *Block) {
    ExprParser Parser(this, true);
    return Parser.ParseExpr(Block);
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
