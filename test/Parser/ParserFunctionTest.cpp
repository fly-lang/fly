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
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTArg.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTArg.h"

#include <AST/ASTParam.h>
#include <AST/ASTType.h>

namespace {

    using namespace fly;

    TEST_F(ParserTest, FunctionVisibilityDefault) {
        llvm::StringRef str = ("void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityDefault", str);

        EXPECT_TRUE(Module->getNodes().size() == 1);
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(Func->getModifiers().empty());
        EXPECT_TRUE(HasBuiltinType(Func->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
     }

     TEST_F(ParserTest, FunctionVisibilityPrivate) {
        llvm::StringRef str = ("private void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPrivate", str);



        EXPECT_TRUE(Module->getNodes().size() == 1);
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasModifier(Func->getModifiers(), ASTModifierKind::MOD_PRIVATE));
        EXPECT_TRUE(HasBuiltinType(Func->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
     }

     TEST_F(ParserTest, FunctionVisibilityPublic) {
        llvm::StringRef str = ("public void func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPublic", str);

        EXPECT_TRUE(Module->getNodes().size() == 1);
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasModifier(Func->getModifiers(), ASTModifierKind::MOD_PUBLIC));
        EXPECT_TRUE(HasBuiltinType(Func->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
     }

    TEST_F(ParserTest, FunctionType) {
        llvm::StringRef str = ("void func(bool a, "
                               "byte b, short c, ushort d, int e, uint f, long g, ulong h, "
                               "float i, double j) {}\n");
        ASTModule *Module = Parse("FunctionType", str);
    	ASSERT_FALSE(HasErrorOccurred());

        // Get Body
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasBuiltinType(Func->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
    	ASTBlockStmt *Body = Func->getBody();
        ASSERT_TRUE(Body->getContent().empty());

        // Test: bool a
        ASTParam *aVar = Func->getParams()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_TRUE(HasBuiltinType(aVar->getType(), ASTBuiltinTypeKind::TYPE_BOOL));

        // Test: byte b
    	ASTParam *bVar = Func->getParams()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_TRUE(HasBuiltinType(bVar->getType(), ASTBuiltinTypeKind::TYPE_BYTE));

        // Test: short c
        ASTParam *cVar = Func->getParams()[2];
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_TRUE(HasBuiltinType(cVar->getType(), ASTBuiltinTypeKind::TYPE_SHORT));

        // Test: ushort d
        ASTParam *dVar = Func->getParams()[3];
        EXPECT_EQ(dVar->getName(), "d");
        EXPECT_TRUE(HasBuiltinType(dVar->getType(), ASTBuiltinTypeKind::TYPE_USHORT));

        // Test: int e
        ASTParam *eVar = Func->getParams()[4];
        EXPECT_EQ(eVar->getName(), "e");
        EXPECT_TRUE(HasBuiltinType(eVar->getType(), ASTBuiltinTypeKind::TYPE_INT));

        // Test: uint f
        ASTParam *fVar = Func->getParams()[5];
        EXPECT_EQ(fVar->getName(), "f");
        EXPECT_TRUE(HasBuiltinType(fVar->getType(), ASTBuiltinTypeKind::TYPE_UINT));

        // Test: long g
        ASTParam *gVar = Func->getParams()[6];
        EXPECT_EQ(gVar->getName(), "g");
        EXPECT_TRUE(HasBuiltinType(gVar->getType(), ASTBuiltinTypeKind::TYPE_LONG));

        // Test: ulong h
        ASTParam *hVar = Func->getParams()[7];
        EXPECT_EQ(hVar->getName(), "h");
        EXPECT_TRUE(HasBuiltinType(hVar->getType(), ASTBuiltinTypeKind::TYPE_ULONG));

        // Test: float i
        ASTParam *iVar = Func->getParams()[8];
        EXPECT_EQ(iVar->getName(), "i");
        EXPECT_TRUE(HasBuiltinType(iVar->getType(), ASTBuiltinTypeKind::TYPE_FLOAT));

        // Test: double j
        ASTParam *jVar = Func->getParams()[9];
        EXPECT_EQ(jVar->getName(), "j");
        EXPECT_TRUE(HasBuiltinType(jVar->getType(), ASTBuiltinTypeKind::TYPE_DOUBLE));

        // Test: Type t
        // ASTAssignStmt *tStmt = static_cast<ASTAssignStmt *>(Body->getContent()[10]);
        // ASTVar *tVar = tStmt->getVarRef()->getDef();
        // EXPECT_EQ(tVar->getName(), "t");
        // EXPECT_EQ(static_cast<ASTIdentityType *>(tVar->getType())->getIdentityTypeKind(), ASTIdentityTypeKind::TYPE_NONE);
        // ASSERT_EQ(tStmt->getExpr(), nullptr);
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        llvm::StringRef str = (
                "private int func(int a, const float b, bool c=false) {\n"
                "  return 1"
                "}\n");
        ASTModule *Module = Parse("FunctionPrivateReturnParams", str);

        EXPECT_TRUE(Module->getNodes().size() == 1); // func() has PRIVATE Visibility
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);

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

        auto *Return = As<ASTReturnStmt>(Func->getBody()->getContent()[0]);
        ASSERT_FALSE(Func->getBody()->getContent().empty());
         EXPECT_EQ(Return->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
         EXPECT_EQ(As<ASTNumberValue>(Return->getExpr())->getValue(), "1");
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



        // Get all functions
        auto *doSome = As<ASTFunction>(Module->getNodes()[0]);
        auto *doOther = As<ASTFunction>(Module->getNodes()[1]);
        auto *doNow = As<ASTFunction>(Module->getNodes()[2]);
        auto *main = As<ASTFunction>(Module->getNodes()[3]);

        ASSERT_TRUE(doSome != nullptr);
        ASSERT_TRUE(doOther != nullptr);
        ASSERT_TRUE(main != nullptr);

        // Get main() Body
        auto *Body = main->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // Test: doSome()
        auto *VarB = As<ASTAssignStmt>(Body->getContent()[0]);
        auto *doSomeCall = As<ASTCall>(VarB->getTarget());
        EXPECT_EQ(doSomeCall->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);

        // Test: doNow()
        auto *B = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *doNowCall = As<ASTCall>(B->getTarget());
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

        // return b
        auto *RetStmt = As<ASTReturnStmt>(Body->getContent()[3]);
        EXPECT_EQ(As<ASTIdentifier>(RetStmt->getExpr())->getName(), "b");
    }

}