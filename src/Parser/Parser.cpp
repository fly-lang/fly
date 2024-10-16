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
#include "Parser/EnumParser.h"
#include "AST/ASTModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTComment.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTScopes.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaBuilderScopes.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
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
 * Start Parsing Input and compose ASTModule
 * @param Module
 * @return true on Success or false on Error
 */
ASTModule *Parser::ParseModule() {
    FLY_DEBUG("Parser", "ParseModule");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    Module = Builder.CreateModule(Input.getFileName());

    // Start with Parse (recursively))
    Parse();

    return Module;
}

ASTModule *Parser::ParseHeader() {
    FLY_DEBUG("Parser", "ParseHeader");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse NameSpace on first
//    if (ParseNameSpace()) {
//        Module = Builder.CreateHeaderModule(Input.getFileName());
//
//        // Parse Top declarations
//        if (Tok.is(tok::eof)) {
//            // Node empty
//            Diag(diag::warn_empty_code);
//        } else {
//            // Parse top definitions
//            do {
//                ParseTopDef();
//            } while (ContinueParsing && Tok.isNot(tok::eof));
//        }
//    }

    return Module;
}

bool Parser::isSuccess() {
    return !Diags.hasErrorOccurred();
}

void Parser:: Parse() {
    if (Tok.is(tok::eof)) {
        return;
    }

    // Parse namespace, imports and top definitions
    if (Tok.is(tok::kw_namespace)) {
        ConsumeToken();

        if (!Tok.isAnyIdentifier()) {
            Diag(Tok, diag::err_parse_identifier_expected);
        } else {

            // Parse NameSpace identifier
            ASTIdentifier *Identifier = ParseIdentifier();

            // Create NameSpace
            Builder.CreateNameSpace(Identifier, Module);
        }
    } else if (Tok.is(tok::kw_import)) {
        ConsumeToken();
        
        if (!Tok.isAnyIdentifier()) {
            Diag(Tok, diag::err_parse_identifier_expected);
        } else {

            // Parse Import identifier
            ASTIdentifier *IdentifierImport = ParseIdentifier();

            ASTAlias *Alias = nullptr;
            if (Tok.is(tok::kw_as)) {
                ConsumeToken();

                if (!Tok.isAnyIdentifier()) {
                    Diag(Tok, diag::err_parse_identifier_expected);
                } else {
                    // Parse Alias identifier
                    ASTIdentifier *IdentifierAlias = ParseIdentifier();

                    // Create Alias
                    Alias = Builder.CreateAlias(IdentifierAlias->getLocation(), IdentifierAlias->getName());
                }
            }

            // Create Import
            Builder.CreateImport(Module, IdentifierImport->getLocation(), IdentifierImport->getName(), Alias);
        }
    } else if (Tok.isOneOf(tok::kw_private,tok::kw_protected, tok::kw_public, tok::kw_const, tok::kw_static)) {

        // Parse Comments
        ASTComment *Comment = Lex.BlockComment.empty() ? nullptr : Builder.CreateComment(SourceLocation(), Lex.BlockComment);

        // Parse Public/Private and Constant
        SmallVector<ASTScope *, 8> Scopes = ParseScopes();

        // Parse TopDef: GlobalVar, Function, Class, Enum
        ParseTopDef(Comment, Scopes);
    } else {

        // Parse Comments
        ASTComment *Comment = Lex.BlockComment.empty() ? nullptr : Builder.CreateComment(SourceLocation(), Lex.BlockComment);

        SmallVector<ASTScope *, 8> Scopes;

        // Parse TopDef: GlobalVar, Function, Class, Enum
        ParseTopDef(Comment, Scopes);
    }

    Parse();
}

