//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Frontend/FrontendAction.h"
#include "Parser/Parser.h"
#include "Frontend/CompilerInstance.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTImport.h"
#include "AST/ASTValue.h"
#include <AST/ASTWhileBlock.h>
#include <unordered_set>
#include <gtest/gtest.h>

namespace {
    using namespace fly;

    class ParserTest : public ::testing::Test {

    public:
        const CompilerInstance CI;
        ASTContext *Context;
        CodeGen *CG;
        DiagnosticsEngine &Diags;

        ParserTest() : CI(*TestUtils::CreateCompilerInstance()),
                      Context(new ASTContext(CI.getDiagnostics())),
                      CG(TestUtils::CreateCodeGen(CI)),
                      Diags(CI.getDiagnostics()) {

        }

        ASTNode *Parse(std::string FileName, llvm::StringRef Source) {
            InputFile Input(Diags, CI.getSourceManager(), FileName);
            Input.Load(Source);
            FrontendAction *Action = new FrontendAction(CI, Context, *CG, &Input);
            Action->Parse();
            return Action->getAST();
        }

    };

    TEST_F(ParserTest, SingleNameSpace) {
        llvm::StringRef str = ("namespace std");
        ASTNode *AST = Parse("SingleNameSpace", str);
        ASSERT_FALSE(Diags.hasErrorOccurred());

        EXPECT_EQ(AST->getName(), "SingleNameSpace");

        // verify AST contains package
        EXPECT_EQ(AST->getNameSpace()->getName(), "std");
        delete AST;
    }

    TEST_F(ParserTest, MultiNamespaceError) {
        llvm::StringRef str = ("namespace std\n"
                         "namespace bad");
        ASTNode *AST = Parse("MultiNamespaceError", str);
        EXPECT_TRUE(Diags.hasErrorOccurred());
    }

