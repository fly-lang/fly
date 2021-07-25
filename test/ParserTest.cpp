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
#include "AST/ASTNameSpace.h"
#include <unordered_set>
#include <gtest/gtest.h>
#include <AST/ASTWhileBlock.h>

namespace {
    using namespace fly;

    class ParserTest : public ::testing::Test {

    public:

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;

        ParserTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                      SourceMgr(Diags, FileMgr) {
        }

        std::unique_ptr<Parser> Parse(llvm::StringRef FileName, llvm::StringRef Source) {

            // Create a lexer starting at the beginning of this token.
            InputFile Input(FileName);
            Input.Load(Source, SourceMgr);

            std::unique_ptr<Parser> P = std::make_unique<Parser>(Input, SourceMgr, Diags);
            ASTContext *Ctx = new ASTContext(Diags);
            ASTNode *AST = new ASTNode(FileName, Input.getFileID(), Ctx);
            if (P->Parse(AST)) {
                AST->Finalize();
            }
            return P;
        }

    };

    TEST_F(ParserTest, SinglePackage) {
        llvm::StringRef str = ("namespace std");
        auto P = Parse("package.fly", str);
        ASTNode *AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        EXPECT_EQ(AST->getFileName(), "package.fly");

        // verify AST contains package
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "std");
        delete AST;
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
        ASSERT_FALSE(AST->getContext().hasErrors());

