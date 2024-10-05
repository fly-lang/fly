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

    TEST_F(ParserTest,  LineComments) {
        llvm::StringRef str = ("// Global var comment\n"
                               "int b\n"
                               "// Func comment\n"
                               "void func() {}\n"
        );

        ASTModule *Module = Parse("LineComments", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *GlobalB = Module->getGlobalVars().find("b")->getValue();
        EXPECT_EQ(GlobalB->getName(), "b");
        EXPECT_EQ(GlobalB->getComment(), "");

        const ASTFunction *Func = *Module->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(Func->getName(), "func");
        EXPECT_EQ(Func->getComment(), "");
    }

    TEST_F(ParserTest, BlockComments) {
        llvm::StringRef str = (" /* Global var block comment */\n"
                               "// Global var line comment\n"
                               "int b\n"
                               "// Func line comment\n"
                               "\t /*   Func block comment \n*/\n"
                               "void func() {\n"
                               "  /* body comment */\n"
                               "}\n"
                               "//body comment\n"
                               "void func2() {}\n"
        );
        ASTModule *Module = Parse("BlockComments", str);


        ASTGlobalVar *GlobalB = Module->getGlobalVars().find("b")->getValue();
        EXPECT_EQ(GlobalB->getName(), "b");
        EXPECT_EQ(GlobalB->getComment(), "/* Global var block comment */");

        ASTFunction *Func = *Module->getFunctions().find("func")->getValue().begin()->second.begin();
        EXPECT_EQ(Func->getName(), "func");
        EXPECT_EQ(Func->getComment(), "/*   Func block comment \n*/");

        ASTFunction *Func2 = *Module->getFunctions().find("func2")->getValue().begin()->second.begin();
        EXPECT_EQ(Func2->getName(), "func2");
        EXPECT_EQ(Func2->getComment(), StringRef());
    }
}