ASTBase *Parser::ParseTopDef(ASTComment *Comment, SmallVector<ASTScope *, 8>& Scopes) {
    assert(!isTokenComment() && "Token comment not expected");

    // Define a Class
    if (Tok.isOneOf(tok::kw_struct, tok::kw_class, tok::kw_interface)) {
        return ParseClassDef(Comment, Scopes);
    }

    if (Tok.is(tok::kw_enum)) {
        return ParseEnumDef(Comment, Scopes);
    }

    // Parse Type
    ASTType *Type = ParseType();
    if (Type == nullptr) {
        Diag(Tok.getLocation(), diag::err_parser_invalid_type);
    } else {
        if (Tok.isAnyIdentifier()) {

            // Define a Function
            if (Lexer::findNextToken(Tok.getLocation(), SourceMgr)->is(tok::l_paren)) {
                return ParseFunctionDef(Comment, Scopes, Type);
            } else {
                // Define a GlobalVar
                return ParseGlobalVarDef(Comment, Scopes, Type);
            }
        } else {
            Diag(Tok, diag::err_parse_identifier_invalid);
        }
    }

    return nullptr;
}

SmallVector<ASTScope *, 8> Parser::ParseScopes() {
    FLY_DEBUG("ClassParser", "ParseScopes");

    SemaBuilderScopes *BuilderScopes = SemaBuilderScopes::Create();

    while (Tok.isNot(tok::eof)) {
        ASTScope *Scope;
        if (Tok.is(tok::kw_private)) {
            BuilderScopes->addVisibility(ConsumeToken(), ASTVisibilityKind::V_PRIVATE);
        } else if (Tok.is(tok::kw_protected)) {
            BuilderScopes->addVisibility(ConsumeToken(), ASTVisibilityKind::V_PROTECTED);
        } else if (Tok.is(tok::kw_public)) {
            BuilderScopes->addVisibility(ConsumeToken(), ASTVisibilityKind::V_PUBLIC);
        } else if (Tok.is(tok::kw_const)) {
            BuilderScopes->addConstant(ConsumeToken(), true);
        } else if (Tok.is(tok::kw_static)) {
            BuilderScopes->addStatic(ConsumeToken(), true);
        } else {
            break;
        }
    }

    return BuilderScopes->getScopes();
}

/**
 * ParseModule Global Var declaration
 * @param Visibility
 * @param Constant
 * @param Type
 * @param Name
 * @param NameLoc
 * @return
 */
ASTGlobalVar *Parser::ParseGlobalVarDef(ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseGlobalVarDef", Logger()
                                                    .AttrList("Scopes", Scopes)
                                                    .Attr("Type", Type).End());
    assert(Tok.isAnyIdentifier() && "Tok must be an Identifier");

    llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
    SourceLocation Loc = ConsumeToken();

    // Parsing =
    ASTExpr *Expr = nullptr;
    if (isTokenAssignOperator(Tok)) {
        ConsumeToken();
        
        // Parse Expr
        Expr = ParseExpr();
    }

    // GlobalVar
    ASTGlobalVar *GlobalVar = Builder.CreateGlobalVar(Module, Loc, Type, Name, Scopes, Expr, Comment);

    return GlobalVar;
}


/**
 * ParseModule Function declaration
 * @param Visibility
 * @param Constant
 * @param Type
 * @param Name
 * @param NameLoc
 * @return
 */
ASTFunction *Parser::ParseFunctionDef(ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes, ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseFunctionDef",  Logger()
                                                    .AttrList("Scopes", Scopes)
                                                    .Attr("Type", Type).End());
    assert(Tok.isAnyIdentifier() && "Tok must be an Identifier");
    StringRef Name = Tok.getIdentifierInfo()->getName();
    const SourceLocation &Loc = ConsumeToken();

    llvm::SmallVector<ASTParam *, 8> Params = FunctionParser::ParseParams(this);
    ASTBlockStmt *Body = isBlockStart() ? FunctionParser::ParseBody(this) : nullptr;
    ASTFunction *Function = Builder.CreateFunction(Module, Loc, Type, Name, Scopes, Params, Body, Comment);
    
    return Function;
}

/**
 * ParseModule Class declaration
 * @param Visibility
 * @param Constant
 * @return
 */
ASTClass * Parser::ParseClassDef(ASTComment *Comment, SmallVector<ASTScope *, 8> &Scopes) {
    FLY_DEBUG_MESSAGE("Parser", "ParseClassDef", Logger().AttrList("Scopes", Scopes).End());

    ASTClass *Class = ClassParser::Parse(this, Comment, Scopes);
    return Class;
}


/**
 * ParseModule Enum declaration
 * @param Visibility
 * @param Constant
 * @return
 */
