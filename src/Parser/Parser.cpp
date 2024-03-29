//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/FunctionParser.h"
#include "Parser/ExprParser.h"
#include "Parser/ClassParser.h"
#include "AST/ASTNode.h"
#include "AST/ASTClass.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTLocalVar.h"
#include "Sema/SemaBuilder.h"
#include "Frontend/InputFile.h"
#include "Basic/Debug.h"

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
ASTNode *Parser::Parse() {
    FLY_DEBUG("Parser", "Parse");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();
    bool Success = true;

    // Parse NameSpace on first
    if (ParseNameSpace()) {
        Node = Builder.CreateNode(Input.getFileName(), NameSpace);

        // Parse Imports
        if (Node && ParseImports()) {

            // Node empty
            if (Tok.is(tok::eof)) {
                Diag(diag::warn_empty_code);
            } else {
                // Parse top definitions until return true
                // if return false an error occurred or parsing is complete
                do {
                    Success = ParseTopDef();
                } while (Success && Tok.isNot(tok::eof));
            }
        }
    }

    return !Diags.hasErrorOccurred() && Success && Node && Builder.AddNode(Node) ? Node : nullptr;
}

ASTNode *Parser::ParseHeader() {
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
                return nullptr;
            }
        }
    }

    return Diags.hasErrorOccurred() ? nullptr : Node;
}

/**
 * Parse package declaration
 * @return true on Success or false on Error
 */
