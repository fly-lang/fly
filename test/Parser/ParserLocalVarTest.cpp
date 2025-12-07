//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserLocalVarTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <AST/ASTFailStmt.h>
#include <AST/ASTHandleStmt.h>
#include <AST/ASTReturnStmt.h>
#include <AST/ASTParam.h>

#include "ParserTest.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTImport.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTArg.h"
#include "AST/ASTAssignStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTClass.h"
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
        auto *F = static_cast<ASTFunction *>(Module->getNodes()[0]);
        EXPECT_TRUE(HasBuiltinType(F->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
        auto *Body = F->getBody();

        // Test: bool a
        auto *aStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        auto *aIdent = As<ASTIdentifier>(aStmt->getSource());
        EXPECT_EQ(aIdent->getName(), "a");
        SemaVar *aSema = aIdent->getSema();
        ASSERT_NE(aSema, nullptr);
        EXPECT_TRUE(aSema->getType()->isBool());

        // Test: byte b
        auto *bStmt = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *bIdent = As<ASTIdentifier>(bStmt->getSource());
        EXPECT_EQ(bIdent->getName(), "b");
        SemaVar *bSema = bIdent->getSema();
        ASSERT_NE(bSema, nullptr);
        EXPECT_TRUE(bSema->getType()->isInteger());
        // RHS Sema value
        auto *bRhsSema = As<SemaValue>(bStmt->getTarget()->getSema());
        ASSERT_NE(bRhsSema, nullptr);
        EXPECT_TRUE(bSema->getType()->isEquals(bRhsSema->getType()));
        ASTNumberValue* ZeroIntValue = Builder->CreateNumberValue(SourceLocation(), "0");
        ASSERT_EQ(As<ASTNumberValue>(bStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: short c
        auto *cStmt = As<ASTAssignStmt>(Body->getContent()[2]);
        auto *cIdent = As<ASTIdentifier>(cStmt->getSource());
        EXPECT_EQ(cIdent->getName(), "c");
        SemaVar *cSema = cIdent->getSema();
        ASSERT_NE(cSema, nullptr);
        EXPECT_TRUE(cSema->getType()->isInteger());
        auto *cRhsSema = As<SemaValue>(cStmt->getTarget()->getSema());
        ASSERT_NE(cRhsSema, nullptr);
        EXPECT_TRUE(cSema->getType()->isEquals(cRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(cStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: ushort d
        auto *dStmt = As<ASTAssignStmt>(Body->getContent()[3]);
        auto *dIdent = As<ASTIdentifier>(dStmt->getSource());
        EXPECT_EQ(dIdent->getName(), "d");
        SemaVar *dSema = dIdent->getSema();
        ASSERT_NE(dSema, nullptr);
        EXPECT_TRUE(dSema->getType()->isInteger());
        auto *dRhsSema = As<SemaValue>(dStmt->getTarget()->getSema());
        ASSERT_NE(dRhsSema, nullptr);
        EXPECT_TRUE(dSema->getType()->isEquals(dRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(dStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: int e
        auto *eStmt = As<ASTAssignStmt>(Body->getContent()[4]);
        auto *eIdent = As<ASTIdentifier>(eStmt->getSource());
        EXPECT_EQ(eIdent->getName(), "e");
        SemaVar *eSema = eIdent->getSema();
        ASSERT_NE(eSema, nullptr);
        EXPECT_TRUE(eSema->getType()->isInteger());
        auto *eRhsSema = As<SemaValue>(eStmt->getTarget()->getSema());
        ASSERT_NE(eRhsSema, nullptr);
        EXPECT_TRUE(eSema->getType()->isEquals(eRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(eStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: uint f
        auto *fStmt = As<ASTAssignStmt>(Body->getContent()[5]);
        auto *fIdent = As<ASTIdentifier>(fStmt->getSource());
        EXPECT_EQ(fIdent->getName(), "f");
        SemaVar *fSema = fIdent->getSema();
        ASSERT_NE(fSema, nullptr);
        EXPECT_TRUE(fSema->getType()->isInteger());
        auto *fRhsSema = As<SemaValue>(fStmt->getTarget()->getSema());
        ASSERT_NE(fRhsSema, nullptr);
        EXPECT_TRUE(fSema->getType()->isEquals(fRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(fStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: long g
        auto *gStmt = As<ASTAssignStmt>(Body->getContent()[6]);
        auto *gIdent = As<ASTIdentifier>(gStmt->getSource());
        EXPECT_EQ(gIdent->getName(), "g");
        SemaVar *gSema = gIdent->getSema();
        ASSERT_NE(gSema, nullptr);
        EXPECT_TRUE(gSema->getType()->isInteger());
        auto *gRhsSema = As<SemaValue>(gStmt->getTarget()->getSema());
        ASSERT_NE(gRhsSema, nullptr);
        EXPECT_TRUE(gSema->getType()->isEquals(gRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(gStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: ulong h
        auto *hStmt = As<ASTAssignStmt>(Body->getContent()[7]);
        auto *hIdent = As<ASTIdentifier>(hStmt->getSource());
        EXPECT_EQ(hIdent->getName(), "h");
        SemaVar *hSema = hIdent->getSema();
        ASSERT_NE(hSema, nullptr);
        EXPECT_TRUE(hSema->getType()->isInteger());
        auto *hRhsSema = As<SemaValue>(hStmt->getTarget()->getSema());
        ASSERT_NE(hRhsSema, nullptr);
        EXPECT_TRUE(hSema->getType()->isEquals(hRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(hStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: float i
        auto *iStmt = As<ASTAssignStmt>(Body->getContent()[8]);
        auto *iIdent = As<ASTIdentifier>(iStmt->getSource());
        EXPECT_EQ(iIdent->getName(), "i");
        SemaVar *iSema = iIdent->getSema();
        ASSERT_NE(iSema, nullptr);
        EXPECT_TRUE(iSema->getType()->isFloatingPoint());
        ASTNumberValue* ZeroFloatValue = Builder->CreateNumberValue(SourceLocation(), "0.0");
        auto *iRhsSema = As<SemaValue>(iStmt->getTarget()->getSema());
        ASSERT_NE(iRhsSema, nullptr);
        EXPECT_TRUE(iSema->getType()->isEquals(iRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(iStmt->getTarget())->getValue(), ZeroFloatValue->getValue());

        // Test: double j
        auto *jStmt = As<ASTAssignStmt>(Body->getContent()[9]);
        auto *jIdent = As<ASTIdentifier>(jStmt->getSource());
        EXPECT_EQ(jIdent->getName(), "j");
        SemaVar *jSema = jIdent->getSema();
        ASSERT_NE(jSema, nullptr);
        EXPECT_TRUE(jSema->getType()->isFloatingPoint());
        auto *jRhsSema = As<SemaValue>(jStmt->getTarget()->getSema());
        ASSERT_NE(jRhsSema, nullptr);
        EXPECT_TRUE(jSema->getType()->isEquals(jRhsSema->getType()));
        ASSERT_EQ(As<ASTNumberValue>(jStmt->getTarget())->getValue(), ZeroFloatValue->getValue());

        // Test: Type t
        // ASTAssignStmt *tStmt = As<ASTAssignStmt>(Body->getContent()[10]);
        // ASTVar *tVar = As<ASTVar>(tStmt->getSource());
        // EXPECT_EQ(tVar->getName(), "t");
        // EXPECT_EQ(As<ASTIdentityType>(tVar->getType())->getIdentityTypeKind(), ASTIdentityTypeKind::TYPE_NONE);
        // ASSERT_EQ(As<ASTNumberValue>(iStmt->getTarget())->getValue(), ZeroFloatValue->getValue());
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        llvm::StringRef str = (
                "private int func(int a, const float b, bool c=false) {\n"
                "  return 1"
                "}\n");
        ASTModule *Module = Parse("FunctionPrivateReturnParams", str);

        ASSERT_TRUE(Resolve());


        EXPECT_TRUE(Module->getNodes().size() == 1); // func() has PRIVATE Visibility
        ASTFunction *Func = static_cast<ASTFunction *>(Module->getNodes()[0]);
        EXPECT_TRUE(HasModifier(Func->getModifiers(), ASTModifierKind::MOD_PRIVATE));

        ASTParam *Par0 = Func->getParams()[0];
        ASTParam *Par1 = Func->getParams()[1];
        ASTParam *Par2 = Func->getParams()[2];

        EXPECT_EQ(Par0->getName(), "a");
        EXPECT_TRUE(HasBuiltinType(Par0->getType(), ASTBuiltinTypeKind::TYPE_INT));
        EXPECT_FALSE(HasModifier(Par0->getModifiers(), ASTModifierKind::MOD_CONSTANT));

        EXPECT_EQ(Par1->getName(), "b");
        EXPECT_TRUE(HasBuiltinType(Par1->getType(), ASTBuiltinTypeKind::TYPE_FLOAT));
        EXPECT_TRUE(HasModifier(Par1->getModifiers(), ASTModifierKind::MOD_CONSTANT));

        EXPECT_EQ(Par2->getName(), "c");
        EXPECT_TRUE(HasBuiltinType(Par2->getType(), ASTBuiltinTypeKind::TYPE_BOOL));
        EXPECT_FALSE(HasModifier(Par2->getModifiers(), ASTModifierKind::MOD_CONSTANT));

        ASTReturnStmt *ReturnStmt = As<ASTReturnStmt>(Func->getBody()->getContent()[0]);
        EXPECT_EQ(ReturnStmt->getStmtKind(), ASTStmtKind::STMT_RETURN);
        EXPECT_EQ((ReturnStmt->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(ReturnStmt->getExpr())->getValue(), "1");
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
        ASTFunction *doSomeFunc = As<ASTFunction>(Module->getNodes()[0]);
        ASTFunction *doOtherFunc = As<ASTFunction>(Module->getNodes()[1]);
        ASTFunction *doNowFunc = As<ASTFunction>(Module->getNodes()[2]);
        ASTFunction *mainFunc = As<ASTFunction>(Module->getNodes()[3]);

        ASSERT_TRUE(doSomeFunc != nullptr);
        ASSERT_TRUE(doOtherFunc != nullptr);
        ASSERT_TRUE(mainFunc != nullptr);

        // Get main() Body
        ASTBlockStmt *Body = mainFunc->getBody();

        // Test: doSome()
        auto *VarBDecl = As<ASTAssignStmt>(Body->getContent()[0]);
        auto *doSomeCall = As<ASTCall>(VarBDecl->getTarget());
        EXPECT_EQ(doSomeCall->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);

        // Test: doNow()
        auto *VarBAssign = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *doNowCall = As<ASTCall>(VarBAssign->getTarget());
        EXPECT_EQ(doNowCall->getName(), "doNow");
        EXPECT_EQ(doNowCall->getExprKind(), ASTExprKind::EXPR_CALL);

        // Test: doOther(a, b)
        auto *doOtherStmt = As<ASTExprStmt>(Body->getContent()[2]);
        EXPECT_EQ(doOtherStmt->getStmtKind(), ASTStmtKind::STMT_EXPR);
        auto *doOtherCall = As<ASTCall>(doOtherStmt->getExpr());
        EXPECT_EQ(doOtherCall->getName(), "doOther");
        auto *Arg0 = doOtherCall->getArgs()[0];
        EXPECT_EQ(As<ASTIdentifier>(Arg0->getExpr())->getName(), "a");
        auto *Arg1 = doOtherCall->getArgs()[1];
        EXPECT_EQ(As<ASTNumberValue>(Arg1->getExpr())->getValue(), "1");

        // return do()
        auto *RetStmt = As<ASTReturnStmt>(Body->getContent()[3]);
        EXPECT_EQ(As<ASTIdentifier>(RetStmt->getExpr())->getName(), "b");
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
        ASTFunction *err0 = As<ASTFunction>(Module->getNodes()[0]);
        ASTFunction *err1 = As<ASTFunction>(Module->getNodes()[1]);
        ASTFunction *err2 = As<ASTFunction>(Module->getNodes()[2]);
        ASTFunction *main = As<ASTFunction>(Module->getNodes()[3]);

        ASSERT_TRUE(err0 != nullptr);
        ASSERT_TRUE(err1 != nullptr);
        ASSERT_TRUE(err2 != nullptr);
        ASSERT_TRUE(main != nullptr);

        // err0()
        ASTFailStmt *Stmt0 = As<ASTFailStmt>(err0->getBody()->getContent()[0]);
        ASSERT_TRUE(Stmt0->getExpr() == nullptr);

        // err1()
        ASTFailStmt *Stmt1 = As<ASTFailStmt>(err1->getBody()->getContent()[0]);
        ASTNumberValue *Val2 = As<ASTNumberValue>(Stmt1->getExpr());
    	ASSERT_TRUE(Val2->getType()->isInteger());
        ASSERT_EQ(Val2->getValue(), "404");

        // err2()
        ASTFailStmt *Stmt2 = As<ASTFailStmt>(err2->getBody()->getContent()[0]);
        ASTStringValue *Val3 = As<ASTStringValue>(Stmt2->getExpr());
        ASSERT_TRUE(Val3->getType()->isString());
        ASSERT_EQ(Val3->getValue(), "Error");

        // Get main() Body

        // handle err0()
        ASTHandleStmt *HandleStmt = As<ASTHandleStmt>(main->getBody()->getContent()[0]);
        ASSERT_TRUE(HandleStmt->getErrorHandler() == nullptr);
    	ASTBlockStmt *Handle = HandleStmt->getHandle();
    	ASTExprStmt *ExprStmt = As<ASTExprStmt>(Handle->getContent()[0]);
        ASSERT_TRUE(As<ASTCall>(ExprStmt->getExpr())->getArgs().empty());

        // bool b = false
        ASTAssignStmt *bool_err0 = As<ASTAssignStmt>(main->getBody()->getContent()[1]);
        ASSERT_TRUE(bool_err0->getSource()->getType()->isBool());
        ASSERT_EQ(As<ASTBoolValue>(bool_err0->getTarget())->getValue(), false);


    	// error err0 = handle { b = err0() }
    	ASTHandleStmt *error_err0 = As<ASTHandleStmt>(main->getBody()->getContent()[2]);
    	ASSERT_TRUE(error_err0->getErrorHandler()->getType()->isError());

    	// int i = 0
    	ASTAssignStmt *int_err1 = As<ASTAssignStmt>(main->getBody()->getContent()[3]);
    	ASSERT_TRUE(int_err1->getSource()->getType()->isInteger());
    	ASSERT_EQ(As<ASTNumberValue>(int_err1->getTarget())->getValue(), "0");

    	// error err1 = handle { i = err1() }
    	ASTHandleStmt *error_err1 = As<ASTHandleStmt>(main->getBody()->getContent()[4]);
    	ASSERT_TRUE(error_err1->getErrorHandler()->getType()->isError());

    	// string s = ""
    	ASTAssignStmt *string_err2 = As<ASTAssignStmt>(main->getBody()->getContent()[5]);
    	ASSERT_TRUE(string_err2->getSource()->getType()->isString());
    	ASSERT_EQ(As<ASTStringValue>(string_err2->getTarget())->getValue(), "");

    	// error err2 = handle { s = err2() }
    	ASTHandleStmt *error_err2 = As<ASTHandleStmt>(main->getBody()->getContent()[6]);
    	ASSERT_TRUE(error_err2->getErrorHandler()->getType()->isError());
    }
}