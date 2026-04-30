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
#include "AST/ASTName.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, SingleNameSpace) {
        llvm::StringRef str = ("namespace std");
        ASTModule *Module = Parse("SingleNameSpace", str);
        ASSERT_FALSE(HasErrorOccurred());

        // Module should have a namespace set with single name "std"
        auto *NS = Module->getNameSpace();
        ASSERT_TRUE(NS != nullptr);
        ASSERT_FALSE(NS->getNames().empty());
        EXPECT_EQ(NS->getNames()[0]->getName(), "std");
    }

    TEST_F(ParserTest, MultiNamespace) {
        llvm::StringRef str = ("namespace my.std");
        ASTModule *Module = Parse("MultiNamespace", str);
        // Parser accepts dotted namespaces (my.std) as a sequence of names
        ASSERT_FALSE(HasErrorOccurred());

        auto *NS = Module->getNameSpace();
        ASSERT_TRUE(NS != nullptr);
        ASSERT_EQ(NS->getNames().size(), 2u);
        EXPECT_EQ(NS->getNames()[0]->getName(), "my");
        EXPECT_EQ(NS->getNames()[1]->getName(), "std");
    }

    TEST_F(ParserTest, SingleImport) {
        llvm::StringRef str = ("import packageA");
        ASTModule *Module = Parse("SingleImport", str);
        ASSERT_FALSE(HasErrorOccurred());

        // Find the import node in module nodes
        ASTImport *Imp = nullptr;
        for (auto *N : Module->getNodes()) {
            if (auto *I = As<ASTImport>(N)) { Imp = I; break; }
        }
        ASSERT_TRUE(Imp != nullptr);
        auto Names = Imp->getNames();
        ASSERT_EQ(Names.size(), 1u);
        EXPECT_EQ(Names[0]->getName(), "packageA");
        EXPECT_TRUE(Imp->getAlias().empty());
    }

    TEST_F(ParserTest, SingleImportAlias) {
        llvm::StringRef str = ("import standard as std");
        ASTModule *Module1 = Parse("standard.fly", str);
        ASSERT_FALSE(HasErrorOccurred());

        ASTImport *Imp = nullptr;
        for (auto *N : Module1->getNodes()) {
            if (auto *I = As<ASTImport>(N)) { Imp = I; break; }
        }
        ASSERT_TRUE(Imp != nullptr);
        auto Names = Imp->getNames();
        auto Alias = Imp->getAlias();
        ASSERT_EQ(Names.size(), 1u);
        ASSERT_EQ(Alias.size(), 1u);
        EXPECT_EQ(Names[0]->getName(), "standard");
        EXPECT_EQ(Alias[0]->getName(), "std");
    }
}