bool Parser::ParseNameSpace() {
    FLY_DEBUG("Parser", "ParseNameSpace");

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
    FLY_DEBUG("Parser", "ParseImports");

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

    ASTVisibilityKind Visibility = ASTVisibilityKind::V_DEFAULT;
    bool Constant = false;

    // Parse Public/Private and Constant
    ASTTopScopes *Scopes = ParseTopScopes();
    bool Success = !Diags.hasErrorOccurred();
    Optional<Token> OptTok = Tok;

    // Define a Class
    if (OptTok->isAnyIdentifier() && (OptTok = Lex.findNextToken(OptTok->getLocation(), SourceMgr)) &&
        OptTok->is(tok::l_brace)) {
        return Success & ParseClassDef(Scopes);
    }

    // Parse Type
    ASTType *Type = ParseType();
    Success = !Diags.hasErrorOccurred();
    if (Type) {

        // Define a Function
        if (Tok.isAnyIdentifier() &&
            Lex.findNextToken(Tok.getLocation(), SourceMgr)->is(tok::l_paren)) {
            return Success & ParseFunctionDef(Scopes, Type);
        }

        // Define a GlobalVar
        return Success & ParseGlobalVarDef(Scopes, Type);
    }

    // Unknown Top Definition
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
ASTTopScopes *Parser::ParseTopScopes() {
    FLY_DEBUG("Parser", "ParseTopScopes");

    bool isPrivate = false;
    bool isPublic = false;
    bool isConst = false;
    bool Found = false;
    do {
        if (Tok.is(tok::kw_private)) {
            if (isPrivate) {
                Diag(Tok, diag::err_scope_visibility_duplicate << (int) ASTVisibilityKind::V_PRIVATE);
            }
            if (isPublic) {
                Diag(Tok, diag::err_scope_visibility_conflict
                    << (int) ASTVisibilityKind::V_PRIVATE << (int) ASTVisibilityKind::V_PUBLIC);
            }
            isPrivate = true;
            Found = true;
            ConsumeToken();
        } else if (Tok.is(tok::kw_public)) {
            if (isPublic) {
                Diag(Tok, diag::err_scope_visibility_conflict
                    << (int) ASTVisibilityKind::V_PUBLIC << (int) ASTVisibilityKind::V_PUBLIC);
            }
            if (isPrivate) {
                Diag(Tok, diag::err_scope_visibility_duplicate
                    << (int) ASTVisibilityKind::V_PRIVATE);
            }
            isPublic = true;
            Found = true;
            ConsumeToken();
        } else if (Tok.is(tok::kw_const)) {
            if (isConst) {
                Diag(Tok, diag::err_scope_const_duplicate);
            }
            isConst = true;
            Found = true;
            ConsumeToken();
        } else {
            Found = false;
        }
    } while (Found);

    ASTVisibilityKind Visibility = isPrivate ? ASTVisibilityKind::V_PRIVATE :
                (isPublic ? ASTVisibilityKind::V_PUBLIC : ASTVisibilityKind::V_DEFAULT);
    return SemaBuilder::CreateTopScopes(Visibility, isConst);
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
bool Parser::ParseGlobalVarDef(ASTTopScopes *Scopes, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseGlobalVarDef", Logger()
                                                    .Attr("Scopes", Scopes)
                                                    .Attr("Type", Type).End());

    assert(Tok.isAnyIdentifier() && "Tok must be an Identifier");

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!BlockComment.empty()) {
        Comment = BlockComment;
        BlockComment = StringRef();
    }
    
    llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
    SourceLocation Loc = ConsumeToken();

    ASTGlobalVar *GlobalVar = Builder.CreateGlobalVar(Node, Loc, Type, Name, Scopes);

    // Parsing =
    ASTExpr *Expr = nullptr;
    if (isTokenAssignOperator()) {
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
bool Parser::ParseFunctionDef(ASTTopScopes *Scopes, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunctionDef",  Logger()
                                                    .Attr("Scopes", Scopes)
                                                    .Attr("Type", Type).End());

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!BlockComment.empty()) {
        Comment = BlockComment;
    }

    const StringRef &Name = Tok.getIdentifierInfo()->getName();
    ASTFunction *Function = Builder.CreateFunction(Node, ConsumeToken(), Type, Name, Scopes);
    if (FunctionParser::Parse(this, Function)) {

        // Error: body must be empty in header declaration
        if (Node->isHeader() && !Function->getBody()->isEmpty()) {
            // TODO
        }

        BlockComment = StringRef();
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
bool Parser::ParseClassDef(ASTTopScopes *Scopes) {
    FLY_DEBUG_MESSAGE("Parser", "ParseClassDef", Logger().Attr("Scopes", Scopes).End());

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!BlockComment.empty()) {
        Comment = BlockComment;
        BlockComment = StringRef();
    }

    ASTClass *Class = ClassParser::Parse(this, Scopes);
    if (Class) {
        return Builder.AddComment(Class, Comment) && Builder.AddClass(Node, Class);
    }

    return false;
}

/**
 * Parse statements between braces
 * @param Block
 * @return
 */
bool Parser::ParseBlock(ASTBlock *Block) {
    FLY_DEBUG_MESSAGE("Parser", "ParseBlock", Logger().Attr("Block", Block).End());
    ConsumeBrace();

    // Parse until find a }
    bool Success = true;
    while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {
        Success &= ParseStmt(Block);
    }

    if (isBlockEnd()) { // Close Brace
        ConsumeBrace();
        return Success;
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
    FLY_DEBUG_MESSAGE("Parser", "ParseStmt", Logger().Attr("Block", Block).End());

    // Parse keywords
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
    if (Tok.is(tok::kw_return)) { // Parse return
        SourceLocation Loc = ConsumeToken();
        ASTReturn *Return = Builder.CreateReturn(Block, Loc);
        ASTExpr *Expr = ParseExpr(Return);
        return Builder.AddStmt(Return);
    }
    if (Tok.is(tok::kw_break)) { // Parse break
        ASTBreak *Break = Builder.CreateBreak(Block, ConsumeToken());
        return Builder.AddStmt(Break);
    }
    if (Tok.is(tok::kw_continue)) { // Parse continue
        ASTContinue *Continue = Builder.CreateContinue(Block, ConsumeToken());
        return Builder.AddStmt(Continue);
    }

    // define a const LocalVar
    bool Const = isConst();

    // Used for know next tokens
    Optional<Token> OptTok1 = Tok; // Used for ASTLocalVar
    Optional<Token> OptTok2 = Tok; // Used for ASTVarAssign
    
    // Define an ASTLocalVar
    // int a
    ASTType *Type = nullptr;
    if (isBuiltinType(Tok)) {
        Type = ParseBuiltinType();
    }

    // Type a
    // Type a = ...
    // a = ...
    // a()
    // a++
    ASTIdentifier *Identifier = nullptr;
    if (Tok.isAnyIdentifier()) {

        if (Type) { // int a
            const StringRef &Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            ASTLocalVar *LocalVar = Builder.CreateLocalVar(Block, Tok.getLocation(), Type, Name, Const);

            // int a = ...
            if (Tok.is(tok::equal)) {
                ConsumeToken();
                ASTExpr *Expr = ParseExpr(LocalVar);
            }

            return Builder.AddStmt(LocalVar);
        }

        Identifier = ParseIdentifier(); // ASTClassType or ASTVarRef or ASTCall

        // Type a: Identifier is ASTType
        if (Tok.isAnyIdentifier()) {
            const StringRef &Name = Tok.getIdentifierInfo()->getName();
            ConsumeToken();

            // ClassType NameSpace must be populated always
            if (Identifier->getNameSpace().empty())
                Identifier->setNameSpace(NameSpace);

            Type = SemaBuilder::CreateClassType(Identifier);
            ASTLocalVar *LocalVar = Builder.CreateLocalVar(Block, Tok.getLocation(), Type, Name, Const);

            // Type a = ...
            if (Tok.is(tok::equal)) {
                ConsumeToken();
                ASTExpr *Expr = ParseExpr(LocalVar);
            }

            return Builder.AddStmt(LocalVar);
        }

        // a = ...
        if (isTokenAssignOperator()) {

            ASTVarRef *VarRef = Builder.CreateVarRef(Identifier);
            ASTVarAssign *VarAssign = Builder.CreateVarAssign(Block, VarRef);
            ExprParser Parser(this, VarAssign);
            ASTExpr *Expr = Parser.ParseAssignExpr();

            return Expr && Builder.AddStmt(VarAssign);
        }
    }

    // a()
    // a++
    ASTExprStmt *ExprStmt = Builder.CreateExprStmt(Block, Tok.getLocation());
    ASTExpr *Expr = ParseExpr(ExprStmt, Identifier);
    return Expr && Builder.AddStmt(ExprStmt);
}

/**
 * Parse open paren ( at start of cycle into condition statements
 * @return true on Success or false on Error
 */
bool Parser::ParseStartParen() {
    FLY_DEBUG("Parser", "ParseStartParen");

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
    FLY_DEBUG_MESSAGE("Parser", "ParseStartParen", Logger().Attr("HasParen", HasParen).End());

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

    FLY_DEBUG_MESSAGE("Parser", "ParseIfStmt", Logger().Attr("Block", Block).End());

    ConsumeToken();
    // Parse (
    bool hasIfParen = ParseStartParen();
    ASTIfBlock *IfBlock = Builder.CreateIfBlock(Block, Tok.getLocation());

    // Parse the group of expressions into parenthesis
    ASTExpr *IfExpr = ParseExpr(IfBlock); // Parse Expr
    if (hasIfParen) {
        ParseEndParen(hasIfParen);
    }
    // Parse statement between braces for If
    bool Success = false;
    if (isBlockStart()) {
        Success = ParseBlock(IfBlock);
    } else { // Only for a single Stmt without braces
        Success = ParseStmt(IfBlock);
    }

    // Add Elsif
    while (Success && Tok.is(tok::kw_elsif)) {
        const SourceLocation &ElsifLoc = ConsumeToken();
        ASTElsifBlock *ElsifBlock = Builder.CreateElsifBlock(IfBlock, ElsifLoc);

        // Parse (
        bool hasElsifParen = ParseStartParen();
        // Parse the group of expressions into parenthesis
        ASTExpr *ElsifExpr = ParseExpr(ElsifBlock); // Parse Expr
        if (hasElsifParen) {
            ParseEndParen(hasElsifParen);
        }

        if (isBlockStart()) {
            Success = ParseBlock(ElsifBlock);
        } else { // Only for a single Stmt without braces
            Success = ParseStmt(ElsifBlock);
        }
    }

    // Add Else
    if (Success && Tok.is(tok::kw_else)) {
        const SourceLocation &ElseLoc = ConsumeToken();
        ASTElseBlock *ElseBlock = Builder.CreateElseBlock(IfBlock, ElseLoc);

        if (isBlockStart()) {
            Success = ParseBlock(ElseBlock);
        } else { // Only for a single Stmt without braces
            Success = ParseStmt(ElseBlock);
        }
    }

    return Success & Builder.AddStmt(IfBlock);
}

/**
 * Parse the Switch statement
 *
 * switch var {
 *  case 1:
 *      ...
 *      break
 *  case 2 {
 *      ...
 *      ...
 *      }
 *      break
 *  case 3:
 *      break
 *  case 4
 *  default
 *      ...
 * }
 *
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseSwitchStmt(ASTBlock *Block) {
    assert(Tok.is(tok::kw_switch) && "Token is switch keyword");

    FLY_DEBUG_MESSAGE("Parser", "ParseSwitchStmt", Logger().Attr("Block", Block).End());

    // Parse switch keyword
    const SourceLocation &SwitchLoc = ConsumeToken();
    ASTSwitchBlock *SwitchBlock = Builder.CreateSwitchBlock(Block, SwitchLoc);

    // Parse (
    bool hasParen = ParseStartParen();

    // Parse Var reference like (a)
    ASTExpr *Expr = ParseExpr(SwitchBlock);
    if (Expr) {

        // Consume Right Parenthesis ) if exists
        if (!ParseEndParen(hasParen)) {
            return false;
        }

        // Init Switch Statement and start parse from brace
        bool RequireEndBrace = false;
        if (Tok.is(tok::l_brace)) {
            ConsumeBrace();
            RequireEndBrace = true;
        }

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
                ASTSwitchCaseBlock *CaseBlock = Builder.CreateSwitchCaseBlock(SwitchBlock, CaseLoc);
                ASTExpr * CaseExpr = ParseExpr(CaseBlock);

                if (Tok.is(tok::colon)) { // Parse a Block of Stmt
                    ConsumeToken();
                    Success = ParseStmt(CaseBlock);
                } else if (isBlockStart()) { // Parse a single Stmt without braces
                    ConsumeBrace();
                    Success = ParseBlock(CaseBlock);
                } else {
                    Diag(diag::err_syntax_error);
                    return false;
                }
            } else if (Tok.is(tok::kw_default)) {
                const SourceLocation &DefaultLoc = ConsumeToken();
                ASTSwitchDefaultBlock *DefaultBlock = Builder.CreateSwitchDefaultBlock(SwitchBlock, DefaultLoc);

                if (Tok.is(tok::colon)) { // Parse a Block of Stmt
                    ConsumeToken();
                    Success = ParseStmt(DefaultBlock);
                } else if (isBlockStart()) { // Parse a single Stmt without braces
                    ConsumeBrace();
                    Success = ParseBlock(DefaultBlock);
                } else {
                    Diag(diag::err_syntax_error);
                    return false;
                }
            }
        }

        // Switch statement is at end of it's time add current Switch to parent statement
        if (RequireEndBrace && Tok.is(tok::r_brace)) {
            ConsumeBrace();
        }
        return Success & Builder.AddStmt(SwitchBlock);
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
    FLY_DEBUG_MESSAGE("Parser", "ParseWhileStmt", Logger().Attr("Block", Block).End());

    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();
    ASTWhileBlock *WhileBlock = Builder.CreateWhileBlock(Block, Loc);

    // Create AST While Block
    ASTExpr *Cond = ParseExpr(WhileBlock);
    if (!Cond) { // Error: empty condition expr
        Diag(diag::err_parse_empty_while_expr);
        return false;
    }

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    // Parse statement between braces
    bool Success = isBlockStart() ?
            ParseBlock(WhileBlock) :
            Success = ParseStmt(WhileBlock); // Only for a single Stmt without braces

    return Success & Builder.AddStmt(WhileBlock);
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
    FLY_DEBUG_MESSAGE("Parser", "ParseForStmt", Logger().Attr("Block", Block).End());

    bool Success = true;
    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Init For Statement and start parse components
    ASTForBlock *ForBlock = Builder.CreateForBlock(Block, Loc);
    ASTForLoopBlock *LoopBlock;
    ASTForPostBlock *PostBlock;

    // parse comma separated stmt in init
    if (ParseForCommaStmt(ForBlock)) {
        assert(Tok.isNot(tok::comma) && "Comma not parsed"); // Already parsed from ParseForCommaStmt()

        // This is an Expression, it could be a Condition
        if (Tok.is(tok::semi)) {
            ConsumeToken();

            ParseExpr(ForBlock);

            if (Tok.is(tok::semi)) {
                PostBlock = Builder.CreateForPostBlock(ForBlock, ConsumeToken());
                Success &= ParseForCommaStmt(PostBlock);
            }
        }
    }

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    // Parse statement between braces
    LoopBlock = Builder.CreateForLoopBlock(ForBlock, Tok.getLocation());
    Success &= isBlockStart() ?
            ParseBlock(LoopBlock) :
            ParseStmt(LoopBlock); // Only for a single Stmt without braces
    return Success & Builder.AddStmt(ForBlock);
}

/**
 * Parse multi statements separated by comma
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseForCommaStmt(ASTBlock *Block) {
    FLY_DEBUG_MESSAGE("Parser", "ParseForCommaStmt", Logger().Attr("Block", Block).End());

    if (ParseStmt(Block)) {
        if (Tok.is(tok::comma)) {
            ConsumeToken();
            return ParseForCommaStmt(Block);
        }
        return true;
    }
    return false;
}

ASTType *Parser::ParseBuiltinType() {
    FLY_DEBUG("Parser", "ParseBuiltinType");

    switch (Tok.getKind()) {
        case tok::kw_bool:
            return SemaBuilder::CreateBoolType(ConsumeToken());
        case tok::kw_byte:
            return SemaBuilder::CreateByteType(ConsumeToken());
        case tok::kw_ushort:
            return SemaBuilder::CreateUShortType(ConsumeToken());
        case tok::kw_short:
            return SemaBuilder::CreateShortType(ConsumeToken());
        case tok::kw_uint:
            return SemaBuilder::CreateUIntType(ConsumeToken());
        case tok::kw_int:
            return SemaBuilder::CreateIntType(ConsumeToken());
        case tok::kw_ulong:
            return SemaBuilder::CreateULongType(ConsumeToken());
        case tok::kw_long:
            return SemaBuilder::CreateLongType(ConsumeToken());
        case tok::kw_float:
            return SemaBuilder::CreateFloatType(ConsumeToken());
        case tok::kw_double:
            return SemaBuilder::CreateDoubleType(ConsumeToken());
        case tok::kw_void:
            return SemaBuilder::CreateVoidType(ConsumeToken());
    }
    return nullptr;
}

ASTArrayType *Parser::ParseArrayType(ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseArrayType", Logger().Attr("Type", Type).End());

    ASTArrayType *ArrayType;
    do {
        const SourceLocation &Loc = ConsumeBracket();

        ASTExpr *Expr;

        if (Tok.is(tok::r_square)) {
            ConsumeBracket();
            Expr = Builder.CreateExpr(nullptr, SemaBuilder::CreateIntegerValue(Loc, 0));
            ArrayType = SemaBuilder::CreateArrayType(Loc, Type, Expr);
        } else {
            Expr = ParseExpr();
            if (Tok.is(tok::r_square)) {
                ConsumeBracket();
                ArrayType = SemaBuilder::CreateArrayType(Loc, Type, Expr);
            } else {
                Diag(Loc, diag::err_parser_unclosed_bracket);
                return nullptr;
            }
        }
    } while (Tok.is(tok::l_square));

    return ArrayType;
}

ASTClassType *Parser::ParseClassType(ASTClassType *Parent) {
    FLY_DEBUG("Parser", "ParseClassType");

    const StringRef &Name = Tok.getIdentifierInfo()->getName();
    const SourceLocation &Loc = Tok.getLocation();
    ConsumeToken();

    // Case 1: NS1:Type
    // Case 2: NS1:Type.SubType
    // Case n: NS1:Type.SubType.SubSubType
    if (!Parent && Tok.is(tok::colon)) {
        ConsumeToken();

        if (Tok.isAnyIdentifier()) {
            ASTClassType *ClassType = SemaBuilder::CreateClassType(Loc, Name, Tok.getIdentifierInfo()->getName(), Parent);
            ConsumeToken();
            
            if (Tok.is(tok::period)) {
                return ParseClassType(ClassType);
            }

            return ClassType;
        }

        Diag(Loc, diag::err_invalid_namespace_id);
        return nullptr;
    }

    // Case 1: Type
    // Case 2: Type.SubType
    // Case n: Type.SubType.SubSubType
    ASTClassType *ClassType = SemaBuilder::CreateClassType(Loc, NameSpace, Name);

    if (Tok.is(tok::period)) {
        return ParseClassType(ClassType);
    }

    return ClassType;
}


/**
 * Parse a data Type
 * @return true on Success or false on Error
 */
ASTType *Parser::ParseType() {
    FLY_DEBUG("Parser", "ParseType");

    ASTType *Type = nullptr;
    if (isBuiltinType(Tok)) {
        Type = ParseBuiltinType();
    } else if (isClassType(Tok)) {
        Type = ParseClassType();
    }

    if (Type && isArrayType(Tok)) {
        return ParseArrayType(Type);
    }

    return Type;
}

/**
 * Parse a Function Call
 * @param Block
 * @param Id
 * @param Loc
 * @return true on Success or false on Error
 */
ASTCall *Parser::ParseCall(ASTStmt *Stmt, ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("Parser", "ParseCall", Logger()
            .Attr("Loc", Identifier).End());
    assert(Tok.is(tok::l_paren) && "Call start with parenthesis");

    // Parse Call args
    if (Identifier->getNameSpace().empty()) // NameSpace must be always populated in ASTCall
        Identifier->setNameSpace(NameSpace);
    ASTCall *Call = Builder.CreateCall(Identifier);
    ConsumeParen(); // consume l_paren

    if (ParseCallArg(Stmt, Call)) {

        if (Tok.is(tok::r_paren)) {
            ConsumeParen();

            return Call; // end
        }
    }

    return nullptr;
}

/**
 * Parse a single Call Argument
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseCallArg(ASTStmt *Stmt, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Parser", "ParseCallArg",
                      Logger().Attr("Call", Call).End());

    // Parse Args in a Function Call
    if (Builder.AddCallArg(Call, ParseExpr(Stmt))) {

        if (Tok.is(tok::comma)) {
            ConsumeToken();

            return ParseCallArg(Stmt, Call);
        }

        return true;
    }

    Diag(Tok.getLocation(), diag::err_func_param);
    return false;
}

/**
 * Parse as Identifier
 * @return
 */
ASTIdentifier *Parser::ParseIdentifier() {
    FLY_DEBUG("Parser", "ParseIdentifier");

    ASTIdentifier *Identifier;
    const StringRef &N1 = Tok.getIdentifierInfo()->getName();
    const SourceLocation &Loc = Tok.getLocation();
    ConsumeToken();
    
    // Case 1: ns1:var -> VarRef
    // Case 2: ns1:func() -> FunctionCall
    // Case 3: ns1:obj.var -> ClassVar
    // Case 4: ns1:obj.func() -> ClassFunction
    // Case 5: ns1:obj.var.a -> ClassVar of ClassVar
    // Case 6: ns1:obj.func().func() -> ClassFunction of return ClassType object
    // Case n: ns1:obj.var.func().a.func() ... -> more and more dot with ClassFunction and ClassVar
    if (Tok.is(tok::colon)) {
        ConsumeToken();
        
        if (Tok.isAnyIdentifier()) {
            const StringRef &N2 = Tok.getIdentifierInfo()->getName();

            while (Tok.is(tok::period)) {
                ConsumeToken();

                if (Tok.isAnyIdentifier()) {
                    Identifier = new ASTIdentifier(Loc, N2, Tok.getIdentifierInfo()->getName());
                    Identifier->setNameSpace(N1);
                    ConsumeToken();
                }
                // FIXME nested var/call
            }

            Identifier = new ASTIdentifier(Loc, N1, Tok.getIdentifierInfo()->getName());
            ConsumeToken();
        }
        
        Diag(Loc, diag::err_invalid_namespace_id);
        return nullptr;
    } else {
        // case 1: obj.var
        // case 2: obj.func()
        // Case 3: var -> VarRef
        // Case 4: func -> FunctionCall
        if (Tok.is(tok::period)) {

            while (Tok.is(tok::period)) {
                ConsumeToken();

                if (Tok.isAnyIdentifier()) {
                    Identifier = new ASTIdentifier(Loc, N1, Tok.getIdentifierInfo()->getName());
                    Identifier->setNameSpace(NameSpace);
                    ConsumeToken();
                }
                // FIXME nested var/call
            }
        } else {
            Identifier = new ASTIdentifier(Loc,N1);
        }
    }

    return Identifier;
}

/**
 * Parse a Value Expression
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValue() {
    FLY_DEBUG("Parser", "ParseValue");

    if (Tok.is(tok::kw_null)) {
        const SourceLocation &Loc = ConsumeToken();
        return SemaBuilder::CreateNullValue(Loc);
    }

    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        std::string Val = std::string(Tok.getLiteralData(), Tok.getLength());
        return ParseValueNumber(Val);
    }

    if (Tok.isCharLiteral()) {
        // empty char is like a zero byte
        if (Tok.getLiteralData() == nullptr && Tok.getLength() == 0) {
            const SourceLocation &Loc = ConsumeToken();
            return SemaBuilder::CreateIntegerValue(Loc, 0);
        }

        StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        char Ch = *Val.begin();

        const SourceLocation &Loc = ConsumeToken();
        return SemaBuilder::CreateCharValue(Loc, Ch);
    }

    if (Tok.isStringLiteral()) {
        const char *Chars = Tok.getLiteralData();
        unsigned int StringLength = Tok.getLength();
        const SourceLocation &Loc = ConsumeStringToken();
        // TODO check Val type if need more than 1 byte of memory
        ASTArrayValue *String = SemaBuilder::CreateArrayValue(Loc);
        for (unsigned int i = 0; i < StringLength ; i++) {
            ASTIntegerValue *StringChar = Builder.CreateIntegerValue(Loc, Chars[i]);
            Builder.AddArrayValue(String, StringChar);
        }
        // set size to ASTArrayType on var declaration
        return String;
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        return SemaBuilder::CreateBoolValue(ConsumeToken(), true);
    }
    if (Tok.is(tok::kw_false)) {
        return SemaBuilder::CreateBoolValue(ConsumeToken(), false);
    }

    // Parse Array values
    if (Tok.is(tok::l_brace)) {
        const SourceLocation &Loc = ConsumeBrace();
        ASTArrayValue *ArrayValues = SemaBuilder::CreateArrayValue(Tok.getLocation());
        if (!ParseValues(*ArrayValues)) {
            return nullptr;
        }
        return ArrayValues;
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return nullptr;
}

ASTValue *Parser::ParseValueNumber(std::string &Str) {
    FLY_DEBUG_MESSAGE("Parser", "ParseValueNumber", Logger().Attr("Str", Str).End());

    const SourceLocation &Loc = ConsumeToken();

    // TODO Check Hex Value
    bool IsNegative = Str[0] == '-';
    uint64_t Integer = 0;
    uint64_t Fraction = 0;
    bool IsFloatingPoint = false;
    for (unsigned I = 0; I < Str.size(); I++) {
        if (Str[I] == '.') { // check if contain decimals
            Integer = 0;
            IsFloatingPoint = true;
            continue;
        }

        unsigned char N = Str[I] - '0';
        if (N > 9) {
            Diag(Loc, diag::err_parse_number) << Str;
            return nullptr;
        }
        // subtract '0' from ASCII value of a digit, to obtain integer value of the digit.
        if (IsFloatingPoint) {
            Fraction += Fraction; // need only for zero verify
        }
        else {
            Integer = Integer * 10 + N;
        }
    }

    if (IsFloatingPoint) {
        return SemaBuilder::CreateFloatingValue(Loc, Str);
    }
    return SemaBuilder::CreateIntegerValue(Loc, Integer, IsNegative);
}

/**
 * Parse Array Value Expression
 * @return the ASTValueExpr
 */
bool Parser::ParseValues(ASTArrayValue &ArrayValues) {
    FLY_DEBUG_MESSAGE("Parser", "ParseValues", Logger().Attr("ArrayValues", &ArrayValues).End());

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

ASTExpr *Parser::ParseExpr(ASTStmt *Stmt, ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("Parser", "ParseExpr", Logger().Attr("Stmt", Stmt).End());
    ExprParser Parser(this, Stmt);
    return Identifier ? Parser.ParseExpr(Identifier) : Parser.ParseExpr();
}

/**
 * Check if Token is one of the Builtin Types
 * @return true on Success or false on Error
 */
bool Parser::isBuiltinType(Token &Tok) {
    FLY_DEBUG("Parser", "isBuiltinType");

    return Tok.isOneOf(tok::kw_void,
                       tok::kw_bool,
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

bool Parser::isArrayType(Token &Tok) {
    FLY_DEBUG("Parser", "isArrayType");

    return Tok.is(tok::l_square);
}

bool Parser::isClassType(Token &Tok1) {
    FLY_DEBUG("Parser", "isClassType");

    if (Tok1.isAnyIdentifier()) {
        const Optional<Token> &Tok2 = Lex.findNextToken(Tok1.getLocation(), SourceMgr);
        if (Tok2->is(tok::colon)) {
            const Optional<Token> &Tok3 = Lex.findNextToken(Tok2->getLocation(), SourceMgr);
            if (Tok3->isAnyIdentifier()) {
                // Ex. NameSpace1:func()
                return true;
            }
            // TODO Error:invalid type
            // Ex. NameSpace1:()
            return false;
        }

        // Ex. ClassType1
        return true;
    }

    return false;
}

/**
 * Check if Token is a Value
 * @return true on Success or false on Error
 */
bool Parser::isValue() {
    FLY_DEBUG("Parser", "isValue");
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false, tok::kw_null, tok::l_brace,
                       tok::char_constant, tok::string_literal);
}

/**
 * Parse const scope of vars and class
 * @param Constant
 * @return true on Success or false on Error
 */
bool Parser::isConst() {
    FLY_DEBUG("Parser", "isConst");
    if (Tok.is(tok::kw_const)) {
        ConsumeToken();
        return true;
    }
    return false;
}

bool Parser::isBlockStart() {
    FLY_DEBUG("Parser", "isBlockStart");
    return Tok.is(tok::l_brace);
}

bool Parser::isBlockEnd() {
    FLY_DEBUG("Parser", "isBlockEnd");
    return Tok.is(tok::r_brace);
}

/**
 * ConsumeToken - Consume the current 'peek token' and lex the next one.
 * This does not work with special tokens: string literals,
 * annotation tokens and balanced tokens must be handled using the specific
 * consume methods.
 * @return the location of the consumed token
 */
SourceLocation Parser::ConsumeToken() {
    FLY_DEBUG("Parser", "ConsumeToken");
    assert(!isTokenSpecial() && "Should consume special tokens with Consume*Token");
    return ConsumeNext();
}

/**
 * isTokenParen - Return true if the cur token is '(' or ')'.
 * @return
 */
bool Parser::isTokenParen() const {
    FLY_DEBUG("Parser", "isTokenParen");
    return Tok.isOneOf(tok::l_paren, tok::r_paren);
}

/**
 * isTokenBracket - Return true if the cur token is '[' or ']'.
 * @return
 */
bool Parser::isTokenBracket() const {
    FLY_DEBUG("Parser", "isTokenBracket");
    return Tok.isOneOf(tok::l_square, tok::r_square);
}

/**
 * isTokenBrace - Return true if the cur token is '{' or '}'.
 * @return
 */
bool Parser::isTokenBrace() const {
    FLY_DEBUG("Parser", "isTokenBrace");
    return Tok.isOneOf(tok::l_brace, tok::r_brace);
}

/**
 * isTokenConsumeParenStringLiteral - True if this token is a string-literal.
 * @return
 */
bool Parser::isTokenStringLiteral() const {
    FLY_DEBUG("Parser", "isTokenStringLiteral");
    return tok::isStringLiteral(Tok.getKind());
}

/**
 * isTokenSpecial - True if this token requires special consumption methods.
 * @return
 */
bool Parser::isTokenSpecial() const {
    FLY_DEBUG("Parser", "isTokenSpecial");
    return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
           isTokenBrace();
}

/**
 * ConsumeParen - This consume method keeps the paren count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeParen() {
    FLY_DEBUG("Parser", "ConsumeParen");
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
    FLY_DEBUG("Parser", "ConsumeBracket");
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
    FLY_DEBUG("Parser", "ConsumeBrace");
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
    FLY_DEBUG("Parser", "isBraceBalanced");
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
    FLY_DEBUG("Parser", "ConsumeStringToken");
    assert(isTokenStringLiteral() && "Should only consume string literals with this method");
    return ConsumeNext();
}

SourceLocation Parser::ConsumeNext() {
    FLY_DEBUG("Parser", "ConsumeNext");
    Lex.Lex(Tok);
    while (Tok.is(tok::comment)) {
        BlockComment = Tok.getCommentData();
        Lex.Lex(Tok);
    }
    return Tok.getLocation();
}

/**
 * Get String between quotes from token
 * @return string
 */
llvm::StringRef Parser::getLiteralString() {
    FLY_DEBUG("Parser", "getLiteralString");
    StringRef Name(Tok.getLiteralData(), Tok.getLength());
    StringRef Str = "";
    if (Name.startswith("\"") && Name.endswith("\"")) {
        StringRef StrRefName = Name.substr(1, Name.size()-2);
        ConsumeStringToken();
        return StrRefName;
    }
    FLY_DEBUG_MESSAGE("Parser", "getLiteralString", "return " << Str);
    return Str;
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

bool Parser::isTokenAssignOperator() const {
    FLY_DEBUG("Parser", "isTokenAssignOperator");
    return Tok.isOneOf(tok::equal, tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal,
                   tok::percentequal, tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal,
                   tok::greatergreaterequal);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool Parser::isNewOperator(Token &Tok) {
    FLY_DEBUG("Parser", "isNewOperator");
    return Tok.is(tok::kw_new);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool Parser::isUnaryPreOperator(Token &Tok) {
    FLY_DEBUG("Parser", "isUnaryPreOperator");
    return Tok.isOneOf(tok::plusplus, tok::minusminus, tok::exclaim);
}

/**
 * Check if Token is one of the Unary Post Operators
 * @return true on Success or false on Error
 */
bool Parser::isUnaryPostOperator() {
    FLY_DEBUG("Parser", "isUnaryPostOperator");
    return Tok.isOneOf(tok::plusplus, tok::minusminus);
}

/**
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool Parser::isBinaryOperator() {
    FLY_DEBUG("Parser", "isBinaryOperator");
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
    FLY_DEBUG("Parser", "isTernaryOperator");
    return Tok.is(tok::question);
}