ASTEnum *Parser::ParseEnumDef(ASTComment *Comment, SmallVector<ASTScope *, 8>&Scopes) {
    FLY_DEBUG_MESSAGE("Parser", "ParseClassDef", Logger().AttrList("Scopes", Scopes).End());
    assert(Tok.is(tok::kw_enum) && "Token Enum expected");

    ASTEnum *Enum = EnumParser::Parse(this, Comment, Scopes);
    return Enum;
}

ASTComment *Parser::ParseComments() {
    FLY_DEBUG("Parser", "ParseComments");
    assert(!Lex.BlockComment.empty() && "Block Comment must be not empty");

    // Parse all as string
    ASTComment *Comment = nullptr;
    if (!Lex.BlockComment.empty()) {
        Comment = Builder.CreateComment(SourceLocation(), Lex.BlockComment);
    }
    return Comment;
}

void Parser::SkipComments() {
    if (isTokenComment()) {
        ConsumeToken();
        SkipComments();
    }
}

/**
 * ParseModule statements between braces
 * @param Stmt
 * @return
 */
bool Parser::ParseBlock(ASTBlockStmt *Block, bool isBody) {
    FLY_DEBUG_MESSAGE("Parser", "ParseStmt", Logger().Attr("Block", Block).End());
    assert(isBlockStart() && "Block Start");

    bool Success = false;
    const SourceLocation &BraceTok = ConsumeBrace(BracketCount);
    if (isBlockEnd()) {

        // Clear Comments
        if (isBody)
            Lex.ClearBlockComment();

        ConsumeBrace(BracketCount);
    } else {
        Success = ParseStmt(Block);
    }

    if (!isBraceBalanced()) {
        // Fixme Error parse brace not balanced
        Diag(BraceTok, diag::err_parser_generic);
        Success = false;
    }

    return Success;
}

/**
 * ParseModule a single statement like Variable declaration, assignment or Function invocation
 *
 * cont int a
 * a = ...
 * a()
 * a v1 = ...
 * int a = ...
 *
 * @param Parent
 * @return true on Success or false on Error
 */
