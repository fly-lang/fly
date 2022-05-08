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
#include "AST/ASTClass.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunctionCall.h"
#include "AST/ASTExpr.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"
#include <regex>

using namespace fly;

/**
 * Parser Constructor
 * @param Input 
 * @param SourceMgr 
 * @param Diags 
 */
Parser::Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, SemaBuilder &Builder) :
    Input(Input), Diags(Diags), SourceMgr(SourceMgr), Builder(Builder),
    Lex(Input.getFileID(), Input.getBuffer(), SourceMgr) {
}

/**
 * Start Parsing Input and compose ASTNode
 * @param Node 
 * @return true on Success or false on Error
 */
bool Parser::Parse() {
    FLY_DEBUG("Parser", "Parse");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse NameSpace on first
    if (ParseNameSpace()) {
        Node = Builder.CreateNode(Input.getFileName(), NameSpace);

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

    return !Diags.hasErrorOccurred();
}

bool Parser::ParseHeader() {
    FLY_DEBUG("Parser", "ParseHeader");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse NameSpace on first
    if (ParseNameSpace()) {
        Node = Builder.CreateHeaderNode(Input.getFileName(), NameSpace);

        // Parse Top declarations
        while (Tok.isNot(tok::eof)) {
            if (!ParseTopDecl()) {
                return false;
            }
        }
    }

    return !Diags.hasErrorOccurred();
}

ASTNode *Parser::getNode() const {
    return Node;
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

        llvm::StringRef Name;
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
        NameSpace = Name.str();
        while (Tok.is(tok::period)) {
            NameSpace += ".";
            ConsumeToken();
            if (Tok.isAnyIdentifier()) {
                NameSpace += Tok.getIdentifierInfo()->getName();
                ConsumeToken();
            } else {
                Diag(Tok, diag::err_namespace_invalid) << Tok.getName();
                return false;
            }
        }
        FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "NameSpace=" << NameSpace);
        return true;
    }

    // Define Default NameSpace also if it has not been defined
    FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "No namespace defined");
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

            ASTImport *Import;
            if (Tok.isAnyIdentifier()) {
                IdentifierInfo *AliasId = Tok.getIdentifierInfo();
                llvm::StringRef Alias = AliasId->getName();
                const SourceLocation &AliasLoc = ConsumeToken();
                Import = new ASTImport(Loc, Name.str(), AliasLoc, Alias.str());
            } else {
                Import = new ASTImport(Loc, Name.str());
            }
            FLY_DEBUG_MESSAGE("Parser", "ParseImportAlias",
                              "Import=" + Import->getName() + " , Alias=" << Import->getAlias());
            return Builder.AddImport(Node, Import) && ParseImports();
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
            return ParseClass(Visibility, Constant);
        }

        // Parse Type
        ASTType *Type = nullptr;
        // Parse Type and after brackets []
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
bool Parser::ParseGlobalVarDecl(VisibilityKind &Visibility, bool &Constant, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseGlobalVarDecl", "Visibility=" << Visibility <<
    ", Constant=" << Constant << ", Type=" << Type->str());

    assert(Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    SourceLocation Loc = Tok.getLocation();

    ASTGlobalVar *GlobalVar = Builder.CreateGlobalVar(Node, Loc, Type, Name.str(), Visibility, Constant);

    // Parsing =
    ConsumeToken();
    ASTValue *Val = nullptr;
    if (Tok.is(tok::equal)) {
        ConsumeToken();
        Val = ParseValue(Type);
    }

    if (GlobalVar) {

        // Add Comment to AST
        if (!BlockComment.empty()) {
            Builder.AddComment(GlobalVar, BlockComment);
            ClearBlockComment(); // Clear for next use
        }

        return Builder.AddGlobalVar(Node, GlobalVar, new ASTValueExpr(Val));
    }

    return false;
}


/**
 * Parse Function declaration
 * @param Visibility
 * @param Constant
 * @param Type
 * @param Name
 * @param NameLoc
 * @return
 */
