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
        EXPECT_EQ(Func->getReturnType()->getStmtKind(), ASTTypeKind::TYPE_VOID);
    }

    TEST_F(ParserTest, FunctionVisibilityPrivate) {
        llvm::StringRef str = ("private void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPrivate", str);

        ASSERT_TRUE(Resolve());

        EXPECT_TRUE(Module->getFunctions().size() == 1);
        ASTFunction *Func = *Module->getFunctions().begin();
        EXPECT_EQ(Func->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_EQ(Func->getReturnType()->getStmtKind(), ASTTypeKind::TYPE_VOID);
    }

    TEST_F(ParserTest, FunctionVisibilityPublic) {
        llvm::StringRef str = ("public void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPublic", str);

        ASSERT_TRUE(Resolve());

        EXPECT_TRUE(Module->getFunctions().size() == 1);
        ASTFunction *Func = *Module->getFunctions().begin();
        EXPECT_EQ(Func->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_EQ(Func->getReturnType()->getStmtKind(), ASTTypeKind::TYPE_VOID);
    }

    TEST_F(ParserTest, FunctionType) {
        llvm::StringRef str = ("void func(bool a, "
                               "byte b, short c, ushort d, int e, uint f, long g, ulong h, "
                               "float i, double j) {}\n");
        ASTModule *Module = Parse("FunctionType", str);

        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = Module->getFunctions()[0];
        EXPECT_EQ(Func->getReturnType()->getStmtKind(), ASTTypeKind::TYPE_VOID);
        const ASTBlockStmt *Body = Func->getBody();

        // Test: bool a
        ASTParam *aVar = Func->getParams()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getType()->getStmtKind(), ASTTypeKind::TYPE_BOOL);
        ASSERT_EQ(aVar->getDefaultValue(), nullptr);

        // Test: byte b
    	ASTParam *bVar = Func->getParams()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(((ASTIntegerType *) bVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        ASSERT_EQ(bVar->getDefaultValue(), nullptr);

        // Test: short c
        ASTParam *cVar = Func->getParams()[2];
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(((ASTIntegerType *) cVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
        ASSERT_EQ(cVar->getDefaultValue(), nullptr);

        // Test: ushort d
        ASTParam *dVar = Func->getParams()[3];
        EXPECT_EQ(dVar->getName(), "d");
        EXPECT_EQ(((ASTIntegerType *) dVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
        ASSERT_EQ(dVar->getDefaultValue(), nullptr);

        // Test: int e
        ASTParam *eVar = Func->getParams()[4];
        EXPECT_EQ(eVar->getName(), "e");
        EXPECT_EQ(((ASTIntegerType *) eVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        ASSERT_EQ(eVar->getDefaultValue(), nullptr);

        // Test: uint f
        ASTParam *fVar = Func->getParams()[5];
        EXPECT_EQ(fVar->getName(), "f");
        EXPECT_EQ(((ASTIntegerType *) fVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
        ASSERT_EQ(fVar->getDefaultValue(), nullptr);

        // Test: long g
        ASTParam *gVar = Func->getParams()[6];
        EXPECT_EQ(gVar->getName(), "g");
        EXPECT_EQ(((ASTIntegerType *) gVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
        ASSERT_EQ(gVar->getDefaultValue(), nullptr);

        // Test: ulong h
        ASTParam *hVar = Func->getParams()[7];
        EXPECT_EQ(hVar->getName(), "h");
        EXPECT_EQ(((ASTIntegerType *) hVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
        ASSERT_EQ(hVar->getDefaultValue(), nullptr);

        // Test: float i
        ASTParam *iVar = Func->getParams()[8];
        EXPECT_EQ(iVar->getName(), "i");
        EXPECT_EQ(((ASTFloatingPointType *) iVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
        ASSERT_EQ(iVar->getDefaultValue(), nullptr);

        // Test: double j
        ASTParam *jVar = Func->getParams()[9];
        EXPECT_EQ(jVar->getName(), "j");
        EXPECT_EQ(((ASTFloatingPointType *) jVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
        ASSERT_EQ(jVar->getDefaultValue(), nullptr);

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
        EXPECT_EQ(Par0->getType()->getStmtKind(), ASTTypeKind::TYPE_INTEGER);
        EXPECT_EQ(Par0->isConstant(), false);

        EXPECT_EQ(Par1->getName(), "b");
        EXPECT_EQ(Par1->getType()->getStmtKind(), ASTTypeKind::TYPE_FLOATING_POINT);
        EXPECT_EQ(Par1->isConstant(), true);

        EXPECT_EQ(Par2->getName(), "c");
        EXPECT_EQ(Par2->getType()->getStmtKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(Par2->isConstant(), false);
        EXPECT_EQ(Par2->getDefaultValue()->getTypeKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(((ASTBoolValue *) Par2->getDefaultValue())->getValue(), false);
        EXPECT_EQ(((ASTBoolValue *) Par2->getDefaultValue())->getValue(), false);

        ASTReturnStmt *Return = (ASTReturnStmt *) VerifyFunc->getBody()->getContent()[0];
        EXPECT_EQ(((ASTValueExpr *) Return->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr*) Return->getExpr())->getValue())->getValue(), "1");
        EXPECT_EQ(Return->getStmtKind(), ASTStmtKind::STMT_RETURN);
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
        EXPECT_EQ(doOtherStmt->getStmtKind(), ASTStmtKind::STMT_EXPR);
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