bool Parser::ParseStmt(ASTBlockStmt *Parent) {
    FLY_DEBUG_MESSAGE("Parser", "ParseStmt", Logger().Attr("Block", Parent).End());

    // Parse if stmt
    if (Tok.is(tok::kw_if)) {
        return ParseIfStmt(Parent);
    }

    // Parse switch stmt
    if (Tok.is(tok::kw_switch)) {
        return ParseSwitchStmt(Parent);
    }

    // Parse for stmt
    if (Tok.is(tok::kw_for)) {
        return ParseForStmt(Parent);
    }

    // Parse while stmt
    if (Tok.is(tok::kw_while)) {
        return ParseWhileStmt(Parent);
    }

    // Parse handle stmt
    if (Tok.is(tok::kw_handle)) {
        return ParseHandleStmt(Parent, nullptr);
    }

    // Parse fail stmt
    if (Tok.is(tok::kw_fail)) {
        return ParseFailStmt(Parent);
    }

    // Parse return stmt
    if (Tok.is(tok::kw_return)) { // Parse return
        SourceLocation Loc = ConsumeToken();
        SemaBuilderStmt *BuilderStmt = Builder.CreateReturnStmt(Parent, Loc);
        ASTExpr *Expr = ParseExpr();
        BuilderStmt->setExpr(Expr);
        return ParseStmt(Parent);
    }

    // Parse break stmt
    if (Tok.is(tok::kw_break)) { // Parse break
        ASTBreakStmt *Break = Builder.CreateBreakStmt(Parent, ConsumeToken());
        return ParseStmt(Parent);
    }

    // Parse continue stmt
    if (Tok.is(tok::kw_continue)) { // Parse continue
        ASTContinueStmt *Continue = Builder.CreateContinueStmt(Parent, ConsumeToken());
        return ParseStmt(Parent);
    }

    // Parse scopes
    SmallVector<ASTScope *, 8> Scopes = ParseScopes();

    // Find next tokens
    const Optional<Token> &Tok1 = Lexer::findNextToken(Tok.getLocation(), SourceMgr);
    const Optional<Token> &Tok2 = Lexer::findNextToken(Tok1->getLocation(), SourceMgr);

    // Parse LocalVar
    if (Tok1->isAnyIdentifier() && Tok2->isAnyIdentifier() || Tok1->isKeyword() && Tok2->isAnyIdentifier()) {
        // const int a
        // Type a
        // int a = ...
        // Type a = ...
        ASTType *Type = ParseType();
        if (Type == nullptr) {
            Diag(Tok.getLocation(), diag::err_parser_invalid_type);
        } else {
            ASTIdentifier *Identifier = ParseIdentifier();
            ASTLocalVar *LocalVar = Builder.CreateLocalVar(Parent, Tok.getLocation(), Type,
                                                           Identifier->getName(), Scopes);

            // Assign to LocalVar
            if (isTokenAssignOperator(Tok)) {
                ConsumeToken();

                if (Tok.is(tok::kw_handle)) {
                    ASTVarRef *ErrorVarRef = Builder.CreateVarRef(LocalVar);
                    ParseHandleStmt(Parent, ErrorVarRef);
                } else {
                    SemaBuilderStmt *BuilderStmt = Builder.CreateVarStmt(Parent, LocalVar);

                    // Parse Expr
                    ASTExpr *Expr = ParseExpr();
                    BuilderStmt->setExpr(Expr);
                }

                return ParseStmt(Parent);
            }
        }
    } else if (Tok1->isAnyIdentifier() && isTokenAssignOperator(Tok2.getValue())) {
        // Parse Var
        // a = ...
        ASTIdentifier *Identifier = ParseIdentifier();
        ASTVarRef *VarRef = Builder.CreateVarRef(Identifier);
        SemaBuilderStmt *BuilderStmt = Builder.CreateVarStmt(Parent, VarRef);
        
        // Consume assign operator
        ConsumeToken();

        // Parse Expr
        ASTExpr *Expr = ParseExpr();
        BuilderStmt->setExpr(Expr);
        return ParseStmt(Parent);
    } else {
        // a()
        // a++
        // ++a
        // new A()
        SemaBuilderStmt *BuilderStmt = Builder.CreateExprStmt(Parent, Tok.getLocation());
        ASTExpr *Expr = ParseExpr();
        BuilderStmt->setExpr(Expr);
    }

    // End of Stmt
    return true;
}

/**
 * ParseModule open paren ( at start of cycle into condition statements
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
 * ParseModule close paren ) at end of cycle into condition statements
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
 * ParseModule If, Elseif and Else statements
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
 * @param Parent
 * @return true on Success or false on Error
 */
bool Parser::ParseIfStmt(ASTBlockStmt *Parent) {
    assert(Tok.is(tok::kw_if) && "Token is if keyword");

    FLY_DEBUG_MESSAGE("Parser", "ParseIfStmt", Logger().Attr("Block", Parent).End());

    ConsumeToken();
    // Parse (
    bool hasIfParen = ParseStartParen();

    // Parse the group of expressions into parenthesis
    ASTExpr *IfCondition = ParseExpr(); // Parse Expr
    if (hasIfParen) {
        ParseEndParen(hasIfParen);
    }
    // Create If
    SemaBuilderIfStmt *IfBuilder = Builder.CreateIfBuilder(Parent);
    ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(Tok.getLocation());
    IfBuilder->If(Tok.getLocation(), IfCondition, IfBlock);

    // Parse statement between braces for If
    bool Success = isBlockStart() ? ParseBlock(IfBlock) : ParseStmt(IfBlock);

    // Add Elsif
    while (Success && Tok.is(tok::kw_elsif)) {
        const SourceLocation &ElsifLoc = ConsumeToken();

        // Parse (
        bool hasElsifParen = ParseStartParen();
        // Parse the group of expressions into parenthesis
        ASTExpr *ElsifCondition = ParseExpr(); // Parse Expr
        if (hasElsifParen) {
            ParseEndParen(hasElsifParen);
        }

        ASTBlockStmt *ElsifBlock = Builder.CreateBlockStmt(Tok.getLocation());
        if (isBlockStart()) {
            Success = ParseStmt(ElsifBlock);
        } else { // Only for a single Stmt without braces
            Success = ParseStmt(ElsifBlock);
        }

        IfBuilder->ElseIf(ElsifLoc, ElsifCondition, ElsifBlock);
    }

    // Add Else
    if (Success && Tok.is(tok::kw_else)) {
        const SourceLocation &ElseLoc = ConsumeToken();
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt(ElseLoc);

        if (isBlockStart()) {
            Success = ParseStmt(ElseBlock);
        } else { // Only for a single Stmt without braces
            Success = ParseStmt(ElseBlock);
        }
    }

    return Success;
}

