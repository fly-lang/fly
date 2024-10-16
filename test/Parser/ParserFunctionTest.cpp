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
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExpr.h"

#include "llvm/ADT/StringMap.h"

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
                               "float i, double j, Type t) {}\n");
        ASTModule *Module = Parse("FunctionType", str);

        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *Func = *Module->getFunctions().begin();
        EXPECT_EQ(Func->getReturnType()->getKind(), ASTTypeKind::TYPE_VOID);
        const ASTBlockStmt *Body = Func->getBody();

        // Test: bool a
        ASTVarStmt *aStmt = (ASTVarStmt *) Body->getContent()[0];
        ASTVar *aVar = aStmt->getVarRef()->getDef();
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        ASSERT_EQ(aStmt->getExpr(), nullptr);

        // Test: byte b
        ASTVarStmt *bStmt = (ASTVarStmt *) Body->getContent()[1];
        ASTVar *bVar = bStmt->getVarRef()->getDef();
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(((ASTIntegerType *) bVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        ASSERT_EQ(bStmt->getExpr(), nullptr);

        // Test: short c
        ASTVarStmt *cStmt = (ASTVarStmt *) Body->getContent()[2];
        ASTVar *cVar = cStmt->getVarRef()->getDef();
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(((ASTIntegerType *) cVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
        ASSERT_EQ(cStmt->getExpr(), nullptr);

        // Test: ushort d
        ASTVarStmt *dStmt = (ASTVarStmt *) Body->getContent()[3];
        ASTVar *dVar = dStmt->getVarRef()->getDef();
        EXPECT_EQ(dVar->getName(), "d");
        EXPECT_EQ(((ASTIntegerType *) dVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
        ASSERT_EQ(dStmt->getExpr(), nullptr);

        // Test: int e
        ASTVarStmt *eStmt = (ASTVarStmt *) Body->getContent()[4];
        ASTVar *eVar = eStmt->getVarRef()->getDef();
        EXPECT_EQ(eVar->getName(), "e");
        EXPECT_EQ(((ASTIntegerType *) eVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        ASSERT_EQ(eStmt->getExpr(), nullptr);

        // Test: uint f
        ASTVarStmt *fStmt = (ASTVarStmt *) Body->getContent()[5];
        ASTVar *fVar = fStmt->getVarRef()->getDef();
        EXPECT_EQ(fVar->getName(), "f");
        EXPECT_EQ(((ASTIntegerType *) fVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
        ASSERT_EQ(fStmt->getExpr(), nullptr);

        // Test: long g
        ASTVarStmt *gStmt = (ASTVarStmt *) Body->getContent()[6];
        ASTVar *gVar = gStmt->getVarRef()->getDef();
        EXPECT_EQ(gVar->getName(), "g");
        EXPECT_EQ(((ASTIntegerType *) gVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
        ASSERT_EQ(gStmt->getExpr(), nullptr);

        // Test: ulong h
        ASTVarStmt *hStmt = (ASTVarStmt *) Body->getContent()[7];
        ASTVar *hVar = hStmt->getVarRef()->getDef();
        EXPECT_EQ(hVar->getName(), "h");
        EXPECT_EQ(((ASTIntegerType *) hVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
        ASSERT_EQ(hStmt->getExpr(), nullptr);

        // Test: float i
        ASTVarStmt *iStmt = (ASTVarStmt *) Body->getContent()[8];
        ASTVar *iVar = iStmt->getVarRef()->getDef();
        EXPECT_EQ(iVar->getName(), "i");
        EXPECT_EQ(((ASTFloatingPointType *) iVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
        ASSERT_EQ(iStmt->getExpr(), nullptr);

        // Test: double j
        ASTVarStmt *jStmt = (ASTVarStmt *) Body->getContent()[9];
        ASTVar *jVar = jStmt->getVarRef()->getDef();
        EXPECT_EQ(jVar->getName(), "j");
        EXPECT_EQ(((ASTFloatingPointType *) jVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
        ASSERT_EQ(jStmt->getExpr(), nullptr);

        // Test: Type t
        ASTVarStmt *tStmt = (ASTVarStmt *) Body->getContent()[10];
        ASTVar *tVar = tStmt->getVarRef()->getDef();
        EXPECT_EQ(tVar->getName(), "t");
        EXPECT_EQ(((ASTIdentityType *) tVar->getType())->getIdentityTypeKind(), ASTIdentityTypeKind::TYPE_NONE);
        ASSERT_EQ(tStmt->getExpr(), nullptr);
    }

//    TEST_F(ParserTest, FunctionPrivateReturnParams) {
//        llvm::StringRef str = (
//                "private int func(int a, const float b, bool c=false) {\n"
//                "  return 1"
//                "}\n");
//        ASTModule *Module = Parse("FunctionPrivateReturnParams", str);
//
//        ASSERT_TRUE(Resolve());
//
//
//        EXPECT_TRUE(Module->getFunctions().size() == 1); // func() has PRIVATE Visibility
//        ASTFunction *VerifyFunc = *Module->getFunctions().begin()->getValue().begin()->second.begin();
//        EXPECT_EQ(VerifyFunc->getVisibility(), ASTVisibilityKind::V_PRIVATE);
//        const auto &NSFuncs = Module->getContext().getDefaultNameSpace()->getFunctions();
//        ASSERT_TRUE(NSFuncs.find(VerifyFunc->getName()) == NSFuncs.end());
//        ASSERT_TRUE(Module->getFunctions().find(VerifyFunc->getName()) != NSFuncs.end());
//
//        ASTParam *Par0 = VerifyFunc->getParams()[0];
//        ASTParam *Par1 = VerifyFunc->getParams()[1];
//        ASTParam *Par2 = VerifyFunc->getParams()[2];
//
//        EXPECT_EQ(Par0->getName(), "a");
//        EXPECT_EQ(Par0->getType()->getKind(), ASTTypeKind::TYPE_INTEGER);
//        EXPECT_EQ(Par0->isConstant(), false);
//
//        EXPECT_EQ(Par1->getName(), "b");
//        EXPECT_EQ(Par1->getType()->getKind(), ASTTypeKind::TYPE_FLOATING_POINT);
//        EXPECT_EQ(Par1->isConstant(), true);
//
//        EXPECT_EQ(Par2->getName(), "c");
//        EXPECT_EQ(Par2->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
//        EXPECT_EQ(Par2->isConstant(), false);
//        EXPECT_EQ(Par2->getDefaultValue()->getTypeKind(), ASTTypeKind::TYPE_BOOL);
//        EXPECT_EQ(((ASTBoolValue *) Par2->getDefaultValue())->getValue(), false);
//        EXPECT_EQ(Par2->getDefaultValue()->print(), "false");
//
//        ASTReturnStmt *Return = (ASTReturnStmt *) VerifyFunc->getBody()->getContent()[0];
//        EXPECT_EQ(((ASTValueExpr *) Return->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
//        EXPECT_EQ(((ASTValueExpr*) Return->getExpr())->getValue()->print(), "1");
//        EXPECT_EQ(Return->getKind(), ASTStmtKind::STMT_RETURN);
//    }
//
//    TEST_F(ParserTest, NullTypeVarReturn) {
//        llvm::StringRef str = ("Type func() {\n"
//                               "  Type t = null"
//                               "  return t\n"
//                               "}\n");
//        ASTModule *Module = Parse("TypeDefaultVarReturn", str);
//
//        ASSERT_TRUE(Resolve());
//
//
//        // Get Body
//        ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
//        EXPECT_EQ(F->getReturnType()->getKind(), ASTTypeKind::TYPE_IDENTITY);
//        const ASTBlockStmt *Body = F->getBody();
//
//        // Test: Type t
//        ASTVarStmt *varStmt = (ASTVarStmt *) Body->getContent()[0];
//        EXPECT_EQ(varStmt->getVarRef()->getName(), "t");
//        ASTIdentityType *ClassType = (ASTIdentityType *) varStmt->getVarRef()->getDef()->getType();
//        EXPECT_EQ(ClassType->getKind(), ASTTypeKind::TYPE_IDENTITY);
//        EXPECT_EQ(ClassType->getName(), "Type");
//        ASSERT_EQ(((ASTNullValue *)((ASTValueExpr *) varStmt->getExpr())->getValue())->print(), "null");
//
//        const ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[1];
//        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
//        EXPECT_EQ(RetRef->getVarRef()->getName(), "t");
//
//    }
//
//    TEST_F(ParserTest, FunctionCall) {
//        llvm::StringRef str = ("private int doSome() {return 1}\n"
//                               "public void doOther(int a, int b) {}\n"
//                               "int doNow() {return 0}"
//                               "int main(int a) {\n"
//                               "  int b = doSome()\n"
//                               "  b = doNow()\n"
//                               "  doOther(a, 1)\n"
//                               "  return b\n"
//                               "}\n");
//        ASTModule *Module = Parse("FunctionCall", str);
//
//        ASSERT_TRUE(Resolve());
//
//
//        // Get all functions
//        ASTFunction *doSome = *Module->getFunctions().find("doSome")->getValue().begin()->second.begin();
//        ASTFunction *doOther = *Module->getFunctions().find("doOther")->getValue().begin()->second.begin();
//        ASTFunction *doNow = *Module->getFunctions().find("doNow")->getValue().begin()->second.begin();
//        ASTFunction *main = *Module->getFunctions().find("main")->getValue().begin()->second.begin();
//
//        ASSERT_TRUE(doSome != nullptr);
//        ASSERT_TRUE(doOther != nullptr);
//        ASSERT_TRUE(main != nullptr);
//
//        // Get main() Body
//        const ASTBlockStmt *Body = main->getBody();
//
//        // Test: doSome()
//        ASTVarStmt *VarB = ((ASTVarStmt *) Body->getContent()[0]);
//        ASTCallExpr *doSomeCall = (ASTCallExpr *) VarB->getExpr();
//        EXPECT_EQ(doSomeCall->getCall()->getName(), "doSome");
//        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);
//        ASSERT_FALSE(doSomeCall->getCall()->getDef() == nullptr);
//
//        // Test: doNow()
//        ASTVarStmt *B = ((ASTVarStmt *) Body->getContent()[1]);
//        ASTCallExpr *doNowCall = (ASTCallExpr *) B->getExpr();
//        EXPECT_EQ(doNowCall->getCall()->getName(), "doNow");
//        EXPECT_EQ(doNowCall->getExprKind(), ASTExprKind::EXPR_CALL);
//        ASSERT_FALSE(doNowCall->getCall()->getDef() == nullptr);
//
//        // Test: doOther(a, b)
//        ASTExprStmt *doOtherStmt = (ASTExprStmt *) Body->getContent()[2];
//        EXPECT_EQ(doOtherStmt->getKind(), ASTStmtKind::STMT_EXPR);
//        ASTCallExpr *doOtherCall = (ASTCallExpr *) doOtherStmt->getExpr();
//        EXPECT_EQ(doOtherCall->getCall()->getName(), "doOther");
//        ASTArg *Arg0 = doOtherCall->getCall()->getArgs()[0];
//        EXPECT_EQ(((ASTVarRefExpr *) Arg0->getExpr())->getVarRef()->getName(), "a");
//        ASTArg *Arg1 = doOtherCall->getCall()->getArgs()[1];
//        EXPECT_EQ(((ASTValueExpr *) Arg1->getExpr())->getValue()->print(), "1");
//
//        // return do()
//        ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[3];
//        ASTVarRefExpr *RetExpr = (ASTVarRefExpr *) Ret->getExpr();
//        EXPECT_EQ(RetExpr->getVarRef()->getName(), "b");
//        EXPECT_FALSE(RetExpr->getVarRef()->getDef() == nullptr);
//    }
//
//    TEST_F(ParserTest, FunctionHandleFail) {
//        llvm::StringRef str = (
//                               "void err0() {\n"
//                               "  fail\n"
//                               "}\n"
//                               "int err1() {\n"
//                               "  fail 404\n"
//                               "}\n"
//                               "int err2() {\n"
//                               "  fail \"Error\"\n"
//                               "}\n"
//                               "void main() {\n"
//                               "  handle err0()\n"
//                               "  bool err0 = handle err0()\n"
//                               "  error err0 = handle err0()\n"
//                               "  int err1 = handle { err1() }\n"
//                               "  error err1 = handle err1()\n"
//                               "  string err2 = handle err2()\n"
//                               "  error err2 = handle { err2() }\n"
//                               "}\n");
//        ASTModule *Module = Parse("FunctionFail", str);
//        ASSERT_TRUE(Resolve());
//
//
//        // Get all functions
//        ASTFunction *err0 = *Module->getFunctions().find("err0")->getValue().begin()->second.begin();
//        ASTFunction *err1 = *Module->getFunctions().find("err1")->getValue().begin()->second.begin();
//        ASTFunction *err2 = *Module->getFunctions().find("err2")->getValue().begin()->second.begin();
//        ASTFunction *main = *Module->getFunctions().find("main")->getValue().begin()->second.begin();
//
//        ASSERT_TRUE(err0 != nullptr);
//        ASSERT_TRUE(err1 != nullptr);
//        ASSERT_TRUE(err2 != nullptr);
//        ASSERT_TRUE(main != nullptr);
//
//        ASTExprStmt *Stmt1 = (ASTExprStmt *) err0->getBody()->getContent()[0];
//        ASSERT_TRUE(((ASTCallExpr *) Stmt1->getExpr())->getCall()->getArgs().empty());
//
//        ASTExprStmt *Stmt2 = (ASTExprStmt *) err1->getBody()->getContent()[0];
//        ASTArg *Arg2 = ((ASTCallExpr *) Stmt2->getExpr())->getCall()->getArgs()[0];
//        ASTValue *Val2 = ((ASTValueExpr *) Arg2->getExpr())->getValue();
//        ASSERT_EQ(Val2->getTypeKind(), ASTTypeKind::TYPE_INTEGER);
//        ASSERT_EQ(((ASTIntegerValue *) Val2)->getValue(), 404);
//
//        ASTExprStmt *Stmt3 = (ASTExprStmt *) err2->getBody()->getContent()[0];
//        ASTArg *Arg3 = ((ASTCallExpr *) Stmt3->getExpr())->getCall()->getArgs()[0];
//        ASTValue *Val3 = ((ASTValueExpr *) Arg3->getExpr())->getValue();
//        ASSERT_EQ(Val3->getTypeKind(), ASTTypeKind::TYPE_STRING);
//        ASSERT_EQ(((ASTStringValue *) Val3)->getValue(), "Error");
//
//        // Get main() Body
//        ASTHandleStmt *HandleStmt = (ASTHandleStmt *) main->getBody()->getContent()[0];
//
//        ASTVarStmt *bool_err0 = (ASTVarStmt *) main->getBody()->getContent()[1];
//        ASSERT_TRUE(((ASTVarRefExpr *) bool_err0->getExpr())->getVarRef()->getDef()->getType()->isBool());
//        ASTVarStmt *error_err0 = (ASTVarStmt *) main->getBody()->getContent()[2];
//        ASSERT_TRUE(((ASTVarRefExpr *) error_err0->getExpr())->getVarRef()->getDef()->getType()->isError());
//
//        ASTVarStmt *int_err1 = (ASTVarStmt *) main->getBody()->getContent()[3];
//        ASSERT_TRUE(((ASTVarRefExpr *) int_err1->getExpr())->getVarRef()->getDef()->getType()->isInteger());
//        ASTVarStmt *error_err1 = (ASTVarStmt *) main->getBody()->getContent()[4];
//        ASSERT_TRUE(((ASTVarRefExpr *) error_err1->getExpr())->getVarRef()->getDef()->getType()->isError());
//
//        ASTVarStmt *string_err2 = (ASTVarStmt *) main->getBody()->getContent()[5];
//        ASSERT_TRUE(((ASTVarRefExpr *) string_err2->getExpr())->getVarRef()->getDef()->getType()->isString());
//        ASTVarStmt *error_err2 = (ASTVarStmt *) main->getBody()->getContent()[6];
//        ASSERT_TRUE(((ASTVarRefExpr *) error_err2->getExpr())->getVarRef()->getDef()->getType()->isError());
//    }
}