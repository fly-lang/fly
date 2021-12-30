//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Frontend/CompilerInstance.h"
#include "AST/ASTContext.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTImport.h"
#include "gtest/gtest.h"

namespace {
    using namespace fly;

    class ASTTest : public ::testing::Test {

    public:
        const CompilerInstance CI;
        ASTContext *Context;
        CodeGen *CG;
        DiagnosticsEngine &Diags;
        SourceLocation SourceLoc;

        ASTTest() : CI(*TestUtils::CreateCompilerInstance()),
                   Context(new ASTContext(CI.getDiagnostics())),
                   CG(TestUtils::CreateCodeGen(CI)),
                   Diags(CI.getDiagnostics()) {

        }

        ~ASTTest() override {
            delete Context;
        }

        ASTNode* NewASTNode(std::string FileName) {

            // Create CodeGen
            CodeGenModule *CGM = CG->CreateModule(FileName);

            // Create AST
            return new ASTNode(FileName, Context, CGM);
        }

    };

    TEST_F(ASTTest, SingleImport) {
        auto Node1 = NewASTNode("file1.fly");
        Node1->setNameSpace("packageA");
        Node1->AddImport(new ASTImport(SourceLoc, "packageB"));
        Node1->Resolve();
        Context->AddNode(Node1);
        
        ASSERT_EQ(Node1->getNameSpace()->getName(), "packageA");
        ASSERT_EQ(Context->getNameSpaces().lookup("packageB"), nullptr);
        ASTNameSpace *NS1 = Context->getNameSpaces().lookup(Node1->getNameSpace()->getName());
        ASSERT_EQ(NS1->getNodes().lookup(Node1->getName())->getName(), "file1.fly");
        ASSERT_EQ(Node1->getImports().lookup("packageA"), nullptr);
        ASSERT_EQ(Node1->getImports().lookup("packageB")->getNameSpace(), nullptr);

        auto Node2 = NewASTNode("file2.fly");
        Node2->setNameSpace("packageB");
        Node2->AddImport(new ASTImport(SourceLoc, "packageA"));
        Node2->Resolve();
        Context->AddNode(Node2);
        ASSERT_EQ(Context->getNameSpaces().size(), 3); // Consider Default namespace
        EXPECT_TRUE(Node1->getImports().lookup("packageB")->getNameSpace() == nullptr);
        EXPECT_TRUE(Node2->getImports().lookup("packageA")->getNameSpace() != nullptr);
        ASSERT_EQ(Node2->getImports().lookup("packageA")->getNameSpace()->getName(), "packageA");

        Context->Resolve();
//        EXPECT_TRUE(Node1->getImports().lookup("packageB")->getNameSpace() != nullptr);
//        ASSERT_EQ(Node1->getImports().lookup("packageB")->getNameSpace()->getName(), "packageB");
    }

    TEST_F(ASTTest, DuplicateImportErr) {
        // Capture cout.
        Diags.getClient()->BeginSourceFile();
        auto Node1 = NewASTNode("file1.fly");
        Node1->setNameSpace("packageA");
        Node1->AddImport(new ASTImport(SourceLoc, "packageB"));
        Node1->AddImport(new ASTImport(SourceLoc, "packageB"));
        Node1->Resolve();
        Context->Resolve();
        Diags.getClient()->EndSourceFile();

        Diags.getClient()->finish();
    }

    TEST_F(ASTTest, GlobalVarVisibility) {
        auto Node1 = NewASTNode("file1.fly");
        const ASTNameSpace *NS = Node1->setNameSpace("packageA");
        SourceLocation &Loc = SourceLoc;
        ASTGlobalVar *G1 = new ASTGlobalVar(Loc, Node1, new ASTIntType(Loc), "a");
        G1->setVisibility(V_PRIVATE);
        Node1->AddGlobalVar(G1);
        ASTGlobalVar *G2 = new ASTGlobalVar(Loc, Node1, new ASTFloatType(Loc), "b"); // Default
        Node1->AddGlobalVar(G2);
        ASTGlobalVar *G3 = new ASTGlobalVar(Loc, Node1, new ASTBoolType(Loc), "c");
        G3->setVisibility(V_PUBLIC);
        Node1->AddGlobalVar(G3);
        Node1->Resolve();
    }
}