/**
 * ParseModule the Switch statement
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
 * @param Parent
 * @return true on Success or false on Error
 */
bool Parser::ParseSwitchStmt(ASTBlockStmt *Parent) {
    assert(Tok.is(tok::kw_switch) && "Token is switch keyword");
    FLY_DEBUG_MESSAGE("Parser", "ParseSwitchStmt", Logger().Attr("Block", Parent).End());

    SemaBuilderSwitchStmt *SwitchBuilder = Builder.CreateSwitchBuilder(Parent);
    
    // Parse (
    bool hasParen = ParseStartParen();
    const SourceLocation &Loc = ConsumeToken();
    
    // Parse switch keyword
    // Parse Var reference like (a)
    ASTExpr *Expr = ParseExpr();
    if (Expr) {
        SwitchBuilder->Switch(Loc, Expr);

        // Consume Right Parenthesis ) if exists
        if (!ParseEndParen(hasParen)) {
            return false;
        }

        // Init Switch Statement and start parse from brace
        if (isBlockStart()) {
            ConsumeBrace(BracketCount);

            if (ParseSwitchCases(SwitchBuilder)) {

                // Switch statement is at end of it's time add current Switch to parent statement
                if (isBlockEnd()) {
                    ConsumeBrace(BracketCount);
                    return true;
                }
            }
        }
    }

    Diag(diag::err_syntax_error);
    return false;
}

bool Parser::ParseSwitchCases(SemaBuilderSwitchStmt *SwitchBuilder) {

    if (Tok.is(tok::kw_case)) {
        const SourceLocation &Loc = ConsumeToken();

        // Parse Expression for different cases
        // for a Value  -> case 1:
        // for a Var -> case a:
        // for a default
        ASTBlockStmt *CaseBlock = Builder.CreateBlockStmt(Loc);
        ASTExpr *Expr = ParseExpr();

        if (Expr->getExprKind() != ASTExprKind::EXPR_VALUE) {
            // Error: unexpected value for case selection
            Diag(diag::err_syntax_error);
        }

        ASTValueExpr *ValueExpr = (ASTValueExpr *) Expr;
        if (ValueExpr->getValue()->getTypeKind() == ASTTypeKind::TYPE_INTEGER) {
            if (Tok.is(tok::colon)) { // Parse a Block of Stmt
                ConsumeToken();
                if (ParseStmt(CaseBlock)) {

                    if (isBlockStart()) { // Parse a single Stmt without braces
                        ConsumeBrace(BracketCount);
                    }
                    SwitchBuilder->Case(Loc, ValueExpr, CaseBlock);
                    return ParseSwitchCases(SwitchBuilder);
                }
            }
        }
    } else if (Tok.is(tok::kw_default)) {
        const SourceLocation &Loc = ConsumeToken();
        ASTBlockStmt *DefaultBlock = Builder.CreateBlockStmt(ConsumeToken());
        if (Tok.is(tok::colon)) { // Parse a Block of Stmt
            ConsumeToken();

            if (isBlockStart()) { // Parse a single Stmt without braces
                ConsumeBrace(BracketCount);
            }
            if (ParseStmt(DefaultBlock)) {
                SwitchBuilder->Default(Loc, DefaultBlock);
                return ParseSwitchCases(SwitchBuilder);
            }
        }
    } else {
        return true;
    }

    Diag(diag::err_syntax_error);
    return false;
}

/**
 * ParseModule the While statement
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
 * @param Parent
 * @return true on Success or false on Error
 */