    TEST_F(ParserTest, SingleImport) {
        llvm::StringRef str = ("namespace std\n"
                         "import packageA");
        ASTNode *AST = Parse("SingleImport", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        ASTImport* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
        EXPECT_EQ(Verify->getAlias(), "");
        delete AST;
    }

    TEST_F(ParserTest, SingleImportAlias) {
        llvm::StringRef str = ("\n import standard std\n");
        ASTNode *AST = Parse("SingleImportAlias", str);

        EXPECT_EQ(AST->getNameSpace()->getName(), "default");
        ASTImport *Import = AST->getImports().lookup("std");
        EXPECT_EQ(Import->getName(), "standard");
        EXPECT_EQ(Import->getAlias(), "std");
        delete AST;
    }

    TEST_F(ParserTest, MultiImports) {
        llvm::StringRef str = ("namespace std\n"
                         "import packageA "
                         "import packageB");
        ASTNode *AST = Parse("MultiImports", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        ASTImport* VerifyB = AST->getImports().lookup("packageB");
        ASTImport* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
        delete AST;
    }

    TEST_F(ParserTest,  LineComments) {
        llvm::StringRef str = ("namespace std\n"
                               "// Global var comment\n"
                               "int b\n"
                               "// Func comment\n"
                               "void func() {}\n"
        );
        ASTNode *AST = Parse("LineComments", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        ASTGlobalVar *GlobalB = AST->getGlobalVars().find("b")->getValue();
        EXPECT_EQ(GlobalB->getName(), "b");
        EXPECT_EQ(GlobalB->getComment(), "");

        const ASTFunc *Func = *AST->getFunctions().begin();
        EXPECT_EQ(Func->getName(), "func");
        EXPECT_EQ(Func->getComment(), "");
    }

    TEST_F(ParserTest, BlockComments) {
        llvm::StringRef str = ("namespace std\n"
                               " /* Global var block comment */\n"
                               "// Global var line comment\n"
                               "int b\n"
                               "// Func line comment\n"
                               "\t /*   Func block comment \n*/\n"
                               "void func() {\n"
                               "  /* body comment */\n"
                               "}\n"
                               "//body comment\n"
                               "void func2() {}\n"
        );
        ASTNode *AST = Parse("BlockComments", str);
        ASSERT_FALSE(Diags.hasErrorOccurred());

        ASTGlobalVar *GlobalB = AST->getGlobalVars().find("b")->getValue();
        EXPECT_EQ(GlobalB->getName(), "b");
        EXPECT_EQ(GlobalB->getComment(), "Global var block comment");

        ASTFunc *Func, *Func2;
        for (auto F : AST->getFunctions()) {
            if (F->getName() == "func") {
                Func = F;
            } else {
                Func2 = F;
            }
        }
        EXPECT_EQ(Func->getName(), "func");
        EXPECT_EQ(Func->getComment(), "Func block comment");

        EXPECT_EQ(Func2->getName(), "func2");
        EXPECT_EQ(Func2->getComment(), "");
    }

    TEST_F(ParserTest, GlobalVars) {
        llvm::StringRef str = ("namespace std\n"
                        "private int a\n"
                        "public float b\n"
                        "bool c\n"
                        "long d\n"
                        "double e\n"
                        "byte f\n"
                        "ushort g\n"
                        "short h\n"
                        "uint i\n"
                        "ulong j\n"
                         );
        ASTNode *AST = Parse("GlobalVars", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        ASTGlobalVar *VerifyA = AST->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *VerifyB = AST->getNameSpace()->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *VerifyC = AST->getGlobalVars().find("c")->getValue();
        ASTGlobalVar *VerifyD = AST->getGlobalVars().find("d")->getValue();
        ASTGlobalVar *VerifyE = AST->getGlobalVars().find("e")->getValue();
        ASTGlobalVar *VerifyF = AST->getGlobalVars().find("f")->getValue();
        ASTGlobalVar *VerifyG = AST->getGlobalVars().find("g")->getValue();
        ASTGlobalVar *VerifyH = AST->getGlobalVars().find("h")->getValue();
        ASTGlobalVar *VerifyI = AST->getGlobalVars().find("i")->getValue();
        ASTGlobalVar *VerifyJ = AST->getGlobalVars().find("j")->getValue();

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

        EXPECT_EQ(VerifyD->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyD->isConstant());
        EXPECT_EQ(VerifyD->getType()->getKind(), TypeKind::TYPE_LONG);
        EXPECT_EQ(VerifyD->getName(), "d");

        EXPECT_EQ(VerifyE->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyE->isConstant());
        EXPECT_EQ(VerifyE->getType()->getKind(), TypeKind::TYPE_DOUBLE);
        EXPECT_EQ(VerifyE->getName(), "e");

        EXPECT_EQ(VerifyF->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyF->isConstant());
        EXPECT_EQ(VerifyF->getType()->getKind(), TypeKind::TYPE_BYTE);
        EXPECT_EQ(VerifyF->getName(), "f");

        EXPECT_EQ(VerifyG->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyG->isConstant());
        EXPECT_EQ(VerifyG->getType()->getKind(), TypeKind::TYPE_USHORT);
        EXPECT_EQ(VerifyG->getName(), "g");

        EXPECT_EQ(VerifyH->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyH->isConstant());
        EXPECT_EQ(VerifyH->getType()->getKind(), TypeKind::TYPE_SHORT);
        EXPECT_EQ(VerifyH->getName(), "h");

        EXPECT_EQ(VerifyI->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyI->isConstant());
        EXPECT_EQ(VerifyI->getType()->getKind(), TypeKind::TYPE_UINT);
        EXPECT_EQ(VerifyI->getName(), "i");

        EXPECT_EQ(VerifyJ->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyJ->isConstant());
        EXPECT_EQ(VerifyJ->getType()->getKind(), TypeKind::TYPE_ULONG);
        EXPECT_EQ(VerifyJ->getName(), "j");

        delete AST;
    }

    TEST_F(ParserTest, GlobalConstants) {
        llvm::StringRef str = ("namespace std\n"
                         "private const int a = 1\n"
                         "const public float b = 2.0\n"
                         "const bool c = false\n"
                         );
        ASTNode *AST = Parse("GlobalConstants", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        ASTGlobalVar *VerifyA = AST->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *VerifyB = AST->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *VerifyC = AST->getGlobalVars().find("c")->getValue();

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_EQ(VerifyA->isConstant(), true);
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");
        EXPECT_EQ(((ASTValueExpr *)VerifyA->getExpr())->getValue().str(), "1");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_EQ(VerifyB->isConstant(), true);
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");
        EXPECT_EQ(((ASTValueExpr *)VerifyB->getExpr())->getValue().str(), "2.0");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        EXPECT_EQ(VerifyC->isConstant(), true);
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");
        EXPECT_EQ(((ASTValueExpr *)VerifyC->getExpr())->getValue().str(), "false");

        delete AST;
    }

    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
        llvm::StringRef str = ("namespace std\n"
                         "void func() {}\n");
        ASTNode *AST = Parse("FunctionDefaultVoidEmpty", str);
        ASSERT_FALSE(Diags.hasErrorOccurred());

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
        ASTNode *AST = Parse("FunctionPrivateReturnParams", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

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
        EXPECT_EQ(Par2->getExpr()->getKind(), ASTExprKind::EXPR_VALUE);
        ASTValueExpr *DefArg2 = (ASTValueExpr *)Par2->getExpr();
        EXPECT_EQ(DefArg2->getValue().str(), "false");

        ASTReturn *Return = (ASTReturn *) VerifyFunc->getBody()->getContent()[0];
        EXPECT_EQ(((ASTValueExpr *) Return->getExpr())->getKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr*) Return->getExpr())->getValue().str(), "1");
        EXPECT_EQ(Return->getKind(), StmtKind::STMT_RETURN);

        delete AST;
    }

    TEST_F(ParserTest, BoolDefaultVarReturn) {
        llvm::StringRef str = ("bool func() {\n"
                                 "  bool c"
                                 "  return c\n"
                                 "}\n");
        ASTNode *AST = Parse("BoolDefaultVarReturn", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        EXPECT_EQ(F->getType()->getKind(), TypeKind::TYPE_BOOL);
        const ASTBlock *Body = F->getBody();

        // Test: bool c
        ASTLocalVar *cVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(cVar->getType()->getKind(), TypeKind::TYPE_BOOL);
        ASSERT_EQ(((ASTValueExpr *) cVar->getExpr())->getValue().str(), "false");

        const ASTReturn *Ret = (ASTReturn *) Body->getContent()[1];
        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetRef->getVarRef()->getName(), "c");

        delete AST;
    }

    TEST_F(ParserTest, IntDefaultVarReturn) {
        llvm::StringRef str = ("int func() {\n"
                               "  int c"
                               "  return c\n"
                               "}\n");
        ASTNode *AST = Parse("IntDefaultVarReturn", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        EXPECT_EQ(F->getType()->getKind(), TypeKind::TYPE_INT);
        const ASTBlock *Body = F->getBody();

        // Test: int c
        ASTLocalVar *cVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(cVar->getType()->getKind(), TypeKind::TYPE_INT);
        ASSERT_EQ(((ASTValueExpr *) cVar->getExpr())->getValue().str(), "0");

        const ASTReturn *Ret = (ASTReturn *) Body->getContent()[1];
        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetRef->getVarRef()->getName(), "c");

        delete AST;
    }

    TEST_F(ParserTest, FloatDefaultVarReturn) {
        llvm::StringRef str = ("float func() {\n"
                               "  float c"
                               "  return c\n"
                               "}\n");
        ASTNode *AST = Parse("FloatDefaultVarReturn", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        EXPECT_EQ(F->getType()->getKind(), TypeKind::TYPE_FLOAT);
        const ASTBlock *Body = F->getBody();

        // Test: int c
        ASTLocalVar *cVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(cVar->getType()->getKind(), TypeKind::TYPE_FLOAT);
        ASSERT_EQ(((ASTValueExpr *) cVar->getExpr())->getValue().str(), "0");

        const ASTReturn *Ret = (ASTReturn *) Body->getContent()[1];
        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetRef->getVarRef()->getName(), "c");

        delete AST;
    }

    TEST_F(ParserTest, TypeDefaultVarReturn) {
        llvm::StringRef str = ("Type func() {\n"
                               "  Type t"
                               "  return t\n"
                               "}\n");
        ASTNode *AST = Parse("TypeDefaultVarReturn", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        EXPECT_EQ(F->getType()->getKind(), TypeKind::TYPE_CLASS);
        const ASTBlock *Body = F->getBody();

        // Test: Type t
        ASTLocalVar *typeVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(typeVar->getName(), "t");
        EXPECT_EQ(typeVar->getType()->getKind(), TypeKind::TYPE_CLASS);
        ASTClassType *TypeT = (ASTClassType *) typeVar->getType();
        EXPECT_EQ(TypeT->getName(), "Type");
        // ASSERT_EQ(((ASTValueExpr *) cVar->getExpr())->getValue().str(), "false"); TODO nullptr test

        const ASTReturn *Ret = (ASTReturn *) Body->getContent()[1];
        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetRef->getVarRef()->getName(), "t");

        delete AST;
    }

    TEST_F(ParserTest, IntBinaryArithOperation) {
        llvm::StringRef str = ("int func() {\n"
                               "  int a += 2\n"
                               "  int b = a + b / a - b\n"
                               "  return a\n"
                               "}\n");
        ASTNode *AST = Parse("IntBinaryArithOperation", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // Test: int a += 2

        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) aVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *aGroup = (ASTBinaryGroupExpr *) aVar->getExpr();
        EXPECT_EQ(((ASTVarRefExpr *) aGroup->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(aGroup->getOperatorKind(), BinaryOpKind::ARITH_ADD);
        EXPECT_EQ(((ASTValueExpr *) aGroup->getSecond())->getValue().str(), "2");

        // Test: int b = a + b / a - b

        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 - b
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        EXPECT_EQ(bGroup->getOperatorKind(), BinaryOpKind::ARITH_SUB);
        EXPECT_EQ(((ASTVarRefExpr *) bGroup->getSecond())->getVarRef()->getName(), "b");

        // G1 = a + G2
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), BinaryOpKind::ARITH_ADD);
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) G1->getSecond();

        // G2 = b / a
        const ASTVarRefExpr *b = (ASTVarRefExpr *) G2->getFirst();
        EXPECT_EQ(G2->getOperatorKind(), BinaryOpKind::ARITH_DIV);
        const ASTVarRefExpr *a = (ASTVarRefExpr *) G2->getSecond();

        delete AST;
    }

    TEST_F(ParserTest, FloatBinaryArithOperation) {
        llvm::StringRef str = ("float func() {\n"
                               "  float a -= 1.0"
                               "  float b = a * b - a / b"
                               "  return b\n"
                               "}\n");
        ASTNode *AST = Parse("FloatBinaryArithOperation", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // Test: float a -= 1.0
        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) aVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *aGroup = (ASTBinaryGroupExpr *) aVar->getExpr();
        EXPECT_EQ(((ASTVarRefExpr *) aGroup->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(aGroup->getOperatorKind(), BinaryOpKind::ARITH_SUB);
        EXPECT_EQ(((ASTValueExpr *) aGroup->getSecond())->getValue().str(), "1.0");

        // Test: int b = a * b - a / b
        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 - G2
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) bGroup->getSecond();
        EXPECT_EQ(bGroup->getOperatorKind(), BinaryOpKind::ARITH_SUB);

        // G1 = a * b
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), BinaryOpKind::ARITH_MUL);
        EXPECT_EQ(((ASTVarRefExpr *) G1->getSecond())->getVarRef()->getName(), "b");

        // G2 = a / b
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), BinaryOpKind::ARITH_DIV);
        EXPECT_EQ(((ASTVarRefExpr *) G2->getSecond())->getVarRef()->getName(), "b");

