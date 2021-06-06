//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "AST/ASTContext.h"
#include <gtest/gtest.h>

namespace {
    using namespace fly;

    class ParserTest : public ::testing::Test {

    public:

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;
        ASTContext *Context;

        ParserTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                      SourceMgr(Diags, FileMgr), Context(new ASTContext(Diags)) {
        }

        virtual ~ParserTest() {
            delete Context;
        }

        std::unique_ptr<Parser> Parse(llvm::StringRef FileName, llvm::StringRef Source) {

            // Create a lexer starting at the beginning of this token.
            InputFile Input (FileName);
            Input.Load(Source, SourceMgr);

            std::unique_ptr<Parser> P = std::make_unique<Parser>(Input, SourceMgr, Diags);
            ASTNode *AST = new ASTNode(FileName, Input.getFileID(), Context);
            P->Parse(AST);
            AST->Finalize();
            return P;
        }

    };

    TEST_F(ParserTest, SinglePackage) {
        llvm::StringRef str = ("namespace std");
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();

        EXPECT_EQ(AST->getFileName(), "package.fly");

        // verify AST contains package
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "std");
    }

    TEST_F(ParserTest, MultiPackageError) {
        llvm::StringRef str = ("namespace std\n"
                         "namespace bad");
        auto P = Parse("error.fly", str);

        EXPECT_TRUE(Diags.hasErrorOccurred());
    }

    TEST_F(ParserTest, SingleImport) {
        llvm::StringRef str = ("namespace std\n"
                         "import \"packageA\"");
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();

        ImportDecl* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
        EXPECT_EQ(Verify->getAlias(), Verify->getName());
    }

    TEST_F(ParserTest, SingleImportAlias) {
        llvm::StringRef str = ("\n import  \"standard\" as \"std\"\n");
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "default");
        EXPECT_EQ(AST->getImports().lookup("standard")->getName(), "standard");
        EXPECT_EQ(AST->getImports().lookup("standard")->getAlias(), "std");
    }

    TEST_F(ParserTest, MultiImports) {
        llvm::StringRef str = ("namespace std\n"
                         "import \"packageA\""
                         "import \"packageB\"");
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();

        ImportDecl* VerifyB = AST->getImports().lookup("packageB");
        ImportDecl* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
    }

    TEST_F(ParserTest, SingleParenImport) {
        llvm::StringRef str = ("namespace std\n"
                         "import (\"packageA\")");
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();

        ImportDecl* Verify = AST->getImports().lookup("packageA");
        EXPECT_EQ(Verify->getName(), "packageA");
    }

    TEST_F(ParserTest, MultiParenImports) {
        llvm::StringRef str = ("namespace std\n"
                         "import (\"packageA\", \"packageB\")");
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();

        ImportDecl* VerifyB = AST->getImports().lookup("packageB");
        ImportDecl* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
    }

    TEST_F(ParserTest, GlobalVars) {
        llvm::StringRef str = ("namespace std\n"
                         "private int a\n"
                         "public float b\n"
                         "bool c\n"
                         );
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();

        GlobalVarDecl *VerifyA = AST->getGlobalVars().lookup("a");
        GlobalVarDecl *VerifyB = AST->getGlobalVars().lookup("b");
        GlobalVarDecl *VerifyC = AST->getGlobalVars().lookup("c");

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_FALSE(VerifyA->isConstant());
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_FALSE(VerifyB->isConstant());
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyC->isConstant());
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");

        delete AST;
    }

    TEST_F(ParserTest, GlobalConstants) {
        llvm::StringRef str = ("namespace std\n"
                         "private const int a = 1\n"
                         "const public float b = 2.0\n"
                         "const bool c = false\n");
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();
        GlobalVarDecl *VerifyA = AST->getGlobalVars().lookup("a");
        GlobalVarDecl *VerifyB = AST->getGlobalVars().lookup("b");
        GlobalVarDecl *VerifyC = AST->getGlobalVars().lookup("c");

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_EQ(VerifyA->isConstant(), true);
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyA->getExpr()->getGroup()[0])->getString(), "1");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_EQ(VerifyB->isConstant(), true);
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyB->getExpr()->getGroup()[0])->getString(), "2.0");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        EXPECT_EQ(VerifyC->isConstant(), true);
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyC->getExpr()->getGroup()[0])->getString(), "false");
    }

    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
        llvm::StringRef str = ("namespace std\n"
                         "void func() {}\n");
        auto P = Parse("function.fly", str);
        auto AST = P->getAST();

        FuncDecl *VerifyFunc = AST->getFunctions().lookup("func");
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyFunc->isConstant());
        ASSERT_TRUE(Context->getNameSpaces().lookup("std")->getFunctions().lookup("func"));
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        llvm::StringRef str = ("namespace std\n"
                         "private int func(int a, const float b, bool c=false) {\n"
                         "  return 1"
                         "}\n");
        auto P = Parse("function.fly", str);
        auto AST = P->getAST();

        FuncDecl *VerifyFunc = AST->getFunctions().lookup("func");
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_PRIVATE);
        ASSERT_FALSE(VerifyFunc->isConstant());
        ASSERT_FALSE(Context->getNameSpaces().lookup("std")->getFunctions().lookup("func"));

        VarDeclStmt *Arg0 = VerifyFunc->getParams()->getVars()[0];
        VarDeclStmt *Arg1 = VerifyFunc->getParams()->getVars()[1];
        VarDeclStmt *Arg2 = VerifyFunc->getParams()->getVars()[2];

        EXPECT_EQ(Arg0->getName(), "a");
        EXPECT_EQ(Arg0->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(Arg0->isConstant(), false);

        EXPECT_EQ(Arg1->getName(), "b");
        EXPECT_EQ(Arg1->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(Arg1->isConstant(), true);

        EXPECT_EQ(Arg2->getName(), "c");
        EXPECT_EQ(Arg2->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(Arg2->isConstant(), false);
        EXPECT_EQ(Arg2->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        ValueExpr *DefArg2 = static_cast<ValueExpr *>(Arg2->getExpr()->getGroup()[0]);
        EXPECT_EQ(DefArg2->getString(), "false");

        GroupExpr *Return = VerifyFunc->getBody()->getReturn()->getExpr();
        EXPECT_EQ(static_cast<ValueExpr*>(Return->getGroup()[0])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr*>(Return->getGroup()[0])->getString(), "1");
        EXPECT_EQ(Return->getKind(), ExprKind::EXPR_GROUP);
    }

    TEST_F(ParserTest, FunctionBodyVar) {
        llvm::StringRef str = ("namespace std\n"
                         "bool func(int a) {\n"
                         "  bool c"
                         "  Type t"
                         "  float b=2.0 + 1.0"
                         "  a += 2"
                         "  c = b == 1.0"
                         "  return c\n"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();

        // Test: bool c
        const VarDeclStmt *cVar = static_cast<VarDeclStmt *>(Body->getContent()[0]);
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(cVar->getType()->getKind(), TypeKind::TYPE_BOOL);
        ASSERT_FALSE(cVar->getExpr());

        // Test: Type t
        const VarDeclStmt *typeVar = static_cast<VarDeclStmt *>(Body->getContent()[1]);
        EXPECT_EQ(typeVar->getName(), "t");
        EXPECT_EQ(typeVar->getType()->getKind(), TypeKind::TYPE_CLASS);
        ClassTypeRef *TypeT = static_cast<ClassTypeRef *>(typeVar->getType());
        EXPECT_EQ(TypeT->getName(), "Type");
        ASSERT_FALSE(TypeT->getStmt());

        // Test: float b=2.0 + 1.0
        const VarDeclStmt *bVar = static_cast<VarDeclStmt *>(Body->getContent()[2]);
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getType()->getKind(), TypeKind::TYPE_FLOAT);
        FloatPrimType *FloatType = static_cast<FloatPrimType *>(bVar->getType());
        EXPECT_EQ(bVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(bVar->getExpr()->getGroup()[0]->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(bVar->getExpr()->getGroup()[0])->getString(), "2.0");
        EXPECT_EQ(bVar->getExpr()->getGroup()[1]->getKind(), ExprKind::EXPR_OPERATOR);
        EXPECT_EQ(bVar->getExpr()->getGroup()[2]->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(bVar->getExpr()->getGroup()[2])->getString(), "1.0");

        // Test: a += 2
        const VarStmt *aVar = static_cast<VarStmt *>(Body->getContent()[3]);
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        VarRefExpr *aExpr = static_cast<VarRefExpr *>(aVar->getExpr()->getGroup()[0]);
        EXPECT_FALSE(aExpr->getRef()->getVarDecl());
        ArithExpr *opCExpr = static_cast<ArithExpr *>(aVar->getExpr()->getGroup()[1]);
        EXPECT_EQ(opCExpr->getArithKind(), ArithOpKind::ARITH_ADD);
        ValueExpr *opC1Expr = static_cast<ValueExpr *>(aVar->getExpr()->getGroup()[2]);
        EXPECT_EQ(opC1Expr->getString(), "2");

        // Test: c = b == 1.0
        const VarStmt *c2Var = static_cast<VarStmt *>(Body->getContent()[4]);
        EXPECT_EQ(c2Var->getName(), "c");
        EXPECT_EQ(c2Var->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        VarRefExpr *c2Expr = static_cast<VarRefExpr *>(c2Var->getExpr()->getGroup()[0]);
        EXPECT_EQ(static_cast<VarRef *>(c2Expr->getRef())->getName(), "b");
        LogicExpr *opC2Expr = static_cast<LogicExpr *>(c2Var->getExpr()->getGroup()[1]);
        EXPECT_EQ(opC2Expr->getLogicKind(), LogicOpKind::LOGIC_EQ);

        const ReturnStmt *Ret = static_cast<ReturnStmt *>(Body->getContent()[5]);
        VarRefExpr *RetRef = static_cast<VarRefExpr *>(Ret->getExpr()->getGroup()[0]);
        EXPECT_EQ(RetRef->getRef()->getName(), "c");
    }

    TEST_F(ParserTest, FunctionBodyCallFunc) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  doSome()"
                         "  doOther(a, b)"
                         "  return do(false, 1, 2.0)"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();

        // Test: doSome()
        const FuncCallStmt *doSome = static_cast<FuncCallStmt *>(Body->getContent()[0]);
        //EXPECT_FALSE(F->getFuncRef().find("doSome")->getValue()->getRef());
        EXPECT_EQ(doSome->getName(), "doSome");
        EXPECT_EQ(doSome->getKind(), StmtKind::STMT_FUNC_CALL);
        ASSERT_FALSE(doSome->getDecl());

        // Test: doOther(a, b)
        const FuncCallStmt *doOther = static_cast<FuncCallStmt *>(Body->getContent()[1]);
        //EXPECT_FALSE(F->getFuncRef().find("doOther")->getValue()->getRef());
        EXPECT_EQ(doOther->getName(), "doOther");
        EXPECT_EQ(doOther->getKind(), StmtKind::STMT_FUNC_CALL);
        EXPECT_EQ(static_cast<VarRefExpr *>(doOther->getParams()->getArgs()[0])->getRef()->getName(),"a");
        EXPECT_EQ(static_cast<VarRefExpr *>(doOther->getParams()->getArgs()[1])->getRef()->getName(),"b");
        ASSERT_FALSE(doOther->getDecl());

        const ReturnStmt *Ret = static_cast<ReturnStmt *>(Body->getContent()[2]);
        FuncRefExpr *RetRef = static_cast<FuncRefExpr *>(Ret->getExpr()->getGroup()[0]);
        EXPECT_EQ(RetRef->getRef()->getName(), "do");
        EXPECT_EQ(RetRef->getKind(), ExprKind::EXPR_REF_FUNC);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[0])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[0])->getString(),"false");
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[1])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[1])->getString(), "1");
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[2])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[2])->getString(), "2.0");
    }

    TEST_F(ParserTest, FunctionBodyIncDec) {
        llvm::StringRef str = ("namespace std\n"
                         "void func() {\n"
                         "  ++a"
                         "  b++"
                         "  --c"
                         "  d--"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();

        // ++a
        const VarStmt *aVar = static_cast<VarStmt *>(Body->getContent()[0]);
        EXPECT_EQ(aVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(aVar->getName(), "a");
        IncDecExpr *aExpr = static_cast<IncDecExpr *>(aVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(aExpr->getIncDecKind(), IncDecOpKind::PRE_INCREMENT);

        // b++
        const VarStmt *bVar = static_cast<VarStmt *>(Body->getContent()[1]);
        EXPECT_EQ(bVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(bVar->getName(), "b");
        IncDecExpr *bExpr = static_cast<IncDecExpr *>(bVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(bExpr->getIncDecKind(), IncDecOpKind::POST_INCREMENT);

        // ++c
        const VarStmt *cVar = static_cast<VarStmt *>(Body->getContent()[2]);
        EXPECT_EQ(cVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(cVar->getName(), "c");
        IncDecExpr *cExpr = static_cast<IncDecExpr *>(cVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(cExpr->getIncDecKind(), IncDecOpKind::PRE_DECREMENT);

        // d++
        const VarStmt *dVar = static_cast<VarStmt *>(Body->getContent()[3]);
        EXPECT_EQ(dVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(dVar->getName(), "d");
        IncDecExpr *dExpr = static_cast<IncDecExpr *>(dVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(dExpr->getIncDecKind(), IncDecOpKind::POST_DECREMENT);
    }

    TEST_F(ParserTest, FunctionBodyIfStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  if (a == 1) {"
                         "    return"
                         "  } elsif (a == 2) {"
                         "    b = 1"
                         "  } else {"
                         "    b = 2"
                         "  }"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();

        // if
        const IfBlockStmt *Stmt = static_cast<IfBlockStmt *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_IF);
        EXPECT_EQ(static_cast<VarRefExpr *>(Stmt->getCondition()->getGroup()[0])->getRef()->getName(), "a");
        EXPECT_EQ(static_cast<LogicExpr *>(Stmt->getCondition()->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ValueExpr *>(Stmt->getCondition()->getGroup()[2])->getString(), "1");
        EXPECT_TRUE(static_cast<ReturnStmt *>(Stmt->getContent()[0])->getExpr()->getGroup().empty());
        EXPECT_FALSE(Stmt->getElsif().empty());
        EXPECT_TRUE(Stmt->getElse());

        // Elsif
        const ElsifBlockStmt *EIStmt = static_cast<ElsifBlockStmt *>(Body->getContent()[1]);
        EXPECT_EQ(EIStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSIF);
        EXPECT_EQ(static_cast<VarRefExpr *>(EIStmt->getCondition()->getGroup()[0])->getRef()->getName(), "a");
        EXPECT_EQ(static_cast<LogicExpr *>(EIStmt->getCondition()->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ValueExpr *>(EIStmt->getCondition()->getGroup()[2])->getString(), "2");
        EXPECT_EQ(static_cast<VarStmt *>(EIStmt->getContent()[0])->getName(), "b");

        // Else
        const ElseBlockStmt *EEStmt = static_cast<ElseBlockStmt *>(Body->getContent()[2]);
        EXPECT_EQ(EEStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSE);
        EXPECT_EQ(static_cast<VarStmt *>(EEStmt->getContent()[0])->getName(), "b");
    }

    TEST_F(ParserTest, FunctionBodyIfInlineStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  if (a == 1) return"
                         "  elsif a == 2 a = 1"
                         "  else a = 2"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();

        // if
        const IfBlockStmt *Stmt = static_cast<IfBlockStmt *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_IF);
        EXPECT_EQ(static_cast<VarRefExpr *>(Stmt->getCondition()->getGroup()[0])->getRef()->getName(), "a");
        EXPECT_EQ(static_cast<LogicExpr *>(Stmt->getCondition()->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ValueExpr *>(Stmt->getCondition()->getGroup()[2])->getString(), "1");
        EXPECT_TRUE(static_cast<ReturnStmt *>(Stmt->getContent()[0])->getExpr()->getGroup().empty());

        EXPECT_FALSE(Stmt->getElsif().empty());
        EXPECT_TRUE(Stmt->getElse());

        // Elsif
        const ElsifBlockStmt *EIStmt = static_cast<ElsifBlockStmt *>(Body->getContent()[1]);
        EXPECT_EQ(EIStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSIF);
        EXPECT_EQ(static_cast<VarRefExpr *>(EIStmt->getCondition()->getGroup()[0])->getRef()->getName(), "a");
        EXPECT_EQ(static_cast<LogicExpr *>(EIStmt->getCondition()->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ValueExpr *>(EIStmt->getCondition()->getGroup()[2])->getString(), "2");

        // Else
        const ElseBlockStmt *EEStmt = static_cast<ElseBlockStmt *>(Body->getContent()[2]);
        EXPECT_EQ(EEStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSE);
        EXPECT_EQ(static_cast<VarStmt *>(EEStmt->getContent()[0])->getName(), "a");
    }

    TEST_F(ParserTest, FunctionBodySwitchStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  switch (a) {"
                         "    case 1:"
                         "      break"
                         "    case 2:"
                         "    default:"
                         "      hello()"
                         "  }"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();

        const SwitchBlockStmt *Stmt = static_cast<SwitchBlockStmt *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_SWITCH);
        EXPECT_EQ(static_cast<ValueExpr *>(Stmt->getCases()[0]->getExpr())->getString(), "1");
        EXPECT_EQ(Stmt->getCases()[0]->getContent()[0]->getKind(), StmtKind::STMT_BREAK);
        EXPECT_EQ(static_cast<ValueExpr *>(Stmt->getCases()[1]->getExpr())->getString(), "2");
        EXPECT_TRUE(Stmt->getCases()[1]->getContent().empty());
        EXPECT_EQ(Stmt->getDefault()->getBlockKind(), BlockStmtKind::BLOCK_STMT_DEFAULT);
        EXPECT_EQ(static_cast<FuncCallStmt *>(Stmt->getDefault()->getContent()[0])->getName(), "hello");
        EXPECT_EQ(static_cast<FuncCallStmt *>(Stmt->getDefault()->getContent()[0])->getKind(), StmtKind::STMT_FUNC_CALL);
    }

    TEST_F(ParserTest, FunctionBodyForStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  for int b = 1, int c = 2; b < 10; b++, --c {"
                         "    if (a == 5) break"
                         "    else continue"
                         "  }"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();
        const ForBlockStmt *Stmt = static_cast<ForBlockStmt *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_FOR);

        EXPECT_EQ(static_cast<VarDeclStmt *>(Stmt->getInit()->getContent()[0])->getName(), "b");
        EXPECT_EQ(static_cast<VarDeclStmt *>(Stmt->getInit()->getContent()[1])->getName(), "c");

        const GroupExpr *Cond = Stmt->getCondition();
        EXPECT_EQ(static_cast<VarRefExpr *>(Cond->getGroup()[0])->getRef()->getName(), "b");
        EXPECT_EQ(static_cast<LogicExpr *>(Cond->getGroup()[1])->getLogicKind(), LogicOpKind::LOGIC_LT);
        EXPECT_EQ(static_cast<ValueExpr *>(Cond->getGroup()[2])->getString(), "10");

        EXPECT_EQ(static_cast<VarStmt *>(Stmt->getPost()->getContent()[0])->getName(), "b");
        IncDecExpr *Expr1 = static_cast<IncDecExpr *>(static_cast<VarStmt *>(Stmt->getPost()->getContent()[0])->getExpr()->getGroup()[0]);
        EXPECT_EQ(Expr1->getIncDecKind(), IncDecOpKind::POST_INCREMENT);
        EXPECT_EQ(static_cast<VarStmt *>(Stmt->getPost()->getContent()[1])->getName(), "c");
        IncDecExpr *Expr2 = static_cast<IncDecExpr *>(static_cast<VarStmt *>(Stmt->getPost()->getContent()[1])->getExpr()->getGroup()[0]);
        EXPECT_EQ(Expr2->getIncDecKind(), IncDecOpKind::PRE_DECREMENT);
    }

    TEST_F(ParserTest, FunctionBodyForCondStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  for (a==1) {}"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();
        const ForBlockStmt *Stmt = static_cast<ForBlockStmt *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_FOR);
        EXPECT_TRUE(Stmt->getInit()->isEmpty());
        EXPECT_FALSE(Stmt->getCondition()->isEmpty());
        EXPECT_TRUE(Stmt->getPost()->isEmpty());

        const GroupExpr *Cond = Stmt->getCondition();
        EXPECT_EQ(static_cast<VarRefExpr *>(Cond->getGroup()[0])->getRef()->getName(), "a");
        EXPECT_EQ(static_cast<LogicExpr *>(Cond->getGroup()[1])->getLogicKind(), LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ValueExpr *>(Cond->getGroup()[2])->getString(), "1");
    }

    TEST_F(ParserTest, FunctionBodyForEmptyStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  for {}"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const BlockStmt *Body = F->getBody();
        const ForBlockStmt *Stmt = static_cast<ForBlockStmt *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_FOR);
        EXPECT_TRUE(Stmt->getInit()->isEmpty());
        EXPECT_TRUE(Stmt->getCondition()->isEmpty());
        EXPECT_TRUE(Stmt->getPost()->isEmpty());
    }
}