bool Parser::ParseWhileStmt(ASTBlockStmt *Parent) {
    assert(Tok.is(tok::kw_while) && "Token is while keyword");
    FLY_DEBUG_MESSAGE("Parser", "ParseWhileStmt", Logger().Attr("Block", Parent).End());

    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Create AST While Block
    ASTExpr *Condition = ParseExpr();
    if (!Condition) { // Error: empty condition expr
        Diag(diag::err_parse_empty_while_expr);
        return false;
    }

    ASTBlockStmt *BlockStmt = Builder.CreateBlockStmt(Tok.getLocation());
    SemaBuilderLoopStmt *LoopBuilder = Builder.CreateLoopBuilder(Parent, Loc);
    LoopBuilder->Loop(Condition, BlockStmt);


    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    // Parse statement between braces
    bool Success = isBlockStart() ?
                   ParseBlock(BlockStmt) :
                   ParseStmt(BlockStmt); // Only for a single Stmt without braces


    return Success;
}

/**
 * ParseModule the For statement
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
 * @param Parent
 * @return true on Success or false on Error
 */
bool Parser::ParseForStmt(ASTBlockStmt *Parent) {
    assert(Tok.is(tok::kw_for) && "Token is for keyword");
    FLY_DEBUG_MESSAGE("Parser", "ParseForStmt", Logger().Attr("Block", Parent).End());

    bool Success = true;
    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Init For Statement and start parse components
    ASTBlockStmt *InitBlock = Builder.CreateBlockStmt(ConsumeToken());
    ASTExpr *Condition = nullptr;
    ASTBlockStmt *PostBlock = Builder.CreateBlockStmt(ConsumeToken());

    // for int a = 1; i < 0; i++

    // parse comma separated stmt in init
    ParseStmt(InitBlock);

    // This is an Expression, it could be a Condition
    if (Tok.is(tok::semi)) {
            ConsumeToken();

            Condition = ParseExpr();

            if (Tok.is(tok::semi)) {
                ParseStmt(PostBlock);
            }
    }

    // Consume Right Parenthesis ) if exists
    if (!ParseEndParen(hasParen)) {
        return false;
    }

    ASTBlockStmt *LoopBlock = Builder.CreateBlockStmt(Tok.getLocation());
    SemaBuilderLoopStmt *LoopBuilder = Builder.CreateLoopBuilder(Parent, Loc);
    LoopBuilder->Loop(Condition, LoopBlock);
    LoopBuilder->Init(InitBlock);
    LoopBuilder->Post(PostBlock);

    // Parse statement between braces
    Success &= isBlockStart() ?
               ParseBlock(LoopBlock) :
               ParseStmt(LoopBlock); // Only for a single Stmt without braces
    return Success;
}

bool Parser::ParseHandleStmt(ASTBlockStmt *Parent, ASTVarRef *Error) {
    assert(Tok.is(tok::kw_handle) && "Token is handle keyword");
    FLY_DEBUG_MESSAGE("Parser", "ParseHandleStmt", Logger().Attr("Error", Error).End());

    // Parse handle block
    const SourceLocation &Loc = ConsumeToken();
    Builder.CreateHandleStmt(Parent, Loc, Error);

    // Parse statement between braces
    return isBlockStart() ?
                   ParseBlock(Parent) :
                   ParseStmt(Parent); // Only for a single Stmt without braces
}

bool Parser::ParseFailStmt(ASTBlockStmt *Parent) {
    FLY_DEBUG("Parser", "ParseFailStmt");
    assert(Tok.is(tok::kw_fail) && "Token is handle keyword");

    const SourceLocation &Loc = ConsumeToken();
    ASTParam *Errorhandler = Parent->getFunction()->getErrorHandler();
    SemaBuilderStmt *BuilderStmt = Builder.CreateFailStmt(Parent, Loc);
    ASTExpr *Expr = ParseExpr();
    BuilderStmt->setExpr(Expr);
    return true;
}

ASTType *Parser::ParseBuiltinType() {
    FLY_DEBUG("Parser", "ParseBuiltinType");
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
        case tok::kw_string:
            Type = Builder.CreateStringType(ConsumeToken());
            break;
        case tok::kw_error:
            Type = Builder.CreateErrorType(ConsumeToken());
            break;
        default:
            Diag(Tok.getLocation(), diag::err_parser_invalid_type);
            Type = nullptr;
    }
    return isArrayType(Tok) ? ParseArrayType(Type) : Type;
}

