//===--------------------------------------------------------------------------------------------------------------===//
// test/IdentifierParserTest.cpp - Identifier Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTBlock.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParams.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, Identifiers1) {
        llvm::StringRef str1 = ("int func() {\n"
                               "  Test t = new Test()"
                               "  int a = t.a\n"
                               "  a = createTest().a"
                               "}\n"
                               "Test createTest() {\n"
                               "  return new Test()"
                               "}\n");
        ASTNode *Node1 = Parse("File1", str1, false);

        llvm::StringRef str2 = ("public struct Test {\n"
                               "  int a = 1\n"
                               "}\n");
        ASTNode *Node2 = Parse("File2", str2);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node1->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(F->getType()->getKind(), ASTTypeKind::TYPE_CLASS);
        const ASTBlock *Body = F->getBody();

        // Test: Type t
        ASTLocalVar *typeVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(typeVar->getName(), "t");
        EXPECT_EQ(typeVar->getType()->getKind(), ASTTypeKind::TYPE_CLASS);
        ASTClassType *ClassType = (ASTClassType *) typeVar->getType();
        EXPECT_EQ(ClassType->getName(), "Type");
        ASSERT_EQ(((ASTNullValue &)((ASTValueExpr *) typeVar->getExpr())->getValue()).print(), "null");

        const ASTReturn *Ret = (ASTReturn *) Body->getContent()[1];
        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetRef->getVarRef()->getName(), "t");

    }
}