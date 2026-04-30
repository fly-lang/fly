//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTModule.h"
#include "AST/ASTFunction.h"
#include "AST/ASTComment.h"
#include "AST/ASTBlockStmt.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest,  LineComments) {
        llvm::StringRef str = (
                               "// Func comment\n"
                               "/** \n"
                               " * Multi-line \n"
							   " * Func comment \n"
							   " */\n"
                               "func() {}\n"
        );

        ASTModule *Module = Parse("LineComments", str);

        // ASTComment *Comment0 = As<ASTComment>(Module->getNodes()[0]);
        // EXPECT_EQ(Comment0->getContent(), "// Func comment");
        //
        // ASTComment *Comment1 = As<ASTComment>(Module->getNodes()[1]);
        // EXPECT_EQ(Comment1->getContent(), "/** \n * Multi-line \n * Func comment \n */");
        //
        // ASTFunction *Func = As<ASTFunction>(Module->getNodes()[2]);
        // EXPECT_EQ(Func->getName(), "func");
    }

    TEST_F(ParserTest, BlockComments) {
        llvm::StringRef str = ("// Func line comment\n"
                               "\t /*   Func block comment \n*/\n"
                               "func() {\n"
                               "  /* body block comment */\n"
                               "  // body inline comment */\n"
                               "}\n"
        );
        ASTModule *Module = Parse("BlockComments", str);

    	// ASTComment *Comment0 = As<ASTComment>(Module->getNodes()[0]);
    	// EXPECT_EQ(Comment0->getContent(), "// Func line comment");
     //
    	// ASTComment *Comment1 = As<ASTComment>(Module->getNodes()[1]);
    	// EXPECT_EQ(Comment1->getContent(), "/*   Func block comment \n*/");
     //
     //    ASTFunction *Func = As<ASTFunction>(Module->getNodes()[2]);
     //    EXPECT_EQ(Func->getName(), "func");
     //
     //    // Check body comments
     //    ASTBlockStmt *Body = Func->getBody();
     //    EXPECT_EQ(Body->getContent().size(), 2);
     //
     //    ASTComment *BodyComment0 = As<ASTComment>(Body->getContent()[0]);
     //    EXPECT_EQ(BodyComment0->getContent(), "/* body block comment */");
     //
     //    ASTComment *BodyComment1 = As<ASTComment>(Body->getContent()[1]);
     //    EXPECT_EQ(BodyComment1->getContent(), "// body inline comment */");
    }
}