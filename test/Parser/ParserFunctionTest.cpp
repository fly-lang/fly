//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTArg.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTModule.h"
#include "AST/ASTValue.h"
#include "ParserTest.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTParam.h>
#include <AST/ASTType.h>

namespace {

    using namespace fly;

    TEST_F(ParserTest, FunctionVisibilityDefault) {
        llvm::StringRef str = ("func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityDefault", str);

        EXPECT_TRUE(Module->getNodes().size() == 1);
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(Func->getModifiers().empty());
     }

     TEST_F(ParserTest, FunctionVisibilityPrivate) {
        llvm::StringRef str = ("private func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPrivate", str);



        EXPECT_TRUE(Module->getNodes().size() == 1);
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasModifier(Func->getModifiers(), ASTModifierKind::MOD_PRIVATE));
     }

     TEST_F(ParserTest, FunctionVisibilityPublic) {
        llvm::StringRef str = ("public func() {}\n");
        ASTModule *Module = Parse("FunctionVisibilityPublic", str);

        EXPECT_TRUE(Module->getNodes().size() == 1);
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasModifier(Func->getModifiers(), ASTModifierKind::MOD_PUBLIC));
     }

    TEST_F(ParserTest, FunctionType) {
        llvm::StringRef str = ("func(bool a, "
                               "byte b, short c, ushort d, int e, uint f, long g, ulong h, "
                               "float i, double j) {}\n");
        ASTModule *Module = Parse("FunctionType", str);
    	ASSERT_FALSE(HasErrorOccurred());

        // Get Body
        auto *Func = As<ASTFunction>(Module->getNodes()[0]);
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
    }

    TEST_F(ParserTest, FunctionPrivateFailParams) {
        llvm::StringRef str = (
                "private func(int a, const float b, bool c=false) {\n"
                "  fail 1\n"
                "}\n");
        ASTModule *Module = Parse("FunctionPrivateFailParams", str);

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

        ASSERT_FALSE(Func->getBody()->getContent().empty());
        auto *FailStmt = As<ASTFailStmt>(Func->getBody()->getContent()[0]);
        EXPECT_EQ(FailStmt->getStmtKind(), ASTStmtKind::STMT_FAIL);
        EXPECT_EQ(FailStmt->getFirstExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNumberValue>(FailStmt->getFirstExpr())->getValue(), "1");
     }

     TEST_F(ParserTest, FunctionCall) {
        llvm::StringRef str = ("private doSome(int b) {}\n"
                               "doNow() {}\n"
                               "do(int a) {\n"
                               "  int b\n"
                               "  doSome(b)\n"
                               "  doNow()\n"
                               "}\n");
        ASTModule *Module = Parse("FunctionCall", str);

        // Get all functions
        auto *doSome = As<ASTFunction>(Module->getNodes()[0]);
        auto *doNow = As<ASTFunction>(Module->getNodes()[1]);
        auto *doFunc = As<ASTFunction>(Module->getNodes()[2]);

        ASSERT_TRUE(doSome != nullptr);
        ASSERT_TRUE(doNow != nullptr);
        ASSERT_TRUE(doFunc != nullptr);

        // Get do() Body
        auto *Body = doFunc->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // Test: int b
        auto *VarBStmt = As<ASTDeclStmt>(Body->getContent()[0]);
        EXPECT_EQ(VarBStmt->getLocalVar()->getName(), "b");

        // Test: doSome(b)
        auto *doSomeStmt = As<ASTExprStmt>(Body->getContent()[1]);
        EXPECT_EQ(doSomeStmt->getStmtKind(), ASTStmtKind::STMT_EXPR);
        auto *doSomeCall = As<ASTCall>(doSomeStmt->getExpr());
        ASSERT_TRUE(doSomeCall != nullptr);
        EXPECT_EQ(doSomeCall->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);
        auto *Arg0 = doSomeCall->getArgs()[0];
        EXPECT_EQ(As<ASTIdentifier>(Arg0->getExpr())->getName(), "b");

        // Test: doNow()
        auto *doNowStmt = As<ASTExprStmt>(Body->getContent()[2]);
        EXPECT_EQ(doNowStmt->getStmtKind(), ASTStmtKind::STMT_EXPR);
        auto *doNowCall = As<ASTCall>(doNowStmt->getExpr());
        ASSERT_TRUE(doNowCall != nullptr);
        EXPECT_EQ(doNowCall->getName(), "doNow");
        EXPECT_EQ(doNowCall->getExprKind(), ASTExprKind::EXPR_CALL);
    }

}