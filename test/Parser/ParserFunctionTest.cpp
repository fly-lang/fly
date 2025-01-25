//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTArg.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, FunctionVisibilityDefault) {
        llvm::StringRef str = ("void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityDefault", str);

        ASSERT_TRUE(Resolve());

        EXPECT_TRUE(Module->getFunctions().size() == 1);
        ASTFunction *Func = *Module->getFunctions().begin();
        EXPECT_EQ(Func->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_EQ(Func->getReturnType()->getKind(), ASTTypeKind::TYPE_VOID);
    }

    TEST_F(ParserTest, FunctionVisibilityPrivate) {
        llvm::StringRef str = ("private void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPrivate", str);

        ASSERT_TRUE(Resolve());

        EXPECT_TRUE(Module->getFunctions().size() == 1);
        ASTFunction *Func = *Module->getFunctions().begin();
        EXPECT_EQ(Func->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_EQ(Func->getReturnType()->getKind(), ASTTypeKind::TYPE_VOID);
    }

    TEST_F(ParserTest, FunctionVisibilityPublic) {
        llvm::StringRef str = ("public void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPublic", str);

        ASSERT_TRUE(Resolve());

        EXPECT_TRUE(Module->getFunctions().size() == 1);
        ASTFunction *Func = *Module->getFunctions().begin();
        EXPECT_EQ(Func->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_EQ(Func->getReturnType()->getKind(), ASTTypeKind::TYPE_VOID);
    }

    TEST_F(ParserTest, FunctionType) {
        llvm::StringRef str = ("void func(bool a, "
                               "byte b, short c, ushort d, int e, uint f, long g, ulong h, "
                               "float i, double j) {}\n");
        ASTModule *Module = Parse("FunctionType", str);

        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = Module->getFunctions()[0];
        EXPECT_EQ(Func->getReturnType()->getKind(), ASTTypeKind::TYPE_VOID);
        const ASTBlockStmt *Body = Func->getBody();

        // Test: bool a
        ASTAssignmentStmt *aStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        ASTVar *aVar = aStmt->getVarRef()->getDef();
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        ASSERT_EQ(aStmt->getExpr(), nullptr);

        // Test: byte b
        ASTAssignmentStmt *bStmt = (ASTAssignmentStmt *) Body->getContent()[1];
        ASTVar *bVar = bStmt->getVarRef()->getDef();
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(((ASTIntegerType *) bVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        ASSERT_EQ(bStmt->getExpr(), nullptr);

        // Test: short c
        ASTAssignmentStmt *cStmt = (ASTAssignmentStmt *) Body->getContent()[2];
        ASTVar *cVar = cStmt->getVarRef()->getDef();
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(((ASTIntegerType *) cVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
        ASSERT_EQ(cStmt->getExpr(), nullptr);

        // Test: ushort d
        ASTAssignmentStmt *dStmt = (ASTAssignmentStmt *) Body->getContent()[3];
        ASTVar *dVar = dStmt->getVarRef()->getDef();
        EXPECT_EQ(dVar->getName(), "d");
        EXPECT_EQ(((ASTIntegerType *) dVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
        ASSERT_EQ(dStmt->getExpr(), nullptr);

        // Test: int e
        ASTAssignmentStmt *eStmt = (ASTAssignmentStmt *) Body->getContent()[4];
        ASTVar *eVar = eStmt->getVarRef()->getDef();
        EXPECT_EQ(eVar->getName(), "e");
        EXPECT_EQ(((ASTIntegerType *) eVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        ASSERT_EQ(eStmt->getExpr(), nullptr);

        // Test: uint f
        ASTAssignmentStmt *fStmt = (ASTAssignmentStmt *) Body->getContent()[5];
        ASTVar *fVar = fStmt->getVarRef()->getDef();
        EXPECT_EQ(fVar->getName(), "f");
        EXPECT_EQ(((ASTIntegerType *) fVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
        ASSERT_EQ(fStmt->getExpr(), nullptr);

        // Test: long g
        ASTAssignmentStmt *gStmt = (ASTAssignmentStmt *) Body->getContent()[6];
        ASTVar *gVar = gStmt->getVarRef()->getDef();
        EXPECT_EQ(gVar->getName(), "g");
        EXPECT_EQ(((ASTIntegerType *) gVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
        ASSERT_EQ(gStmt->getExpr(), nullptr);

        // Test: ulong h
        ASTAssignmentStmt *hStmt = (ASTAssignmentStmt *) Body->getContent()[7];
        ASTVar *hVar = hStmt->getVarRef()->getDef();
        EXPECT_EQ(hVar->getName(), "h");
        EXPECT_EQ(((ASTIntegerType *) hVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
        ASSERT_EQ(hStmt->getExpr(), nullptr);

        // Test: float i
        ASTAssignmentStmt *iStmt = (ASTAssignmentStmt *) Body->getContent()[8];
        ASTVar *iVar = iStmt->getVarRef()->getDef();
        EXPECT_EQ(iVar->getName(), "i");
        EXPECT_EQ(((ASTFloatingPointType *) iVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
        ASSERT_EQ(iStmt->getExpr(), nullptr);

        // Test: double j
        ASTAssignmentStmt *jStmt = (ASTAssignmentStmt *) Body->getContent()[9];
        ASTVar *jVar = jStmt->getVarRef()->getDef();
        EXPECT_EQ(jVar->getName(), "j");
        EXPECT_EQ(((ASTFloatingPointType *) jVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
        ASSERT_EQ(jStmt->getExpr(), nullptr);

        // Test: Type t
        // ASTAssignmentStmt *tStmt = (ASTAssignmentStmt *) Body->getContent()[10];
        // ASTVar *tVar = tStmt->getVarRef()->getDef();
        // EXPECT_EQ(tVar->getName(), "t");
        // EXPECT_EQ(((ASTIdentityType *) tVar->getType())->getIdentityTypeKind(), ASTIdentityTypeKind::TYPE_NONE);
        // ASSERT_EQ(tStmt->getExpr(), nullptr);
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        llvm::StringRef str = (
                "private int func(int a, const float b, bool c=false) {\n"
                "  return 1"
                "}\n");
        ASTModule *Module = Parse("FunctionPrivateReturnParams", str);

        ASSERT_TRUE(Resolve());


        EXPECT_TRUE(Module->getFunctions().size() == 1); // func() has PRIVATE Visibility
        ASTFunction *VerifyFunc = Module->getFunctions()[0];
        EXPECT_EQ(VerifyFunc->getVisibility(), ASTVisibilityKind::V_PRIVATE);

        ASTParam *Par0 = VerifyFunc->getParams()[0];
        ASTParam *Par1 = VerifyFunc->getParams()[1];
        ASTParam *Par2 = VerifyFunc->getParams()[2];

        EXPECT_EQ(Par0->getName(), "a");
        EXPECT_EQ(Par0->getType()->getKind(), ASTTypeKind::TYPE_INTEGER);
        EXPECT_EQ(Par0->isConstant(), false);

        EXPECT_EQ(Par1->getName(), "b");
        EXPECT_EQ(Par1->getType()->getKind(), ASTTypeKind::TYPE_FLOATING_POINT);
        EXPECT_EQ(Par1->isConstant(), true);

        EXPECT_EQ(Par2->getName(), "c");
        EXPECT_EQ(Par2->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(Par2->isConstant(), false);
        EXPECT_EQ(Par2->getDefaultValue()->getTypeKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(((ASTBoolValue *) Par2->getDefaultValue())->getValue(), false);
        EXPECT_EQ(((ASTBoolValue *) Par2->getDefaultValue())->getValue(), false);

        ASTReturnStmt *Return = (ASTReturnStmt *) VerifyFunc->getBody()->getContent()[0];
        EXPECT_EQ(((ASTValueExpr *) Return->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr*) Return->getExpr())->getValue())->getValue(), "1");
        EXPECT_EQ(Return->getKind(), ASTStmtKind::STMT_RETURN);
    }



    TEST_F(ParserTest, FunctionCall) {
        llvm::StringRef str = ("private int doSome() {return 1}\n"
                               "public void doOther(int a, int b) {}\n"
                               "int doNow() {return 0}"
                               "int main(int a) {\n"
                               "  int b = doSome()\n"
                               "  b = doNow()\n"
                               "  doOther(a, 1)\n"
                               "  return b\n"
                               "}\n");
        ASTModule *Module = Parse("FunctionCall", str);

        ASSERT_TRUE(Resolve());

        // Get all functions
        ASTFunction *doSome = Module->getFunctions()[0];
        ASTFunction *doOther = Module->getFunctions()[1];
        ASTFunction *doNow = Module->getFunctions()[2];
        ASTFunction *main = Module->getFunctions()[3];

        ASSERT_TRUE(doSome != nullptr);
        ASSERT_TRUE(doOther != nullptr);
        ASSERT_TRUE(main != nullptr);

        // Get main() Body
        const ASTBlockStmt *Body = main->getBody();

        // Test: doSome()
        ASTAssignmentStmt *VarB = (ASTAssignmentStmt *) Body->getContent()[0];
        ASTCallExpr *doSomeCall = (ASTCallExpr *) VarB->getExpr();
        EXPECT_EQ(doSomeCall->getCall()->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);

        // Test: doNow()
        ASTAssignmentStmt *B = (ASTAssignmentStmt *) Body->getContent()[1];
        ASTCallExpr *doNowCall = (ASTCallExpr *) B->getExpr();
        EXPECT_EQ(doNowCall->getCall()->getName(), "doNow");
        EXPECT_EQ(doNowCall->getExprKind(), ASTExprKind::EXPR_CALL);

        // Test: doOther(a, b)
        ASTExprStmt *doOtherStmt = (ASTExprStmt *) Body->getContent()[2];
        EXPECT_EQ(doOtherStmt->getKind(), ASTStmtKind::STMT_EXPR);
        ASTCallExpr *doOtherCall = (ASTCallExpr *) doOtherStmt->getExpr();
        EXPECT_EQ(doOtherCall->getCall()->getName(), "doOther");
        ASTArg *Arg0 = doOtherCall->getCall()->getArgs()[0];
        EXPECT_EQ(((ASTVarRefExpr *) Arg0->getExpr())->getVarRef()->getName(), "a");
        ASTArg *Arg1 = doOtherCall->getCall()->getArgs()[1];
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *) Arg1->getExpr())->getValue())->getValue(), "1");

        // return do()
        ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[3];
        ASTVarRefExpr *RetExpr = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetExpr->getVarRef()->getName(), "b");
    }
}