ASTArrayType *Parser::ParseArrayType(ASTType *Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseArrayType", Logger().Attr("Type", Type).End());
    assert(isArrayType(Tok) && "Invalid array parse");

    ASTArrayType *ArrayType = nullptr;
    do {
        const SourceLocation &Loc = ConsumeBracket();
        ASTExpr *Expr;

        if (Tok.is(tok::r_square)) {
            ConsumeBracket();
            Expr = Builder.CreateExpr(Builder.CreateIntegerValue(Loc, 0));
            Type = Builder.CreateArrayType(Loc, Type, Expr);
        } else {
            Expr = ParseExpr();
            if (Tok.is(tok::r_square)) {
                ConsumeBracket();
                Type = Builder.CreateArrayType(Loc, Type, Expr);
            } else {
                Diag(Loc, diag::err_parser_unclosed_bracket);
            }
        }
    } while (Tok.is(tok::l_square));

    return ArrayType;
}

/**
 * ParseModule a data Type
 * @return true on Success or false on Error
 */
ASTType *Parser::ParseType() {
    FLY_DEBUG("Parser", "ParseType");

    ASTType *Type = nullptr;
    if (Tok.isAnyIdentifier()) {
        ASTIdentifier *Identifier = ParseIdentifier();
        if (!Identifier->isCall()) {
            Type = Builder.CreateClassType(Identifier);
            if (Type) {
                if (isArrayType(Tok)) {
                    Type = ParseArrayType(Type);
                }
            }
        }
    } else {
        Type = ParseBuiltinType();
    }

    return Type;
}

/**
 * ParseModule a Function Call
 * @param Block
 * @param Id
 * @param Loc
 * @return true on Success or false on Error
 */
ASTCall *Parser::ParseCall(ASTIdentifier *&Identifier) {
    FLY_DEBUG_MESSAGE("Parser", "ParseCall", Logger()
            .Attr("Loc", Identifier).End());
    assert(Tok.is(tok::l_paren) && "Call start with parenthesis");

    // Parse Call args
    ConsumeParen(); // consume l_paren
    ASTCall *Call = nullptr;

    // Parse Args in a Function Call
    llvm::SmallVector<ASTExpr *, 8> Args;
    ASTExpr *Arg;
    while ((Arg = ParseExpr())) {
        Args.push_back(Arg);
        if (Tok.isNot(tok::comma)) {
            break;
        }
    }
    if (Tok.is(tok::r_paren)) {
        ConsumeParen();
        Call = Builder.CreateCall(Identifier, Args, ASTCallKind::CALL_FUNCTION);
    }

    return Call;
}

/**
 * ParseModule as Identifier
 * @return
 */
ASTIdentifier *Parser::ParseIdentifier(ASTIdentifier *Parent) {
    FLY_DEBUG("Parser", "ParseIdentifier");
    assert(Tok.isAnyIdentifier() && "Token Identifier expected");

    // Create the Child and work on it
    ASTIdentifier *Child;
    llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
    if (Parent == nullptr) {
        Child = Builder.CreateIdentifier(ConsumeToken(), Name);
    } else {
        Parent->AddChild(Builder.CreateIdentifier(ConsumeToken(), Name));
        Child = Parent->getChild();
    }

    // case 0: Class
    // case 1: var -> VarRef
    // case 2: func() -> FunctionCall
    // Case 3: ns1.var -> VarRef
    // Case 4: ns1.func() -> FunctionCall
    // Case 5: ns1.Enum.CONST -> ClassVar
    // Case 6: ns1.Class.method() -> ClassFunction
    // Case 7: ns1.Class.var -> Method return Object
    if (Tok.is(tok::period)) {
        ConsumeToken();

        if (Tok.isAnyIdentifier()) {
            return ParseIdentifier(Child);
        } else {
            Diag(Tok.getLocation(), diag::err_invalid_id) << Tok.getKind();
            return Child;
        }
    } else if (Tok.is(tok::l_paren)) {

        if (ParseCall(Child) && Tok.is(tok::period)) {
            ConsumeToken();

            return ParseIdentifier(Child);
        }
    }

    return Child;
}

