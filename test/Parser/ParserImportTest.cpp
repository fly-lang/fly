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

namespace {

    using namespace fly;

    TEST_F(ParserTest, SingleNameSpace) {
        llvm::StringRef str = ("namespace std");
        ASTModule *Module = Parse("SingleNameSpace", str);
        ASSERT_TRUE(Resolve());

        // Verify
        EXPECT_EQ(Module->getName(), "SingleNameSpace");
        EXPECT_EQ(Module->getNameSpace()->getName(), "std");
    }

    TEST_F(ParserTest, MultiNamespaceError) {
        llvm::StringRef str = ("namespace std\n"
                               "namespace bad");
        ASTModule *Module = Parse("MultiNamespaceError", str);
        ASSERT_FALSE(Resolve());
    }

    TEST_F(ParserTest, SingleImport) {
        llvm::StringRef str1 = ("namespace packageA");
        llvm::StringRef str2 = ("namespace std\n"
                         "import packageA");
        ASTModule *Module1 = Parse("packageA.fly", str1);
        ASTModule *Module2 = Parse("std.fly", str2);
        ASSERT_TRUE(Resolve());

        // Select packageA import
        ASTImport *PackageA = nullptr;
        for (auto Import : Module2->getImports()) {
            if (Import->getName() == "packageA") {
                PackageA = Import;
            }
        }

        EXPECT_NE(PackageA, nullptr);
        EXPECT_EQ(PackageA->getName(), "packageA");
        EXPECT_EQ(PackageA->getAlias(), nullptr);
    }

    TEST_F(ParserTest, SingleImportAlias) {
        llvm::StringRef str1 = ("namespace standard");
        llvm::StringRef str2 = ("import standard as std");
        ASTModule *Module1 = Parse("standard.fly", str1);
        ASTModule *Module2 = Parse("default.fly", str2);

        ASSERT_TRUE(Resolve());

        EXPECT_EQ(Module2->getNameSpace()->getName(), "default");

        // Select standard import
        ASTImport *standardImport = nullptr;
        ASTAlias *standardImportAlias = nullptr;
        for (auto Import : Module2->getImports()) {
            if (Import->getName() == "standard") {
                standardImport = Import;
                standardImportAlias = standardImport->getAlias();
            }
        }

        EXPECT_NE(standardImport, nullptr);
        EXPECT_EQ(standardImport->getName(), "standard");
        EXPECT_EQ(standardImport->getAlias()->getName(), "std");

        EXPECT_NE(standardImportAlias, nullptr);
        EXPECT_EQ(standardImportAlias->getName(), "std");

        // Select standard alias
        ASTImport *AliasImport = nullptr;
        for (auto Alias : Module2->getAliasImports()) {
            if (Alias->getName() == "standard") {
                AliasImport = Alias;
            }
        }

        EXPECT_NE(AliasImport, nullptr);
        EXPECT_EQ(standardImport->getName(), "standard");
        EXPECT_EQ(AliasImport->getAlias()->getName(), "std");
    }
}