bool Parser::ParseFunction(VisibilityKind &Visibility, bool Constant, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunction","VisKind=" << Visibility << ", Constant="
                                                           << Constant << ", Type=" << Type->str());
    ASTFunction *Function = FunctionParser::Parse(this, Visibility, Type, Node->isHeader());
    if (Function) {

        // Add Comment to AST
        if (!BlockComment.empty()) {
            Builder.AddComment(Function, BlockComment);
            ClearBlockComment(); // Clear for next use
        }

        return Builder.AddFunction(Node, Function);
    }

    return false;
}

/**
 * Parse Class declaration
 * @param Visibility
 * @param Constant
 * @return
 */
bool Parser::ParseClass(VisibilityKind &Visibility, bool &Constant) {
    FLY_DEBUG("Parser", "ParseClass");
    ASTClass *Class = ClassParser::Parse(this, Visibility, Constant);
    if (Class) {

        // Add Comment to AST
        if (!BlockComment.empty()) {
            Builder.AddComment(Class, BlockComment);
            ClearBlockComment(); // Clear for next use
        }

        return Builder.AddClass(Node, Class);
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
            assert(!OnlyBuiltin && "Unknown builtin type");
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

        if (Block) { // if block is true it comes from a local var assign
            ASTExpr *Expr = ParseExpr(Block);
            if (Expr && !Expr->getType()->isInteger()) {
                // Error: array size must be of integer type TODO
                return false;
            }
            Type = new ASTArrayType(Loc, Type, Expr);
        } else if (Tok.is(tok::numeric_constant)) { // Parse unsigned int array size
            std::string Size = std::string(Tok.getLiteralData(), Tok.getLength());
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
        return Builder.AddReturn(Block, Loc, Expr);
    } else if (Tok.is(tok::kw_break)) { // Parse break
        return Builder.AddBreak(Block, ConsumeToken());
    } else if (Tok.is(tok::kw_continue)) { // Parse continue
        return Builder.AddContinue(Block, ConsumeToken());
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
                    return ParseLocalVar(Block, Constant, Type);
                }

                // Error: Invalid Type format
                Diag(diag::err_parser_invalid_type);
                return false;
            } else if (ExprParser::isAssignOperator(Tok)) { // variable assignment
                // a = ...
                // if is += or -= ... create the ref to var
                ASTVarRef* VarRef = new ASTVarRef(Loc, std::string(Name), std::string(NameSpace));
                ASTExpr *Expr = ParseAssignmentExpr(Block, VarRef);
                if (Expr) {
                    ASTVarAssign *VarAssign = new ASTVarAssign(Loc, Block, VarRef, Expr);
                    return Builder.AddVarAssign(Block, VarAssign);
                }
            } else {
                // a()
                // a++
                // a  FIXME only var ??
                ASTExprStmt *ExprStmt = new ASTExprStmt(Tok.getLocation());
                ASTExpr *Expr = ParseExpr(Block, Name, NameSpace, Loc);
                if (Expr != nullptr) {
                    ExprStmt->setExpr(Expr);
                    return Builder.AddExprStmt(Block, ExprStmt);
                }
            }
        }
    } else if (isBuiltinType()) { // variable declaration
        // int a = ...
        // int a
        ASTType *Type = nullptr;
        if (ParseType(Type, /* OnlyBuiltin */ true)) {
            return ParseLocalVar(Block, Constant, Type);
        }

        Diag(diag::err_parser_invalid_type);
        return false;
    } else if (ExprParser::isUnaryPreOperator(Tok)) {
        ASTExprStmt *ExprStmt = new ASTExprStmt(Tok.getLocation());
        ASTUnaryGroupExpr *Expr = ExprParser::ParseUnaryPreExpr(this, Block);
        if (Expr != nullptr) {
            ExprStmt->setExpr(Expr);
            return Builder.AddExprStmt(Block, ExprStmt);
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

    // Get previous ASTIfBlock if there is another 'if' statement
    ASTStmt *PrevIf = Builder.getLastStmt(Block);
    ASTIfBlock *IfBlock = nullptr;
    if (!Block->isEmpty()) {
        if (PrevIf && PrevIf->getKind() == StmtKind::STMT_BLOCK &&
            ((ASTBlock *) PrevIf)->getBlockKind() == ASTBlockKind::BLOCK_STMT_IF) {
            IfBlock = ((ASTIfBlock *) PrevIf);
        }
    }

    ASTBlock *CurrentBlock;
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
            if (IfBlock) {
                Diag(Loc, diag::err_duplicate_if);
                return false;
            }
            CurrentBlock = Builder.AddIfBlock(Block, Loc, Expr);
            break;
        }
        case tok::kw_elsif: {
            ConsumeToken();
            // Parse (
            bool hasParen = ParseStartParen();
            // Parse the group of expressions into parenthesis
            ASTExpr *Expr = ParseExpr(Block);
            // Parse ) if exists
            if (!Expr || !ParseEndParen(hasParen)) {
                return false;
            }
            CurrentBlock = Builder.AddElsifBlock(IfBlock, Loc, Expr);
            break;
        }
        case tok::kw_else: {
            ConsumeToken();
            CurrentBlock = Builder.AddElseBlock(IfBlock, Loc);
            break;
        }
        default:
            assert(0 && "Unknown conditional statement");
    }

    if (!CurrentBlock) { // Error on AddElseBlock or AddElsifBlock
        return false;
    }

    // Parse statement between braces for If, Elsif, Else
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (ParseInnerBlock(CurrentBlock)) {
            return true;
        }
    } else if (ParseStmt(CurrentBlock)) { // Only for a single Stmt without braces
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
        ASTSwitchBlock *SwitchBlock = Builder.AddSwitchBlock(Block, SwitchLoc, Expr);
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
                        ASTSwitchCaseBlock *CaseBlock = SwitchBlock->AddCase(CaseLoc, CaseExp);
                        while (!Tok.isOneOf(tok::r_brace, tok::kw_case, tok::kw_default, tok::eof)) {
                            ParseBlock(CaseBlock);
                        }
                    }
                } else if (Tok.is(tok::kw_default)) {
                    ConsumeToken();

                    if (Tok.is(tok::colon)) {
                        const SourceLocation &Loc = ConsumeToken();

                        // Add Default to Switch statement and parse statement not contained into braces
                        ASTSwitchDefaultBlock *DefaultBlock = SwitchBlock->setDefault(Loc);
                        ParseBlock(DefaultBlock);
                    } else {
                        Diag(diag::err_syntax_error);
                        return false;
                    }
                }
            }

            // Switch statement is at end of it's time add current Switch to parent statement
            if (Tok.is(tok::r_brace)) {
                ConsumeBrace();
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
    ASTWhileBlock *While = Builder.AddWhileBlock(Block, Loc, Cond);

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    // Parse statement between braces
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (ParseInnerBlock(While)) {
            return true;
        }
    } else if (ParseStmt(While)) { // Only for a single Stmt without braces
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
    ASTExpr *Condition = Builder.CreateExpr(Loc, Builder.CreateValue(Loc,true));
    ASTBlock *LoopBlock = Builder.CreateBlock(Loc);
    ASTBlock *PostBlock = Builder.CreateBlock(Loc);
    ASTForBlock *ForBlock = Builder.CreateForBlock(Loc, Condition, PostBlock, LoopBlock);

    // parse comma separated or single statements
    if (ParseForCommaStmt(ForBlock)) {
        assert(Tok.isNot(tok::comma) && "Comma not parsed"); // Already parsed from ParseForCommaStmt()

        // This is an Expression, it could be a Condition
        if (Tok.is(tok::semi)) {
            ConsumeToken();

            Condition = ParseExpr(ForBlock);

            if (Tok.is(tok::semi)) {
                PostBlock = Builder.CreateBlock(ConsumeToken());
                Success &= ParseForCommaStmt(PostBlock);
            }
        }
    }

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    // Parse statement between braces
    LoopBlock = Builder.CreateBlock(Tok.getLocation());
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (Success && ParseInnerBlock(LoopBlock)) {
            return Builder.AddForBlock(Block, ForBlock);
        }
    } else if (Success && ParseStmt(LoopBlock)) { // Only for a single Stmt without braces
        return Builder.AddForBlock(Block, ForBlock);
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
bool Parser::ParseIdentifier(llvm::StringRef &Name, llvm::StringRef NameSpace, SourceLocation &Loc) {
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
bool Parser::ParseFunctionCall(ASTBlock *Block, llvm::StringRef Name, llvm::StringRef NameSpace,
                                            SourceLocation &Loc) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunctionCall", "Name=" << Name + ", NameSpace=" << NameSpace);
    std::string NameStr = Name.str();
    std::string NameSpaceStr = NameSpace.str();
    ASTFunctionCall *Call = Builder.CreateFunctionCall(Loc, NameStr, NameSpaceStr);
    if (ParseCallArgs(Call, Block)) {
        return Builder.AddFunctionCall(Block, Call);
    }

    return false;
}

/**
 * Parse Call Arguments
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseCallArgs(ASTFunctionCall *Call, ASTBlock *Block) {
    if (Tok.is(tok::l_paren)) { // parse start of function ()
        ConsumeParen(); // consume l_paren
    }

    if (Tok.is(tok::r_paren)) {
        ConsumeParen();
        return true; // end
    }

    return ParseCallArg(Call, Block);
}

/**
 * Parse a single Call Argument
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseCallArg(ASTFunctionCall *Call, ASTBlock *Block) {

    // Parse Args in a Function Call
    ASTExpr *Expr = ParseExpr(Block);

    if (Expr) {
        ASTCallArg *Arg = Builder.CreateCallArg(Expr);
        Builder.AddCallArg(Call, Arg);

        if (Tok.is(tok::comma)) {
            ConsumeToken();
            return ParseCallArg(Call, Block);
        }

        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            return true; // end
        }
    }

    Diag(Tok.getLocation(), diag::err_func_param);
    return false;
}


/**
 * Parse a Value Expression
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValue(ASTType *Type) {
    FLY_DEBUG("Parser", "ParseValue");

    if (Tok.is(tok::kw_null)) {
        const SourceLocation &Loc = ConsumeToken();
        if (Type->getKind() != TYPE_CLASS) {
            Diag(diag::err_parser_invalid_null_value);
            return nullptr;
        }
        return new ASTClassValue(Loc, (ASTClassType *) Type);
    }

    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        std::string StrVal = std::string(Tok.getLiteralData(), Tok.getLength());
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
bool Parser::ParseLocalVar(ASTBlock *Block, bool Constant, ASTType *Type) {
    IdentifierInfo *Id = Tok.getIdentifierInfo();
    if (!Id) {
        Diag(Tok, diag::err_var_undefined);
        return false;
    }

    //Assign to ASTLocalVar
    const std::string Name = std::string(Id->getName());
    const SourceLocation Loc = Tok.getLocation();
    FLY_DEBUG_MESSAGE("Parser", "ParseLocalVar",
                      "Name=" << Name << ", Constant=" << Constant << ", Type=" << Type->str());
    ASTLocalVar *LocalVar = Builder.CreateLocalVar(Block, Loc, Type, Name, Constant);
    ConsumeToken();

    // if is += or -= ... create the ref to var
    // Need to create a reference and assign the previous Var declaration in order to be found from references
    ASTVarRef *VarRef = Builder.CreateVarRef(LocalVar);

    // Parse the assignment if exists
    ASTExpr *Expr = ParseAssignmentExpr(Block, VarRef);
    if (Expr) {
        return Builder.AddLocalVar(Block, LocalVar, Expr);
    }

    return false;
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
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false, tok::kw_null);
}
