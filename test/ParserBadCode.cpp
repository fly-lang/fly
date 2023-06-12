//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "Frontend/FrontendAction.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParams.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"

#include "llvm/ADT/StringMap.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, DISABLED_BadColon) {
        llvm::StringRef str = (
                "void func() {\n"
                "  a:"
                "  return"
                "}\n");
        ASTNode *Node = Parse("BadColon", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // ++a
        ASTExprStmt *a1Stmt = (ASTExprStmt *) Body->getContent()[0];
        ASTUnaryGroupExpr *a1Unary = (ASTUnaryGroupExpr *) a1Stmt->getExpr();
        EXPECT_EQ(a1Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);
        EXPECT_EQ(a1Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_PRE);
        EXPECT_EQ(((ASTVarRefExpr *) a1Unary->getFirst())->getVarRef()->getName(), "a");
    }
}