        ASTImport* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
        EXPECT_EQ(Verify->getAlias(), Verify->getName());
        delete AST;
    }

    TEST_F(ParserTest, SingleImportAlias) {
        llvm::StringRef str = ("\n import  \"standard\" as \"std\"\n");
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "default");
        EXPECT_EQ(AST->getImports().lookup("standard")->getName(), "standard");
        EXPECT_EQ(AST->getImports().lookup("standard")->getAlias(), "std");
        delete AST;
    }

    TEST_F(ParserTest, MultiImports) {
        llvm::StringRef str = ("namespace std\n"
                         "import \"packageA\""
                         "import \"packageB\"");
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        ASTImport* VerifyB = AST->getImports().lookup("packageB");
        ASTImport* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
        delete AST;
    }

    TEST_F(ParserTest, SingleParenImport) {
        llvm::StringRef str = ("namespace std\n"
                         "import (\"packageA\")");
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        ASTImport* Verify = AST->getImports().lookup("packageA");
        EXPECT_EQ(Verify->getName(), "packageA");
        delete AST;
    }

    TEST_F(ParserTest, MultiParenImports) {
        llvm::StringRef str = ("namespace std\n"
                         "import (\"packageA\", \"packageB\")");
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        ASTImport* VerifyB = AST->getImports().lookup("packageB");
        ASTImport* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
        delete AST;
    }

    TEST_F(ParserTest, GlobalVars) {
        llvm::StringRef str = ("namespace std\n"
                         "private int a\n"
                         "public float b\n"
                         "bool c\n"
                         );
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        ASTGlobalVar *VerifyA = AST->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *VerifyB = AST->getNameSpace()->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *VerifyC = AST->getGlobalVars().find("c")->getValue();

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
        ASSERT_FALSE(AST->getContext().hasErrors());

        ASTGlobalVar *VerifyA = AST->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *VerifyB = AST->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *VerifyC = AST->getGlobalVars().find("c")->getValue();

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_EQ(VerifyA->isConstant(), true);
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");
        EXPECT_EQ(static_cast<ASTValueExpr *>(VerifyA->getExpr())->getValue().str(), "1");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_EQ(VerifyB->isConstant(), true);
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");
        EXPECT_EQ(static_cast<ASTValueExpr *>(VerifyB->getExpr())->getValue().str(), "2.0");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        EXPECT_EQ(VerifyC->isConstant(), true);
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");
        EXPECT_EQ(static_cast<ASTValueExpr *>(VerifyC->getExpr())->getValue().str(), "false");

        delete AST;
    }

    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
        llvm::StringRef str = ("namespace std\n"
                         "void func() {}\n");
        auto P = Parse("function.fly", str);
        auto *AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        EXPECT_TRUE(AST->getFunctions().size() == 1); // Fun has DEFAULT Visibility
        EXPECT_TRUE(AST->getNameSpace()->getFunctions().size() == 1);
        ASTFunc *VerifyFunc = *(AST->getNameSpace()->getFunctions().begin());
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyFunc->isConstant());
        const std::unordered_set<ASTFunc *> &NSFuncs = AST->getContext().getNameSpaces().lookup("std")->getFunctions();
        ASSERT_TRUE(NSFuncs.find(VerifyFunc) != NSFuncs.end());

        delete AST;
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        llvm::StringRef str = ("namespace std\n"
                         "private int func(int a, const float b, bool c=false) {\n"
                         "  return 1"
                         "}\n");
        auto P = Parse("function.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        EXPECT_TRUE(AST->getFunctions().size() == 1); // func() has PRIVATE Visibility
        ASTFunc *VerifyFunc = *(AST->getFunctions().begin());
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_PRIVATE);
        ASSERT_FALSE(VerifyFunc->isConstant());
        const std::unordered_set<ASTFunc *> &NSFuncs = AST->getContext().getNameSpaces()
                .lookup("std")->getFunctions();
        ASSERT_TRUE(NSFuncs.find(VerifyFunc) == NSFuncs.end());
        ASSERT_TRUE(AST->getFunctions().find(VerifyFunc) != NSFuncs.end());

        ASTFuncParam *Par0 = VerifyFunc->getHeader()->getParams()[0];
        ASTFuncParam *Par1 = VerifyFunc->getHeader()->getParams()[1];
        ASTFuncParam *Par2 = VerifyFunc->getHeader()->getParams()[2];

        EXPECT_EQ(Par0->getName(), "a");
        EXPECT_EQ(Par0->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(Par0->isConstant(), false);

        EXPECT_EQ(Par1->getName(), "b");
        EXPECT_EQ(Par1->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(Par1->isConstant(), true);

        EXPECT_EQ(Par2->getName(), "c");
        EXPECT_EQ(Par2->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(Par2->isConstant(), false);
        EXPECT_EQ(Par2->getExpr()->getKind(), ExprKind::EXPR_VALUE);
        ASTValueExpr *DefArg2 = static_cast<ASTValueExpr *>(Par2->getExpr());
        EXPECT_EQ(DefArg2->getValue().str(), "false");

        ASTReturn *Return = static_cast<ASTReturn*>(VerifyFunc->getBody()->getContent()[0]);
        EXPECT_EQ(static_cast<ASTValueExpr*>(Return->getExpr())->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ASTValueExpr*>(Return->getExpr())->getValue().str(), "1");
        EXPECT_EQ(Return->getKind(), StmtKind::STMT_RETURN);

        delete AST;
    }

    TEST_F(ParserTest, FunctionBodyVar) {
        llvm::StringRef str = ("namespace std\n"
                         "bool func(int a) {\n"
                         "  bool c"
                         "  Type t"
                         "  float b=2.0 + 1.0"
                         "  a += 2"
                         "  c = (b == 1.0) && (true)"
                         "  return c\n"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // Test: bool c
        ASTLocalVar *cVar = static_cast<ASTLocalVar *>(Body->getContent()[0]);
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(cVar->getType()->getKind(), TypeKind::TYPE_BOOL);
        ASSERT_FALSE(cVar->getExpr());

        // Test: Type t
        ASTLocalVar *typeVar = static_cast<ASTLocalVar *>(Body->getContent()[1]);
        EXPECT_EQ(typeVar->getName(), "t");
        EXPECT_EQ(typeVar->getType()->getKind(), TypeKind::TYPE_CLASS);
        ClassTypeRef *TypeT = static_cast<ClassTypeRef *>(typeVar->getType());
        EXPECT_EQ(TypeT->getName(), "Type");

        // Test: float b=2.0 + 1.0
        ASTLocalVar *bVar = static_cast<ASTLocalVar *>(Body->getContent()[2]);
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getType()->getKind(), TypeKind::TYPE_FLOAT);
        FloatPrimType *FloatType = static_cast<FloatPrimType *>(bVar->getType());
        EXPECT_EQ(bVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        ASTGroupExpr *Group1 = static_cast<ASTGroupExpr *>(bVar->getExpr());
        EXPECT_EQ(Group1->getGroup()[0]->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ASTValueExpr *>(Group1->getGroup()[0])->getValue().str(), "2.0");
        EXPECT_EQ(Group1->getGroup()[1]->getKind(), ExprKind::EXPR_OPERATOR);
        EXPECT_EQ(static_cast<ASTOperatorExpr *>(Group1->getGroup()[1])->getOpKind(), OpKind::OP_ARITH);
        EXPECT_EQ(static_cast<ASTArithExpr *>(Group1->getGroup()[1])->getArithKind(), ArithOpKind::ARITH_ADD);
        EXPECT_EQ(Group1->getGroup()[2]->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ASTValueExpr *>(Group1->getGroup()[2])->getValue().str(), "1.0");

        // Test: a += 2
        const ASTLocalVarStmt *aVar = static_cast<ASTLocalVarStmt *>(Body->getContent()[3]);
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        ASTGroupExpr *Group2 = static_cast<ASTGroupExpr *>(aVar->getExpr());
        ASTVarRefExpr *aExpr = static_cast<ASTVarRefExpr *>(Group2->getGroup()[0]);
        EXPECT_TRUE(aExpr->getVarRef()->getDecl() != nullptr);
        ASTArithExpr *opCExpr = static_cast<ASTArithExpr *>(Group2->getGroup()[1]);
        EXPECT_EQ(opCExpr->getArithKind(), ArithOpKind::ARITH_ADD);
        ASTValueExpr *opC1Expr = static_cast<ASTValueExpr *>(Group2->getGroup()[2]);
        EXPECT_EQ(opC1Expr->getValue().str(), "2");

        // Test: c = b == 1.0
        const ASTLocalVarStmt *c2Var = static_cast<ASTLocalVarStmt *>(Body->getContent()[4]);
        EXPECT_EQ(c2Var->getName(), "c");
        EXPECT_EQ(c2Var->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        ASTGroupExpr *Group3 = static_cast<ASTGroupExpr *>(c2Var->getExpr());
        ASTVarRefExpr *c2Expr = static_cast<ASTVarRefExpr *>(Group3->getGroup()[0]);
        EXPECT_EQ(static_cast<ASTVarRef *>(c2Expr->getVarRef())->getName(), "b");
        ASTLogicExpr *opC2Expr = static_cast<ASTLogicExpr *>(Group3->getGroup()[1]);
        EXPECT_EQ(opC2Expr->getLogicKind(), LogicOpKind::LOGIC_EQ);

        const ASTReturn *Ret = static_cast<ASTReturn *>(Body->getContent()[5]);
        ASTVarRefExpr *RetRef = static_cast<ASTVarRefExpr *>(Ret->getExpr());
        EXPECT_EQ(RetRef->getVarRef()->getName(), "c");

        delete AST;
    }

    TEST_F(ParserTest, FunctionBodyCallFunc) {
        llvm::StringRef str = ("namespace std\n"
                               "private int doSome() {return 1}\n"
                               "public void doOther(int a, int b) {}\n"
                               "int do() {return 0}"
                               "int main(int a) {\n"
                               "  int b = doSome()"
                               "  doOther(a, 1)"
                               "  return do()"
                               "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get all functions
        ASTFunc *doSome = *AST->getFunctions().begin();
        ASTFunc *doOther = nullptr;
        ASTFunc *main = nullptr;

        for (auto *F : AST->getNameSpace()->getFunctions()) {
            if (F->getName().equals("doOther")) {
                doOther = F;
            } else if (F->getName().equals("main")) {
                main = F;
            }
        }
        ASSERT_TRUE(doSome != nullptr);
        ASSERT_TRUE(doOther != nullptr);
        ASSERT_TRUE(main != nullptr);

        // Get main() Body
        const ASTBlock *Body = main->getBody();

        // Test: doSome()
        ASTLocalVar *VarB = static_cast<ASTLocalVar *>(Body->getContent()[0]);
        ASTFuncCallExpr *doSomeCall = (ASTFuncCallExpr *) VarB->getExpr();
        EXPECT_EQ(doSomeCall->getCall()->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getKind(), ExprKind::EXPR_REF_FUNC);
        ASSERT_FALSE(doSomeCall->getCall()->getDecl() == nullptr);

        // Test: doOther(a, b)
        ASTFuncCallStmt *doOtherStmt = static_cast<ASTFuncCallStmt *>(Body->getContent()[1]);
        EXPECT_EQ(doOtherStmt->getKind(), StmtKind::STMT_FUNC_CALL);
        ASTFuncCall *doOtherCall = doOtherStmt->getCall();
        EXPECT_EQ(doOtherCall->getName(), "doOther");
        ASTVarRefExpr *VRefExpr = static_cast<ASTVarRefExpr *>(doOtherCall->getArgs()[0]->getValue());
        EXPECT_EQ(VRefExpr->getVarRef()->getName(), "a");
        ASTValueExpr *ValExpr = static_cast<ASTValueExpr *>(doOtherCall->getArgs()[1]->getValue());
        EXPECT_EQ(ValExpr->getValue().str(), "1");
        EXPECT_FALSE(doOtherCall->getDecl() == nullptr);

        ASTReturn *Ret = static_cast<ASTReturn *>(Body->getContent()[2]);
        ASTFuncCallExpr *RetCallEx = static_cast<ASTFuncCallExpr *>(Ret->getExpr());
        EXPECT_EQ(RetCallEx->getCall()->getName(), "do");
        EXPECT_EQ(RetCallEx->getKind(), ExprKind::EXPR_REF_FUNC);
        EXPECT_TRUE(RetCallEx->getCall()->getArgs().empty());

        delete AST;
    }

    TEST_F(ParserTest, FunctionBodyIncDec) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  ++a"
                         "  a++"
                         "  --a"
                         "  a--"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // ++a
        ASTLocalVarStmt *a1Var = static_cast<ASTLocalVarStmt *>(Body->getContent()[0]);
        EXPECT_EQ(a1Var->getExpr()->getKind(), ExprKind::EXPR_OPERATOR);
        EXPECT_EQ(a1Var->getName(), "a");

        ASTIncDecExpr *aExpr = static_cast<ASTIncDecExpr *>(a1Var->getExpr());
        EXPECT_EQ(aExpr->getIncDecKind(), IncDecOpKind::PRE_INCREMENT);

        // b++
        ASTLocalVarStmt *a2Var = static_cast<ASTLocalVarStmt *>(Body->getContent()[1]);
        EXPECT_EQ(a2Var->getExpr()->getKind(), ExprKind::EXPR_OPERATOR);
        EXPECT_EQ(a2Var->getName(), "a");
        ASTIncDecExpr *bExpr = static_cast<ASTIncDecExpr *>(a2Var->getExpr());
        EXPECT_EQ(bExpr->getIncDecKind(), IncDecOpKind::POST_INCREMENT);

        // ++c
        ASTLocalVarStmt *a3Var = static_cast<ASTLocalVarStmt *>(Body->getContent()[2]);
        EXPECT_EQ(a3Var->getExpr()->getKind(), ExprKind::EXPR_OPERATOR);
        EXPECT_EQ(a3Var->getName(), "a");
        ASTIncDecExpr *cExpr = static_cast<ASTIncDecExpr *>(a3Var->getExpr());
        EXPECT_EQ(cExpr->getIncDecKind(), IncDecOpKind::PRE_DECREMENT);

        // d++
        const ASTLocalVarStmt *a4Var = static_cast<ASTLocalVarStmt *>(Body->getContent()[3]);
        EXPECT_EQ(a4Var->getExpr()->getKind(), ExprKind::EXPR_OPERATOR);
        EXPECT_EQ(a4Var->getName(), "a");
        ASTIncDecExpr *dExpr = static_cast<ASTIncDecExpr *>(a4Var->getExpr());
        EXPECT_EQ(dExpr->getIncDecKind(), IncDecOpKind::POST_DECREMENT);

        delete AST;
    }

    TEST_F(ParserTest, FunctionBodyIfStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a, int b) {\n"
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
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // if
        ASTIfBlock *Stmt = static_cast<ASTIfBlock *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_IF);
        ASTGroupExpr *IfCond = static_cast<ASTGroupExpr *>(Stmt->getCondition());
        EXPECT_EQ(static_cast<ASTVarRefExpr *>(IfCond->getGroup()[0])->getVarRef()->getName(), "a");
        EXPECT_EQ(static_cast<ASTLogicExpr *>(IfCond->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ASTValueExpr *>(IfCond->getGroup()[2])->getValue().str(), "1");
        EXPECT_TRUE(static_cast<ASTReturn *>(Stmt->getContent()[0])->getExpr() == nullptr);
        EXPECT_FALSE(Stmt->getElsif().empty());
        EXPECT_TRUE(Stmt->getElse());

        // Elsif
        ElsifBlockStmt *EIStmt = static_cast<ElsifBlockStmt *>(Body->getContent()[1]);
        EXPECT_EQ(EIStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSIF);
        ASTGroupExpr *ElsifCond = static_cast<ASTGroupExpr *>(EIStmt->getCondition());
        EXPECT_EQ(static_cast<ASTVarRefExpr *>(ElsifCond->getGroup()[0])->getVarRef()->getName(), "a");
        EXPECT_EQ(static_cast<ASTLogicExpr *>(ElsifCond->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ASTValueExpr *>(ElsifCond->getGroup()[2])->getValue().str(), "2");
        EXPECT_EQ(static_cast<ASTLocalVarStmt *>(EIStmt->getContent()[0])->getName(), "b");

        // Else
        ElseBlockStmt *EEStmt = static_cast<ElseBlockStmt *>(Body->getContent()[2]);
        EXPECT_EQ(EEStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSE);
        EXPECT_EQ(static_cast<ASTLocalVarStmt *>(EEStmt->getContent()[0])->getName(), "b");

        delete AST;
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
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // if
        ASTIfBlock *Stmt = static_cast<ASTIfBlock *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_IF);
        ASTGroupExpr *IfCond = static_cast<ASTGroupExpr *>(Stmt->getCondition());
        EXPECT_EQ(static_cast<ASTVarRefExpr *>(IfCond->getGroup()[0])->getVarRef()->getName(), "a");
        EXPECT_EQ(static_cast<ASTLogicExpr *>(IfCond->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ASTValueExpr *>(IfCond->getGroup()[2])->getValue().str(), "1");
        EXPECT_TRUE(static_cast<ASTReturn *>(Stmt->getContent()[0])->getExpr() == nullptr);

        EXPECT_FALSE(Stmt->getElsif().empty());
        EXPECT_TRUE(Stmt->getElse());

        // Elsif
        ElsifBlockStmt *EIStmt = static_cast<ElsifBlockStmt *>(Body->getContent()[1]);
        EXPECT_EQ(EIStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSIF);
        ASTGroupExpr *ElsifCond = static_cast<ASTGroupExpr *>(EIStmt->getCondition());
        EXPECT_EQ(static_cast<ASTVarRefExpr *>(ElsifCond->getGroup()[0])->getVarRef()->getName(), "a");
        EXPECT_EQ(static_cast<ASTLogicExpr *>(ElsifCond->getGroup()[1])->getLogicKind(),
                  LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ASTValueExpr *>(ElsifCond->getGroup()[2])->getValue().str(), "2");

        // Else
        ElseBlockStmt *EEStmt = static_cast<ElseBlockStmt *>(Body->getContent()[2]);
        EXPECT_EQ(EEStmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_ELSE);
        EXPECT_EQ(static_cast<ASTLocalVarStmt *>(EEStmt->getContent()[0])->getName(), "a");

        delete AST;
    }

    TEST_F(ParserTest, FunctionBodySwitchStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "private void func(int a) {\n"
                         "  switch (a) {"
                         "    case 1:"
                         "      break"
                         "    case 2:"
                         "    default:"
                         "      return"
                         "  }"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get Body
        ASTFunc *F = *(AST->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        ASTSwitchBlock *Stmt = static_cast<ASTSwitchBlock *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_SWITCH);
        EXPECT_EQ(static_cast<ASTValueExpr *>(Stmt->getCases()[0]->getExpr())->getValue().str(), "1");
        EXPECT_EQ(Stmt->getCases()[0]->getContent()[0]->getKind(), StmtKind::STMT_BREAK);
        EXPECT_EQ(static_cast<ASTValueExpr *>(Stmt->getCases()[1]->getExpr())->getValue().str(), "2");
        EXPECT_TRUE(Stmt->getCases()[1]->getContent().empty());
        EXPECT_EQ(Stmt->getDefault()->getBlockKind(), BlockStmtKind::BLOCK_STMT_DEFAULT);
        EXPECT_EQ(static_cast<ASTFuncCallStmt *>(Stmt->getDefault()->getContent()[0])->getKind(), StmtKind::STMT_RETURN);

        delete AST;
    }

    TEST_F(ParserTest, FunctionBodyForStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "private void func(int a) {\n"
                         "  for int b = 1, int c = 2; b < 10; b++, --c {"
                         "  }"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get Body
        ASTFunc *F = *(AST->getFunctions().begin());
        const ASTBlock *Body = F->getBody();
        ASTForBlock *Stmt = static_cast<ASTForBlock *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_FOR);

        EXPECT_EQ(static_cast<ASTLocalVar *>(Stmt->getInit()->getContent()[0])->getName(), "b");
        EXPECT_EQ(static_cast<ASTLocalVar *>(Stmt->getInit()->getContent()[1])->getName(), "c");

        ASTGroupExpr *Cond = static_cast<ASTGroupExpr *>(Stmt->getCondition());
        EXPECT_EQ(static_cast<ASTVarRefExpr *>(Cond->getGroup()[0])->getVarRef()->getName(), "b");
        EXPECT_EQ(static_cast<ASTLogicExpr *>(Cond->getGroup()[1])->getLogicKind(), LogicOpKind::LOGIC_LT);
        EXPECT_EQ(static_cast<ASTValueExpr *>(Cond->getGroup()[2])->getValue().str(), "10");

        EXPECT_EQ(static_cast<ASTLocalVarStmt *>(Stmt->getPost()->getContent()[0])->getName(), "b");
        ASTIncDecExpr *Expr1 = static_cast<ASTIncDecExpr *>(static_cast<ASTLocalVarStmt *>(Stmt->getPost()->getContent()[0])->getExpr());
        EXPECT_EQ(Expr1->getIncDecKind(), IncDecOpKind::POST_INCREMENT);
        EXPECT_EQ(static_cast<ASTLocalVarStmt *>(Stmt->getPost()->getContent()[1])->getName(), "c");
        ASTIncDecExpr *Expr2 = static_cast<ASTIncDecExpr *>(static_cast<ASTLocalVarStmt *>(Stmt->getPost()->getContent()[1])->getExpr());
        EXPECT_EQ(Expr2->getIncDecKind(), IncDecOpKind::PRE_DECREMENT);

        EXPECT_TRUE(Stmt->getLoop()->isEmpty());

        delete AST;
    }

    TEST_F(ParserTest, FunctionBodyWhileStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "private void func(int a) {\n"
                         "  while (a==1) {}"
                         "  while {}"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();
        ASSERT_FALSE(AST->getContext().hasErrors());

        // Get Body
        ASTFunc *F = *(AST->getFunctions().begin());
        const ASTBlock *Body = F->getBody();
        ASTWhileBlock *Stmt = static_cast<ASTWhileBlock *>(Body->getContent()[0]);
        EXPECT_EQ(Stmt->getBlockKind(), BlockStmtKind::BLOCK_STMT_WHILE);
        EXPECT_FALSE(Stmt->getCondition() == nullptr);
        EXPECT_TRUE(Stmt->getLoop()->isEmpty());

        const ASTGroupExpr *Cond = static_cast<ASTGroupExpr *>(Stmt->getCondition());
        EXPECT_EQ(static_cast<ASTVarRefExpr *>(Cond->getGroup()[0])->getVarRef()->getName(), "a");
        EXPECT_EQ(static_cast<ASTLogicExpr *>(Cond->getGroup()[1])->getLogicKind(), LogicOpKind::LOGIC_EQ);
        EXPECT_EQ(static_cast<ASTValueExpr *>(Cond->getGroup()[2])->getValue().str(), "1");

        ASTWhileBlock *Stmt2 = static_cast<ASTWhileBlock *>(Body->getContent()[1]);
        EXPECT_EQ(Stmt2->getBlockKind(), BlockStmtKind::BLOCK_STMT_WHILE);
        EXPECT_TRUE(Stmt2->getCondition() == nullptr);
        EXPECT_TRUE(Stmt2->getLoop()->isEmpty());

        delete AST;
    }
}