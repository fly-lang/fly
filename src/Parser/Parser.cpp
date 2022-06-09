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

    bool Success;

    // Parse NameSpace on first
    if ((Success = ParseNameSpace())) {
        Node = Builder.CreateNode(Input.getFileName(), NameSpace);

        // Parse Imports
        if ((Success = ParseImports())) {

            // Node empty
            if (Tok.is(tok::eof)) {
                Diag(Tok.getLocation(), diag::warn_empty_code);
            }

            // Parse All
            while (Tok.isNot(tok::eof) && Success) {

                // if parse "namespace" there is multiple package declarations
                if (Tok.is(tok::kw_namespace)) {

                    // Multiple Package declaration is invalid, you can define only one and on top of declarations
                    Diag(Tok, diag::err_namespace_multi_defined);
                    return false;
                }

                // if parse "import" there is import declaration position error
                if (Tok.is(tok::kw_import)) {

                    // Import has to be defined after namespace and before all definitions
                    Diag(Tok, diag::err_import_invalid) << Tok.getName();
                    return false;
                }

                // Parse top definitions
                Success = ParseTopDef();
            }
        }
    }

    // Check if error is correctly set
    assert(Success == !Diags.hasErrorOccurred());

    return Success && Builder.AddNode(Node);
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
            if (!ParseTopDef()) {
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
        } else if (Tok.isAnyIdentifier()) { // Check if a different namespace identifier has been defined
            Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
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
            SourceLocation NameLoc = ConsumeToken();

            ASTImport *Import;
            if (Tok.isAnyIdentifier()) {
                IdentifierInfo *AliasId = Tok.getIdentifierInfo();
                llvm::StringRef Alias = AliasId->getName();
                const SourceLocation &AliasLoc = ConsumeToken();
                FLY_DEBUG_MESSAGE("Parser", "ParseImports", "Name=" << Name << ", Alias=" << Alias);
                Import = Builder.CreateImport(NameLoc, Name, AliasLoc, Alias);
            } else {
                FLY_DEBUG_MESSAGE("Parser", "ParseImports", "Name=" << Name << ", Alias=");
                Import = Builder.CreateImport(Loc, Name.str());
            }
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
bool Parser::ParseTopDef() {
    FLY_DEBUG("Parser", "ParseTopDef");

    VisibilityKind Visibility = VisibilityKind::V_DEFAULT;
    bool Constant = false;

    // Parse Public or Private and Constant
    if (ParseTopScopes(Visibility, Constant)) {

        if (Tok.is(tok::kw_class)) {
            return ParseClass(Visibility, Constant);
        }

        // Parse Type and after brackets []
        ASTType *Type = ParseType();
        if (Type) {

            if (Tok.isAnyIdentifier()) {
                const Optional<Token> &NextTok = Lex.findNextToken(Tok.getLocation(), SourceMgr);

                // parse function
                if (NextTok->is(tok::l_paren)) {
                    return ParseFunction(Visibility, Constant, Type);
                }

                // parse global var
                return ParseGlobalVar(Visibility, Constant, Type);
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
 * Parse Global Var declaration
 * @param Visibility
 * @param Constant
 * @param Type
 * @param Name
 * @param NameLoc
 * @return
 */
bool Parser::ParseGlobalVar(VisibilityKind &Visibility, bool &Constant, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseGlobalVar",
                      "Visibility=" << Visibility << ", Constant=" << Constant << ", Type=" << Type->str());

    assert(Tok.isAnyIdentifier() && "Tok must be an Identifier");

    // Add Comment to AST
    std::string Comment;
    if (!BlockComment.empty()) {
        Comment = BlockComment;
        ClearBlockComment(); // Clear for next use
    }
    
    IdentifierInfo *Id = Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    SourceLocation Loc = ConsumeToken();

    ASTGlobalVar *GlobalVar = Builder.CreateGlobalVar(Node, Loc, Type, Name.str(), Visibility, Constant);

    // Parsing =
    ASTExpr *Expr = nullptr;
    if (isTokenAssign()) {
        ConsumeToken();
        Expr = ParseExpr();
    }

    return Builder.AddGlobalVar(Node, GlobalVar, Expr) &&
        Builder.AddComment(GlobalVar, Comment);
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

    // Add Comment to AST
    std::string Comment;
    if (!BlockComment.empty()) {
        Comment = BlockComment;
        ClearBlockComment(); // Clear for next use
    }

    ASTFunction *Function = FunctionParser::Parse(this, Visibility, Type, Node->isHeader());
    if (Function) {
        return Builder.AddComment(Function, Comment) &&
            Builder.AddFunction(Node, Function);
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
 * Parse statements not yet contained into braces
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseBlock(ASTBlock *Block) {
    FLY_DEBUG("Parser", "ParseBlock");
    if (Tok.is(tok::kw_if)) {
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
        ASTReturn *Return = Builder.CreateReturn(Loc);
        ASTExpr *Expr = ParseExpr(Return);
        return Builder.AddStmt(Block, Return, Expr);
    } else if (Tok.is(tok::kw_break)) { // Parse break
        ASTBreak *Break = Builder.CreateBreak(ConsumeToken());
        return Builder.AddStmt(Block, Break);
    } else if (Tok.is(tok::kw_continue)) { // Parse continue
        ASTContinue *Continue = Builder.CreateContinue(ConsumeToken());
        return Builder.AddStmt(Block, Continue);
    }

    // define a const LocalVar
    bool Const = isConst();

    // Used for know next tokens
    Optional<Token> OptTok1 = Tok;
    Optional<Token> OptTok2 = Tok;
    
    // Define an ASTLocalVar
    // Type a
    // int a
    if (isType(OptTok1) && (OptTok1 = Lex.findNextToken(OptTok1->getLocation(), SourceMgr)) &&
        isIdentifier(OptTok1)) {

        // parse the type
        ASTType *Type = ParseType();
        if (!Type) {
            Diag(diag::err_parser_invalid_type);
            return false;
        }

        // parse the identifier
        IdentifierInfo *Id = Tok.getIdentifierInfo();
        const std::string Name = std::string(Id->getName());
        ConsumeToken();

        // create/add LocalVar
        ASTLocalVar *LocalVar = Builder.CreateLocalVar(Tok.getLocation(), Type, Name, Const);
        // Need to create a reference and assign the previous Var declaration in order to be found from references
        if (isTokenAssign()) {
            ConsumeToken();
            ASTExpr *Expr = ParseExpr(LocalVar);
            return Expr && Builder.AddLocalVar(Block, LocalVar, Expr);
        }
        
        if (isTokenOperator() || isTokenAssignOperator()) {
            Diag(diag::err_parser_generic);
            return false;
        }
        
        return Builder.AddLocalVar(Block, LocalVar, nullptr);
    } else if (isIdentifier(OptTok2) &&
            isTokenAssignOperator(Lex.findNextToken(OptTok2->getLocation(), SourceMgr))) { // define an ASTVarAssign

        ASTVarRef* VarRef = ParseVarRef();
        ASTVarAssign *VarAssign = Builder.CreateVarAssign(VarRef);

        ExprParser Parser(this, VarAssign);
        ASTExpr *Expr = Parser.ParseAssignExpr();

        return Expr && Builder.AddStmt(Block, VarAssign, Expr);
    } else { // parse an ASTExprStmt
        ASTExprStmt *ExprStmt = Builder.CreateExprStmt(Tok.getLocation());
        ASTExpr *Expr = ParseExpr(ExprStmt);
        return Expr && Builder.AddStmt(Block, ExprStmt, Expr);
    }
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
    assert(Tok.is(tok::kw_if) && "Token is if keyword");
    FLY_DEBUG("Parser", "ParseIfStmt");

    ConsumeToken();
    // Parse (
    bool hasIfParen = ParseStartParen();
    // Parse the group of expressions into parenthesis
    ASTExpr *Expr = ParseExpr(); // Parse Expr
    if (hasIfParen) {
        ParseEndParen(hasIfParen);
    }
    ASTIfBlock *IfBlock = Builder.CreateIfBlock(Tok.getLocation(), Expr);
    // Parse statement between braces for If
    bool Success = false;
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (ParseInnerBlock(IfBlock)) {
            Success = Builder.AddIfBlock(Block, IfBlock);
        }
    } else if (ParseStmt(IfBlock)) { // Only for a single Stmt without braces
        Success = Builder.AddIfBlock(Block, IfBlock);
    }

    // Add Elsif
    while (Success && Tok.is(tok::kw_elsif)) {
        const SourceLocation &ElsifLoc = ConsumeToken();
        // Parse (
        bool hasElsifParen = ParseStartParen();
        // Parse the group of expressions into parenthesis
        ASTExpr *Expr = ParseExpr(); // Parse Expr
        if (hasElsifParen) {
            ParseEndParen(hasElsifParen);
        }
        ASTElsifBlock *ElsifBlock = Builder.CreateElsifBlock(ElsifLoc, Expr);
        if (Tok.is(tok::l_brace)) {
            ConsumeBrace();
            Success = ParseInnerBlock(ElsifBlock) && Builder.AddElsifBlock(IfBlock, ElsifBlock);
        } else if (ParseStmt(ElsifBlock)) { // Only for a single Stmt without braces
            Success = Builder.AddElsifBlock(IfBlock, ElsifBlock);
        }
    }

    // Add Else
    if (Success && Tok.is(tok::kw_else)) {
        const SourceLocation &ElseLoc = ConsumeToken();
        ASTElseBlock *ElseBlock = Builder.CreateElseBlock(ElseLoc);
        if (Tok.is(tok::l_brace)) {
            ConsumeBrace();
            Success = ParseInnerBlock(ElseBlock) && Builder.AddElseBlock(IfBlock, ElseBlock);
        } else if (ParseStmt(ElseBlock)) { // Only for a single Stmt without braces
            Success = Builder.AddElseBlock(IfBlock, ElseBlock);
        }
    }

    return Success;
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
    assert(Tok.is(tok::kw_switch) && "Token is switch keyword");
    FLY_DEBUG("Parser", "ParseSwitchStmt");

    // Parse switch keyword
    const SourceLocation &SwitchLoc = ConsumeToken();

    // Parse (
    bool hasParen = ParseStartParen();

    // Parse Var reference like (a)
    ASTExpr *Expr = ParseExpr();
    if (Expr) {

        // Consume Right Parenthesis ) if exists
        if (!ParseEndParen(hasParen)) {
            return false;
        }

        // Init Switch Statement and start parse from brace
        if (Tok.is(tok::l_brace)) {
            ConsumeBrace();
            ASTSwitchBlock *SwitchBlock = Builder.CreateSwitchBlock(SwitchLoc, Expr);
            bool Success = true;
            // Exit only Statement find a closed brace or EOF
            while (Success && Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {

                // Parse case keyword
                if (Tok.is(tok::kw_case)) {
                    const SourceLocation &CaseLoc = ConsumeToken();

                    // Parse Expression for different cases
                    // for a Value  -> case 1:
                    // for a Var -> case a:
                    // for a default
                    ASTExpr * CaseExpr = ParseExpr();
                    if (Tok.is(tok::colon)) {
                        ConsumeToken();

                        // Add Case to Switch statement and parse statement not contained into braces
                        ASTSwitchCaseBlock *CaseBlock = Builder.CreateSwitchCaseBlock(CaseLoc, CaseExpr);
                        Success = ParseInnerBlock(CaseBlock) &&
                                Builder.AddSwitchCaseBlock(SwitchBlock, CaseBlock);
                    } else {
                        Diag(diag::err_syntax_error);
                        return false;
                    }
                } else if (Tok.is(tok::kw_default)) {
                    const SourceLocation &DefaultLoc = ConsumeToken();
                    ASTSwitchDefaultBlock *DefaultBlock = Builder.CreateSwitchDefaultBlock(DefaultLoc);

                    if (Tok.is(tok::colon)) {
                        ConsumeToken();

                        // Add Default to Switch statement and parse statement not contained into braces
                        Success = ParseInnerBlock(DefaultBlock) &&
                                Builder.setSwitchDefaultBlock(SwitchBlock, DefaultBlock);
                    } else {
                        Diag(diag::err_syntax_error);
                        return false;
                    }
                }
            }

            // Switch statement is at end of it's time add current Switch to parent statement
            if (Success && Tok.is(tok::r_brace)) {
                ConsumeBrace();
                return Builder.AddSwitchBlock(Block, SwitchBlock);
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
    ASTExpr *Cond = ParseExpr();

    ASTWhileBlock *WhileBlock = Builder.CreateWhileBlock(Loc, Cond);

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    // Parse statement between braces
    if (Tok.is(tok::l_brace)) {
        ConsumeBrace();
        if (ParseInnerBlock(WhileBlock) && Tok.is(tok::r_brace)) {
            ConsumeBrace();
            return Builder.AddWhileBlock(Block, WhileBlock);
        }
    } else {
        // Only for a single Stmt without braces
        return ParseStmt(WhileBlock) && Builder.AddWhileBlock(Block, WhileBlock);
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
    ASTBlock *LoopBlock = Builder.CreateBlock(Loc);
    ASTBlock *PostBlock = Builder.CreateBlock(Loc);
    ASTForBlock *ForBlock = Builder.CreateForBlock(Loc, PostBlock, LoopBlock);
    ASTExpr *Condition = Builder.CreateExpr(ForBlock, Builder.CreateBoolValue(Loc, true));

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
            return Builder.AddForBlock(Block, ForBlock, Condition);
        }
    } else if (Success && ParseStmt(LoopBlock)) { // Only for a single Stmt without braces
        return Builder.AddForBlock(Block, ForBlock, Condition);
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
 * Parse a data Type
 * @return true on Success or false on Error
 */
ASTType *Parser::ParseType() {
    FLY_DEBUG("Parser", "ParseType");

    ASTType *Type;
    switch (Tok.getKind()) {
        case tok::kw_bool:
            Type = Builder.CreateBoolType(ConsumeToken());
            break;
        case tok::kw_byte:
            Type = Builder.CreateByteType(ConsumeToken());
            break;
        case tok::kw_ushort:
            Type = Builder.CreateUShortType(ConsumeToken());
            break;
        case tok::kw_short:
            Type = Builder.CreateShortType(ConsumeToken());
            break;
        case tok::kw_uint:
            Type = Builder.CreateUIntType(ConsumeToken());
            break;
        case tok::kw_int:
            Type = Builder.CreateIntType(ConsumeToken());
            break;
        case tok::kw_ulong:
            Type = Builder.CreateULongType(ConsumeToken());
            break;
        case tok::kw_long:
            Type = Builder.CreateLongType(ConsumeToken());
            break;
        case tok::kw_float:
            Type = Builder.CreateFloatType(ConsumeToken());
            break;
        case tok::kw_double:
            Type = Builder.CreateDoubleType(ConsumeToken());
            break;
        case tok::kw_void:
            Type = Builder.CreateVoidType(ConsumeToken());
            break;
        default: {
            if (Tok.isAnyIdentifier()) {
                SourceLocation Loc = Tok.getLocation();
                llvm::StringRef Name;
                llvm::StringRef NameSpace;
                ParseIdentifier(Loc, Name, NameSpace);
                Type = Builder.CreateClassType(Loc, Name, NameSpace);
            }
        }
    }

    while (Tok.is(tok::l_square)) {
        const SourceLocation &Loc = ConsumeBracket();

        ASTExpr *Expr;

        if (Tok.is(tok::r_square)) {
            ConsumeBracket();
            Expr = Builder.CreateExpr(nullptr, Builder.CreateIntegerValue(Loc, 0));
            Type = Builder.CreateArrayType(Loc, Type, Expr);
        } else {
            Expr = ParseExpr();
            if (Tok.is(tok::r_square)) {
                ConsumeBracket();
                Type = Builder.CreateArrayType(Loc, Type, Expr);
            } else {
                Diag(Loc, diag::err_parser_unclosed_bracket);
                return nullptr;
            }
        }
    }
    return Type;
}

ASTVarRef *Parser::ParseVarRef() {
    const StringRef &N1 = Tok.getIdentifierInfo()->getName();
    const SourceLocation &Loc = ConsumeToken();

    if (Tok.is(tok::colon)) {
        ConsumeToken();

        if (Tok.isAnyIdentifier()) {
            const StringRef &N2 = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            return Builder.CreateVarRef(Loc, N2, N1);
        }

        Diag(Loc, diag::err_invalid_namespace_id);
        return nullptr;
    }

    return Builder.CreateVarRef(Loc, N1, NameSpace);;
}

/**
 * Parse as Identifier with a Name and NameSpace
 * @param Name
 * @param NameSpace
 * @return
 */
bool Parser::ParseIdentifier(SourceLocation &Loc, llvm::StringRef &Name, llvm::StringRef &NameSpace) {
    Name = Tok.getIdentifierInfo()->getName();
    Loc = Tok.getLocation();
    ConsumeToken();
    if (Tok.is(tok::colon)) {
        ConsumeToken();
        if (Tok.isAnyIdentifier()) {
            NameSpace = Name;
            Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
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
ASTFunctionCall *Parser::ParseFunctionCall(ASTStmt *Stmt, SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef NameSpace) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunctionCall", "Name=" << Name + ", NameSpace=" << NameSpace);
    std::string NameStr = Name.str();
    std::string NameSpaceStr = NameSpace.str();
    ASTFunctionCall *Call = Builder.CreateFunctionCall(Stmt, Loc, NameStr, NameSpaceStr);
    // Parse Call args
    if (ParseCallArgs(Call)) {
        return Call;
    }

    return nullptr;
}

/**
 * Parse Call Arguments
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseCallArgs(ASTFunctionCall *Call) {
    if (Tok.is(tok::l_paren)) { // parse start of function ()
        ConsumeParen(); // consume l_paren
    }

    if (Tok.is(tok::r_paren)) {
        ConsumeParen();
        return true; // end
    }

    return ParseCallArg(Call);
}

/**
 * Parse a single Call Argument
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseCallArg(ASTFunctionCall *Call) {

    ASTArg *Arg = Builder.CreateArg(Tok.getLocation());
    // Parse Args in a Function Call
    ASTExpr *Expr = ParseExpr(Arg);

    if (Builder.AddFunctionCallArg(Call, Arg, Expr)) {

        if (Tok.is(tok::comma)) {
            ConsumeToken();
            return ParseCallArg(Call);
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
ASTValue *Parser::ParseValue() {
    FLY_DEBUG("Parser", "ParseValue");

    if (Tok.is(tok::kw_null)) {
        const SourceLocation &Loc = ConsumeToken();
        return Builder.CreateNullValue(Loc);
    }

    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        std::string Val = std::string(Tok.getLiteralData(), Tok.getLength());
        const SourceLocation &Loc = ConsumeToken();
        return Builder.CreateValue(Loc, Val);
    }

    if (Tok.isCharLiteral()) {
        // empty char is like a zero byte
        if (Tok.getLiteralData() == nullptr && Tok.getLength() == 0) {
            const SourceLocation &Loc = ConsumeToken();
            return Builder.CreateIntegerValue(Loc, 0);
        }

        const StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        char Ch = *Val.begin();

        const SourceLocation &Loc = ConsumeToken();
        return Builder.CreateCharValue(Loc, Ch);
    }

    if (Tok.isStringLiteral()) {
        const char *Chars = Tok.getLiteralData();
        unsigned int StringLength = Tok.getLength();
        const SourceLocation &Loc = ConsumeStringToken();
        // TODO check Val type if need more than 1 byte of memory
        ASTArrayValue *String = Builder.CreateArrayValue(Loc);
        for (unsigned int i = 0; i < StringLength ; i++) {
            ASTIntegerValue *StringChar = Builder.CreateIntegerValue(Loc, Chars[i]);
            Builder.AddArrayValue(String, StringChar);
        }
        // set size to ASTArrayType on var declaration
        return String;
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        return Builder.CreateBoolValue(ConsumeToken(), true);
    }
    if (Tok.is(tok::kw_false)) {
        return Builder.CreateBoolValue(ConsumeToken(), false);
    }

    // Parse Array values
    if (Tok.is(tok::l_brace)) {
        const SourceLocation &Loc = ConsumeBrace();
        ASTArrayValue *ArrayValues = Builder.CreateArrayValue(Tok.getLocation());
        if (!ParseValues(*ArrayValues)) {
            return nullptr;
        }
        return ArrayValues;
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return nullptr;
}

/**
 * Parse Array Value Expression
 * @return the ASTValueExpr
 */
bool Parser::ParseValues(ASTArrayValue &ArrayValues) {
    // Parse array values Ex. {1, 2, 3}
    while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {
        ASTValue *Value = ParseValue();
        if (Value) {
            Builder.AddArrayValue(&ArrayValues, Value);
            if (Tok.is(tok::comma)) {
                ConsumeToken();
            } else {
                break;
            }
        } else {
            Diag(diag::err_invalid_value) << Tok.getName();
            return false;
        }
    }

    // End of Array
    if (Tok.is(tok::r_brace)) {
        ConsumeBrace();
        return true;
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return false;
}

ASTExpr *Parser::ParseExpr(ASTStmt *Stmt) {
    ExprParser Parser(this, Stmt);
    return Parser.ParseExpr();
}

bool Parser::isType(Optional<Token> &Tok1) {

    // Check if is Builtin Type
    if (isBuiltinType(Tok1.getValue())) {
        return true;
    }

    // Check if is Class Type
    if (Tok1->isAnyIdentifier()) {
        const Optional<Token> &Tok2 = Lex.findNextToken(Tok1->getLocation(), SourceMgr);
        const Optional<Token> &Tok3 = Lex.findNextToken(Tok2->getLocation(), SourceMgr);
        if (Tok2->is(tok::colon) && Tok3->isAnyIdentifier()) {
            Tok1 = Tok3;
            return true;
        }
        return true;
    }

    return false;
}

/**
 * Check if Token is one of the Builtin Types
 * @return true on Success or false on Error
 */
bool Parser::isBuiltinType(Token &Tok) {
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

bool Parser::isIdentifier() {
    Optional<Token> OptTok = Tok;
    return isIdentifier(OptTok);
}

bool Parser::isIdentifier(Optional<Token> &Tok1) {
    if (Tok1->isAnyIdentifier()) {
        const Optional<Token> &Tok2 = Lex.findNextToken(Tok1->getLocation(), SourceMgr);
        const Optional<Token> &Tok3 = Lex.findNextToken(Tok2->getLocation(), SourceMgr);
        if (Tok2->is(tok::colon) && Tok3->isAnyIdentifier()) {
            Tok1 = Tok3;
            return true;
        }
        return true;
    }

    return false;
}

/**
 * Check if Token is a Value
 * @return true on Success or false on Error
 */
bool Parser::isValue() {
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false, tok::kw_null, tok::l_brace,
                       tok::char_constant, tok::string_literal);
}

/**
 * Parse const scope of vars and class
 * @param Constant
 * @return true on Success or false on Error
 */
bool Parser::isConst() {
    if (Tok.is(tok::kw_const)) {
        ConsumeToken();
        return true;
    }
    return false;
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

bool Parser::isTokenOperator() const {
    return false;
}

bool Parser::isTokenAssignOperator() const {
    Optional<Token> None;
    return isTokenAssignOperator(None);
}

bool Parser::isTokenAssignOperator(Optional<Token> OptTok) const {
    const Token &CurTok = OptTok ? OptTok.getValue() : Tok;
    return CurTok.isOneOf(tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal,
                   tok::percentequal, tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal,
                   tok::greatergreaterequal);
}

bool Parser::isTokenAssign() const {
    return Tok.is(tok::equal);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool Parser::isUnaryPreOperator(Token &Tok) {
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
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool Parser::isBinaryOperator() {
    return Tok.isOneOf(
            // Arithmetci Operators
            tok::plus, // + add
            tok::minus, // - subtract
            tok::star, // * multiply
            tok::slash, // / divide
            tok::percent, // % percentage

            // Logic Operators
            tok::ampamp, // && logic and
            tok::pipepipe, // || logic or
            tok::less, // <
            tok::greater, // >
            tok::lessequal, // <= less than
            tok::greaterequal, // >= greater than
            tok::equalequal, // == equal compare
            tok::exclaimequal, // != different compare

            // Bit operators
            tok::amp, // & and
            tok::pipe, // | or
            tok::caret, // ^ xor
            tok::lessless, // << shift left
            tok::greatergreater // >> shift right
    );
}

/**
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool Parser::isTernaryOperator() {
    return Tok.is(tok::question);
}
