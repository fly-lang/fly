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
#include "AST/ASTNode.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTHandleStmt.h"
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

    Node = Builder.CreateNode(Input.getFileName());

    // Parse NameSpace on first
    if (ParseNameSpace() && Builder.AddNode(Node)) {

        // Parse Imports
        if (ParseImports()) {

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

    return !Diags.hasErrorOccurred() && Success ? Node : nullptr;
}

ASTNode *Parser::ParseHeader() {
    FLY_DEBUG("Parser", "ParseHeader");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse NameSpace on first
    if (ParseNameSpace()) {
        Node = Builder.CreateHeaderNode(Input.getFileName());

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
    ASTNameSpace *NameSpace;

    // Check namespace declaration
    if (Tok.is(tok::kw_namespace)) {
        ConsumeToken();

        // Parse NameSpace identifier
        ASTIdentifier *Identifier = ParseIdentifier();

        // Check identifier Error
        if (Identifier == nullptr) {
            Diag(Tok, diag::err_namespace_invalid) << Tok.getName();
            return false;
        }
        NameSpace = Builder.CreateNameSpace(Identifier);
    } else {

        // Define Default NameSpace also if it has not been defined
        NameSpace = Builder.CreateDefaultNameSpace();
    }
    FLY_DEBUG_MESSAGE("Parser", "ParseNameSpace", "NameSpace=" << NameSpace);

    return Builder.AddNameSpace(NameSpace, Node);
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

            FLY_DEBUG_MESSAGE("Parser", "ParseImports", "Name=" << Name);
            ASTImport *Import = Builder.CreateImport(Loc, Name);
            if (Tok.isAnyIdentifier()) {
                IdentifierInfo *AliasId = Tok.getIdentifierInfo();
                llvm::StringRef AliasName = AliasId->getName();
                const SourceLocation &AliasLoc = ConsumeToken();
                ASTAlias *Alias = Builder.CreateAlias(AliasLoc, AliasName);
                FLY_DEBUG_MESSAGE("Parser", "ParseImports", "Name=" << Import->getName() << ", Alias=" << Alias->getName());
                Import->setAlias(Alias);
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

    // Parse Public/Private and Constant
    ASTScopes *Scopes = Builder.CreateScopes();
    if (ParseScopes(Scopes)) {

        // Define a Class
        if (Tok.isOneOf(tok::kw_struct, tok::kw_class, tok::kw_interface)) {
            return ParseClassDef(Scopes);
        }

        if (Tok.is(tok::kw_enum)) {
            return ParseEnumDef(Scopes);
        }

        // Parse Type
        ASTType *Type = nullptr;
        if (ParseType(Type)) {

            // Define a Function
            if (Tok.isAnyIdentifier() &&
                Lex.findNextToken(Tok.getLocation(), SourceMgr)->is(tok::l_paren)) {
                return ParseFunctionDef(Scopes, Type);
            }

            // Define a GlobalVar
            return ParseGlobalVarDef(Scopes, Type);
        }
    }

    // Unknown Top Definition
    return false;
}

bool Parser::ParseScopes(ASTScopes *&Scopes) {
    FLY_DEBUG("ClassParser", "ParseScopes");

    bool isVisibleAssigned = false;
    bool isConst = false;
    bool isStatic = false;
    while (true) {
        if (Tok.isOneOf(tok::kw_private,tok::kw_protected, tok::kw_public)) {
            if (isVisibleAssigned) {
                Diag(Tok, diag::err_scope_visibility_conflict
                        << (int) ASTVisibilityKind::V_PRIVATE << (int) ASTVisibilityKind::V_PUBLIC);
                return false;
            }
            Scopes->setVisibility(Tok.is(tok::kw_private) ? ASTVisibilityKind::V_PRIVATE :
                         Tok.is(tok::kw_protected) ? ASTVisibilityKind::V_PROTECTED :
                         Tok.is(tok::kw_public) ? ASTVisibilityKind::V_PUBLIC :
                         ASTVisibilityKind::V_DEFAULT);
            isVisibleAssigned = true;
            ConsumeToken();
        } else if (Tok.is(tok::kw_const)) {
            if (isConst) {
                Diag(Tok, diag::err_scope_const_duplicate);
                return false;
            }
            Scopes->setConstant(true);
            isConst = true;
            ConsumeToken();
        } else if (Tok.is(tok::kw_static)) {
            if (isStatic) {
                Diag(Tok, diag::err_scope_static_duplicate);
                return false;
            }
            Scopes->setStatic(true);
            isStatic = true;
            ConsumeToken();
        } else {
            break;
        }
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
bool Parser::ParseGlobalVarDef(ASTScopes *Scopes, ASTType *Type) {
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

    ASTValue *Value = nullptr;
    if (Expr->getExprKind() == ASTExprKind::EXPR_VALUE) {
        Value = ((ASTValueExpr *) Expr)->getValue();
    } else {
        Diag(Tok.getLocation(), diag::err_invalid_gvar_value);
    }

    return Builder.AddGlobalVar(GlobalVar, Value) &&
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
bool Parser::ParseFunctionDef(ASTScopes *Scopes, ASTType *Type) {
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
            Builder.AddFunction(Function);
    }

    return false;
}

/**
 * Parse Class declaration
 * @param Visibility
 * @param Constant
 * @return
 */
bool Parser::ParseClassDef(ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("Parser", "ParseClassDef", Logger().Attr("Scopes", Scopes).End());

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!BlockComment.empty()) {
        Comment = BlockComment;
        BlockComment = StringRef();
    }

    ASTClass *Class = ClassParser::Parse(this, Scopes);
    if (Class) {
        return Builder.AddComment(Class, Comment) && Builder.AddIdentity(Class);
    }

    return false;
}


/**
 * Parse Enum declaration
 * @param Visibility
 * @param Constant
 * @return
 */
bool Parser::ParseEnumDef(ASTScopes *Scopes) {
    FLY_DEBUG_MESSAGE("Parser", "ParseClassDef", Logger().Attr("Scopes", Scopes).End());

    // Add Comment to AST
    llvm::StringRef Comment;
    if (!BlockComment.empty()) {
        Comment = BlockComment;
        BlockComment = StringRef();
    }

    ASTEnum *Enum = EnumParser::Parse(this, Scopes);
    if (Enum) {
        return Builder.AddComment(Enum, Comment) && Builder.AddIdentity(Enum);
    }

    return false;
}

/**
 * Parse statements between braces
 * @param Stmt
 * @return
 */
bool Parser::ParseBlock(ASTBlock *Parent) {
    assert(isBlockStart() && "Block Start");
    FLY_DEBUG_MESSAGE("Parser", "ParseStmt", Logger().Attr("Block", Parent).End());
    ConsumeBrace(BracketCount);

    if (ParseStmt(Parent) && isBlockEnd()) {
        ConsumeBrace(BracketCount);
        return true;
    }

    Diag(Tok.getLocation(), diag::err_parse_block);
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
 * @param Parent
 * @return true on Success or false on Error
 */
bool Parser::ParseStmt(ASTBlock *Parent, bool StopParse) {
    FLY_DEBUG_MESSAGE("Parser", "ParseStmt", Logger().Attr("Block", Parent).End());

    // Parse keywords
    if (Tok.is(tok::kw_if)) {
        return ParseIfStmt(Parent);
    }
    if (Tok.is(tok::kw_switch)) {
        return ParseSwitchStmt(Parent);
    }
    if (Tok.is(tok::kw_for)) {
        return ParseForStmt(Parent);
    }
    if (Tok.is(tok::kw_while)) {
        return ParseWhileStmt(Parent);
    }
    if (Tok.is(tok::kw_handle)) {
        return ParseHandleStmt(Parent, nullptr);
    }
    if (Tok.is(tok::kw_fail)) {
        return ParseFailStmt(Parent);
    }

    if (Tok.is(tok::kw_return)) { // Parse return
        SourceLocation Loc = ConsumeToken();
        ASTReturnStmt *Return = Builder.CreateReturn(Parent, Loc);
        ASTExpr *Expr = ParseExpr(Return);
        return Builder.AddStmt(Return) && (StopParse || ParseStmt(Parent));
    }
    if (Tok.is(tok::kw_break)) { // Parse break
        ASTBreakStmt *Break = Builder.CreateBreak(Parent, ConsumeToken());
        return Builder.AddStmt(Break) && (StopParse || ParseStmt(Parent));
    }
    if (Tok.is(tok::kw_continue)) { // Parse continue
        ASTContinueStmt *Continue = Builder.CreateContinue(Parent, ConsumeToken());
        return Builder.AddStmt(Continue) && (StopParse || ParseStmt(Parent));
    }

    // define a const LocalVar
    ASTScopes *Scopes = Builder.CreateScopes();
    ParseScopes(Scopes);
    
    // Define an ASTLocalVar
    // int a
    ASTType *Type = nullptr;
    isBuiltinType(Tok) && ParseBuiltinType(Type);

    // Type a
    // Type a = ...
    // a = ...
    ASTIdentifier *Identifier1 = nullptr;
    if (Tok.isAnyIdentifier()) {

        // ASTCall or ASTClassType, ASTVarRef, ASTLocalVar
        Identifier1 = ParseIdentifier();

        if (!Identifier1->isCall()) { // a() t.a()
            if (Type != nullptr) { // int a
                // FIXME check Identifier for LocalVar
                ASTLocalVar *LocalVar = Builder.CreateLocalVar(Tok.getLocation(), Type,
                                                               Identifier1->getName(), Scopes);
                ASTVarStmt *VarStmt = Builder.CreateVarStmt(Parent, LocalVar);

                // int a = ...
                if (Tok.is(tok::equal)) {
                    ConsumeToken();
                    ASTExpr *Expr = ParseExpr(VarStmt);
                }
                return Builder.AddStmt(VarStmt) && (StopParse || ParseStmt(Parent));

            } else if (Tok.isAnyIdentifier()) { // Type a: Identifier is ASTType
                ASTIdentifier *Identifier2 = ParseIdentifier();

                // Type is a IdentityType or an Array of IdentityType
                Type = Builder.CreateIdentityType(Identifier1);

                // FIXME check Identifier for LocalVar
                ASTLocalVar *LocalVar = Builder.CreateLocalVar(Tok.getLocation(), Type,
                                                               Identifier2->getName(), Scopes);

                ASTStmt *Stmt = nullptr;
                // Type a = ...
                if (Tok.is(tok::equal)) {
                    ConsumeToken();

                    if (Tok.is(tok::kw_handle)) {
                        ASTVarRef *VarRef = Builder.CreateVarRef(LocalVar);
                        return ParseHandleStmt(Parent, VarRef) && (StopParse || ParseStmt(Parent));
                    } else {
                        Stmt = Builder.CreateVarStmt(Parent, LocalVar);
                        ASTExpr *Expr = ParseExpr(Stmt);
                        return Builder.AddStmt(Stmt) && (StopParse || ParseStmt(Parent));
                    }
                }

            }

            if (isTokenAssignOperator()) { // a = ...

                ASTVarRef *VarRef = Builder.CreateVarRef(Identifier1);
                ASTVarStmt *VarDefine = Builder.CreateVarStmt(Parent, VarRef);

                ExprParser Parser(this, VarDefine);
                ASTExpr *Expr = Parser.ParseAssignExpr();
                return Expr && Builder.AddStmt(VarDefine)  && (StopParse || ParseStmt(Parent));
            }
        }
    }

    // a()
    // a++
    // ++a
    // new A()
    ASTExprStmt *ExprStmt = Builder.CreateExprStmt(Parent, Tok.getLocation());
    ASTExpr *Expr = ParseExpr(ExprStmt, Identifier1);
    if (Expr->getExprKind() != ASTExprKind::EXPR_EMPTY) {
        return Builder.AddStmt(ExprStmt) && (StopParse || ParseStmt(Parent));
    }

    return true;
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
    bool Success;
    if (isBlockStart()) {
        Success = ParseStmt(IfBlock);
    } else { // Only for a single Stmt without braces
        Success = ParseStmt(IfBlock, true);
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
            Success = ParseStmt(ElsifBlock);
        } else { // Only for a single Stmt without braces
            Success = ParseStmt(ElsifBlock, true);
        }
    }

    // Add Else
    if (Success && Tok.is(tok::kw_else)) {
        const SourceLocation &ElseLoc = ConsumeToken();
        ASTElseBlock *ElseBlock = Builder.CreateElseBlock(IfBlock, ElseLoc);

        if (isBlockStart()) {
            Success = ParseStmt(ElseBlock);
        } else { // Only for a single Stmt without braces
            Success = ParseStmt(ElseBlock, true);
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
    ASTSwitchBlock *SwitchBlock = Builder.CreateSwitchBlock(Block, ConsumeToken());

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
        if (isBlockStart()) {
            ConsumeBrace(BracketCount);

            if (ParseSwitchCases(SwitchBlock)) {

                // Switch statement is at end of it's time add current Switch to parent statement
                if (isBlockEnd()) {
                    ConsumeBrace(BracketCount);
                    return Builder.AddStmt(SwitchBlock);
                }
            }
        }
    }

    Diag(diag::err_syntax_error);
    return false;
}

bool Parser::ParseSwitchCases(ASTSwitchBlock *SwitchBlock) {

    ASTBlock *CaseBlock;
    if (Tok.is(tok::kw_case)) {

        // Parse Expression for different cases
        // for a Value  -> case 1:
        // for a Var -> case a:
        // for a default
        CaseBlock = Builder.CreateSwitchCaseBlock(SwitchBlock, ConsumeToken());
        ASTExpr *CaseExpr = ParseExpr(CaseBlock);
    } else if (Tok.is(tok::kw_default)) {
        CaseBlock = Builder.CreateSwitchDefaultBlock(SwitchBlock, ConsumeToken());
    } else {
        return true;
    }

    if (Tok.is(tok::colon)) { // Parse a Block of Stmt
        ConsumeToken();
        return ParseStmt(CaseBlock) && ParseSwitchCases(SwitchBlock);
    } else if (isBlockStart()) { // Parse a single Stmt without braces
        ConsumeBrace(BracketCount);
        return ParseStmt(CaseBlock) && ParseSwitchCases(SwitchBlock);
    }

    Diag(diag::err_syntax_error);
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
    assert(Tok.is(tok::kw_while) && "Token is while keyword");
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
                   ParseStmt(WhileBlock) :
                   ParseStmt(WhileBlock, true); // Only for a single Stmt without braces

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
    assert(Tok.is(tok::kw_for) && "Token is for keyword");
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
               ParseStmt(LoopBlock) :
               ParseStmt(LoopBlock, true); // Only for a single Stmt without braces
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

bool Parser::ParseHandleStmt(ASTBlock *Block, ASTVarRef *Error) {
    assert(Tok.is(tok::kw_handle) && "Token is handle keyword");
    FLY_DEBUG_MESSAGE("Parser", "ParseHandleStmt", Logger().Attr("Error", Error).End());

    // Parse handle block
    const SourceLocation &Loc = ConsumeToken();
    ASTHandleStmt *HandleStmt = Builder.CreateHandleStmt(Block, Loc, Error);

    // Parse statement between braces
    bool Success = isBlockStart() ?
                   ParseBlock(Block) :
                   ParseStmt(Block, true); // Only for a single Stmt without braces

    return Success & Builder.AddStmt(HandleStmt);
}

bool Parser::ParseFailStmt(ASTBlock *Block) {
    assert(Tok.is(tok::kw_fail) && "Token is handle keyword");
    FLY_DEBUG("Parser", "ParseFailStmt");

    const SourceLocation &Loc = ConsumeToken();
    ASTFailStmt *FailStmt = Builder.CreateFail(Block, Loc);
    bool Success = ParseExpr(FailStmt);
    return Success & Builder.AddStmt(FailStmt);
}

bool Parser::ParseBuiltinType(ASTType *&Type) {
    FLY_DEBUG("Parser", "ParseBuiltinType");
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
        default:
            Diag(Tok.getLocation(), diag::err_parser_invalid_type);
            return false;
    }
    return !isArrayType(Tok) || ParseArrayType(Type);
}

bool Parser::ParseArrayType(ASTType *&Type) {
    FLY_DEBUG_MESSAGE("Parser", "ParseArrayType", Logger().Attr("Type", Type).End());

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
                return false;
            }
        }
    } while (Tok.is(tok::l_square));

    return true;
}

/**
 * Parse a data Type
 * @return true on Success or false on Error
 */
bool Parser::ParseType(ASTType *&Type) {
    FLY_DEBUG("Parser", "ParseType");

    if (isBuiltinType(Tok) && ParseBuiltinType(Type)) {
        // nothing
    } else if (Tok.isAnyIdentifier()) {
        ASTIdentifier *Identifier = ParseIdentifier();
        if (Identifier->isCall()) {
            Diag(Identifier->getLocation(), diag::err_parser_invalid_type);
            return false;
        }
        Type = Builder.CreateClassType(Identifier);
    } else {
        Diag(Tok.getLocation(), diag::err_parser_empty_type);
        return false;
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
bool Parser::ParseCall(ASTIdentifier *&Identifier) {
    FLY_DEBUG_MESSAGE("Parser", "ParseCall", Logger()
            .Attr("Loc", Identifier).End());
    assert(Tok.is(tok::l_paren) && "Call start with parenthesis");

    // Parse Call args
    ASTCall *Call = Builder.CreateCall(Identifier);
    ConsumeParen(); // consume l_paren

    if (ParseCallArg(Call)) {

        if (Tok.is(tok::r_paren)) {
            ConsumeParen();

            Identifier = Call;
            return true; // end
        }
    }

    return false;
}

/**
 * Parse a single Call Argument
 * @param Block
 * @return true on Success or false on Error
 */
bool Parser::ParseCallArg(ASTCall *Call) {
    FLY_DEBUG_MESSAGE("Parser", "ParseCallArg",
                      Logger().Attr("Call", Call).End());

    // Parse Args in a Function Call
    if (Builder.AddCallArg(Call, ParseExpr(nullptr))) {

        if (Tok.is(tok::comma)) {
            ConsumeToken();

            return ParseCallArg(Call);
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
ASTIdentifier *Parser::ParseIdentifier(ASTIdentifier *Parent) {
    FLY_DEBUG("Parser", "ParseIdentifier");

    if (!Tok.isAnyIdentifier()) {
        Diag(Tok, diag::err_parse_identifier_invalid) << Tok.getName();
        return nullptr;
    }

    // Create the Child and work on it
    ASTIdentifier *Child;
    llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
    if (Parent == nullptr) {
        Child = Builder.CreateIdentifier(ConsumeToken(), Name);
    } else {
        Child = Parent->AddChild(Builder.CreateIdentifier(ConsumeToken(), Name));
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
 * Parse Array Value Expression
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValues() {
    FLY_DEBUG("Parser", "ParseValues");
    const SourceLocation &StartLoc = ConsumeBrace(BracketCount);
    
    // Init with a zero value
    ASTValue *Values = nullptr;

    // Parse array values Ex. {1, 2, 3}
    while (Tok.isNot(tok::r_brace) && Tok.isNot(tok::eof)) {

        // if is Identifier -> struct
        if (Tok.isAnyIdentifier()) {
            const StringRef &Key = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            
            if (Tok.is(tok::equal)) {
                Values = Builder.CreateStructValue(StartLoc);
                ConsumeToken();

                ASTValue *Value = ParseValue();
                if (Value) {
                    Builder.AddStructValue((ASTStructValue *) Values, Key, Value);
                    if (Tok.is(tok::comma)) {
                        ConsumeToken();
                    } else {
                        break;
                    }
                } else {
                    Diag(diag::err_invalid_value) << Tok.getName();
                    return Values;
                }
            }
        } else { // if is Value -> array
            if (Values == nullptr)
                Values = Builder.CreateArrayValue(StartLoc);
            ASTValue *Value = ParseValue();
            if (Value) {
                Builder.AddArrayValue((ASTArrayValue *) Values, Value);
                if (Tok.is(tok::comma)) {
                    ConsumeToken();
                } else {
                    break;
                }
            } else {
                Diag(diag::err_invalid_value) << Tok.getName();
                return Values;
            }
        }

    }

    // End of Array
    if (Tok.is(tok::r_brace)) {
        ConsumeBrace(BracketCount);
        if (Values == nullptr)
            Values = Builder.CreateZeroValue(StartLoc);
        return Values;
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return Values;
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
                       tok::kw_double,
                       tok::kw_string);
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
SourceLocation Parser::ConsumeBrace(unsigned short &BraceCount) {
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