/**
 * ParseModule a Value Expression
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
        return ParseValueNumber(Val);
    }

    if (Tok.isCharLiteral()) {
        // empty char is like a zero byte
        if (Tok.getLiteralData() == nullptr && Tok.getLength() == 0) {
            const SourceLocation &Loc = ConsumeToken();
            return Builder.CreateIntegerValue(Loc, 0);
        }

        StringRef Val = StringRef(Tok.getLiteralData(), Tok.getLength());
        char Ch = *Val.begin();

        const SourceLocation &Loc = ConsumeToken();
        return Builder.CreateCharValue(Loc, Ch);
    }

    if (Tok.isStringLiteral()) {
        const char *Data = Tok.getLiteralData();
        size_t Length = Tok.getLength();
        return Builder.CreateStringValue(ConsumeStringToken(), StringRef(Data, Length));
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        return Builder.CreateBoolValue(ConsumeToken(), true);
    }
    if (Tok.is(tok::kw_false)) {
        return Builder.CreateBoolValue(ConsumeToken(), false);
    }

    // Parse Array or Struct
    if (Tok.is(tok::l_brace)) {
        return ParseValues();
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
        return Builder.CreateFloatingValue(Loc, Str);
    }
    return Builder.CreateIntegerValue(Loc, Integer, IsNegative);
}

/**
 * ParseModule Array Value Expression
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValues() {
    FLY_DEBUG("Parser", "ParseValues");
    const SourceLocation &StartLoc = ConsumeBrace(BracketCount);
    
    // Set Values Struct and Array for next
    bool isStruct = false;
    llvm::StringMap<ASTValue *> StructValues;
    llvm::SmallVector<ASTValue *, 8> ArrayValues;

    // Parse array values Ex. {1, 2, 3}
    while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {

        // if is Identifier -> struct
        if (Tok.isAnyIdentifier()) {
            isStruct = true;
            const StringRef &Key = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            
            if (Tok.is(tok::equal)) {
                // FIXME
                ConsumeToken();

                ASTValue *Value = ParseValue();
                if (Value) {
                    StructValues.insert(std::make_pair(Key, Value));
                    if (Tok.is(tok::comma)) {
                        ConsumeToken();
                    } else {
                        break;
                    }
                } else {
                    Diag(diag::err_invalid_value) << Tok.getName();
                }
            }
        } else { // if is Value -> array
            ASTValue *Value = ParseValue();
            if (Value) {
                ArrayValues.push_back(Value);
                if (Tok.is(tok::comma)) {
                    ConsumeToken();
                } else {
                    break;
                }
            } else {
                Diag(diag::err_invalid_value) << Tok.getName();
            }
        }

    }

    // End of Array
    if (Tok.is(tok::r_brace)) {
        ConsumeBrace(BracketCount);
        if (isStruct) {
            return Builder.CreateStructValue(StartLoc, StructValues);
        } else {
            return Builder.CreateArrayValue(StartLoc, ArrayValues);
        }
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return Builder.CreateZeroValue(Tok.getLocation());
}

ASTExpr *Parser::ParseExpr(ASTIdentifier *Identifier, bool isRoot) {
    FLY_DEBUG_MESSAGE("Parser", "ParseExpr", Logger().Attr("Identifier", Identifier).End());
    ExprParser Parser(this);
    ASTExpr *Expr = Identifier ? Parser.ParseExpr(Identifier) : Parser.ParseExpr();
    return Expr;
}

bool Parser::isArrayType(Token &Tok) {
    FLY_DEBUG("Parser", "isArrayType");

    return Tok.is(tok::l_square);
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
 * ParseModule const scope of vars and class
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
    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
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

bool Parser::isTokenComment() const {
    FLY_DEBUG("Parser", "isTokenComment");
    return Tok.getKind() == tok::comment;
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

    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
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

    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
}

/**
 * ConsumeBrace - This consume method keeps the brace count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeBrace(unsigned short &BraceCount) {
    FLY_DEBUG("Parser", "ConsumeBrace");
    assert(isTokenBrace() && "wrong consume method");
    if (Tok.getKind() == tok::l_brace)
        ++BraceCount;
    else if (BraceCount) {
        //AngleBrackets.clear(*this);
        --BraceCount;     // Don't let unbalanced }'s drive the count negative.
    }

    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
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
    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
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

bool Parser::isTokenAssignOperator(const Token &Tok) const {
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
