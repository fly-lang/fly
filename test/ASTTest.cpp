//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Frontend/TextDiagnosticPrinter.h>
#include <Basic/FileManager.h>
#include <Basic/SourceManager.h>
#include <AST/ASTGlobalVar.h>
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTGlobalVar.h"
#include "gtest/gtest.h"

namespace {
    using namespace fly;

    class ASTTest : public ::testing::Test {

    public:

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticOptions *DiagOpts;
        TextDiagnosticPrinter *DiagClient;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;
        SourceLocation SourceLoc;
        ASTContext *Context;

        ASTTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                   DiagOpts(new DiagnosticOptions),
                   DiagClient(new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts)),
                      Diags(DiagID, DiagOpts, DiagClient),
                      SourceMgr(Diags, FileMgr), Context(new ASTContext(Diags)) {
        }

        ~ASTTest() override {
            delete Context;
        }

        ASTNode* NewASTNode(llvm::StringRef FileName) {

            // Set Source Manager file id
            std::unique_ptr<llvm::MemoryBuffer> Buf = llvm::MemoryBuffer::getMemBuffer("");
            llvm::MemoryBuffer *b = Buf.get();
            const FileID &FID = SourceMgr.createFileID(std::move(Buf));
            SourceMgr.setMainFileID(FID);
            SourceLoc = SourceMgr.getLocForStartOfFile(FID);

            // Create a lexer starting at the beginning of this token.
            ASTNode *Node = new ASTNode(FileName, FID, Context);
            return Node;
        }

    };

    TEST_F(ASTTest, SingleImport) {
        auto Node1 = NewASTNode("file1.fly");
        Node1->setNameSpace("packageA");
        Node1->addImport(new ASTImport(SourceLoc, "packageB"));
        Node1->Finalize();
        ASSERT_EQ(Node1->getNameSpace()->getNameSpace(), "packageA");
        ASSERT_EQ(Context->getNameSpaces().lookup("packageB"), nullptr);
        ASTNameSpace *NS1 = Context->getNameSpaces().lookup(Node1->getNameSpace()->getNameSpace());
        ASSERT_EQ(NS1->getNodes().lookup(Node1->getFileName())->getFileName(), "file1.fly");
        ASSERT_EQ(Node1->getImports().lookup("packageA"), nullptr);
        ASSERT_EQ(Node1->getImports().lookup("packageB")->getNameSpace(), nullptr);

        auto Node2 = NewASTNode("file2.fly");
        Node2->setNameSpace("packageB");
        Node2->addImport(new ASTImport(SourceLoc, "packageA"));
        Node2->Finalize();
        ASSERT_EQ(Context->getNameSpaces().size(), 3); // Consider Default namespace
        EXPECT_TRUE(Node1->getImports().lookup("packageB")->getNameSpace() == nullptr);
        EXPECT_TRUE(Node2->getImports().lookup("packageA")->getNameSpace() != nullptr);
        ASSERT_EQ(Node2->getImports().lookup("packageA")->getNameSpace()->getNameSpace(), "packageA");

        Context->Finalize();
        Node1->getImports().lookup("packageB")->getNameSpace();
        Node1->getImports().lookup("packageB")->getNameSpace()->getNameSpace();
        EXPECT_TRUE(Node1->getImports().lookup("packageB")->getNameSpace() != nullptr);
        ASSERT_EQ(Node1->getImports().lookup("packageB")->getNameSpace()->getNameSpace(), "packageB");
    }

    TEST_F(ASTTest, DuplicateImportErr) {
        // Capture cout.
        Diags.getClient()->BeginSourceFile();
        auto Node1 = NewASTNode("file1.fly");
        Node1->setNameSpace("packageA");
        Node1->addImport(new ASTImport(SourceLoc, "packageB"));
        Node1->addImport(new ASTImport(SourceLoc, "packageB"));
        Node1->Finalize();
        Context->Finalize();
        Diags.getClient()->EndSourceFile();

        Diags.getClient()->finish();
    }

    TEST_F(ASTTest, GlobalVarVisibility) {
        auto Node1 = NewASTNode("file1.fly");
        const ASTNameSpace *NS = Node1->setNameSpace("packageA");
        SourceLocation &Loc = SourceLoc;
        ASTGlobalVar *G1 = new ASTGlobalVar(Node1, Loc, new IntPrimType(Loc), "a");
        G1->setVisibility(V_PRIVATE);
        Node1->addGlobalVar(G1);
        ASTGlobalVar *G2 = new ASTGlobalVar(Node1, Loc, new FloatPrimType(Loc), "b"); // Default
        Node1->addGlobalVar(G2);
        ASTGlobalVar *G3 = new ASTGlobalVar(Node1, Loc, new BoolPrimType(Loc), "c");
        G3->setVisibility(V_PUBLIC);
        Node1->addGlobalVar(G3);
        Node1->Finalize();
    }
}