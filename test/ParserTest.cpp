//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Frontend/FrontendAction.h"
#include "Frontend/CompilerInstance.h"
#include "Parser/Parser.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParams.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTClass.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"

#include "llvm/ADT/StringMap.h"
#include <gtest/gtest.h>

namespace {

    using namespace fly;

    class ParserTest : public ::testing::Test {

    public:
        const CompilerInstance CI;
        ASTContext *Context;
        SemaBuilder *Builder;
        DiagnosticsEngine &Diags;
        bool Success = false;

        ParserTest() : CI(*TestUtils::CreateCompilerInstance()),
                      Diags(CI.getDiagnostics()),
                      Builder(Sema::CreateBuilder(CI.getDiagnostics())) {
            Diags.getClient()->BeginSourceFile();
        }

        virtual ~ParserTest() {
            Diags.getClient()->EndSourceFile();
            Builder->Destroy();
        }

        ASTNode *Parse(std::string FileName, llvm::StringRef Source, bool DoBuild = true) {
            InputFile Input(Diags, CI.getSourceManager(), FileName);
            Input.Load(Source);
            Parser *P = new Parser(Input, CI.getSourceManager(), Diags, *Builder);
            ASTNode *Node = P->Parse();
            Success = !Diags.hasErrorOccurred() && Node;
            if (DoBuild)
                Success &= Builder->Build();
            return Node;
        }

        bool isSuccess() const {
            return Success;
        }

    };

    TEST_F(ParserTest, SingleNameSpace) {
        llvm::StringRef str = ("namespace std");
        ASTNode *Node = Parse("SingleNameSpace", str);
        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node->getName(), "SingleNameSpace");

