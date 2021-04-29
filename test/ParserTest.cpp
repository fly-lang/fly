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

namespace {
    using namespace fly;

    class ParserTest : public ::testing::Test {

    public:

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;
        ASTContext *Context;

        ParserTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                      SourceMgr(Diags, FileMgr), Context(new ASTContext(Diags)) {
        }

        virtual ~ParserTest() {
            delete Context;
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
            ASTNode *AST = new ASTNode(FileName, FID, Context);
            P->Parse(AST);
            AST->Finalize();
            return P;
        }

    };

    TEST_F(ParserTest, SinglePackage) {
        StringRef str = ("namespace std");

        // Verify FileName
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();
        EXPECT_EQ(AST->getFileName(), "package.fly");

        // verify AST contains package
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "std");
    }

    TEST_F(ParserTest, MultiPackageError) {
        StringRef str = ("namespace std\n"
                         "namespace bad");

        // Verify FileName
        auto P = Parse("error.fly", str);

        EXPECT_TRUE(Diags.hasErrorOccurred());
    }

    TEST_F(ParserTest, SingleImport) {
        StringRef str = ("namespace std\n"
                         "import \"packageA\"");

        // Verify FileName
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();
        ImportDecl* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
        EXPECT_EQ(Verify->getAlias(), Verify->getName());
    }

    TEST_F(ParserTest, SingleImportAlias) {
        StringRef str = ("\n import  \"standard\" as \"std\"\n");
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "default");
        EXPECT_EQ(AST->getImports().lookup("standard")->getName(), "standard");
        EXPECT_EQ(AST->getImports().lookup("standard")->getAlias(), "std");
    }

    TEST_F(ParserTest, MultiImports) {
        StringRef str = ("namespace std\n"
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
        StringRef str = ("namespace std\n"
                         "import (\"packageA\")");

        // Verify FileName
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();
        ImportDecl* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
    }

    TEST_F(ParserTest, MultiParenImports) {
        StringRef str = ("namespace std\n"
                         "import (\"packageA\", \"packageB\")");

        // Verify FileName
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();
        ImportDecl* VerifyB = AST->getImports().lookup("packageB");
        ImportDecl* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
    }

    TEST_F(ParserTest, GlobalVars) {
        StringRef str = ("namespace std\n"
                         "private int a\n"
                         "public float b\n"
                         "bool c\n"
                         );

        // Verify FileName
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();
        GlobalVarDecl *VerifyA = AST->getVars().lookup("a");
        GlobalVarDecl *VerifyB = AST->getVars().lookup("b");
        GlobalVarDecl *VerifyC = AST->getVars().lookup("c");

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_FALSE(VerifyA->isConstant());
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_FALSE(VerifyB->isConstant());
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyC->isConstant());
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");

        delete AST;
    }

    TEST_F(ParserTest, GlobalConstants) {
        StringRef str = ("namespace std\n"
                         "private const int a = 1\n"
                         "const public float b = 2.0\n"
                         "const bool c = false\n");

        // Verify FileName
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();
        GlobalVarDecl *VerifyA = AST->getVars().lookup("a");
        GlobalVarDecl *VerifyB = AST->getVars().lookup("b");
        GlobalVarDecl *VerifyC = AST->getVars().lookup("c");

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_EQ(VerifyA->isConstant(), true);
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyA->getExpr())->getString(), "1");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_EQ(VerifyB->isConstant(), true);
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyB->getExpr())->getString(), "2.0");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        EXPECT_EQ(VerifyC->isConstant(), true);
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyC->getExpr())->getString(), "false");
    }

    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
        StringRef str = ("namespace std\n"
                         "void func() {}\n");

        // Verify FileName
        auto P = Parse("function.fly", str);
        auto AST = P->getAST();

        FunctionDecl *VerifyFunc = AST->getFunctions().lookup("func");
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyFunc->isConstant());
        ASSERT_TRUE(Context->getNameSpaces().lookup("std")->getFunctions().lookup("func"));
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        StringRef str = ("namespace std\n"
                         "private int func(int a, const float b, bool c=false) {\n"
                         "  return 1"
                         "}\n");

        // Verify FileName
        auto P = Parse("function.fly", str);
        auto AST = P->getAST();

        FunctionDecl *VerifyFunc = AST->getFunctions().lookup("func");
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_PRIVATE);
        ASSERT_FALSE(VerifyFunc->isConstant());
        ASSERT_FALSE(Context->getNameSpaces().lookup("std")->getFunctions().lookup("func"));

        VarDecl *Arg0 = VerifyFunc->getParams()->getVars()[0];
        VarDecl *Arg1 = VerifyFunc->getParams()->getVars()[1];
        VarDecl *Arg2 = VerifyFunc->getParams()->getVars()[2];

        EXPECT_EQ(Arg0->getName(), "a");
        EXPECT_EQ(Arg0->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(Arg0->isConstant(), false);

        EXPECT_EQ(Arg1->getName(), "b");
        EXPECT_EQ(Arg1->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(Arg1->isConstant(), true);

        EXPECT_EQ(Arg2->getName(), "c");
        EXPECT_EQ(Arg2->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(Arg2->isConstant(), false);
        EXPECT_EQ(Arg2->getExpr()->getKind(), ExprKind::EXPR_VALUE);
        ValueExpr *DefArg2 = static_cast<ValueExpr *>(Arg2->getExpr());
        EXPECT_EQ(DefArg2->getString(), "false");

        ValueExpr *Return = static_cast<ValueExpr*>(VerifyFunc->getBody()->getReturn()->getExpr());
        EXPECT_EQ(Return->getString(), "1");
        EXPECT_EQ(Return->getKind(), ExprKind::EXPR_VALUE);
    }
}