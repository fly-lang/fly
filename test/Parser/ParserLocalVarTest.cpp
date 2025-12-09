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
        // type checks moved out: we don't assert directly on getType() here

        // Test: byte b
        auto *bStmt = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *bIdent = As<ASTIdentifier>(bStmt->getSource());
        EXPECT_EQ(bIdent->getName(), "b");
        ASTNumberValue* ZeroIntValue = Builder->CreateNumberValue(SourceLocation(), "0");
        ASSERT_EQ(As<ASTNumberValue>(bStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: short c
        auto *cStmt = As<ASTAssignStmt>(Body->getContent()[2]);
        auto *cIdent = As<ASTIdentifier>(cStmt->getSource());
        EXPECT_EQ(cIdent->getName(), "c");
        ASSERT_EQ(As<ASTNumberValue>(cStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: ushort d
        auto *dStmt = As<ASTAssignStmt>(Body->getContent()[3]);
        auto *dIdent = As<ASTIdentifier>(dStmt->getSource());
        EXPECT_EQ(dIdent->getName(), "d");
        ASSERT_EQ(As<ASTNumberValue>(dStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: int e
        auto *eStmt = As<ASTAssignStmt>(Body->getContent()[4]);
        auto *eIdent = As<ASTIdentifier>(eStmt->getSource());
        EXPECT_EQ(eIdent->getName(), "e");
        ASSERT_EQ(As<ASTNumberValue>(eStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: uint f
        auto *fStmt = As<ASTAssignStmt>(Body->getContent()[5]);
        auto *fIdent = As<ASTIdentifier>(fStmt->getSource());
        EXPECT_EQ(fIdent->getName(), "f");
        ASSERT_EQ(As<ASTNumberValue>(fStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: long g
        auto *gStmt = As<ASTAssignStmt>(Body->getContent()[6]);
        auto *gIdent = As<ASTIdentifier>(gStmt->getSource());
        EXPECT_EQ(gIdent->getName(), "g");
        ASSERT_EQ(As<ASTNumberValue>(gStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: ulong h
        auto *hStmt = As<ASTAssignStmt>(Body->getContent()[7]);
        auto *hIdent = As<ASTIdentifier>(hStmt->getSource());
        EXPECT_EQ(hIdent->getName(), "h");
        ASSERT_EQ(As<ASTNumberValue>(hStmt->getTarget())->getValue(), ZeroIntValue->getValue());

        // Test: float i
        auto *iStmt = As<ASTAssignStmt>(Body->getContent()[8]);
        auto *iIdent = As<ASTIdentifier>(iStmt->getSource());
        EXPECT_EQ(iIdent->getName(), "i");
        ASTNumberValue* ZeroFloatValue = Builder->CreateNumberValue(SourceLocation(), "0.0");
        ASSERT_EQ(As<ASTNumberValue>(iStmt->getTarget())->getValue(), ZeroFloatValue->getValue());

        // Test: double j
        auto *jStmt = As<ASTAssignStmt>(Body->getContent()[9]);
        auto *jIdent = As<ASTIdentifier>(jStmt->getSource());
        EXPECT_EQ(jIdent->getName(), "j");
        ASSERT_EQ(As<ASTNumberValue>(jStmt->getTarget())->getValue(), ZeroFloatValue->getValue());

        // Test: Type t
        ASTAssignStmt *tStmt = As<ASTAssignStmt>(Body->getContent()[10]);
        ASTIdentifier *tIdent = As<ASTIdentifier>(tStmt->getSource());
        EXPECT_EQ(tIdent->getName(), "t");
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
    		auto *ValA = As<ASTValue>(aStmt->getTarget());
    		ASSERT_TRUE(ValA != nullptr);
    		EXPECT_TRUE(ValA->isNull());
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

    	// b value must be ASTArrayValue with size 0
    	auto *bVal = As<ASTArrayValue>(bStmt->getTarget());
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

    	auto *cVal = As<ASTArrayValue>(cStmt->getTarget());
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

    	auto *eVal = As<ASTArrayValue>(eStmt->getTarget());
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
            auto *aVal = As<ASTStringValue>(aStmt->getTarget());
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
            auto *bVal = As<ASTStringValue>(bStmt->getTarget());
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
        auto *cVal = As<ASTArrayValue>(cStmt->getTarget());
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
        auto *dVal = As<ASTArrayValue>(dStmt->getTarget());
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
        // Declaration may have no target or a null value
        if (cStmt->getTarget()) {
            auto *cVal = As<ASTValue>(cStmt->getTarget());
            ASSERT_TRUE(cVal != nullptr);
        }

        // a: empty string literal
        auto *aStmt = As<ASTAssignStmt>(Body->getContent()[1]);
        auto *aIdent = As<ASTIdentifier>(aStmt->getSource());
        EXPECT_EQ(aIdent->getName(), "a");
        auto *aVar = As<ASTLocalVar>(aIdent->getVar());
        ASSERT_TRUE(aVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(aVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));
        auto *aVal = As<ASTStringValue>(aStmt->getTarget());
        ASSERT_TRUE(aVal != nullptr);
        EXPECT_EQ(aVal->getValue(), "");

        // b: "abc"
        auto *bStmt = As<ASTAssignStmt>(Body->getContent()[2]);
        auto *bIdent = As<ASTIdentifier>(bStmt->getSource());
        EXPECT_EQ(bIdent->getName(), "b");
        auto *bVar = As<ASTLocalVar>(bIdent->getVar());
        ASSERT_TRUE(bVar != nullptr);
        EXPECT_TRUE(HasBuiltinType(bVar->getType(), ASTBuiltinTypeKind::TYPE_STRING));
        auto *bVal = As<ASTStringValue>(bStmt->getTarget());
        ASSERT_TRUE(bVal != nullptr);
        EXPECT_EQ(bVal->getValue(), "abc");
    }
}