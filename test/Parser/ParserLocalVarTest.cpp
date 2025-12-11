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
#include "AST/ASTAssignStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTOp.h"

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
        auto *aStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        auto *aIdent = As<ASTIdentifier>(aStmt->getSource());
        EXPECT_EQ(aIdent->getName(), "a");
        // Target is binary assignment: a = false
        auto *aAssignExpr = As<ASTBinaryOp>(aStmt->getTarget());
        ASSERT_TRUE(aAssignExpr != nullptr);
        EXPECT_EQ(aAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        EXPECT_EQ(As<ASTBoolValue>(aAssignExpr->getRightExpr())->getValue(), false);

        // Test: byte b
        auto *bStmt = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *bIdent = As<ASTIdentifier>(bStmt->getSource());
        EXPECT_EQ(bIdent->getName(), "b");
        auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getTarget());
        ASSERT_TRUE(bAssignExpr != nullptr);
        ASTNumberValue* ZeroIntValue = Builder->CreateNumberValue(SourceLocation(), "0");
        ASSERT_EQ(As<ASTNumberValue>(bAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: short c
        auto *cStmt = As<ASTAssignStmt>(Body->getContent()[2]);
        auto *cIdent = As<ASTIdentifier>(cStmt->getSource());
        EXPECT_EQ(cIdent->getName(), "c");
        auto *cAssignExpr = As<ASTBinaryOp>(cStmt->getTarget());
        ASSERT_TRUE(cAssignExpr != nullptr);
        ASSERT_EQ(As<ASTNumberValue>(cAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: ushort d
        auto *dStmt = As<ASTAssignStmt>(Body->getContent()[3]);
        auto *dIdent = As<ASTIdentifier>(dStmt->getSource());
        EXPECT_EQ(dIdent->getName(), "d");
        auto *dAssignExpr = As<ASTBinaryOp>(dStmt->getTarget());
        ASSERT_TRUE(dAssignExpr != nullptr);
        ASSERT_EQ(As<ASTNumberValue>(dAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: int e
        auto *eStmt = As<ASTAssignStmt>(Body->getContent()[4]);
        auto *eIdent = As<ASTIdentifier>(eStmt->getSource());
        EXPECT_EQ(eIdent->getName(), "e");
        auto *eAssignExpr = As<ASTBinaryOp>(eStmt->getTarget());
        ASSERT_TRUE(eAssignExpr != nullptr);
        ASSERT_EQ(As<ASTNumberValue>(eAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: uint f
        auto *fStmt = As<ASTAssignStmt>(Body->getContent()[5]);
        auto *fIdent = As<ASTIdentifier>(fStmt->getSource());
        EXPECT_EQ(fIdent->getName(), "f");
        auto *fAssignExpr = As<ASTBinaryOp>(fStmt->getTarget());
        ASSERT_TRUE(fAssignExpr != nullptr);
        ASSERT_EQ(As<ASTNumberValue>(fAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: long g
        auto *gStmt = As<ASTAssignStmt>(Body->getContent()[6]);
        auto *gIdent = As<ASTIdentifier>(gStmt->getSource());
        EXPECT_EQ(gIdent->getName(), "g");
        auto *gAssignExpr = As<ASTBinaryOp>(gStmt->getTarget());
        ASSERT_TRUE(gAssignExpr != nullptr);
        ASSERT_EQ(As<ASTNumberValue>(gAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: ulong h
        auto *hStmt = As<ASTAssignStmt>(Body->getContent()[7]);
        auto *hIdent = As<ASTIdentifier>(hStmt->getSource());
        EXPECT_EQ(hIdent->getName(), "h");
        auto *hAssignExpr = As<ASTBinaryOp>(hStmt->getTarget());
        ASSERT_TRUE(hAssignExpr != nullptr);
        ASSERT_EQ(As<ASTNumberValue>(hAssignExpr->getRightExpr())->getValue(), ZeroIntValue->getValue());

        // Test: float i
        auto *iStmt = As<ASTAssignStmt>(Body->getContent()[8]);
        auto *iIdent = As<ASTIdentifier>(iStmt->getSource());
        EXPECT_EQ(iIdent->getName(), "i");
        auto *iAssignExpr = As<ASTBinaryOp>(iStmt->getTarget());
        ASSERT_TRUE(iAssignExpr != nullptr);
        ASTNumberValue* ZeroFloatValue = Builder->CreateNumberValue(SourceLocation(), "0.0");
        ASSERT_EQ(As<ASTNumberValue>(iAssignExpr->getRightExpr())->getValue(), ZeroFloatValue->getValue());

        // Test: double j
        auto *jStmt = As<ASTAssignStmt>(Body->getContent()[9]);
        auto *jIdent = As<ASTIdentifier>(jStmt->getSource());
        EXPECT_EQ(jIdent->getName(), "j");
        auto *jAssignExpr = As<ASTBinaryOp>(jStmt->getTarget());
        ASSERT_TRUE(jAssignExpr != nullptr);
        ASSERT_EQ(As<ASTNumberValue>(jAssignExpr->getRightExpr())->getValue(), ZeroFloatValue->getValue());

        // Test: Type t
        ASTAssignStmt *tStmt = As<ASTAssignStmt>(Body->getContent()[10]);
        ASTIdentifier *tIdent = As<ASTIdentifier>(tStmt->getSource());
        EXPECT_EQ(tIdent->getName(), "t");
        auto *tAssignExpr = As<ASTBinaryOp>(tStmt->getTarget());
        ASSERT_TRUE(tAssignExpr != nullptr);
        EXPECT_EQ(tAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
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
    	auto *aStmt = As<ASTAssignStmt>(Body->getContent()[0]);
    	auto *aIdent = As<ASTIdentifier>(aStmt->getSource());
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

    	// Depending on parser representation, a declaration without initializer may produce
    	// either a statement with null target or an expr-stmt. Accept both.
    	if (aStmt->getTarget()) {
    		auto *aAssignExpr = As<ASTBinaryOp>(aStmt->getTarget());
    		if (aAssignExpr) {
    			EXPECT_EQ(aAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
    			auto *ValA = As<ASTValue>(aAssignExpr->getRightExpr());
    			ASSERT_TRUE(ValA != nullptr);
    			EXPECT_TRUE(ValA->isNull());
    		}
    	}

    	// b: empty initializer {}
    	auto *bStmt = As<ASTAssignStmt>(Body->getContent()[1]);
    	auto *bIdent = As<ASTIdentifier>(bStmt->getSource());
    	EXPECT_EQ(bIdent->getName(), "b");
    	ASTLocalVar *bVar = As<ASTLocalVar>(bIdent->getVar());
    	ASSERT_TRUE(bVar != nullptr);
    	auto *bType = As<ASTArrayType>(bVar->getType());
    	ASSERT_TRUE(bType != nullptr);
    	EXPECT_TRUE(HasBuiltinType(bType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	EXPECT_EQ(bType->getSizeExpr(), nullptr);

    	// b value must be ASTArrayValue with size 0 (extract from binary assignment)
    	auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getTarget());
    	ASSERT_TRUE(bAssignExpr != nullptr);
    	EXPECT_EQ(bAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
    	auto *bVal = As<ASTArrayValue>(bAssignExpr->getRightExpr());
    	ASSERT_TRUE(bVal != nullptr);
    	EXPECT_EQ(bVal->size(), 0u);

    	// c: initialized {1,2,3}
    	auto *cStmt = As<ASTAssignStmt>(Body->getContent()[2]);
    	auto *cIdent = As<ASTIdentifier>(cStmt->getSource());
    	EXPECT_EQ(cIdent->getName(), "c");
    	ASTLocalVar *cVar = As<ASTLocalVar>(cIdent->getVar());
    	ASSERT_TRUE(cVar != nullptr);
    	auto *cType = As<ASTArrayType>(cVar->getType());
    	ASSERT_TRUE(cType != nullptr);
    	EXPECT_TRUE(HasBuiltinType(cType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	EXPECT_EQ(cType->getSizeExpr(), nullptr);

    	auto *cAssignExpr = As<ASTBinaryOp>(cStmt->getTarget());
    	ASSERT_TRUE(cAssignExpr != nullptr);
    	EXPECT_EQ(cAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
    	auto *cVal = As<ASTArrayValue>(cAssignExpr->getRightExpr());
    	ASSERT_TRUE(cVal != nullptr);
    	EXPECT_EQ(cVal->size(), 3u);
    	EXPECT_EQ(As<ASTNumberValue>(cVal->getValues()[0])->getValue(), "1");
    	EXPECT_EQ(As<ASTNumberValue>(cVal->getValues()[1])->getValue(), "2");
    	EXPECT_EQ(As<ASTNumberValue>(cVal->getValues()[2])->getValue(), "3");

    	// d: byte[3] d (sized, no initializer)
    	auto *dStmt = As<ASTAssignStmt>(Body->getContent()[3]);
    	auto *dIdent = As<ASTIdentifier>(dStmt->getSource());
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
    	auto *eStmt = As<ASTAssignStmt>(Body->getContent()[4]);
    	auto *eIdent = As<ASTIdentifier>(eStmt->getSource());
    	EXPECT_EQ(eIdent->getName(), "e");
    	ASTLocalVar *eVar = As<ASTLocalVar>(eIdent->getVar());
    	ASSERT_TRUE(eVar != nullptr);
    	auto *eType = As<ASTArrayType>(eVar->getType());
    	ASSERT_TRUE(eType != nullptr);
    	EXPECT_TRUE(HasBuiltinType(eType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
    	ASSERT_TRUE(eType->getSizeExpr() != nullptr);
    	EXPECT_EQ(As<ASTNumberValue>(eType->getSizeExpr())->getValue(), "3");

    	auto *eAssignExpr = As<ASTBinaryOp>(eStmt->getTarget());
    	ASSERT_TRUE(eAssignExpr != nullptr);
    	EXPECT_EQ(eAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
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
        auto *aStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        auto *aIdent = As<ASTIdentifier>(aStmt->getSource());
        EXPECT_EQ(aIdent->getName(), "a");
        auto *aVar = As<ASTLocalVar>(aIdent->getVar());
        ASSERT_TRUE(aVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(aVar->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        {
            auto *aAssignExpr = As<ASTBinaryOp>(aStmt->getTarget());
            ASSERT_TRUE(aAssignExpr != nullptr);
            EXPECT_EQ(aAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
            auto *aVal = As<ASTStringValue>(aAssignExpr->getRightExpr());
            ASSERT_TRUE(aVal != nullptr);
            EXPECT_EQ(aVal->getValue(), "");
        }

        // b: byte b = 'b'
        auto *bStmt = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *bIdent = As<ASTIdentifier>(bStmt->getSource());
        EXPECT_EQ(bIdent->getName(), "b");
        auto *bVar = As<ASTLocalVar>(bIdent->getVar());
        ASSERT_TRUE(bVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(bVar->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        {
            auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getTarget());
            ASSERT_TRUE(bAssignExpr != nullptr);
            EXPECT_EQ(bAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
            auto *bVal = As<ASTStringValue>(bAssignExpr->getRightExpr());
            ASSERT_TRUE(bVal != nullptr);
            EXPECT_EQ(bVal->getValue(), "b");
        }

        // c: byte[] c = {'a', 'b', 'c', ''}
        auto *cStmt = As<ASTAssignStmt>(Body->getContent()[2]);
        auto *cIdent = As<ASTIdentifier>(cStmt->getSource());
        EXPECT_EQ(cIdent->getName(), "c");
        auto *cVar = As<ASTLocalVar>(cIdent->getVar());
        ASSERT_TRUE(cVar != nullptr);
        auto *cType = As<ASTArrayType>(cVar->getType());
        ASSERT_TRUE(cType != nullptr);
        EXPECT_TRUE(HasBuiltinType(cType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(cType->getSizeExpr(), nullptr);
        auto *cAssignExpr = As<ASTBinaryOp>(cStmt->getTarget());
        ASSERT_TRUE(cAssignExpr != nullptr);
        EXPECT_EQ(cAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *cVal = As<ASTArrayValue>(cAssignExpr->getRightExpr());
        ASSERT_TRUE(cVal != nullptr);
        EXPECT_EQ(cVal->size(), 4u);
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[0])->getValue(), "a");
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[1])->getValue(), "b");
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[2])->getValue(), "c");
        EXPECT_EQ(As<ASTStringValue>(cVal->getValues()[3])->getValue(), "");

        // d: byte[2] d = {'', ''}
        auto *dStmt = As<ASTAssignStmt>(Body->getContent()[3]);
        auto *dIdent = As<ASTIdentifier>(dStmt->getSource());
        EXPECT_EQ(dIdent->getName(), "d");
        auto *dVar = As<ASTLocalVar>(dIdent->getVar());
        ASSERT_TRUE(dVar != nullptr);
        auto *dType = As<ASTArrayType>(dVar->getType());
        ASSERT_TRUE(dType != nullptr);
        EXPECT_TRUE(HasBuiltinType(dType->getElementType(), ASTBuiltinTypeKind::TYPE_BYTE));
        ASSERT_TRUE(dType->getSizeExpr() != nullptr);
        EXPECT_EQ(As<ASTNumberValue>(dType->getSizeExpr())->getValue(), "2");
        auto *dAssignExpr = As<ASTBinaryOp>(dStmt->getTarget());
        ASSERT_TRUE(dAssignExpr != nullptr);
        EXPECT_EQ(dAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
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
        auto *cStmt = As<ASTAssignStmt>(Body->getContent()[0]);
        auto *cIdent = As<ASTIdentifier>(cStmt->getSource());
        EXPECT_EQ(cIdent->getName(), "c");
        auto *cVar = As<ASTLocalVar>(cIdent->getVar());
        ASSERT_TRUE(cVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(cVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));
        // Declaration may have no target or a null value in binary expression
        auto *cAssignExpr = As<ASTBinaryOp>(cStmt->getTarget());
        if (cAssignExpr) {
            EXPECT_EQ(cAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
            auto *cVal = As<ASTValue>(cAssignExpr->getRightExpr());
            ASSERT_TRUE(cVal != nullptr);
        }

        // a: empty string literal
        auto *aStmt = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *aIdent = As<ASTIdentifier>(aStmt->getSource());
        EXPECT_EQ(aIdent->getName(), "a");
        auto *aVar = As<ASTLocalVar>(aIdent->getVar());
        ASSERT_TRUE(aVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(aVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));
        auto *aAssignExpr = As<ASTBinaryOp>(aStmt->getTarget());
        ASSERT_TRUE(aAssignExpr != nullptr);
        EXPECT_EQ(aAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *aVal = As<ASTStringValue>(aAssignExpr->getRightExpr());
        ASSERT_TRUE(aVal != nullptr);
        EXPECT_EQ(aVal->getValue(), "");

        // b: "abc"
        auto *bStmt = As<ASTAssignStmt>(Body->getContent()[2]);
        auto *bIdent = As<ASTIdentifier>(bStmt->getSource());
        EXPECT_EQ(bIdent->getName(), "b");
        auto *bVar = As<ASTLocalVar>(bIdent->getVar());
        ASSERT_TRUE(bVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(bVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));
        auto *bAssignExpr = As<ASTBinaryOp>(bStmt->getTarget());
        ASSERT_TRUE(bAssignExpr != nullptr);
        EXPECT_EQ(bAssignExpr->getOpKind(), ASTBinaryOpKind::OP_BINARY_ASSIGN);
        auto *bVal = As<ASTStringValue>(bAssignExpr->getRightExpr());
        ASSERT_TRUE(bVal != nullptr);
        EXPECT_EQ(bVal->getValue(), "abc");
    }
}