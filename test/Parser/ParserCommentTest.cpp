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
//        ASTFunction *Func = static_cast<ASTFunction *>(Module->getNodes()[0]);
//        EXPECT_EQ(Func->getName(), "func");
//        EXPECT_EQ(Func->getComment()->getContent(), "// Func comment");
//    }

    TEST_F(ParserTest, BlockComments) {
        llvm::StringRef str = ("// Func line comment\n"
                               "\t /*   Func block comment \n*/\n"
                               "void func() {\n"
                               "  /* body block comment */\n"
                               "  // body inline comment */\n"
                               "}\n"
        );
        ASTModule *Module = Parse("BlockComments", str);

    	ASTComment *Comment0 = As<ASTComment>(Module->getNodes()[0]);
    	EXPECT_EQ(Comment0->getContent(), "// Func line comment");

    	ASTComment *Comment1 = As<ASTComment>(Module->getNodes()[1]);
    	EXPECT_EQ(Comment1->getContent(), "/*   Func block comment \n*/");

        ASTFunction *Func = As<ASTFunction>(Module->getNodes()[2]);
        EXPECT_EQ(Func->getName(), "func");
    }
}