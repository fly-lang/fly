//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserLocalVarTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTModule.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTType.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTOp.h"

#include <AST/ASTExprStmt.h>

namespace {

    using namespace fly;

    TEST_F(ParserTest, LocalVarBuiltinType) {
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
                                   "Type t = null\n"
                               "}\n");
        ASTModule *Module = Parse("LocalVarBuiltinType", str);

        // Get Body
        auto *F = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasBuiltinType(F->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
        auto *Body = F->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // Test: bool a
        auto *aStmt = As<ASTExprStmt>(Body->getContent()[0]);
        auto *aAssignExpr = As<ASTBinaryOp>(aStmt->getExpr());
        ASSERT_TRUE(aAssignExpr != nullptr);
        EXPECT_EQ(aAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *aIdent = As<ASTIdentifier>(aAssignExpr->getLeftExpr());
        EXPECT_EQ(aIdent->getName(), "a");
        EXPECT_EQ(As<ASTBoolValue>(aAssignExpr->getRightExpr())->getValue(), false);

        // Test: byte b
        auto *bStmt = As<ASTExprStmt>(Body->getContent()[1]);
        auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getExpr());
        ASSERT_TRUE(bAssignExpr != nullptr);
        auto *bIdent = As<ASTIdentifier>(bAssignExpr->getLeftExpr());
        EXPECT_EQ(bIdent->getName(), "b");
        ASTNumberValue* ZeroIntValue = Builder->CreateNumberValue(SourceLocation(), "0");
        ASSERT_EQ(As<ASTNumberValue>(bAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: short c
        auto *cStmt = As<ASTExprStmt>(Body->getContent()[2]);
        auto *cAssignExpr = As<ASTBinaryOp>(cStmt->getExpr());
        ASSERT_TRUE(cAssignExpr != nullptr);
        auto *cIdent = As<ASTIdentifier>(cAssignExpr->getLeftExpr());
        EXPECT_EQ(cIdent->getName(), "c");
        ASSERT_EQ(As<ASTNumberValue>(cAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: ushort d
        auto *dStmt = As<ASTExprStmt>(Body->getContent()[3]);
        auto *dAssignExpr = As<ASTBinaryOp>(dStmt->getExpr());
        ASSERT_TRUE(dAssignExpr != nullptr);
        auto *dIdent = As<ASTIdentifier>(dAssignExpr->getLeftExpr());
        EXPECT_EQ(dIdent->getName(), "d");
        ASSERT_EQ(As<ASTNumberValue>(dAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: int e
        auto *eStmt = As<ASTExprStmt>(Body->getContent()[4]);
        auto *eAssignExpr = As<ASTBinaryOp>(eStmt->getExpr());
        ASSERT_TRUE(eAssignExpr != nullptr);
        auto *eIdent = As<ASTIdentifier>(eAssignExpr->getLeftExpr());
        EXPECT_EQ(eIdent->getName(), "e");
        ASSERT_EQ(As<ASTNumberValue>(eAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: uint f
        auto *fStmt = As<ASTExprStmt>(Body->getContent()[5]);
        auto *fAssignExpr = As<ASTBinaryOp>(fStmt->getExpr());
        ASSERT_TRUE(fAssignExpr != nullptr);
        auto *fIdent = As<ASTIdentifier>(fAssignExpr->getLeftExpr());
        EXPECT_EQ(fIdent->getName(), "f");
        ASSERT_EQ(As<ASTNumberValue>(fAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: long g
        auto *gStmt = As<ASTExprStmt>(Body->getContent()[6]);
        auto *gAssignExpr = As<ASTBinaryOp>(gStmt->getExpr());
        ASSERT_TRUE(gAssignExpr != nullptr);
        auto *gIdent = As<ASTIdentifier>(gAssignExpr->getLeftExpr());
        EXPECT_EQ(gIdent->getName(), "g");
        ASSERT_EQ(As<ASTNumberValue>(gAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: ulong h
        auto *hStmt = As<ASTExprStmt>(Body->getContent()[7]);
        auto *hAssignExpr = As<ASTBinaryOp>(hStmt->getExpr());
        ASSERT_TRUE(hAssignExpr != nullptr);
        auto *hIdent = As<ASTIdentifier>(hAssignExpr->getLeftExpr());
        EXPECT_EQ(hIdent->getName(), "h");
        ASSERT_EQ(As<ASTNumberValue>(hAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: float i
        auto *iStmt = As<ASTExprStmt>(Body->getContent()[8]);
        auto *iAssignExpr = As<ASTBinaryOp>(iStmt->getExpr());
        ASSERT_TRUE(iAssignExpr != nullptr);
        auto *iIdent = As<ASTIdentifier>(iAssignExpr->getLeftExpr());
        EXPECT_EQ(iIdent->getName(), "i");
        ASTNumberValue* ZeroFloatValue = Builder->CreateNumberValue(SourceLocation(), "0.0");
        ASSERT_EQ(As<ASTNumberValue>(iAssignExpr->getRightExpr())->getValue(), ZeroFloatValue->getValue());

        // Test: double j
        auto *jStmt = As<ASTExprStmt>(Body->getContent()[9]);
        auto *jAssignExpr = As<ASTBinaryOp>(jStmt->getExpr());
        ASSERT_TRUE(jAssignExpr != nullptr);
        auto *jIdent = As<ASTIdentifier>(jAssignExpr->getLeftExpr());
        EXPECT_EQ(jIdent->getName(), "j");
        ASSERT_EQ(As<ASTNumberValue>(jAssignExpr->getRightExpr())->getValue(), ZeroFloatValue->getValue());

        // Test: Type t
        auto *tStmt = As<ASTExprStmt>(Body->getContent()[10]);
        auto *tAssignExpr = As<ASTBinaryOp>(tStmt->getExpr());
        ASSERT_TRUE(tAssignExpr != nullptr);
        EXPECT_EQ(tAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *tIdent = As<ASTIdentifier>(tAssignExpr->getLeftExpr());
        EXPECT_EQ(tIdent->getName(), "t");
        // The right side should be a null value
        EXPECT_EQ(tAssignExpr->getRightExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(As<ASTNamedType>(tIdent->getVar()->getType())->getTypeKind(), ASTTypeKind::TYPE_NAMED);
    }

	TEST_F(ParserTest, LocalVarArray) {
    	llvm::StringRef str = ("void func() {\n"
		                       "byte[] a\n"
		                       "byte[] b = {}\n"// array of zero bytes
		                       "byte[] c = {1, 2, 3}\n"
		                       "byte[3] d\n"
		                       "byte[3] e = {1, 2, 3}\n"
		               "}\n");
    	ASTModule *Module = Parse("LocalVarArrayNull", str);

    	// Get Body
    	auto *F = As<ASTFunction>(Module->getNodes()[0]);
    	EXPECT_TRUE(HasBuiltinType(F->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
    	auto *Body = F->getBody();
    	ASSERT_FALSE(Body->getContent().empty());

    	// a: declared byte[] a
    	auto *aStmt = As<ASTExprStmt>(Body->getContent()[0]);
    	auto *aExpr = aStmt->getExpr();

    	// For declaration without initializer, the expression is just the identifier
    	ASTIdentifier *aIdent = nullptr;
    	if (aExpr->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
    		aIdent = As<ASTIdentifier>(aExpr);
    	} else if (aExpr->getExprKind() == ASTExprKind::EXPR_BINARY) {
    		auto *aAssignExpr = As<ASTBinaryOp>(aExpr);
    		aIdent = As<ASTIdentifier>(aAssignExpr->getLeftExpr());
    	}
    	ASSERT_TRUE(aIdent != nullptr);
    	EXPECT_EQ(aIdent->getName(), "a");

    	ASTLocalVar *aVar = As<ASTLocalVar>(aIdent->getVar());
    	ASSERT_TRUE(aVar != nullptr);
    	ASTType *aType = aVar->getType();
    	ASSERT_TRUE(aType != nullptr);
    	EXPECT_EQ(aType->getTypeKind(), ASTTypeKind::TYPE_ARRAY);
    	auto *ArrTypeA = As<ASTArrayType>(aType);
    	ASSERT_TRUE(ArrTypeA != nullptr);
    	EXPECT_TRUE(HasBuiltinType(ArrTypeA->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	EXPECT_EQ(ArrTypeA->getSizeExpr(), nullptr);

    	// b: empty initializer {}
    	auto *bStmt = As<ASTExprStmt>(Body->getContent()[1]);
    	auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getExpr());
    	ASSERT_TRUE(bAssignExpr != nullptr);
    	EXPECT_EQ(bAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
    	auto *bIdent = As<ASTIdentifier>(bAssignExpr->getLeftExpr());
    	EXPECT_EQ(bIdent->getName(), "b");
    	ASTLocalVar *bVar = As<ASTLocalVar>(bIdent->getVar());
    	ASSERT_TRUE(bVar != nullptr);
    	auto *bType = As<ASTArrayType>(bVar->getType());
    	ASSERT_TRUE(bType != nullptr);
    	EXPECT_TRUE(HasBuiltinType(bType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	EXPECT_EQ(bType->getSizeExpr(), nullptr);

    	// b value must be ASTArrayValue with size 0
    	auto *bVal = As<ASTArrayValue>(bAssignExpr->getRightExpr());
    	ASSERT_TRUE(bVal != nullptr);
    	EXPECT_EQ(bVal->size(), 0u);

    	// c: initialized {1,2,3}
    	auto *cStmt = As<ASTExprStmt>(Body->getContent()[2]);
    	auto *cAssignExpr = As<ASTBinaryOp>(cStmt->getExpr());
    	ASSERT_TRUE(cAssignExpr != nullptr);
    	EXPECT_EQ(cAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
    	auto *cIdent = As<ASTIdentifier>(cAssignExpr->getLeftExpr());
    	EXPECT_EQ(cIdent->getName(), "c");
    	ASTLocalVar *cVar = As<ASTLocalVar>(cIdent->getVar());
    	ASSERT_TRUE(cVar != nullptr);
    	auto *cType = As<ASTArrayType>(cVar->getType());
    	ASSERT_TRUE(cType != nullptr);
    	EXPECT_TRUE(HasBuiltinType(cType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	EXPECT_EQ(cType->getSizeExpr(), nullptr);

    	auto *cVal = As<ASTArrayValue>(cAssignExpr->getRightExpr());
    	ASSERT_TRUE(cVal != nullptr);
    	EXPECT_EQ(cVal->size(), 3u);
    	EXPECT_EQ(As<ASTNumberValue>(cVal->getValues()[0])->getValue(), "1");
    	EXPECT_EQ(As<ASTNumberValue>(cVal->getValues()[1])->getValue(), "2");
    	EXPECT_EQ(As<ASTNumberValue>(cVal->getValues()[2])->getValue(), "3");

    	// d: byte[3] d (sized, no initializer)
    	auto *dStmt = As<ASTExprStmt>(Body->getContent()[3]);
    	auto *dExpr = dStmt->getExpr();

    	// For declaration without initializer, the expression is just the identifier
    	ASTIdentifier *dIdent = nullptr;
    	if (dExpr->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
    		dIdent = As<ASTIdentifier>(dExpr);
    	} else if (dExpr->getExprKind() == ASTExprKind::EXPR_BINARY) {
    		auto *dAssignExpr = As<ASTBinaryOp>(dExpr);
    		dIdent = As<ASTIdentifier>(dAssignExpr->getLeftExpr());
    	}
    	ASSERT_TRUE(dIdent != nullptr);
    	EXPECT_EQ(dIdent->getName(), "d");
    	ASTLocalVar *dVar = As<ASTLocalVar>(dIdent->getVar());
    	ASSERT_TRUE(dVar != nullptr);
    	auto *dType = As<ASTArrayType>(dVar->getType());
    	ASSERT_TRUE(dType != nullptr);
    	EXPECT_TRUE(HasBuiltinType(dType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	// size expr should be a number value "3"
    	ASSERT_TRUE(dType->getSizeExpr() != nullptr);
    	EXPECT_EQ(As<ASTNumberValue>(dType->getSizeExpr())->getValue(), "3");

    	// e: byte[3] e = {1,2,3}
    	auto *eStmt = As<ASTExprStmt>(Body->getContent()[4]);
    	auto *eAssignExpr = As<ASTBinaryOp>(eStmt->getExpr());
    	ASSERT_TRUE(eAssignExpr != nullptr);
    	EXPECT_EQ(eAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
    	auto *eIdent = As<ASTIdentifier>(eAssignExpr->getLeftExpr());
    	EXPECT_EQ(eIdent->getName(), "e");
    	ASTLocalVar *eVar = As<ASTLocalVar>(eIdent->getVar());
    	ASSERT_TRUE(eVar != nullptr);
    	auto *eType = As<ASTArrayType>(eVar->getType());
    	ASSERT_TRUE(eType != nullptr);
    	EXPECT_TRUE(HasBuiltinType(eType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	ASSERT_TRUE(eType->getSizeExpr() != nullptr);
    	EXPECT_EQ(As<ASTNumberValue>(eType->getSizeExpr())->getValue(), "3");

    	auto *eVal = As<ASTArrayValue>(eAssignExpr->getRightExpr());
    	ASSERT_TRUE(eVal != nullptr);
    	EXPECT_EQ(eVal->size(), 3u);
    	EXPECT_EQ(As<ASTNumberValue>(eVal->getValues()[0])->getValue(), "1");
    	EXPECT_EQ(As<ASTNumberValue>(eVal->getValues()[1])->getValue(), "2");
    	EXPECT_EQ(As<ASTNumberValue>(eVal->getValues()[2])->getValue(), "3");
    }

    TEST_F(ParserTest, LocalVarChar) {
    	llvm::StringRef str = ("void func() {\n"
						"byte a = ''\n"
						"byte b = 'b'\n"
						"byte[] c = {'a', 'b', 'c', ''}\n"
						"byte[2] d = {'', ''}\n" // Empty string
					"}\n");
        ASTModule *Module = Parse("LocalVarChar", str);

        // Get Body
        auto *F = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasBuiltinType(F->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
        auto *Body = F->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // a: byte a = ''
        auto *aStmt = As<ASTExprStmt>(Body->getContent()[0]);
        auto *aAssignExpr = As<ASTBinaryOp>(aStmt->getExpr());
        ASSERT_TRUE(aAssignExpr != nullptr);
        EXPECT_EQ(aAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *aIdent = As<ASTIdentifier>(aAssignExpr->getLeftExpr());
        EXPECT_EQ(aIdent->getName(), "a");
        auto *aVar = As<ASTLocalVar>(aIdent->getVar());
        ASSERT_TRUE(aVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(aVar->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        {
            auto *aVal = As<ASTStringValue>(aAssignExpr->getRightExpr());
            ASSERT_TRUE(aVal != nullptr);
            EXPECT_EQ(aVal->getValue(), "");
        }

        // b: byte b = 'b'
        auto *bStmt = As<ASTExprStmt>(Body->getContent()[1]);
        auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getExpr());
        ASSERT_TRUE(bAssignExpr != nullptr);
        EXPECT_EQ(bAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *bIdent = As<ASTIdentifier>(bAssignExpr->getLeftExpr());
        EXPECT_EQ(bIdent->getName(), "b");
        auto *bVar = As<ASTLocalVar>(bIdent->getVar());
        ASSERT_TRUE(bVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(bVar->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        {
            auto *bVal = As<ASTStringValue>(bAssignExpr->getRightExpr());
            ASSERT_TRUE(bVal != nullptr);
            EXPECT_EQ(bVal->getValue(), "b");
        }

        // c: byte[] c = {'a', 'b', 'c', ''}
        auto *cStmt = As<ASTExprStmt>(Body->getContent()[2]);
        auto *cAssignExpr = As<ASTBinaryOp>(cStmt->getExpr());
        ASSERT_TRUE(cAssignExpr != nullptr);
        EXPECT_EQ(cAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *cIdent = As<ASTIdentifier>(cAssignExpr->getLeftExpr());
        EXPECT_EQ(cIdent->getName(), "c");
        auto *cVar = As<ASTLocalVar>(cIdent->getVar());
        ASSERT_TRUE(cVar != nullptr);
        auto *cType = As<ASTArrayType>(cVar->getType());
        ASSERT_TRUE(cType != nullptr);
        EXPECT_TRUE(HasBuiltinType(cType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(cType->getSizeExpr(), nullptr);
        auto *cVal = As<ASTArrayValue>(cAssignExpr->getRightExpr());
        ASSERT_TRUE(cVal != nullptr);
        EXPECT_EQ(cVal->size(), 4u);
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[0])->getValue(), "a");
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[1])->getValue(), "b");
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[2])->getValue(), "c");
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[3])->getValue(), "");

        // d: byte[2] d = {'', ''}
        auto *dStmt = As<ASTExprStmt>(Body->getContent()[3]);
        auto *dAssignExpr = As<ASTBinaryOp>(dStmt->getExpr());
        ASSERT_TRUE(dAssignExpr != nullptr);
        EXPECT_EQ(dAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *dIdent = As<ASTIdentifier>(dAssignExpr->getLeftExpr());
        EXPECT_EQ(dIdent->getName(), "d");
        auto *dVar = As<ASTLocalVar>(dIdent->getVar());
        ASSERT_TRUE(dVar != nullptr);
        auto *dType = As<ASTArrayType>(dVar->getType());
        ASSERT_TRUE(dType != nullptr);
        EXPECT_TRUE(HasBuiltinType(dType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
        ASSERT_TRUE(dType->getSizeExpr() != nullptr);
        EXPECT_EQ(As<ASTNumberValue>(dType->getSizeExpr())->getValue(), "2");
        auto *dVal = As<ASTArrayValue>(dAssignExpr->getRightExpr());
        ASSERT_TRUE(dVal != nullptr);
        EXPECT_EQ(dVal->size(), 2u);
        EXPECT_EQ(As<ASTStringValue>(dVal->getValues()[0])->getValue(), "");
        EXPECT_EQ(As<ASTStringValue>(dVal->getValues()[1])->getValue(), "");
    }

    TEST_F(ParserTest, LocalVarString) {
    	llvm::StringRef str = ("void func() {\n"
						"string c\n"
						"string a = \"\"\n" // array of zero bytes
						"string b = \"abc\"\n" // string abc
					"}\n");
        ASTModule *Module = Parse("LocalVarString", str);

        // Get Body
        auto *F = As<ASTFunction>(Module->getNodes()[0]);
        EXPECT_TRUE(HasBuiltinType(F->getReturnType(), ASTBuiltinTypeKind::TYPE_VOID));
        auto *Body = F->getBody();
        ASSERT_FALSE(Body->getContent().empty());

        // c: declaration without initializer
        auto *cStmt = As<ASTExprStmt>(Body->getContent()[0]);
        auto *cExpr = cStmt->getExpr();

        // For declaration without initializer, the expression is just the identifier
        ASTIdentifier *cIdent = nullptr;
        if (cExpr->getExprKind() == ASTExprKind::EXPR_IDENTIFIER) {
            cIdent = As<ASTIdentifier>(cExpr);
        } else if (cExpr->getExprKind() == ASTExprKind::EXPR_BINARY) {
            auto *cAssignExpr = As<ASTBinaryOp>(cExpr);
            cIdent = As<ASTIdentifier>(cAssignExpr->getLeftExpr());
        }
        ASSERT_TRUE(cIdent != nullptr);
        EXPECT_EQ(cIdent->getName(), "c");
        auto *cVar = As<ASTLocalVar>(cIdent->getVar());
        ASSERT_TRUE(cVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(cVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));

        // a: empty string literal
        auto *aStmt = As<ASTExprStmt>(Body->getContent()[1]);
        auto *aAssignExpr = As<ASTBinaryOp>(aStmt->getExpr());
        ASSERT_TRUE(aAssignExpr != nullptr);
        EXPECT_EQ(aAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *aIdent = As<ASTIdentifier>(aAssignExpr->getLeftExpr());
        EXPECT_EQ(aIdent->getName(), "a");
        auto *aVar = As<ASTLocalVar>(aIdent->getVar());
        ASSERT_TRUE(aVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(aVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));
        auto *aVal = As<ASTStringValue>(aAssignExpr->getRightExpr());
        ASSERT_TRUE(aVal != nullptr);
        EXPECT_EQ(aVal->getValue(), "");

        // b: "abc"
        auto *bStmt = As<ASTExprStmt>(Body->getContent()[2]);
        auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getExpr());
        ASSERT_TRUE(bAssignExpr != nullptr);
        EXPECT_EQ(bAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *bIdent = As<ASTIdentifier>(bAssignExpr->getLeftExpr());
        EXPECT_EQ(bIdent->getName(), "b");
        auto *bVar = As<ASTLocalVar>(bIdent->getVar());
        ASSERT_TRUE(bVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(bVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));
        auto *bVal = As<ASTStringValue>(bAssignExpr->getRightExpr());
        ASSERT_TRUE(bVal != nullptr);
        EXPECT_EQ(bVal->getValue(), "abc");
    }
}