//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTContext.h"
#include "gtest/gtest.h"
#include "string"

namespace {
    using namespace fly;

    class ASTTest : public ::testing::Test {

    public:

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;
        ASTContext *Context;

        ASTTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                      SourceMgr(Diags, FileMgr), Context(new ASTContext) {
        }

//        virtual ~ASTTest() {
//            delete Context;
//        }

        std::unique_ptr<ASTNode> NewASTNode(llvm::StringRef FileName) {

            // Set Source Manager file id
            std::unique_ptr<llvm::MemoryBuffer> Buf = llvm::MemoryBuffer::getMemBuffer("");
            llvm::MemoryBuffer *b = Buf.get();
            const FileID &FID = SourceMgr.createFileID(std::move(Buf));
            SourceMgr.setMainFileID(FID);

            // Create a lexer starting at the beginning of this token.
            std::unique_ptr<ASTNode> AST = std::make_unique<ASTNode>(ASTNode(FileName, FID, Context));
            return AST;
        }

    };

    TEST_F(ASTTest, SingleImport) {
        auto Node1 = NewASTNode("file1.fly");
        Node1->setNameSpace("packageA");
        Node1->addImport("packageB");
        Node1->Finalize();
        ASSERT_EQ(Node1->getNameSpace()->getNameSpace(), "packageA");
        ASSERT_EQ(Context->getNameSpaces().lookup("packageB"), nullptr);
        ASTNameSpace *NS1 = Context->getNameSpaces().lookup(Node1->getNameSpace()->getNameSpace());
        ASSERT_EQ(NS1->getNodes().lookup(Node1->getFileName())->getFileName(), "file1.fly");
        ASSERT_EQ(Node1->getImports().lookup("packageA"), nullptr);
        ASSERT_EQ(Node1->getImports().lookup("packageB")->getNameSpace(), nullptr);

        auto Node2 = NewASTNode("file2.fly");
        Node2->setNameSpace("packageB");
        Node2->addImport("packageA");
        Node2->Finalize();
        ASSERT_EQ(Context->getNameSpaces().size(), 2);
        ASSERT_EQ(Node1->getImports().lookup("packageB")->getNameSpace(), nullptr);
        ASSERT_NE(Node2->getImports().lookup("packageA")->getNameSpace(), nullptr);
        ASSERT_EQ(Node2->getImports().lookup("packageA")->getNameSpace()->getNameSpace(), "packageA");

        Context->Finalize();
        ASSERT_NE(Node1->getImports().lookup("packageB")->getNameSpace(), nullptr);
        ASSERT_EQ(Node1->getImports().lookup("packageB")->getNameSpace()->getNameSpace(), "packageB");
    }

    TEST_F(ASTTest, GlobalVarVisibility) {
        auto Node1 = NewASTNode("file1.fly");
        Node1->setNameSpace("packageA");
        Node1->addIntVar(VisibilityKind::Public, ModifiableKind::Variable, "a",
                         new int(1));
        Node1->addFloatVar(VisibilityKind::Private, ModifiableKind::Variable, "b",
                           new float (2.0));
        Node1->addBoolVar(VisibilityKind::Default, ModifiableKind::Constant, "c",
                          new bool (true));
        Node1->Finalize();
    }
}