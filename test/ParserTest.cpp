//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "AST/ASTContext.h"
#include <gtest/gtest.h>
#include <string>

namespace {
    using namespace fly;

    class ParserTest : public ::testing::Test {

    public:

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;

        ParserTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                      SourceMgr(Diags, FileMgr) {
        }

        std::unique_ptr<Parser> Parse(llvm::StringRef FileName, StringRef Source) {

            // Set Source Manager file id
            std::unique_ptr<llvm::MemoryBuffer> Buf = llvm::MemoryBuffer::getMemBuffer(Source);
            llvm::MemoryBuffer *b = Buf.get();
            const FileID &FID = SourceMgr.createFileID(std::move(Buf));
            SourceMgr.setMainFileID(FID);

            // Create a lexer starting at the beginning of this token.
            Lexer TheLexer(FID, b, SourceMgr);
            std::unique_ptr<Parser> P = std::make_unique<Parser>(TheLexer, Diags);
            ASTContext *Ctx = new ASTContext;
            ASTNode *AST = new ASTNode(FileName, FID, Ctx);
            P->Parse(AST);
            AST->Finalize();
            return P;
        }

    };

    TEST_F(ParserTest, SinglePackage) {
        StringRef str = ("package \"std\"");

        // Verify FileName
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();
        EXPECT_EQ(AST->getFileName(), "package.fly");

        // verify AST contains package
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "std");

        StringRef str2 = ("\n package  \"std\"\n");
        P = Parse("package.fly", str2);
        auto AST2 = P->getAST();
        EXPECT_EQ(AST2->getNameSpace()->getNameSpace(), "std");
    }

    TEST_F(ParserTest, MultiPackageError) {
        StringRef str = ("package \"std\"\n"
                         "package \"bad\"");

        // Verify FileName
        auto P = Parse("error.fly", str);

        EXPECT_TRUE(Diags.hasErrorOccurred());
    }

    TEST_F(ParserTest, SingleImport) {
        StringRef str = ("package \"std\"\n"
                         "import \"packageA\"");

        // Verify FileName
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();
        ImportDecl* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
    }

    TEST_F(ParserTest, MultiImports) {
        StringRef str = ("package \"std\"\n"
                         "import \"packageA\""
                         "import \"packageB\"");

        // Verify FileName
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();
        ImportDecl* VerifyB = AST->getImports().lookup("packageB");
        ImportDecl* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
    }

    TEST_F(ParserTest, SingleParenImport) {
        StringRef str = ("package \"std\"\n"
                         "import (\"packageA\")");

        // Verify FileName
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();
        ImportDecl* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
    }

    TEST_F(ParserTest, MultiParenImports) {
        StringRef str = ("package \"std\"\n"
                         "import (\"packageA\", \"packageB\")");

        // Verify FileName
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();
        ImportDecl* VerifyB = AST->getImports().lookup("packageB");
        ImportDecl* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
    }

    TEST_F(ParserTest, SingleVar) {
        StringRef str = ("package \"std\"\n"
                         "int a");

        // Verify FileName
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();
        GlobalVarDecl* VerifyA = AST->getVars().lookup("a");

        EXPECT_EQ(VerifyA->getType(), TypeKind::Int);
        EXPECT_EQ(VerifyA->getName(), "a");
    }

    TEST_F(ParserTest, MultiVar) {
        StringRef str = ("package \"std\"\n"
                         "int a\n"
                         "float b\n"
                         "bool c");

        // Verify FileName
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();
        GlobalVarDecl *VerifyC = AST->getVars().lookup("c");
        GlobalVarDecl *VerifyB = AST->getVars().lookup("b");
        GlobalVarDecl *VerifyA = AST->getVars().lookup("a");

        EXPECT_EQ(VerifyA->getType(), TypeKind::Int);
        EXPECT_EQ(VerifyA->getName(), "a");

        EXPECT_EQ(VerifyB->getType(), TypeKind::Float);
        EXPECT_EQ(VerifyB->getName(), "b");

        EXPECT_EQ(VerifyC->getType(), TypeKind::Bool);
        EXPECT_EQ(VerifyC->getName(), "c");
    }

}