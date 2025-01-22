//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <AST/ASTFailStmt.h>
#include <AST/ASTHandleStmt.h>
#include <AST/ASTReturnStmt.h>

#include "ParserTest.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTArg.h"
#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTBlockStmt.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, LocalVarType) {
        llvm::StringRef str = ("void func() {\n"
                                   "bool a = false\n"
                                   "byte b = 0\n"
                                   "short c = 0\n"
                                   "ushort d = 0\n"
                                   "int e = 0\n"
                                   "uint f = 0\n"
                                   "long g = 0\n"
                                   "ulong h = 0\n"
                                   "float i = 0.0\n" // TODO need to accept also 0
                                   "double j = 0.0\n"
                               "}\n");
        ASTModule *Module = Parse("FunctionType", str);

        ASSERT_TRUE(Resolve());

        // Get Body
        ASTFunction *F = *Module->getFunctions().begin();
        EXPECT_EQ(F->getReturnType()->getKind(), ASTTypeKind::TYPE_VOID);
        const ASTBlockStmt *Body = F->getBody();

        // Test: bool a
        ASTAssignmentStmt *aStmt = (ASTAssignmentStmt *) Body->getContent()[0];
        ASTVar *aVar = aStmt->getVarRef()->getDef();
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        ASTBoolValue* BoolValue = S->getBuilder().CreateBoolValue(SourceLocation(), false);
        ASSERT_EQ(((ASTBoolValue *)((ASTValueExpr *) aStmt->getExpr())->getValue())->getValue(), BoolValue->getValue());

        // Test: byte b
        ASTAssignmentStmt *bStmt = (ASTAssignmentStmt *) Body->getContent()[1];
        ASTVar *bVar = bStmt->getVarRef()->getDef();
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(((ASTIntegerType *) bVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        ASTIntegerValue* ZeroIntValue = S->getBuilder().CreateIntegerValue(SourceLocation(), "0");
        ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) bStmt->getExpr())->getValue())->getValue(), ZeroIntValue->getValue());

        // Test: short c
        ASTAssignmentStmt *cStmt = (ASTAssignmentStmt *) Body->getContent()[2];
        ASTVar *cVar = cStmt->getVarRef()->getDef();
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(((ASTIntegerType *) cVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
        ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) cStmt->getExpr())->getValue())->getValue(), ZeroIntValue->getValue());

        // Test: ushort d
        ASTAssignmentStmt *dStmt = (ASTAssignmentStmt *) Body->getContent()[3];
        ASTVar *dVar = dStmt->getVarRef()->getDef();
        EXPECT_EQ(dVar->getName(), "d");
        EXPECT_EQ(((ASTIntegerType *) dVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
        ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) dStmt->getExpr())->getValue())->getValue(), ZeroIntValue->getValue());

        // Test: int e
        ASTAssignmentStmt *eStmt = (ASTAssignmentStmt *) Body->getContent()[4];
        ASTVar *eVar = eStmt->getVarRef()->getDef();
        EXPECT_EQ(eVar->getName(), "e");
        EXPECT_EQ(((ASTIntegerType *) eVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) eStmt->getExpr())->getValue())->getValue(), ZeroIntValue->getValue());

        // Test: uint f
        ASTAssignmentStmt *fStmt = (ASTAssignmentStmt *) Body->getContent()[5];
        ASTVar *fVar = fStmt->getVarRef()->getDef();
        EXPECT_EQ(fVar->getName(), "f");
        EXPECT_EQ(((ASTIntegerType *) fVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
        ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) fStmt->getExpr())->getValue())->getValue(), ZeroIntValue->getValue());

        // Test: long g
        ASTAssignmentStmt *gStmt = (ASTAssignmentStmt *) Body->getContent()[6];
        ASTVar *gVar = gStmt->getVarRef()->getDef();
        EXPECT_EQ(gVar->getName(), "g");
        EXPECT_EQ(((ASTIntegerType *) gVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
        ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) gStmt->getExpr())->getValue())->getValue(), ZeroIntValue->getValue());

        // Test: ulong h
        ASTAssignmentStmt *hStmt = (ASTAssignmentStmt *) Body->getContent()[7];
        ASTVar *hVar = hStmt->getVarRef()->getDef();
        EXPECT_EQ(hVar->getName(), "h");
        EXPECT_EQ(((ASTIntegerType *) hVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
        ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) hStmt->getExpr())->getValue())->getValue(), ZeroIntValue->getValue());

        // Test: float i
        ASTAssignmentStmt *iStmt = (ASTAssignmentStmt *) Body->getContent()[8];
        ASTVar *iVar = iStmt->getVarRef()->getDef();
        EXPECT_EQ(iVar->getName(), "i");
        EXPECT_EQ(((ASTFloatingPointType *) iVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
        ASTFloatingValue* ZeroFloatValue = S->getBuilder().CreateFloatingValue(SourceLocation(), "0.0");
        ASSERT_EQ(((ASTFloatingValue *)((ASTValueExpr *) iStmt->getExpr())->getValue())->getValue(), ZeroFloatValue->getValue());

        // Test: double j
        ASTAssignmentStmt *jStmt = (ASTAssignmentStmt *) Body->getContent()[9];
        ASTVar *jVar = jStmt->getVarRef()->getDef();
        EXPECT_EQ(jVar->getName(), "j");
        EXPECT_EQ(((ASTFloatingPointType *) jVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
        ASSERT_EQ(((ASTFloatingValue *)((ASTValueExpr *) jStmt->getExpr())->getValue())->getValue(), ZeroFloatValue->getValue());

        // Test: Type t
        // ASTAssignmentStmt *tStmt = (ASTAssignmentStmt *) Body->getContent()[10];
        // ASTVar *tVar = tStmt->getVarRef()->getDef();
        // EXPECT_EQ(tVar->getName(), "t");
        // EXPECT_EQ(((ASTIdentityType *) tVar->getType())->getIdentityTypeKind(), ASTIdentityTypeKind::TYPE_NONE);
        // ASSERT_EQ(((ASTIntegerValue *)((ASTValueExpr *) iStmt->getExpr())->getValue())->getValue(), ZeroFloatValue->getValue());
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        llvm::StringRef str = (
                "private int func(int a, const float b, bool c=false) {\n"
                "  return 1"
                "}\n");
        ASTModule *Module = Parse("FunctionPrivateReturnParams", str);

        ASSERT_TRUE(Resolve());


        EXPECT_TRUE(Module->getFunctions().size() == 1); // func() has PRIVATE Visibility
        ASTFunction *Func = *Module->getFunctions().begin();
        EXPECT_EQ(Func->getVisibility(), ASTVisibilityKind::V_PRIVATE);

        ASTParam *Par0 = Func->getParams()[0];
        ASTParam *Par1 = Func->getParams()[1];
        ASTParam *Par2 = Func->getParams()[2];

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

        ASTReturnStmt *ReturnStmt = (ASTReturnStmt *) Func->getBody()->getContent()[0];
        EXPECT_EQ(ReturnStmt->getKind(), ASTStmtKind::STMT_RETURN);
        EXPECT_EQ((ReturnStmt->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr*) ReturnStmt->getExpr())->getValue())->getValue(), "1");
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
        auto doSomeFunc = Module->getFunctions()[0];
        auto doOtherFunc = Module->getFunctions()[1];
        auto doNowFunc = Module->getFunctions()[2];
        auto mainFunc = Module->getFunctions()[3];

        ASSERT_TRUE(doSomeFunc != nullptr);
        ASSERT_TRUE(doOtherFunc != nullptr);
        ASSERT_TRUE(mainFunc != nullptr);

        // Get main() Body
        const ASTBlockStmt *Body = mainFunc->getBody();

        // Test: doSome()
        ASTAssignmentStmt *VarBDecl = ((ASTAssignmentStmt *) Body->getContent()[0]);
        ASTCallExpr *doSomeCall = (ASTCallExpr *) VarBDecl->getExpr();
        EXPECT_EQ(doSomeCall->getCall()->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);

        // Test: doNow()
        ASTAssignmentStmt *VarBAssign = ((ASTAssignmentStmt *) Body->getContent()[1]);
        ASTCallExpr *doNowCall = (ASTCallExpr *) VarBAssign->getExpr();
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
        ASTReturnStmt *RetStmt = (ASTReturnStmt *) Body->getContent()[3];
        ASTVarRefExpr *RetExpr = (ASTVarRefExpr *) RetStmt->getExpr();
        EXPECT_EQ(RetExpr->getVarRef()->getName(), "b");
    }

    TEST_F(ParserTest, FunctionHandleFail) {
        llvm::StringRef str = (
                               "void err0() {\n"
                               "  fail\n"
                               "}\n"
                               "int err1() {\n"
                               "  fail 404\n"
                               "}\n"
                               "string err2() {\n"
                               "  fail \"Error\"\n"
                               "}\n"
                               "void main() {\n"
                               "  handle err0()\n"
                               "  bool b = false\n"
                               "  error err0 = handle { b = err0() }\n"
                               "  int i = 0\n"
                               "  error err1 = handle { i = err1() }\n"
                               "  string s = \"\"\n"
                               "  error err2 = handle { s = err2() }\n"
                               "}\n");
        ASTModule *Module = Parse("FunctionFail", str);
        ASSERT_TRUE(Resolve());


        // Get all functions
        ASTFunction *err0 = Module->getFunctions()[0];
        ASTFunction *err1 = Module->getFunctions()[1];
        ASTFunction *err2 = Module->getFunctions()[2];
        ASTFunction *main = Module->getFunctions()[3];

        ASSERT_TRUE(err0 != nullptr);
        ASSERT_TRUE(err1 != nullptr);
        ASSERT_TRUE(err2 != nullptr);
        ASSERT_TRUE(main != nullptr);

        // err0()
        ASTFailStmt *Stmt0 = (ASTFailStmt *) err0->getBody()->getContent()[0];
        ASSERT_TRUE(Stmt0->getExpr() == nullptr);

        // err1()
        ASTFailStmt *Stmt1 = (ASTFailStmt *) err1->getBody()->getContent()[0];
        ASTValue *Val2 = ((ASTValueExpr *) Stmt1->getExpr())->getValue();
        ASSERT_EQ(Val2->getTypeKind(), ASTTypeKind::TYPE_INTEGER);
        ASSERT_EQ(((ASTIntegerValue *) Val2)->getValue(), "404");

        // err2()
        ASTFailStmt *Stmt2 = (ASTFailStmt *) err2->getBody()->getContent()[0];
        ASTValue *Val3 = ((ASTValueExpr *) Stmt2->getExpr())->getValue();
        ASSERT_EQ(Val3->getTypeKind(), ASTTypeKind::TYPE_STRING);
        ASSERT_EQ(((ASTStringValue *) Val3)->getValue(), "Error");

        // Get main() Body

        // handle err0()
        ASTHandleStmt *HandleStmt = (ASTHandleStmt *) main->getBody()->getContent()[0];
        ASSERT_TRUE(((ASTCallExpr *) ((ASTExprStmt *) HandleStmt->getHandle()->getContent()[0])->getExpr())->getCall()->getArgs().empty());

        // bool b = false
        ASTAssignmentStmt *bool_err0 = (ASTAssignmentStmt *) main->getBody()->getContent()[1];
        ASSERT_TRUE(bool_err0->getVarRef()->getDef()->getType()->isBool());
        ASSERT_EQ(((ASTBoolValue *) ((ASTValueExpr *) bool_err0->getExpr())->getValue())->getValue(), false);

        // error err0 = handle { b = err0() }
        ASTHandleStmt *error_err0 = (ASTHandleStmt *) main->getBody()->getContent()[2];
        ASSERT_TRUE(error_err0->getErrorHandlerRef()->getDef()->getType()->isError());

        // int i = 0
        ASTAssignmentStmt *int_err1 = (ASTAssignmentStmt *) main->getBody()->getContent()[3];
        ASSERT_TRUE(int_err1->getVarRef()->getDef()->getType()->isInteger());
        ASSERT_EQ(((ASTIntegerValue *) ((ASTValueExpr *) int_err1->getExpr())->getValue())->getValue(), "0");

        // error err1 = handle { i = err1() }
        ASTHandleStmt *error_err1 = (ASTHandleStmt *) main->getBody()->getContent()[4];
        ASSERT_TRUE(error_err1->getErrorHandlerRef()->getDef()->getType()->isError());

        // string s = ""
        ASTAssignmentStmt *string_err2 = (ASTAssignmentStmt *) main->getBody()->getContent()[5];
        ASSERT_TRUE(string_err2->getVarRef()->getDef()->getType()->isString());
        ASSERT_EQ(((ASTStringValue *) ((ASTValueExpr *) string_err2->getExpr())->getValue())->getValue(), "");

        // error err2 = handle { s = err2() }
        ASTHandleStmt *error_err2 = (ASTHandleStmt *) main->getBody()->getContent()[6];
        ASSERT_TRUE(error_err2->getErrorHandlerRef()->getDef()->getType()->isError());
    }
}