        // verify AST contains package
        EXPECT_EQ(Node->getNameSpace()->getName(), "std");
    }

    TEST_F(ParserTest, MultiNamespaceError) {
        llvm::StringRef str = ("namespace std\n"
                                "namespace bad");
        ASTNode *Node = Parse("MultiNamespaceError", str);
        EXPECT_FALSE(isSuccess());
    }

    TEST_F(ParserTest, SingleImport) {
        llvm::StringRef str1 = ("namespace packageA");
        llvm::StringRef str2 = ("namespace std\n"
                         "import packageA");
        ASTNode *Node1 = Parse("packageA.fly", str1, false);
        ASTNode *Node2 = Parse("std.fly", str2);
        ASSERT_TRUE(isSuccess());

        ASTImport* Verify = Node2->getImports().lookup("packageA");
        EXPECT_EQ(Verify->getName(), "packageA");
        EXPECT_EQ(Verify->getAlias(), "");
    }

    TEST_F(ParserTest, SingleImportAlias) {
        llvm::StringRef str1 = ("namespace standard");
        llvm::StringRef str2 = ("import standard std");
        ASTNode *Node1 = Parse("standard.fly", str1, false);
        ASTNode *Node2 = Parse("default.fly", str2);
        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node2->getNameSpace()->getName(), "default");
        ASTImport *Import = Node2->getImports().lookup("std");
        EXPECT_EQ(Import->getName(), "standard");
        EXPECT_EQ(Import->getAlias(), "std");
    }

    TEST_F(ParserTest,  LineComments) {
        llvm::StringRef str = ("// Global var comment\n"
                               "int b\n"
                               "// Func comment\n"
                               "void func() {}\n"
        );

        ASTNode *Node = Parse("LineComments", str);
        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *GlobalB = Node->getGlobalVars().find("b")->getValue();
        EXPECT_EQ(GlobalB->getName(), "b");
        EXPECT_EQ(GlobalB->getComment(), "");

        const ASTFunction *Func = *Node->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(Func->getName(), "func");
        EXPECT_EQ(Func->getComment(), "");
    }

    TEST_F(ParserTest, BlockComments) {
        llvm::StringRef str = (" /* Global var block comment */\n"
                               "// Global var line comment\n"
                               "int b\n"
                               "// Func line comment\n"
                               "\t /*   Func block comment \n*/\n"
                               "void func() {\n"
                               "  /* body comment */\n"
                               "}\n"
                               "//body comment\n"
                               "void func2() {}\n"
        );
        ASTNode *Node = Parse("BlockComments", str);
        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *GlobalB = Node->getGlobalVars().find("b")->getValue();
        EXPECT_EQ(GlobalB->getName(), "b");
        EXPECT_EQ(GlobalB->getComment(), "/* Global var block comment */");

        ASTFunction *Func = *Node->getFunctions().find("func")->getValue().begin()->second.begin();
        EXPECT_EQ(Func->getName(), "func");
        EXPECT_EQ(Func->getComment(), "/*   Func block comment \n*/");

        ASTFunction *Func2 = *Node->getFunctions().find("func2")->getValue().begin()->second.begin();
        EXPECT_EQ(Func2->getName(), "func2");
        EXPECT_EQ(Func2->getComment(), StringRef());
    }

    TEST_F(ParserTest, GlobalVars) {
        llvm::StringRef str = ("private int a\n"
                                "public float b\n"
                                "bool c\n"
                                "long d\n"
                                "double e\n"
                                "byte f\n"
                                "ushort g\n"
                                "short h\n"
                                "uint i\n"
                                "ulong j\n"
                                 );
        ASTNode *Node = Parse("GlobalVars", str);

        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *VerifyA = Node->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *VerifyB = Node->getNameSpace()->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *VerifyC = Node->getGlobalVars().find("c")->getValue();
        ASTGlobalVar *VerifyD = Node->getGlobalVars().find("d")->getValue();
        ASTGlobalVar *VerifyE = Node->getGlobalVars().find("e")->getValue();
        ASTGlobalVar *VerifyF = Node->getGlobalVars().find("f")->getValue();
        ASTGlobalVar *VerifyG = Node->getGlobalVars().find("g")->getValue();
        ASTGlobalVar *VerifyH = Node->getGlobalVars().find("h")->getValue();
        ASTGlobalVar *VerifyI = Node->getGlobalVars().find("i")->getValue();
        ASTGlobalVar *VerifyJ = Node->getGlobalVars().find("j")->getValue();

        EXPECT_EQ(VerifyA->getScopes()->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_FALSE(VerifyA->getScopes()->isConstant());
        EXPECT_EQ(VerifyA->getType()->getKind(), ASTTypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");

        EXPECT_EQ(VerifyB->getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_FALSE(VerifyB->getScopes()->isConstant());
        EXPECT_EQ(VerifyB->getType()->getKind(), ASTTypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");

        EXPECT_EQ(VerifyC->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyC->getScopes()->isConstant());
        EXPECT_EQ(VerifyC->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");

        EXPECT_EQ(VerifyD->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyD->getScopes()->isConstant());
        EXPECT_EQ(VerifyD->getType()->getKind(), ASTTypeKind::TYPE_LONG);
        EXPECT_EQ(VerifyD->getName(), "d");

        EXPECT_EQ(VerifyE->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyE->getScopes()->isConstant());
        EXPECT_EQ(VerifyE->getType()->getKind(), ASTTypeKind::TYPE_DOUBLE);
        EXPECT_EQ(VerifyE->getName(), "e");

        EXPECT_EQ(VerifyF->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyF->getScopes()->isConstant());
        EXPECT_EQ(VerifyF->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(VerifyF->getName(), "f");

        EXPECT_EQ(VerifyG->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyG->getScopes()->isConstant());
        EXPECT_EQ(VerifyG->getType()->getKind(), ASTTypeKind::TYPE_USHORT);
        EXPECT_EQ(VerifyG->getName(), "g");

        EXPECT_EQ(VerifyH->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyH->getScopes()->isConstant());
        EXPECT_EQ(VerifyH->getType()->getKind(), ASTTypeKind::TYPE_SHORT);
        EXPECT_EQ(VerifyH->getName(), "h");

        EXPECT_EQ(VerifyI->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyI->getScopes()->isConstant());
        EXPECT_EQ(VerifyI->getType()->getKind(), ASTTypeKind::TYPE_UINT);
        EXPECT_EQ(VerifyI->getName(), "i");

        EXPECT_EQ(VerifyJ->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyJ->getScopes()->isConstant());
        EXPECT_EQ(VerifyJ->getType()->getKind(), ASTTypeKind::TYPE_ULONG);
        EXPECT_EQ(VerifyJ->getName(), "j");

    }

    TEST_F(ParserTest, GlobalConstants) {
        llvm::StringRef str = (
                "private const int a = 1\n"
                "const public float b = 2.0\n"
                "const bool c = false\n"
                );
        ASTNode *Node = Parse("GlobalConstants", str);

        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *VerifyA = Node->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *VerifyB = Node->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *VerifyC = Node->getGlobalVars().find("c")->getValue();

        EXPECT_EQ(VerifyA->getScopes()->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_EQ(VerifyA->getScopes()->isConstant(), true);
        EXPECT_EQ(VerifyA->getType()->getKind(), ASTTypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)VerifyA->getExpr())->getValue()).getValue(), 1);

        EXPECT_EQ(VerifyB->getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_EQ(VerifyB->getScopes()->isConstant(), true);
        EXPECT_EQ(VerifyB->getType()->getKind(), ASTTypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");
        EXPECT_EQ(((ASTFloatingValue &) ((ASTValueExpr *) VerifyB->getExpr())->getValue()).getValue(), "2.0");

        EXPECT_EQ(VerifyC->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_EQ(VerifyC->getScopes()->isConstant(), true);
        EXPECT_EQ(VerifyC->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");
        EXPECT_EQ(((ASTBoolValue &) ((ASTValueExpr *)VerifyC->getExpr())->getValue()).getValue(), false);

    }

    TEST_F(ParserTest, GlobalArray) {
        llvm::StringRef str = (
                               "byte[] a\n" // array of zero bytes
                               "byte[] b = {}\n"
                               "byte[] c = {1, 2, 3}\n"
                               "byte[3] d\n" // array of 4 bytes without values
                               "byte[3] e = {1, 2, 3}\n"
                               );
        ASTNode *Node = Parse("GlobalArray", str);
        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *a = Node->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *b = Node->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *c = Node->getGlobalVars().find("c")->getValue();
        ASTGlobalVar *d = Node->getGlobalVars().find("d")->getValue();
        ASTGlobalVar *e = Node->getGlobalVars().find("e")->getValue();

        // a
        EXPECT_EQ(a->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) a->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) a->getType())->getSize())->getValue()).getValue(), 0);
        EXPECT_EQ(a->getExpr(), nullptr);

        // b
        EXPECT_EQ(b->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) b->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) b->getType())->getSize())->getValue()).getValue(), 0);
        EXPECT_NE(b->getExpr(), nullptr);
        ASTValueExpr *bExpr = (ASTValueExpr *) b->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) bExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).size(), 0);
        EXPECT_TRUE(((const ASTArrayValue &) bExpr->getValue()).empty());

        // c
        EXPECT_EQ(c->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) c->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) c->getType())->getSize())->getValue()).getValue(), 3); // FIXME?
        EXPECT_NE(c->getExpr(), nullptr);
        ASTValueExpr *cExpr = (ASTValueExpr *) c->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) cExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).size(), 3);
        EXPECT_FALSE(((const ASTArrayValue &) cExpr->getValue()).empty());
        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getValues()[0]->print(), "1");
        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getValues()[1]->print(), "2");
        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getValues()[2]->print(), "3");

        // d
        EXPECT_EQ(d->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) d->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) d->getType())->getSize())->getValue()).getValue(), 3);
        EXPECT_EQ(d->getExpr(), nullptr);

        // e
        EXPECT_EQ(e->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) e->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) e->getType())->getSize())->getValue()).getValue(), 3);
        EXPECT_NE(e->getExpr(), nullptr);
        ASTValueExpr *eExpr = (ASTValueExpr *) c->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) eExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) eExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
        EXPECT_EQ(((const ASTArrayValue &) eExpr->getValue()).size(), 3);
        EXPECT_FALSE(((const ASTArrayValue &) eExpr->getValue()).empty());
        EXPECT_EQ(((const ASTArrayValue &) eExpr->getValue()).getValues()[0]->print(), "1");
        EXPECT_EQ(((const ASTArrayValue &) eExpr->getValue()).getValues()[1]->print(), "2");
        EXPECT_EQ(((const ASTArrayValue &) eExpr->getValue()).getValues()[2]->print(), "3");

    }

    TEST_F(ParserTest, GlobalChar) {
        llvm::StringRef str = (
               "byte a = ''\n"
               "byte b = 'b'\n"

               "byte[] c = {'a', 'b', 'c', 0}\n"
               "byte[2] d = {'', ''}\n" // Empty string
        );
        ASTNode *Node = Parse("GlobalChar", str);
        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *a = Node->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *b = Node->getGlobalVars().find("b")->getValue();
        ASTGlobalVar *c = Node->getGlobalVars().find("c")->getValue();
        ASTGlobalVar *d = Node->getGlobalVars().find("d")->getValue();

        // a
        EXPECT_EQ(a->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_NE(a->getExpr(), nullptr);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)a->getExpr())->getValue()).getValue(), 0);

        // b
        EXPECT_EQ(b->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_NE(b->getExpr(), nullptr);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)b->getExpr())->getValue()).getValue(), 'b');

        // c
        EXPECT_EQ(c->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) c->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) c->getType())->getSize())->getValue()).getValue(), 4); FIXME?
        EXPECT_NE(c->getExpr(), nullptr);
        ASTValueExpr *cExpr = (ASTValueExpr *) c->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) cExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); FIXME?
        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).size(), 4);
        EXPECT_FALSE(((const ASTArrayValue &) cExpr->getValue()).empty());
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue &) cExpr->getValue()).getValues()[0])->getValue(), (uint64_t)'a');
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue &) cExpr->getValue()).getValues()[1])->getValue(), 'b');
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue &) cExpr->getValue()).getValues()[2])->getValue(), 'c');
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue &) cExpr->getValue()).getValues()[3])->getValue(), 0);

        // d
        EXPECT_EQ(d->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) d->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) d->getType())->getSize())->getValue()).getValue(), 2);
        EXPECT_NE(d->getExpr(), nullptr);
        ASTValueExpr *dExpr = (ASTValueExpr *) d->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) dExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) dExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); FIXME?
        EXPECT_EQ(((const ASTArrayValue &) dExpr->getValue()).size(), 2);
        EXPECT_FALSE(((const ASTArrayValue &) dExpr->getValue()).empty());
        EXPECT_EQ(((const ASTArrayValue &) dExpr->getValue()).getValues()[0]->print(), "0");
        EXPECT_EQ(((const ASTArrayValue &) dExpr->getValue()).getValues()[1]->print(), "0");

    }

    TEST_F(ParserTest, GlobalString) {
        llvm::StringRef str = (
               "byte[] a = \"\"\n" // array of zero bytes
               "byte[] b = \"abc\"\n" // string abc/0 -> array of 4 bytes
        );
        ASTNode *Node = Parse("GlobalString", str);
        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *a = Node->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *b = Node->getGlobalVars().find("b")->getValue();

        // a
        EXPECT_EQ(a->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) a->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) a->getType())->getSize())->getValue()).getValue(), 0);
        EXPECT_NE(a->getExpr(), nullptr);
        const ASTValue &Val = ((ASTValueExpr *) a->getExpr())->getValue();
        EXPECT_EQ(((ASTArrayValue &) Val).getValues().size(), 0);

        // b
        EXPECT_EQ(b->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) b->getType())->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTArrayValue &) ((ASTValueExpr *) b->getExpr())->getValue()).getValues().size(), 3);
        EXPECT_NE(b->getExpr(), nullptr);
        ASTValueExpr *bExpr = (ASTValueExpr *) b->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); FIXme/
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) bExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); FIXME?
        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).size(), 3);
        EXPECT_FALSE(((const ASTArrayValue &) bExpr->getValue()).empty());
        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).getValues()[0]->print(), "97");
        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).getValues()[1]->print(), "98");
        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).getValues()[2]->print(), "99");

    }

    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
        llvm::StringRef str = ("void func() {}\n");
        ASTNode *AST = Parse("FunctionDefaultVoidEmpty", str);
        ASSERT_TRUE(isSuccess());

        EXPECT_TRUE(AST->getFunctions().size() == 1); // Fun has DEFAULT Visibility
        EXPECT_TRUE(AST->getNameSpace()->getFunctions().size() == 1);
        ASTFunction *VerifyFunc = *AST->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(VerifyFunc->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        const auto &NSFuncs = AST->getContext().getDefaultNameSpace()->getFunctions();
        ASSERT_TRUE(NSFuncs.find(VerifyFunc->getName()) != NSFuncs.end());
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        llvm::StringRef str = (
                "private int func(int a, const float b, bool c=false) {\n"
                "  return 1"
                "}\n");
        ASTNode *Node = Parse("FunctionPrivateReturnParams", str);

        ASSERT_TRUE(isSuccess());

        EXPECT_TRUE(Node->getFunctions().size() == 1); // func() has PRIVATE Visibility
        ASTFunction *VerifyFunc = *Node->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(VerifyFunc->getScopes()->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        const auto &NSFuncs = Node->getContext().getDefaultNameSpace()->getFunctions();
        ASSERT_TRUE(NSFuncs.find(VerifyFunc->getName()) == NSFuncs.end());
        ASSERT_TRUE(Node->getFunctions().find(VerifyFunc->getName()) != NSFuncs.end());

        ASTParam *Par0 = VerifyFunc->getParams()->getList()[0];
        ASTParam *Par1 = VerifyFunc->getParams()->getList()[1];
        ASTParam *Par2 = VerifyFunc->getParams()->getList()[2];

        EXPECT_EQ(Par0->getName(), "a");
        EXPECT_EQ(Par0->getType()->getKind(), ASTTypeKind::TYPE_INT);
        EXPECT_EQ(Par0->isConstant(), false);

        EXPECT_EQ(Par1->getName(), "b");
        EXPECT_EQ(Par1->getType()->getKind(), ASTTypeKind::TYPE_FLOAT);
        EXPECT_EQ(Par1->isConstant(), true);

        EXPECT_EQ(Par2->getName(), "c");
        EXPECT_EQ(Par2->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(Par2->isConstant(), false);
        EXPECT_EQ(Par2->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        ASTValueExpr *DefArg2 = (ASTValueExpr *)Par2->getExpr();
        EXPECT_EQ(((ASTBoolValue &) DefArg2->getValue()).getValue(), false);
        EXPECT_EQ(DefArg2->getValue().print(), "false");

        ASTReturn *Return = (ASTReturn *) VerifyFunc->getBody()->getContent()[0];
        EXPECT_EQ(((ASTValueExpr *) Return->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr*) Return->getExpr())->getValue().print(), "1");
        EXPECT_EQ(Return->getKind(), ASTStmtKind::STMT_RETURN);

    }

    TEST_F(ParserTest, NullTypeVarReturn) {
        llvm::StringRef str = ("Type func() {\n"
                               "  Type t = null"
                               "  return t\n"
                               "}\n");
        ASTNode *Node = Parse("TypeDefaultVarReturn", str, false);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(F->getType()->getKind(), ASTTypeKind::TYPE_CLASS);
        const ASTBlock *Body = F->getBody();

        // Test: Type t
        ASTLocalVar *typeVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(typeVar->getName(), "t");
        EXPECT_EQ(typeVar->getType()->getKind(), ASTTypeKind::TYPE_CLASS);
        ASTClassType *ClassType = (ASTClassType *) typeVar->getType();
        EXPECT_EQ(ClassType->getName(), "Type");
        ASSERT_EQ(((ASTNullValue &)((ASTValueExpr *) typeVar->getExpr())->getValue()).print(), "null");

        const ASTReturn *Ret = (ASTReturn *) Body->getContent()[1];
        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetRef->getVarRef()->getName(), "t");

    }

    TEST_F(ParserTest, UndefLocalVar) {
        llvm::StringRef str = ("void func() {\n"
                               "bool a\n"
                               "byte b\n"
                               "short c\n"
                               "ushort d\n"
                               "int e\n"
                               "uint f\n"
                               "long g\n"
                               "ulong h\n"
                               "float i\n"
                               "double j\n"
                               "Type t\n"
                               "}\n");
        ASTNode *Node = Parse("UndefLocalVar", str, false);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(F->getType()->getKind(), ASTTypeKind::TYPE_VOID);
        const ASTBlock *Body = F->getBody();

        // Test: bool a
        ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        ASSERT_EQ(aVar->getExpr(), nullptr);

        // Test: byte b
        ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getType()->getKind(), ASTTypeKind::TYPE_BYTE);
        ASSERT_EQ(bVar->getExpr(), nullptr);

        // Test: short c
        ASTLocalVar *cVar = (ASTLocalVar *) Body->getContent()[2];
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(cVar->getType()->getKind(), ASTTypeKind::TYPE_SHORT);
        ASSERT_EQ(cVar->getExpr(), nullptr);

        // Test: ushort d
        ASTLocalVar *dVar = (ASTLocalVar *) Body->getContent()[3];
        EXPECT_EQ(dVar->getName(), "d");
        EXPECT_EQ(dVar->getType()->getKind(), ASTTypeKind::TYPE_USHORT);
        ASSERT_EQ(dVar->getExpr(), nullptr);

        // Test: int e
        ASTLocalVar *eVar = (ASTLocalVar *) Body->getContent()[4];
        EXPECT_EQ(eVar->getName(), "e");
        EXPECT_EQ(eVar->getType()->getKind(), ASTTypeKind::TYPE_INT);
        ASSERT_EQ(eVar->getExpr(), nullptr);

        // Test: uint f
        ASTLocalVar *fVar = (ASTLocalVar *) Body->getContent()[5];
        EXPECT_EQ(fVar->getName(), "f");
        EXPECT_EQ(fVar->getType()->getKind(), ASTTypeKind::TYPE_UINT);
        ASSERT_EQ(fVar->getExpr(), nullptr);

        // Test: long g
        ASTLocalVar *gVar = (ASTLocalVar *) Body->getContent()[6];
        EXPECT_EQ(gVar->getName(), "g");
        EXPECT_EQ(gVar->getType()->getKind(), ASTTypeKind::TYPE_LONG);
        ASSERT_EQ(gVar->getExpr(), nullptr);

        // Test: ulong h
        ASTLocalVar *hVar = (ASTLocalVar *) Body->getContent()[7];
        EXPECT_EQ(hVar->getName(), "h");
        EXPECT_EQ(hVar->getType()->getKind(), ASTTypeKind::TYPE_ULONG);
        ASSERT_EQ(hVar->getExpr(), nullptr);

        // Test: float i
        ASTLocalVar *iVar = (ASTLocalVar *) Body->getContent()[8];
        EXPECT_EQ(iVar->getName(), "i");
        EXPECT_EQ(iVar->getType()->getKind(), ASTTypeKind::TYPE_FLOAT);
        ASSERT_EQ(iVar->getExpr(), nullptr);

        // Test: double j
        ASTLocalVar *jVar = (ASTLocalVar *) Body->getContent()[9];
        EXPECT_EQ(jVar->getName(), "j");
        EXPECT_EQ(jVar->getType()->getKind(), ASTTypeKind::TYPE_DOUBLE);
        ASSERT_EQ(jVar->getExpr(), nullptr);

        // Test: Type t
        ASTLocalVar *tVar = (ASTLocalVar *) Body->getContent()[10];
        EXPECT_EQ(tVar->getName(), "t");
        EXPECT_EQ(tVar->getType()->getKind(), ASTTypeKind::TYPE_CLASS);
        ASSERT_EQ(tVar->getExpr(), nullptr);

    }

    TEST_F(ParserTest, UnaryExpr) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  ++a"
                "  a++"
                "  --a"
                "  a--"
                "  a = ++a + 1"
                "}\n");
        ASTNode *Node = Parse("UnaryExpr", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // ++a
        ASTExprStmt *a1Stmt = (ASTExprStmt *) Body->getContent()[0];
        ASTUnaryGroupExpr *a1Unary = (ASTUnaryGroupExpr *) a1Stmt->getExpr();
        EXPECT_EQ(a1Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);
        EXPECT_EQ(a1Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_PRE);
        EXPECT_EQ(((ASTVarRefExpr *) a1Unary->getFirst())->getVarRef()->getName(), "a");

        // a++
        ASTExprStmt *a2Stmt = (ASTExprStmt *) Body->getContent()[1];
        ASTUnaryGroupExpr *a2Unary = (ASTUnaryGroupExpr *) a2Stmt->getExpr();
        EXPECT_EQ(a2Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);
        EXPECT_EQ(a2Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_POST);
        EXPECT_EQ(((ASTVarRefExpr *) a2Unary->getFirst())->getVarRef()->getName(), "a");

        // --a
        ASTExprStmt *a3Stmt = (ASTExprStmt *) Body->getContent()[2];
        ASTUnaryGroupExpr *a3Unary = (ASTUnaryGroupExpr *) a3Stmt->getExpr();
        EXPECT_EQ(a3Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_DECR);
        EXPECT_EQ(a3Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_PRE);
        EXPECT_EQ(((ASTVarRefExpr *) a3Unary->getFirst())->getVarRef()->getName(), "a");

        // a--
        ASTExprStmt *a4Stmt = (ASTExprStmt *) Body->getContent()[3];
        ASTUnaryGroupExpr *a4Unary = (ASTUnaryGroupExpr *) a4Stmt->getExpr();
        EXPECT_EQ(a4Unary->getOperatorKind(), ASTUnaryOperatorKind::ARITH_DECR);
        EXPECT_EQ(a4Unary->getOptionKind(), ASTUnaryOptionKind::UNARY_POST);
        EXPECT_EQ(((ASTVarRefExpr *) a4Unary->getFirst())->getVarRef()->getName(), "a");

        // a = ++a + 1
        const ASTVarAssign *a5Var = (ASTVarAssign *) Body->getContent()[4];
        EXPECT_EQ(a5Var->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        ASTBinaryGroupExpr *Group = (ASTBinaryGroupExpr *) a5Var->getExpr();
        EXPECT_EQ(Group->getOperatorKind(), ASTBinaryOperatorKind::ARITH_ADD);
        EXPECT_EQ(Group->getOptionKind(), ASTBinaryOptionKind::BINARY_ARITH);
        const ASTUnaryGroupExpr *E1 = (ASTUnaryGroupExpr *) Group->getFirst();
        EXPECT_EQ(E1->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);
        EXPECT_EQ(E1->getOptionKind(), ASTUnaryOptionKind::UNARY_PRE);
        ASTValueExpr *ValueExpr = (ASTValueExpr *) Group->getSecond();
        EXPECT_EQ(ValueExpr->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(ValueExpr->getValue().print(), "1");
    }

    TEST_F(ParserTest, IntBinaryArithOperation) {
        llvm::StringRef str = ("int func() {\n"
                               "  int a = 2\n"
                               "  int b = a + b / a - b\n"
                               "  return a\n"
                               "}\n");
        ASTNode *Node = Parse("IntBinaryArithOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *(Node->getNameSpace()->getFunctions().begin())->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: int a = 2

        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue().print(), "2");

        // Test: int b = a + b / a - b

        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 - b
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::ARITH_SUB);
        EXPECT_EQ(((ASTVarRefExpr *) bGroup->getSecond())->getVarRef()->getName(), "b");

        // G1 = a + G2
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::ARITH_ADD);
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) G1->getSecond();

        // G2 = b / a
        const ASTVarRefExpr *b = (ASTVarRefExpr *) G2->getFirst();
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::ARITH_DIV);
        const ASTVarRefExpr *a = (ASTVarRefExpr *) G2->getSecond();

    }

    TEST_F(ParserTest, FloatBinaryArithOperation) {
        llvm::StringRef str = ("float func() {\n"
                               "  float a = 1.0"
                               "  float b = a * b - a / b"
                               "  return b\n"
                               "}\n");
        ASTNode *Node = Parse("FloatBinaryArithOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: float a -= 1.0
        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue().print(), "1.0");

        // Test: int b = a * b - a / b
        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 - G2
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) bGroup->getSecond();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::ARITH_SUB);

        // G1 = a * b
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::ARITH_MUL);
        EXPECT_EQ(((ASTVarRefExpr *) G1->getSecond())->getVarRef()->getName(), "b");

        // G2 = a / b
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::ARITH_DIV);
        EXPECT_EQ(((ASTVarRefExpr *) G2->getSecond())->getVarRef()->getName(), "b");

    }

    TEST_F(ParserTest, BoolBinaryLogicOperation) {
        llvm::StringRef str = ("bool func() {\n"
                               "  bool a = true"
                               "  bool b = a || false && a == true\n"
                               "  return b\n"
                               "}\n");
        ASTNode *Node = Parse("BoolBinaryLogicOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: bool a = true
        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue().print(), "true");

        // Test: bool b = a || false && a == true
        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // int b = G1 == true
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) bGroup->getSecond())->getValue().print(), "true");

        // G1 = G2 && a
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) G1->getFirst();
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::LOGIC_AND);
        EXPECT_EQ(((ASTValueExpr *) G2->getSecond())->getValue().print(), "false");

        // G2 = a || false
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::LOGIC_OR);
        EXPECT_EQ(((ASTValueExpr *) G2->getSecond())->getValue().print(), "false");

    }

    TEST_F(ParserTest, LongBinaryArithOperation) {
        llvm::StringRef str = ("long func() {\n"
                               "  long a = 1\n"
                               "  long b = (a + b) / (a - b)\n"
                               "  return b\n"
                               "}\n");
        ASTNode *Node = Parse("LongBinaryArithOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // Test: long a = 1

        const ASTLocalVar *aVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr *) aVar->getExpr())->getValue().print(), "1");

        // Test: long b = (a + b) / (a - b)

        const ASTLocalVar *bVar = (ASTLocalVar *) Body->getContent()[1];
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getExpr()->getExprKind(), ASTExprKind::EXPR_GROUP);
        EXPECT_EQ(((ASTGroupExpr *) bVar->getExpr())->getGroupKind(), ASTExprGroupKind::GROUP_BINARY);
        ASTBinaryGroupExpr *bGroup = (ASTBinaryGroupExpr *) bVar->getExpr();

        // long b = G1 / G2
        const ASTBinaryGroupExpr *G1 = (ASTBinaryGroupExpr *) bGroup->getFirst();
        const ASTBinaryGroupExpr *G2 = (ASTBinaryGroupExpr *) bGroup->getSecond();
        EXPECT_EQ(bGroup->getOperatorKind(), ASTBinaryOperatorKind::ARITH_DIV);

        // G1 = a + b
        EXPECT_EQ(((ASTVarRefExpr *) G1->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G1->getOperatorKind(), ASTBinaryOperatorKind::ARITH_ADD);
        EXPECT_EQ(((ASTVarRefExpr *) G1->getSecond())->getVarRef()->getName(), "b");

        // G2 = a - b
        EXPECT_EQ(((ASTVarRefExpr *) G2->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(G2->getOperatorKind(), ASTBinaryOperatorKind::ARITH_SUB);
        EXPECT_EQ(((ASTVarRefExpr *) G2->getSecond())->getVarRef()->getName(), "b");

    }

    TEST_F(ParserTest, CondTernaryOperation) {
        llvm::StringRef str = ("int func(int a) {\n"
                               "  return a==1 ? 1 : a\n"
                               "}\n");
        ASTNode *Node = Parse("CondTernaryOperation", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        ASTParam *a = F->getParams()->getList()[0];
        const ASTBlock *Body = F->getBody();

        ASTReturn *Ret = (ASTReturn *) Body->getContent()[0];
        ASTTernaryGroupExpr *Expr = (ASTTernaryGroupExpr *) Ret->getExpr();
        ASTBinaryGroupExpr *Comp = ((ASTBinaryGroupExpr *) Expr->getFirst());
        EXPECT_EQ(Comp->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTVarRefExpr *) Comp->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(((ASTValueExpr *) Comp->getSecond())->getValue().print(), "1");
        EXPECT_EQ(((ASTValueExpr *) Expr->getSecond())->getValue().print(), "1");
        EXPECT_EQ(((ASTVarRefExpr *) Expr->getThird())->getVarRef()->getName(), "a");

    }

    TEST_F(ParserTest, FunctionCall) {
        llvm::StringRef str = ("private int doSome() {return 1}\n"
                               "public void doOther(int a, int b) {}\n"
                               "int doNow() {return 0}"
                               "int main(int a) {\n"
                               "  int b = doSome()\n"
                               "  b = doNow()\n"
                               "  doOther(a, 1)\n"
                               "  return b\n"
                               "}\n");
        ASTNode *Node = Parse("FunctionCall", str);

        ASSERT_TRUE(isSuccess());

        // Get all functions
        ASTFunction *doSome = *Node->getFunctions().find("doSome")->getValue().begin()->second.begin();
        ASTFunction *doOther = *Node->getFunctions().find("doOther")->getValue().begin()->second.begin();
        ASTFunction *doNow = *Node->getFunctions().find("doNow")->getValue().begin()->second.begin();
        ASTFunction *main = *Node->getFunctions().find("main")->getValue().begin()->second.begin();

        ASSERT_TRUE(doSome != nullptr);
        ASSERT_TRUE(doOther != nullptr);
        ASSERT_TRUE(main != nullptr);

        // Get main() Body
        const ASTBlock *Body = main->getBody();

        // Test: doSome()
        ASTLocalVar *VarB = ((ASTLocalVar *) Body->getContent()[0]);
        ASTCallExpr *doSomeCall = (ASTCallExpr *) VarB->getExpr();
        EXPECT_EQ(doSomeCall->getCall()->getName(), "doSome");
        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);
        ASSERT_FALSE(doSomeCall->getCall()->getDef() == nullptr);

        // Test: doNow()
        ASTVarAssign *B = ((ASTVarAssign *) Body->getContent()[1]);
        ASTCallExpr *doNowCall = (ASTCallExpr *) B->getExpr();
        EXPECT_EQ(doNowCall->getCall()->getName(), "doNow");
        EXPECT_EQ(doNowCall->getExprKind(), ASTExprKind::EXPR_CALL);
        ASSERT_FALSE(doNowCall->getCall()->getDef() == nullptr);

        // Test: doOther(a, b)
        ASTExprStmt *doOtherStmt = (ASTExprStmt *) Body->getContent()[2];
        EXPECT_EQ(doOtherStmt->getKind(), ASTStmtKind::STMT_EXPR);
        ASTCallExpr *doOtherCall = (ASTCallExpr *) doOtherStmt->getExpr();
        EXPECT_EQ(doOtherCall->getCall()->getName(), "doOther");
        ASTArg *Arg0 = doOtherCall->getCall()->getArgs()[0];
        EXPECT_EQ(((ASTVarRefExpr *) Arg0->getExpr())->getVarRef()->getName(), "a");
        ASTArg *Arg1 = doOtherCall->getCall()->getArgs()[1];
        EXPECT_EQ(((ASTValueExpr *) Arg1->getExpr())->getValue().print(), "1");

        // return do()
        ASTReturn *Ret = (ASTReturn *) Body->getContent()[3];
        ASTVarRefExpr *RetExpr = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetExpr->getVarRef()->getName(), "b");
        EXPECT_FALSE(RetExpr->getVarRef()->getDef() == nullptr);

    }

    TEST_F(ParserTest, IfElsifElseStmt) {
        llvm::StringRef str = (
                         "void func(int a, int b) {\n"
                         "  if (a == 1) {"
                         "    return"
                         "  } elsif (a == 2) {"
                         "    b = 1"
                         "  } else {"
                         "    b = 2"
                         "  }"
                         "}\n");
        ASTNode *Node = Parse("IfElsifElseStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // If
        ASTIfBlock *IfBlock = (ASTIfBlock *) Body->getContent()[0];
        EXPECT_EQ(IfBlock->getBlockKind(), ASTBlockKind::BLOCK_IF);
        ASTBinaryGroupExpr *IfCond = (ASTBinaryGroupExpr *) IfBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) IfCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(IfCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) IfCond->getSecond())->getValue().print(), "1");
        EXPECT_TRUE(((ASTEmptyExpr *)((ASTReturn *) IfBlock->getContent()[0])->getExpr())->getExprKind() == ASTExprKind::EXPR_EMPTY);
        EXPECT_FALSE(IfBlock->getElsifBlocks().empty());
        EXPECT_TRUE(IfBlock->getElseBlock());

        // Elsif
        ASTElsifBlock *ElsifBlock = IfBlock->getElsifBlocks()[0];
        ASTBinaryGroupExpr *ElsifCond = (ASTBinaryGroupExpr *) ElsifBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(ElsifCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) ElsifCond->getSecond())->getValue().print(), "2");
        EXPECT_EQ(((ASTVarAssign *) ElsifBlock->getContent()[0])->getVarRef()->getName(), "b");

        // Else
        ASTElseBlock *ElseBlock = IfBlock->getElseBlock();
        EXPECT_EQ(ElseBlock->getBlockKind(), ASTBlockKind::BLOCK_ELSE);
        EXPECT_EQ(((ASTVarAssign *)ElseBlock->getContent()[0])->getVarRef()->getName(), "b");

    }

    TEST_F(ParserTest, IfElsifElseInlineStmt) {
        llvm::StringRef str = (
                         "void func(int a) {\n"
                         "  if (a == 1) return"
                         "  elsif a == 2 a = 1"
                         "  else a = 2"
                         "}\n");
        ASTNode *Node = Parse("IfElsifElseInlineStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        // if
        ASTIfBlock *IfBlock = (ASTIfBlock *) Body->getContent()[0];
        EXPECT_EQ(IfBlock->getBlockKind(), ASTBlockKind::BLOCK_IF);
        ASTBinaryGroupExpr *IfCond = (ASTBinaryGroupExpr *) IfBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) IfCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(IfCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) IfCond->getSecond())->getValue().print(), "1");
        EXPECT_TRUE(((ASTEmptyExpr *)((ASTReturn *) IfBlock->getContent()[0])->getExpr())->getExprKind() == ASTExprKind::EXPR_EMPTY);

        EXPECT_FALSE(IfBlock->getElsifBlocks().empty());
        EXPECT_TRUE(IfBlock->getElseBlock());

        // Elsif
        ASTElsifBlock *ElsifBlock = IfBlock->getElsifBlocks()[0];
        EXPECT_EQ(ElsifBlock->getBlockKind(), ASTBlockKind::BLOCK_ELSIF);
        ASTBinaryGroupExpr *ElsifCond = (ASTBinaryGroupExpr *) ElsifBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) ElsifCond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(ElsifCond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) ElsifCond->getSecond())->getValue().print(), "2");

        // Else
        ASTElseBlock *ElseBlock = IfBlock->getElseBlock();
        EXPECT_EQ(ElseBlock->getBlockKind(), ASTBlockKind::BLOCK_ELSE);
        EXPECT_EQ(((ASTVarAssign *) ElseBlock->getContent()[0])->getVarRef()->getName(), "a");

    }

    TEST_F(ParserTest, SwitchCaseDefaultStmt) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  switch (a) {"
                "    case 1:"
                "      break"
                "    case 2:"
                "    default:"
                "      return"
                "  }"
                "}\n");
        ASTNode *Node = Parse("SwitchCaseDefaultStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();

        ASTSwitchBlock *SwitchBlock = (ASTSwitchBlock *) Body->getContent()[0];
        EXPECT_EQ(SwitchBlock->getBlockKind(), ASTBlockKind::BLOCK_SWITCH);
        EXPECT_EQ(((ASTValueExpr *) SwitchBlock->getCases()[0]->getExpr())->getValue().print(), "1");
        EXPECT_EQ(SwitchBlock->getCases()[0]->getContent()[0]->getKind(), ASTStmtKind::STMT_BREAK);
        EXPECT_EQ(((ASTValueExpr *) SwitchBlock->getCases()[1]->getExpr())->getValue().print(), "2");
        EXPECT_EQ(((ASTExprStmt *) SwitchBlock->getCases()[1]->getContent()[0])->getExpr()->getExprKind(), ASTExprKind::EXPR_EMPTY);
        EXPECT_EQ(SwitchBlock->getDefault()->getBlockKind(), ASTBlockKind::BLOCK_SWITCH_DEFAULT);
        EXPECT_EQ((SwitchBlock->getDefault()->getContent()[0])->getKind(), ASTStmtKind::STMT_RETURN);

    }

    TEST_F(ParserTest, WhileStmt) {
        llvm::StringRef str = (
                "void func(int a) {\n"
                "  while (a==1) {"
                "    a++"
                "  }\n"
                "}\n");
        ASTNode *Node = Parse("WhileStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();
        ASTWhileBlock *WhileBlock = (ASTWhileBlock *) Body->getContent()[0];
        EXPECT_EQ(WhileBlock->getBlockKind(), ASTBlockKind::BLOCK_WHILE);
        EXPECT_FALSE(WhileBlock->getCondition() == nullptr);
        EXPECT_FALSE(WhileBlock->isEmpty());

        const ASTBinaryGroupExpr *Cond = (ASTBinaryGroupExpr *) WhileBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) Cond->getFirst())->getVarRef()->getName(), "a");
        EXPECT_EQ(Cond->getOperatorKind(), ASTBinaryOperatorKind::COMP_EQ);
        EXPECT_EQ(((ASTValueExpr *) Cond->getSecond())->getValue().print(), "1");

    }

    TEST_F(ParserTest, WhileValueStmt) {
        llvm::StringRef str = ("void func(int a) {\n"
                               "  while true a++\n"
                               "}\n");
        ASTNode *Node = Parse("WhileValueStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();
        ASTWhileBlock *WhileBlock = (ASTWhileBlock *) Body->getContent()[0];
        EXPECT_EQ(WhileBlock->getBlockKind(), ASTBlockKind::BLOCK_WHILE);
        EXPECT_EQ(((ASTValueExpr *) WhileBlock->getCondition())->getValue().print(), "true");
        EXPECT_FALSE(WhileBlock->isEmpty());
    }

    TEST_F(ParserTest, ForStmt) {
        llvm::StringRef str = (
                "private void func(int a) {\n"
                "  for int b = 1, int c = 2; b < 10; b++, --c {"
                "  }"
                "}\n");
        ASTNode *Node = Parse("ForStmt", str);

        ASSERT_TRUE(isSuccess());

        // Get Body
        ASTFunction *F = *Node->getFunctions().begin()->getValue().begin()->second.begin();
        const ASTBlock *Body = F->getBody();
        ASTForBlock *ForBlock = (ASTForBlock *) Body->getContent()[0];
        EXPECT_EQ(ForBlock->getBlockKind(), ASTBlockKind::BLOCK_FOR);

        // int b =1
        EXPECT_EQ(((ASTLocalVar *) ForBlock->getContent()[0])->getName(), "b");
        // int c = 2
        EXPECT_EQ(((ASTLocalVar *) ForBlock->getContent()[1])->getName(), "c");

        // b < 10
        ASTBinaryGroupExpr *Cond = (ASTBinaryGroupExpr *) ForBlock->getCondition();
        EXPECT_EQ(((ASTVarRefExpr *) Cond->getFirst())->getVarRef()->getName(), "b");
        EXPECT_EQ(Cond->getOperatorKind(), ASTBinaryOperatorKind::COMP_LT);
        EXPECT_EQ(((ASTValueExpr *) Cond->getSecond())->getValue().print(), "10");

        // b++
        ASTExprStmt * ExprStmt1 = (ASTExprStmt *) ForBlock->getPost()->getContent()[0];
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt1->getExpr())->getFirst()->getVarRef()->getName(), "b");
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt1->getExpr())->getOperatorKind(), ASTUnaryOperatorKind::ARITH_INCR);

        // c--
        ASTExprStmt * ExprStmt2 = (ASTExprStmt *) ForBlock->getPost()->getContent()[1];
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt2->getExpr())->getFirst()->getVarRef()->getName(), "c");
        EXPECT_EQ(((ASTUnaryGroupExpr *) ExprStmt2->getExpr())->getOperatorKind(), ASTUnaryOperatorKind::ARITH_DECR);

        EXPECT_TRUE(ForBlock->getLoop()->isEmpty());
    }

    TEST_F(ParserTest, ClassEmpty) {
        llvm::StringRef str = ("public Test {}\n");
        ASTNode *Node = Parse("ClassEmpty", str);
        ASSERT_TRUE(isSuccess());

        EXPECT_FALSE(Node->getClass() == nullptr);
        EXPECT_TRUE(Node->getNameSpace()->getClasses().size() == 1);
        ASTClass &Class = *Node->getNameSpace()->getClasses().begin()->second;
        EXPECT_EQ(Class.getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        const auto &NSClassess = Node->getContext().getDefaultNameSpace()->getClasses();
        const auto &ClassTest = NSClassess.find("Test");
        ASSERT_TRUE(ClassTest != NSClassess.end());
    }

    TEST_F(ParserTest, ClassVars) {
        llvm::StringRef str = ("public Test {\n"
                               "  int a = 1\n"
                               "  public int b = 2\n"
                               "  private const int c\n"
                               "}\n");
        ASTNode *Node = Parse("ClassVars", str, false);

        EXPECT_FALSE(Node->getClass()->getVars().empty());
        EXPECT_EQ(Node->getClass()->getVars().size(), 3);
        ASTClassVar *aVar = Node->getClass()->getVars().find("a")->getValue();
        EXPECT_EQ(aVar->getScopes()->getVisibility(), ASTClassVisibilityKind::CLASS_V_DEFAULT);
        EXPECT_FALSE(aVar->getScopes()->isConstant());
        ASTClassVar *bVar = Node->getClass()->getVars().find("b")->getValue();
        EXPECT_EQ(bVar->getScopes()->getVisibility(), ASTClassVisibilityKind::CLASS_V_PUBLIC);
        EXPECT_FALSE(bVar->getScopes()->isConstant());
        ASTClassVar *cVar = Node->getClass()->getVars().find("c")->getValue();
        EXPECT_EQ(cVar->getScopes()->getVisibility(), ASTClassVisibilityKind::CLASS_V_PRIVATE);
        EXPECT_TRUE(cVar->getScopes()->isConstant());

        llvm::StringRef str2 = (
                "void func() {\n"
                "  Test t = new Test()"
                "  t.a = 1"
                "}\n");
        ASTNode *Node2 = Parse("Identifier", str2);
        ASSERT_TRUE(isSuccess());
    }

    TEST_F(ParserTest, ClassFunctions) {
        llvm::StringRef str = ("public Test {\n"
                               "  int a() { return 1 }\n"
                               "}\n");
        ASTNode *Node = Parse("ClassVars", str, false);

        llvm::StringRef str2 = (
                "void func() {\n"
                "  Test t = new Test()"
                "  t.a()"
                "}\n");
        ASTNode *Node2 = Parse("Identifier", str2);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node->getClass()->getMethods().size(), 1);
        ASTClassFunction *aMethod = *Node->getClass()->getMethods().find("a")->getValue().begin()->second.begin();
        EXPECT_EQ(aMethod->getScopes()->getVisibility(), ASTClassVisibilityKind::CLASS_V_DEFAULT);
        EXPECT_FALSE(aMethod->getScopes()->isConstant());
    }
}