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
#include "AST/ASTComment.h"

#include "llvm/ADT/StringMap.h"

namespace {

    using namespace fly;

//    TEST_F(ParserTest,  LineComments) {
//        llvm::StringRef str = ("// Global var comment\n"
//                               "int b\n"
//                               "// Func comment\n"
//                               "void func() {}\n"
//        );
//
//        ASTModule *Module = Parse("LineComments", str);
//        ASSERT_TRUE(Resolve());
//
//        ASTGlobalVar *VarB = *Module->getGlobalVars().begin();
//        EXPECT_EQ(VarB->getName(), "b");
//        EXPECT_EQ(VarB->getComment()->getContent(), "// Global var comment");
//
//        ASTFunction *Func = *Module->getFunctions().begin();
//        EXPECT_EQ(Func->getName(), "func");
//        EXPECT_EQ(Func->getComment()->getContent(), "// Func comment");
//    }

    TEST_F(ParserTest, BlockComments) {
        llvm::StringRef str = (" /* Global var block comment */\n"
                               "// Global var line comment\n"
                               "int b\n"
                               "// Func line comment\n"
                               "\t /*   Func block comment \n*/\n"
                               "void func() {\n"
                               "  /* body block comment */\n"
                               "  // body inline comment */\n"
                               "}\n"
                               "void func1() {}\n"
                               "/*   Func2 block comment \n*/\n"
                               "//func2 line comment\n"
                               "void func2() {}\n"
        );
        ASTModule *Module = Parse("BlockComments", str);

        ASTGlobalVar *VarB = *Module->getGlobalVars().begin();
        EXPECT_EQ(VarB->getName(), "b");
        EXPECT_EQ(VarB->getComment()->getContent(), "/* Global var block comment */");

        ASTFunction *Func = Module->getFunctions().begin()[0];
        EXPECT_EQ(Func->getName(), "func");
        EXPECT_EQ(Func->getComment()->getContent(), "/*   Func block comment \n*/");

        ASTFunction *Func1 = Module->getFunctions().begin()[1];
        EXPECT_EQ(Func1->getName(), "func1");
        EXPECT_EQ(Func1->getComment(), nullptr);

        ASTFunction *Func2 = Module->getFunctions().begin()[2];
        EXPECT_EQ(Func2->getName(), "func2");
        EXPECT_EQ(Func2->getComment()->getContent(), "/*   Func2 block comment \n*/");
    }
}