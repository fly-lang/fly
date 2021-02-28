//===----------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include <llvm/ADT/StringRef.h>
#include <gtest/gtest.h>
#include <string>

namespace {
    using namespace fly;

    class ParserTest : public ::testing::Test {

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;

    protected:
        ParserTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                      SourceMgr(Diags, FileMgr) {
        }

        ASTContext& Parse(string fileName, StringRef Source) {
            std::unique_ptr<llvm::MemoryBuffer> Buf =
                    llvm::MemoryBuffer::getMemBuffer(Source);
            llvm::MemoryBuffer *b = Buf.get();
            const FileID &FID = SourceMgr.createFileID(std::move(Buf));
            SourceMgr.setMainFileID(FID);


            // Create a lexer starting at the beginning of this token.
            Lexer TheLexer(FID, b, SourceMgr);
            auto P = Parser(fileName, TheLexer);
            return P.getASTContext();
        }
    };

    TEST_F(ParserTest, SinglePackage) {
        StringRef str = ("package \"std\"");

        // Verify FileName
        ASTContext& context = Parse("parseArgs.fly", str);
        EXPECT_EQ(context.getFileName(), "parseArgs.fly");

        // verify AST contains package
        const PackageDecl& package = context.getPackage();
        EXPECT_EQ(package.getType(), ASTTypes::Package);
        EXPECT_TRUE(package.getName().compare("\"std\""));
    }

}