        delete AST;
    }

    TEST_F(ParserTest, BoolBinaryLogicOperation) {
        llvm::StringRef str = ("bool func() {\n"
                               "  bool a &= true"
                               "  bool b = a || false && a == true"
                               "  return c\n"
                               "}\n");
        ASTNode *AST = Parse("FloatBinaryArithOperation", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // Test: bool a = true
        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) aVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *aGroup = (ASTBinaryGroupExpr *) aVar->getExpr();
        EXPECT_EQ(((ASTVarRefExpr *) aGroup->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(aGroup->getOperatorKind(), BinaryOpKind::ARITH_AND);
        EXPECT_EQ(((ASTValueExpr *) aGroup->getSecond())->getValue().str(), "true");

        // Test: bool b = a || false && a == true
        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 == true
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        EXPECT_EQ(bGroup->getOperatorKind(), BinaryOpKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) bGroup->getSecond())->getValue().str(), "true");

        // G1 = G2 && a
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) G1->getFirst();
        EXPECT_EQ(G1->getOperatorKind(), BinaryOpKind::LOGIC_AND);
        EXPECT_EQ(((ASTValueExpr *) G2->getSecond())->getValue().str(), "false");

        // G2 = a || false
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), BinaryOpKind::LOGIC_OR);
        EXPECT_EQ(((ASTValueExpr *) G2->getSecond())->getValue().str(), "false");

        delete AST;
    }

    TEST_F(ParserTest, LongBinaryArithOperation) {
        llvm::StringRef str = ("long func() {\n"
                               "  long a = 1\n"
                               "  long b = (a + b) / (a - b)\n"
                               "  return b\n"
                               "}\n");
        ASTNode *AST = Parse("IntBinaryArithOperation", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // Test: long a = 1

        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue().str(), "1");

        // Test: long b = (a + b) / (a - b)

        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // long b = G1 / G2
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) bGroup->getSecond();
        EXPECT_EQ(bGroup->getOperatorKind(), BinaryOpKind::ARITH_DIV);

        // G1 = a + b
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), BinaryOpKind::ARITH_ADD);
        EXPECT_EQ(((ASTVarRefExpr *) G1->getSecond())->getVarRef()->getName(), "b");

        // G2 = a - b
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), BinaryOpKind::ARITH_SUB);
        EXPECT_EQ(((ASTVarRefExpr *) G2->getSecond())->getVarRef()->getName(), "b");

        delete AST;
    }

    TEST_F(ParserTest, CondTernaryOperation) {
        llvm::StringRef str = ("int func(bool a) {\n"
                               "  return a==1 ? 1 : a\n"
                               "}\n");
        ASTNode *AST = Parse("CondTernaryOperation", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        ASTFuncParam *a = F->getHeader()->getParams()[0];
        const ASTBlock *Body = F->getBody();

        ASTReturn *Ret = (ASTReturn *) Body->getContent()[0];
        ASTTernaryGroupExpr *Expr = (ASTTernaryGroupExpr *) Ret->getExpr();
        ASTBinaryGroupExpr *Comp = ((ASTBinaryGroupExpr *) Expr->getFirst());
        EXPECT_EQ(Comp->getOperatorKind(), BinaryOpKind::COMP_EQ);
        EXPECT_EQ(((ASTVarRefExpr *) Comp->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(((ASTValueExpr *) Comp->getSecond())->getValue().str(), "1");
        EXPECT_EQ(((ASTValueExpr *) Expr->getSecond())->getValue().str(), "1");
        EXPECT_EQ(((ASTVarRefExpr *) Expr->getThird())->getVarRef()->getName(), "a");

        delete AST;
    }

    TEST_F(ParserTest, FunctionCall) {
        llvm::StringRef str = ("namespace std\n"
                               "private int doSome() {return 1}\n"
                               "public void doOther(int a, int b) {}\n"
                               "int do() {return 0}"
                               "int main(int a) {\n"
                               "  int b = doSome()"
                               "  doOther(a, 1)"
                               "  return do()"
                               "}\n");
        ASTNode *AST = Parse("FunctionBodyCallFunc", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get all functions
        ASTFunc *doSome = *AST->getFunctions().begin();
        ASTFunc *doOther = nullptr;
        ASTFunc *main = nullptr;

        for (auto *F : AST->getNameSpace()->getFunctions()) {
            if (F->getName() == "doOther") {
                doOther = F;
            } else if (F->getName() == "main") {
                main = F;
            }
        }
        ASSERT_TRUE(doSome != nullptr);
        ASSERT_TRUE(doOther != nullptr);
        ASSERT_TRUE(main != nullptr);

        // Get main() Body
        const ASTBlock *Body = main->getBody();

        // Test: doSome()
        ASTLocalVar *VarB = ((ASTLocalVar *) Body->getContent()[0]);
        ASTFuncCallExpr *doSomeCall = (ASTFuncCallExpr *) VarB->getExpr();
        EXPECT_EQ(doSomeCall->getCall()->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getKind(), ASTExprKind::EXPR_REF_FUNC);
        ASSERT_FALSE(doSomeCall->getCall()->getDecl() == nullptr);

        // Test: doOther(a, b)
        ASTExprStmt *doOtherStmt = (ASTExprStmt *) Body->getContent()[1];
        EXPECT_EQ(doOtherStmt->getKind(), StmtKind::STMT_EXPR);
        ASTFuncCallExpr *doOtherCall = (ASTFuncCallExpr *) doOtherStmt->getExpr();
        EXPECT_EQ(doOtherCall->getCall()->getName(), "doOther");
        ASTVarRefExpr *VRefExpr = (ASTVarRefExpr *) doOtherCall->getCall()->getArgs()[0]->getValue();
        EXPECT_EQ(VRefExpr->getVarRef()->getName(), "a");
        ASTValueExpr *ValExpr = (ASTValueExpr *) doOtherCall->getCall()->getArgs()[1]->getValue();
        EXPECT_EQ(ValExpr->getValue().str(), "1");

        // return do()
        ASTReturn *Ret = (ASTReturn *) Body->getContent()[2];
        ASTFuncCallExpr *RetCallEx = (ASTFuncCallExpr *) Ret->getExpr();
        EXPECT_EQ(RetCallEx->getCall()->getName(), "do");
        EXPECT_EQ(RetCallEx->getKind(), ASTExprKind::EXPR_REF_FUNC);
        EXPECT_TRUE(RetCallEx->getCall()->getArgs().empty());

        delete AST;
    }

    TEST_F(ParserTest, UnaryExpr) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  ++a"
                         "  a++"
                         "  --a"
                         "  a--"
                         "  a = ++a + 1"
                         "}\n");
        ASTNode *AST = Parse("FunctionBodyIncDec", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // ++a
        ASTExprStmt *a1Stmt = (ASTExprStmt *) Body->getContent()[0];
        ASTUnaryGroupExpr *a1Unary = (ASTUnaryGroupExpr *) a1Stmt->getExpr();
        EXPECT_EQ(a1Unary->getOperatorKind(), UnaryOpKind::ARITH_INCR);
        EXPECT_EQ(a1Unary->getOptionKind(), UnaryOptionKind::UNARY_PRE);
        EXPECT_EQ(((ASTVarRefExpr *) a1Unary->getFirst())->getVarRef()->getName(), "a");

        // a++
        ASTExprStmt *a2Stmt = (ASTExprStmt *) Body->getContent()[1];
        ASTUnaryGroupExpr *a2Unary = (ASTUnaryGroupExpr *) a2Stmt->getExpr();
        EXPECT_EQ(a2Unary->getOperatorKind(), UnaryOpKind::ARITH_INCR);
        EXPECT_EQ(a2Unary->getOptionKind(), UnaryOptionKind::UNARY_POST);
        EXPECT_EQ(((ASTVarRefExpr *) a2Unary->getFirst())->getVarRef()->getName(), "a");

        // --a
        ASTExprStmt *a3Stmt = (ASTExprStmt *) Body->getContent()[2];
        ASTUnaryGroupExpr *a3Unary = (ASTUnaryGroupExpr *) a3Stmt->getExpr();
        EXPECT_EQ(a3Unary->getOperatorKind(), UnaryOpKind::ARITH_DECR);
        EXPECT_EQ(a3Unary->getOptionKind(), UnaryOptionKind::UNARY_PRE);
        EXPECT_EQ(((ASTVarRefExpr *) a3Unary->getFirst())->getVarRef()->getName(), "a");

        // a--
        ASTExprStmt *a4Stmt = (ASTExprStmt *) Body->getContent()[3];
        ASTUnaryGroupExpr *a4Unary = (ASTUnaryGroupExpr *) a4Stmt->getExpr();
        EXPECT_EQ(a4Unary->getOperatorKind(), UnaryOpKind::ARITH_DECR);
        EXPECT_EQ(a4Unary->getOptionKind(), UnaryOptionKind::UNARY_POST);
        EXPECT_EQ(((ASTVarRefExpr *) a4Unary->getFirst())->getVarRef()->getName(), "a");

        // a = ++a + 1
        const ASTLocalVarRef *a5Var = (ASTLocalVarRef *) Body->getContent()[4];
        EXPECT_EQ(a5Var->getExpr()->getKind(), EXPR_GROUP);
        ASTBinaryGroupExpr *Group = (ASTBinaryGroupExpr *) a5Var->getExpr();
        EXPECT_EQ(Group->getOperatorKind(), BinaryOpKind::ARITH_ADD);
        EXPECT_EQ(Group->getOptionKind(), BinaryOptionKind::BINARY_ARITH);
        const ASTUnaryGroupExpr *E1 = (ASTUnaryGroupExpr *) Group->getFirst();
        EXPECT_EQ(E1->getOperatorKind(), UnaryOpKind::ARITH_INCR);
        EXPECT_EQ(E1->getOptionKind(), UnaryOptionKind::UNARY_PRE);
        ASTValueExpr *ValueExpr = (ASTValueExpr *) Group->getSecond();
        EXPECT_EQ(ValueExpr->getKind(), EXPR_VALUE);
        EXPECT_EQ(ValueExpr->getValue().str(), "1");
        delete AST;
    }

    TEST_F(ParserTest, IfElsifElseStmt) {
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
        ASTNode *AST = Parse("FunctionBodyIfStmt", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // if
        ASTIfBlock *Stmt = (ASTIfBlock *) Body->getContent()[0];
        EXPECT_EQ(Stmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_IF);
        ASTBinaryGroupExpr *IfCond = (ASTBinaryGroupExpr *) Stmt->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) IfCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(IfCond->getOperatorKind(),BinaryOpKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) IfCond->getSecond())->getValue().str(), "1");
        EXPECT_TRUE(((ASTReturn *) Stmt->getContent()[0])->getExpr() == nullptr);
        EXPECT_FALSE(Stmt->getElsif().empty());
        EXPECT_TRUE(Stmt->getElse());

        // Elsif
        ASTElsifBlock *EIStmt = (ASTElsifBlock *) Body->getContent()[1];
        EXPECT_EQ(EIStmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_ELSIF);
        ASTBinaryGroupExpr *ElsifCond = (ASTBinaryGroupExpr *) EIStmt->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(ElsifCond->getOperatorKind(), BinaryOpKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) ElsifCond->getSecond())->getValue().str(), "2");
        EXPECT_EQ(((ASTLocalVarRef *) EIStmt->getContent()[0])->getName(), "b");

        // Else
        ASTElseBlock *EEStmt = (ASTElseBlock *) Body->getContent()[2];
        EXPECT_EQ(EEStmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_ELSE);
        EXPECT_EQ(((ASTLocalVarRef *)EEStmt->getContent()[0])->getName(), "b");

        delete AST;
    }

    TEST_F(ParserTest, IfElsifElseInlineStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  if (a == 1) return"
                         "  elsif a == 2 a = 1"
                         "  else a = 2"
                         "}\n");
        ASTNode *AST = Parse("FunctionBodyIfInlineStmt", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getNameSpace()->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        // if
        ASTIfBlock *Stmt = (ASTIfBlock *) Body->getContent()[0];
        EXPECT_EQ(Stmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_IF);
        ASTBinaryGroupExpr *IfCond = (ASTBinaryGroupExpr *) Stmt->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) IfCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(IfCond->getOperatorKind(), BinaryOpKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) IfCond->getSecond())->getValue().str(), "1");
        EXPECT_TRUE(((ASTReturn *) Stmt->getContent()[0])->getExpr() == nullptr);

        EXPECT_FALSE(Stmt->getElsif().empty());
        EXPECT_TRUE(Stmt->getElse());

        // Elsif
        ASTElsifBlock *EIStmt = (ASTElsifBlock *) Body->getContent()[1];
        EXPECT_EQ(EIStmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_ELSIF);
        ASTBinaryGroupExpr *ElsifCond = (ASTBinaryGroupExpr *) EIStmt->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(ElsifCond->getOperatorKind(), BinaryOpKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) ElsifCond->getSecond())->getValue().str(), "2");

        // Else
        ASTElseBlock *EEStmt = (ASTElseBlock *) Body->getContent()[2];
        EXPECT_EQ(EEStmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_ELSE);
        EXPECT_EQ(((ASTLocalVarRef *) EEStmt->getContent()[0])->getName(), "a");

        delete AST;
    }

    TEST_F(ParserTest, SwitchCaseDefaultStmt) {
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
        ASTNode *AST = Parse("FunctionBodySwitchStmt", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getFunctions().begin());
        const ASTBlock *Body = F->getBody();

        ASTSwitchBlock *Stmt = (ASTSwitchBlock *) Body->getContent()[0];
        EXPECT_EQ(Stmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_SWITCH);
        EXPECT_EQ(((ASTValueExpr *) Stmt->getCases()[0]->getExpr())->getValue().str(), "1");
        EXPECT_EQ(Stmt->getCases()[0]->getContent()[0]->getKind(), StmtKind::STMT_BREAK);
        EXPECT_EQ(((ASTValueExpr *) Stmt->getCases()[1]->getExpr())->getValue().str(), "2");
        EXPECT_TRUE(Stmt->getCases()[1]->getContent().empty());
        EXPECT_EQ(Stmt->getDefault()->getBlockKind(), ASTBlockKind::BLOCK_STMT_DEFAULT);
        EXPECT_EQ((Stmt->getDefault()->getContent()[0])->getKind(), StmtKind::STMT_RETURN);

        delete AST;
    }

    TEST_F(ParserTest, ForStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "private void func(int a) {\n"
                         "  for int b = 1, int c = 2; b < 10; b++, --c {"
                         "  }"
                         "}\n");
        ASTNode *AST = Parse("FunctionBodyForStmt", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getFunctions().begin());
        const ASTBlock *Body = F->getBody();
        ASTForBlock *ForBlock = (ASTForBlock *) Body->getContent()[0];
        EXPECT_EQ(ForBlock->getBlockKind(), ASTBlockKind::BLOCK_STMT_FOR);

        // int b =1
        EXPECT_EQ(((ASTLocalVar *) ForBlock->getContent()[0])->getName(), "b");
        // int c = 2
        EXPECT_EQ(((ASTLocalVar *) ForBlock->getContent()[1])->getName(), "c");

        ASTExprStmt *ExprStmt = (ASTExprStmt *) ForBlock->getCondition()->getContent()[0];
        ASTBinaryGroupExpr *Cond = (ASTBinaryGroupExpr *) ExprStmt->getExpr();
        EXPECT_EQ(((ASTVarRefExpr *) Cond->getFirst())->getVarRef()->getName(), "b");
        EXPECT_EQ(Cond->getOperatorKind(), BinaryOpKind::COMP_LT);
        EXPECT_EQ(((ASTValueExpr *) Cond->getSecond())->getValue().str(), "10");

        ASTExprStmt * ExprStmt1 = (ASTExprStmt *) ForBlock->getPost()->getContent()[0];
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt1->getExpr())->getFirst()->getVarRef()->getName(), "b");
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt1->getExpr())->getOperatorKind(), UnaryOpKind::ARITH_INCR);

        ASTExprStmt * ExprStmt2 = (ASTExprStmt *) ForBlock->getPost()->getContent()[1];
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt2->getExpr())->getFirst()->getVarRef()->getName(), "c");
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt2->getExpr())->getOperatorKind(), UnaryOpKind::ARITH_DECR);

        EXPECT_TRUE(ForBlock->getLoop()->isEmpty());

        delete AST;
    }

    TEST_F(ParserTest, WhileStmt) {
        llvm::StringRef str = ("namespace std\n"
                         "private void func(int a) {\n"
                         "  while (a==1) {}"
                         "  while {}"
                         "}\n");
        ASTNode *AST = Parse("FunctionBodyWhileStmt", str);

        ASSERT_FALSE(Diags.hasErrorOccurred());

        // Get Body
        ASTFunc *F = *(AST->getFunctions().begin());
        const ASTBlock *Body = F->getBody();
        ASTWhileBlock *Stmt = (ASTWhileBlock *) Body->getContent()[0];
        EXPECT_EQ(Stmt->getBlockKind(), ASTBlockKind::BLOCK_STMT_WHILE);
        EXPECT_FALSE(Stmt->getCondition() == nullptr);
        EXPECT_TRUE(Stmt->isEmpty());

        const ASTBinaryGroupExpr *Cond = (ASTBinaryGroupExpr *) Stmt->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) Cond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(Cond->getOperatorKind(), COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) Cond->getSecond())->getValue().str(), "1");

        ASTWhileBlock *Stmt2 = (ASTWhileBlock *) Body->getContent()[1];
        EXPECT_EQ(Stmt2->getBlockKind(), ASTBlockKind::BLOCK_STMT_WHILE);
        EXPECT_TRUE(Stmt2->getCondition() == nullptr);
        EXPECT_TRUE(Stmt2->isEmpty());

        delete AST;
    }
}