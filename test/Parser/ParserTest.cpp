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
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTExpr.h"
#include "AST/ASTExprStmt.h"

#include "llvm/ADT/StringMap.h"

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

//    TEST_F(ParserTest,  LineComments) {
//        llvm::StringRef str = ("// Global var comment\n"
//                               "int b\n"
//                               "// Func comment\n"
//                               "void func() {}\n"
//        );
//
//        ASTModule *Module = Parse("LineComments", str);
//        ASSERT_TRUE(Resolve());

//
//        ASTGlobalVar *GlobalB = Module->getGlobalVars().find("b")->getValue();
//        EXPECT_EQ(GlobalB->getName(), "b");
//        EXPECT_EQ(GlobalB->getComment(), "");
//
//        const ASTFunction *Func = *Module->getFunctions().begin()->getValue().begin()->second.begin();
//        EXPECT_EQ(Func->getName(), "func");
//        EXPECT_EQ(Func->getComment(), "");
//    }
//
//    TEST_F(ParserTest, BlockComments) {
//        llvm::StringRef str = (" /* Global var block comment */\n"
//                               "// Global var line comment\n"
//                               "int b\n"
//                               "// Func line comment\n"
//                               "\t /*   Func block comment \n*/\n"
//                               "void func() {\n"
//                               "  /* body comment */\n"
//                               "}\n"
//                               "//body comment\n"
//                               "void func2() {}\n"
//        );
//        ASTModule *Module = Parse("BlockComments", str);

//
//        ASTGlobalVar *GlobalB = Module->getGlobalVars().find("b")->getValue();
//        EXPECT_EQ(GlobalB->getName(), "b");
//        EXPECT_EQ(GlobalB->getComment(), "/* Global var block comment */");
//
//        ASTFunction *Func = *Module->getFunctions().find("func")->getValue().begin()->second.begin();
//        EXPECT_EQ(Func->getName(), "func");
//        EXPECT_EQ(Func->getComment(), "/*   Func block comment \n*/");
//
//        ASTFunction *Func2 = *Module->getFunctions().find("func2")->getValue().begin()->second.begin();
//        EXPECT_EQ(Func2->getName(), "func2");
//        EXPECT_EQ(Func2->getComment(), StringRef());
//    }
//
//    TEST_F(ParserTest, GlobalVars) {
//        llvm::StringRef str = ("private int a\n"
//                                "public float b\n"
//                                "bool c\n"
//                                "long d\n"
//                                "double e\n"
//                                "byte f\n"
//                                "ushort g\n"
//                                "short h\n"
//                                "uint i\n"
//                                "ulong j\n"
//                                 );
//        ASTModule *Module = Parse("GlobalVars", str);
//
//        ASSERT_TRUE(Resolve());

//
//        ASTGlobalVar *VerifyA = Module->getGlobalVars().find("a")->getValue();
//        ASTGlobalVar *VerifyB = Module->getNameSpace()->getGlobalVars().find("b")->getValue();
//        ASTGlobalVar *VerifyC = Module->getGlobalVars().find("c")->getValue();
//        ASTGlobalVar *VerifyD = Module->getGlobalVars().find("d")->getValue();
//        ASTGlobalVar *VerifyE = Module->getGlobalVars().find("e")->getValue();
//        ASTGlobalVar *VerifyF = Module->getGlobalVars().find("f")->getValue();
//        ASTGlobalVar *VerifyG = Module->getGlobalVars().find("g")->getValue();
//        ASTGlobalVar *VerifyH = Module->getGlobalVars().find("h")->getValue();
//        ASTGlobalVar *VerifyI = Module->getGlobalVars().find("i")->getValue();
//        ASTGlobalVar *VerifyJ = Module->getGlobalVars().find("j")->getValue();
//
//        EXPECT_EQ(VerifyA->getVisibility(), ASTVisibilityKind::V_PRIVATE);
//        EXPECT_FALSE(VerifyA->isConstant());
//        EXPECT_EQ(((ASTIntegerType *) VerifyA->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
//        EXPECT_EQ(VerifyA->getName(), "a");
//
//        EXPECT_EQ(VerifyB->getVisibility(), ASTVisibilityKind::V_PUBLIC);
//        EXPECT_FALSE(VerifyB->isConstant());
//        EXPECT_EQ(((ASTFloatingPointType *) VerifyB->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
//        EXPECT_EQ(VerifyB->getName(), "b");
//
//        EXPECT_EQ(VerifyC->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyC->isConstant());
//        EXPECT_EQ(VerifyC->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
//        EXPECT_EQ(VerifyC->getName(), "c");
//
//        EXPECT_EQ(VerifyD->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyD->isConstant());
//        EXPECT_EQ(((ASTIntegerType *)VerifyD->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
//        EXPECT_EQ(VerifyD->getName(), "d");
//
//        EXPECT_EQ(VerifyE->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyE->isConstant());
//        EXPECT_EQ(((ASTFloatingPointType *) VerifyE->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
//        EXPECT_EQ(VerifyE->getName(), "e");
//
//        EXPECT_EQ(VerifyF->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyF->isConstant());
//        EXPECT_EQ(((ASTIntegerType *)VerifyF->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(VerifyF->getName(), "f");
//
//        EXPECT_EQ(VerifyG->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyG->isConstant());
//        EXPECT_EQ(((ASTIntegerType *)VerifyG->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
//        EXPECT_EQ(VerifyG->getName(), "g");
//
//        EXPECT_EQ(VerifyH->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyH->isConstant());
//        EXPECT_EQ(((ASTIntegerType *)VerifyH->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
//        EXPECT_EQ(VerifyH->getName(), "h");
//
//        EXPECT_EQ(VerifyI->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyI->isConstant());
//        EXPECT_EQ(((ASTIntegerType *)VerifyI->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
//        EXPECT_EQ(VerifyI->getName(), "i");
//
//        EXPECT_EQ(VerifyJ->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        ASSERT_FALSE(VerifyJ->isConstant());
//        EXPECT_EQ(((ASTIntegerType *)VerifyJ->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
//        EXPECT_EQ(VerifyJ->getName(), "j");
//    }
//
//    TEST_F(ParserTest, GlobalConstants) {
//        llvm::StringRef str = (
//                "private const int a = 1\n"
//                "const public float b = 2.0\n"
//                "const bool c = false\n"
//                );
//        ASTModule *Module = Parse("GlobalConstants", str);
//
//        ASSERT_TRUE(Resolve());

//
//        ASTGlobalVar *VerifyA = Module->getGlobalVars().find("a")->getValue();
//        ASTGlobalVar *VerifyB = Module->getGlobalVars().find("b")->getValue();
//        ASTGlobalVar *VerifyC = Module->getGlobalVars().find("c")->getValue();
//
//        EXPECT_EQ(VerifyA->getVisibility(), ASTVisibilityKind::V_PRIVATE);
//        EXPECT_EQ(VerifyA->isConstant(), true);
//        EXPECT_EQ(VerifyA->getType()->getKind(), ASTTypeKind::TYPE_INTEGER);
//        EXPECT_EQ(VerifyA->getName(), "a");
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)VerifyA->getExpr())->getValue())->getValue(), 1);
//
//        EXPECT_EQ(VerifyB->getVisibility(), ASTVisibilityKind::V_PUBLIC);
//        EXPECT_EQ(VerifyB->isConstant(), true);
//        EXPECT_EQ(VerifyB->getType()->getKind(), ASTTypeKind::TYPE_FLOATING_POINT);
//        EXPECT_EQ(VerifyB->getName(), "b");
//        EXPECT_EQ(((ASTFloatingValue *) ((ASTValueExpr *) VerifyB->getExpr())->getValue())->getValue(), "2.0");
//
//        EXPECT_EQ(VerifyC->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        EXPECT_EQ(VerifyC->isConstant(), true);
//        EXPECT_EQ(VerifyC->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
//        EXPECT_EQ(VerifyC->getName(), "c");
//        EXPECT_EQ(((ASTBoolValue *) ((ASTValueExpr *)VerifyC->getExpr())->getValue())->getValue(), false);
//
//    }
//
//    TEST_F(ParserTest, GlobalArray) {
//        llvm::StringRef str = (
//                               "byte[] a = null\n" // array of zero bytes
//                               "byte[] b = {}\n"
//                               "byte[] c = {1, 2, 3}\n"
//                               "byte[3] d\n" // array of 4 bytes without values
//                               "byte[3] e = {1, 2, 3}\n"
//                               );
//        ASTModule *Module = Parse("GlobalArray", str);
//        ASSERT_TRUE(Resolve());

//
//        ASTGlobalVar *a = Module->getGlobalVars().find("a")->getValue();
//        ASTGlobalVar *b = Module->getGlobalVars().find("b")->getValue();
//        ASTGlobalVar *c = Module->getGlobalVars().find("c")->getValue();
//        ASTGlobalVar *d = Module->getGlobalVars().find("d")->getValue();
//        ASTGlobalVar *e = Module->getGlobalVars().find("e")->getValue();
//
//        // a
//        EXPECT_EQ(a->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
//        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) a->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) a->getType())->getSize())->getValue())->getValue(), 0);
//        EXPECT_EQ(((ASTValueExpr *) a->getExpr())->getValue()->print(), "null");
//
//        // b
//        EXPECT_EQ(b->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
//        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) b->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) b->getType())->getSize())->getValue())->getValue(), 0);
//        EXPECT_NE(b->getExpr(), nullptr);
//        ASTValueExpr *bExpr = (ASTValueExpr *) b->getExpr();
////        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
////        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) bExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
//        EXPECT_EQ(((const ASTArrayValue *) bExpr->getValue())->print(), "zero");
//
//        // c
//        EXPECT_EQ(c->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
//        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) c->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
////        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) c->getType())->getSize())->getValue()).getValue(), 3); // FIXME?
//        EXPECT_NE(c->getExpr(), nullptr);
//        ASTValueExpr *cExpr = (ASTValueExpr *) c->getExpr();
////        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
////        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) cExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
//        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->size(), 3);
//        EXPECT_FALSE(((const ASTArrayValue *) cExpr->getValue())->empty());
//        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->getValues()[0]->print(), "1");
//        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->getValues()[1]->print(), "2");
//        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->getValues()[2]->print(), "3");
//
//        // d
//        EXPECT_EQ(d->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
//        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) d->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) d->getType())->getSize())->getValue())->getValue(), 3);
//        EXPECT_EQ(d->getExpr(), nullptr);
//
//        // e
//        EXPECT_EQ(e->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
//        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) e->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) e->getType())->getSize())->getValue())->getValue(), 3);
//        EXPECT_NE(e->getExpr(), nullptr);
//        ASTValueExpr *eExpr = (ASTValueExpr *) c->getExpr();
////        EXPECT_EQ(((const ASTArrayValue &) eExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
////        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) eExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
//        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->size(), 3);
//        EXPECT_FALSE(((const ASTArrayValue *) eExpr->getValue())->empty());
//        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->getValues()[0]->print(), "1");
//        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->getValues()[1]->print(), "2");
//        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->getValues()[2]->print(), "3");
//
//    }
//
//    TEST_F(ParserTest, GlobalChar) {
//        llvm::StringRef str = (
//               "byte a = ''\n"
//               "byte b = 'b'\n"
//
//               "byte[] c = {'a', 'b', 'c', 0}\n"
//               "byte[2] d = {'', ''}\n" // Empty string
//        );
//        ASTModule *Module = Parse("GlobalChar", str);
//        ASSERT_TRUE(Resolve());

//
//        ASTGlobalVar *a = Module->getGlobalVars().find("a")->getValue();
//        ASTGlobalVar *b = Module->getGlobalVars().find("b")->getValue();
//        ASTGlobalVar *c = Module->getGlobalVars().find("c")->getValue();
//        ASTGlobalVar *d = Module->getGlobalVars().find("d")->getValue();
//
//        // a
//        EXPECT_EQ(((ASTIntegerType *) a->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_NE(a->getExpr(), nullptr);
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)a->getExpr())->getValue())->getValue(), 0);
//
//        // b
//        EXPECT_EQ(((ASTIntegerType *) b->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_NE(b->getExpr(), nullptr);
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)b->getExpr())->getValue())->getValue(), 'b');
//
//        // c
//        EXPECT_EQ(c->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
//        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) c->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
////        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) c->getType())->getSize())->getValue()).getValue(), 4); FIXME?
//        EXPECT_NE(c->getExpr(), nullptr);
//        ASTValueExpr *cExpr = (ASTValueExpr *) c->getExpr();
////        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); FIXME?
////        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) cExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); FIXME?
//        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->size(), 4);
//        EXPECT_FALSE(((const ASTArrayValue *) cExpr->getValue())->empty());
//        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[0])->getValue(), (uint64_t)'a');
//        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[1])->getValue(), 'b');
//        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[2])->getValue(), 'c');
//        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[3])->getValue(), 0);
//
//        // d
//        EXPECT_EQ(d->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
//        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) c->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) d->getType())->getSize())->getValue())->getValue(), 2);
//        EXPECT_NE(d->getExpr(), nullptr);
//        ASTValueExpr *dExpr = (ASTValueExpr *) d->getExpr();
////        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); FIXME?
////        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue *) dExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); FIXME?
//        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue())->size(), 2);
//        EXPECT_FALSE(((const ASTArrayValue *) dExpr->getValue())->empty());
//        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue())->getValues()[0]->print(), "0");
//        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue())->getValues()[1]->print(), "0");
//
//    }
//
//    TEST_F(ParserTest, GlobalString) {
//        llvm::StringRef str = (
//               "string a = \"\"\n" // array of zero bytes
//               "string b = \"abc\"\n" // string abc/0 -> array of 4 bytes
//        );
//        ASTModule *Module = Parse("GlobalString", str);
//        ASSERT_TRUE(Resolve());

//
//        ASTGlobalVar *a = Module->getGlobalVars().find("a")->getValue();
//        ASTGlobalVar *b = Module->getGlobalVars().find("b")->getValue();
//
//        // a
//        EXPECT_EQ(a->getType()->getKind(), ASTTypeKind::TYPE_STRING);
//        EXPECT_NE(a->getExpr(), nullptr);
//        const ASTValue *Val = ((ASTValueExpr *) a->getExpr())->getValue();
//        EXPECT_EQ(((ASTArrayValue *) Val)->getValues().size(), 0);
//
//        // b
//        EXPECT_EQ(b->getType()->getKind(), ASTTypeKind::TYPE_STRING);
//        StringRef Str = ((ASTStringValue *) ((ASTValueExpr *) b->getExpr())->getValue())->getValue();
//        EXPECT_EQ(Str.size(), 3);
//        EXPECT_EQ(Str.data()[0], 'a');
//        EXPECT_EQ(Str.data()[1], 'b');
//        EXPECT_EQ(Str.data()[2], 'c');
//
//    }
//
//    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
//        llvm::StringRef str = ("void func() {}\n");
//        ASTModule *AST = Parse("FunctionDefaultVoidEmpty", str);
//        ASSERT_TRUE(Resolve());

//
//        EXPECT_TRUE(AST->getFunctions().size() == 1); // Func has DEFAULT Visibility
//        EXPECT_TRUE(AST->getNameSpace()->getFunctions().size() == 2); // Default contains also fail() function
//        ASTFunction *VerifyFunc = *AST->getNameSpace()->getFunctions().begin()->getValue().begin()->second.begin();
//        EXPECT_EQ(VerifyFunc->getVisibility(), ASTVisibilityKind::V_DEFAULT);
//        const auto &NSFuncs = AST->getContext().getDefaultNameSpace()->getFunctions();
//        ASSERT_TRUE(NSFuncs.find(VerifyFunc->getName()) != NSFuncs.end());
//    }
//
//    TEST_F(ParserTest, FunctionPrivateReturnParams) {
//        llvm::StringRef str = (
//                "private int func(int a, const float b, bool c=false) {\n"
//                "  return 1"
//                "}\n");
//        ASTModule *Module = Parse("FunctionPrivateReturnParams", str);
//
//        ASSERT_TRUE(Resolve());

//
//        EXPECT_TRUE(Module->getFunctions().size() == 1); // func() has PRIVATE Visibility
//        ASTFunction *VerifyFunc = *Module->getFunctions().begin()->getValue().begin()->second.begin();
//        EXPECT_EQ(VerifyFunc->getVisibility(), ASTVisibilityKind::V_PRIVATE);
//        const auto &NSFuncs = Module->getContext().getDefaultNameSpace()->getFunctions();
//        ASSERT_TRUE(NSFuncs.find(VerifyFunc->getName()) == NSFuncs.end());
//        ASSERT_TRUE(Module->getFunctions().find(VerifyFunc->getName()) != NSFuncs.end());
//
//        ASTParam *Par0 = VerifyFunc->getParams()[0];
//        ASTParam *Par1 = VerifyFunc->getParams()[1];
//        ASTParam *Par2 = VerifyFunc->getParams()[2];
//
//        EXPECT_EQ(Par0->getName(), "a");
//        EXPECT_EQ(Par0->getType()->getKind(), ASTTypeKind::TYPE_INTEGER);
//        EXPECT_EQ(Par0->isConstant(), false);
//
//        EXPECT_EQ(Par1->getName(), "b");
//        EXPECT_EQ(Par1->getType()->getKind(), ASTTypeKind::TYPE_FLOATING_POINT);
//        EXPECT_EQ(Par1->isConstant(), true);
//
//        EXPECT_EQ(Par2->getName(), "c");
//        EXPECT_EQ(Par2->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
//        EXPECT_EQ(Par2->isConstant(), false);
//        EXPECT_EQ(Par2->getDefaultValue()->getTypeKind(), ASTTypeKind::TYPE_BOOL);
//        EXPECT_EQ(((ASTBoolValue *) Par2->getDefaultValue())->getValue(), false);
//        EXPECT_EQ(Par2->getDefaultValue()->print(), "false");
//
//        ASTReturnStmt *Return = (ASTReturnStmt *) VerifyFunc->getBody()->getContent()[0];
//        EXPECT_EQ(((ASTValueExpr *) Return->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
//        EXPECT_EQ(((ASTValueExpr*) Return->getExpr())->getValue()->print(), "1");
//        EXPECT_EQ(Return->getKind(), ASTStmtKind::STMT_RETURN);
//    }
//
//    TEST_F(ParserTest, NullTypeVarReturn) {
//        llvm::StringRef str = ("Type func() {\n"
//                               "  Type t = null"
//                               "  return t\n"
//                               "}\n");
//        ASTModule *Module = Parse("TypeDefaultVarReturn", str);
//
//        ASSERT_TRUE(Resolve());

//
//        // Get Body
//        ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
//        EXPECT_EQ(F->getReturnType()->getKind(), ASTTypeKind::TYPE_IDENTITY);
//        const ASTBlockStmt *Body = F->getBody();
//
//        // Test: Type t
//        ASTVarStmt *varStmt = (ASTVarStmt *) Body->getContent()[0];
//        EXPECT_EQ(varStmt->getVarRef()->getName(), "t");
//        ASTIdentityType *ClassType = (ASTIdentityType *) varStmt->getVarRef()->getDef()->getType();
//        EXPECT_EQ(ClassType->getKind(), ASTTypeKind::TYPE_IDENTITY);
//        EXPECT_EQ(ClassType->getName(), "Type");
//        ASSERT_EQ(((ASTNullValue *)((ASTValueExpr *) varStmt->getExpr())->getValue())->print(), "null");
//
//        const ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[1];
//        ASTVarRefExpr *RetRef = (ASTVarRefExpr *) Ret->getExpr();
//        EXPECT_EQ(RetRef->getVarRef()->getName(), "t");
//
//    }
//
//    TEST_F(ParserTest, UndefLocalVar) {
//        llvm::StringRef str = ("void func() {\n"
//                               "bool a\n"
//                               "byte b\n"
//                               "short c\n"
//                               "ushort d\n"
//                               "int e\n"
//                               "uint f\n"
//                               "long g\n"
//                               "ulong h\n"
//                               "float i\n"
//                               "double j\n"
//                               "Type t\n"
//                               "}\n");
//        ASTModule *Module = Parse("UndefLocalVar", str);
//
//        ASSERT_TRUE(Resolve());

//
//        // Get Body
//        ASTFunction *F = *Module->getFunctions().begin()->getValue().begin()->second.begin();
//        EXPECT_EQ(F->getReturnType()->getKind(), ASTTypeKind::TYPE_VOID);
//        const ASTBlockStmt *Body = F->getBody();
//
//        // Test: bool a
//        ASTVarStmt *aStmt = (ASTVarStmt *) Body->getContent()[0];
//        ASTVar *aVar = aStmt->getVarRef()->getDef();
//        EXPECT_EQ(aVar->getName(), "a");
//        EXPECT_EQ(aVar->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
//        ASSERT_EQ(aStmt->getExpr(), nullptr);
//
//        // Test: byte b
//        ASTVarStmt *bStmt = (ASTVarStmt *) Body->getContent()[1];
//        ASTVar *bVar = bStmt->getVarRef()->getDef();
//        EXPECT_EQ(bVar->getName(), "b");
//        EXPECT_EQ(((ASTIntegerType *) bVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        ASSERT_EQ(bStmt->getExpr(), nullptr);
//
//        // Test: short c
//        ASTVarStmt *cStmt = (ASTVarStmt *) Body->getContent()[2];
//        ASTVar *cVar = cStmt->getVarRef()->getDef();
//        EXPECT_EQ(cVar->getName(), "c");
//        EXPECT_EQ(((ASTIntegerType *) cVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
//        ASSERT_EQ(cStmt->getExpr(), nullptr);
//
//        // Test: ushort d
//        ASTVarStmt *dStmt = (ASTVarStmt *) Body->getContent()[3];
//        ASTVar *dVar = dStmt->getVarRef()->getDef();
//        EXPECT_EQ(dVar->getName(), "d");
//        EXPECT_EQ(((ASTIntegerType *) dVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
//        ASSERT_EQ(dStmt->getExpr(), nullptr);
//
//        // Test: int e
//        ASTVarStmt *eStmt = (ASTVarStmt *) Body->getContent()[4];
//        ASTVar *eVar = eStmt->getVarRef()->getDef();
//        EXPECT_EQ(eVar->getName(), "e");
//        EXPECT_EQ(((ASTIntegerType *) eVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
//        ASSERT_EQ(eStmt->getExpr(), nullptr);
//
//        // Test: uint f
//        ASTVarStmt *fStmt = (ASTVarStmt *) Body->getContent()[5];
//        ASTVar *fVar = fStmt->getVarRef()->getDef();
//        EXPECT_EQ(fVar->getName(), "f");
//        EXPECT_EQ(((ASTIntegerType *) fVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
//        ASSERT_EQ(fStmt->getExpr(), nullptr);
//
//        // Test: long g
//        ASTVarStmt *gStmt = (ASTVarStmt *) Body->getContent()[6];
//        ASTVar *gVar = gStmt->getVarRef()->getDef();
//        EXPECT_EQ(gVar->getName(), "g");
//        EXPECT_EQ(((ASTIntegerType *) gVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
//        ASSERT_EQ(gStmt->getExpr(), nullptr);
//
//        // Test: ulong h
//        ASTVarStmt *hStmt = (ASTVarStmt *) Body->getContent()[7];
//        ASTVar *hVar = hStmt->getVarRef()->getDef();
//        EXPECT_EQ(hVar->getName(), "h");
//        EXPECT_EQ(((ASTIntegerType *) hVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
//        ASSERT_EQ(hStmt->getExpr(), nullptr);
//
//        // Test: float i
//        ASTVarStmt *iStmt = (ASTVarStmt *) Body->getContent()[8];
//        ASTVar *iVar = iStmt->getVarRef()->getDef();
//        EXPECT_EQ(iVar->getName(), "i");
//        EXPECT_EQ(((ASTFloatingPointType *) iVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
//        ASSERT_EQ(iStmt->getExpr(), nullptr);
//
//        // Test: double j
//        ASTVarStmt *jStmt = (ASTVarStmt *) Body->getContent()[9];
//        ASTVar *jVar = jStmt->getVarRef()->getDef();
//        EXPECT_EQ(jVar->getName(), "j");
//        EXPECT_EQ(((ASTFloatingPointType *) jVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
//        ASSERT_EQ(jStmt->getExpr(), nullptr);
//
//        // Test: Type t
//        ASTVarStmt *tStmt = (ASTVarStmt *) Body->getContent()[10];
//        ASTVar *tVar = tStmt->getVarRef()->getDef();
//        EXPECT_EQ(tVar->getName(), "t");
//        EXPECT_EQ(((ASTIdentityType *) tVar->getType())->getIdentityTypeKind(), ASTIdentityTypeKind::TYPE_NONE);
//        ASSERT_EQ(tStmt->getExpr(), nullptr);
//
//    }
//
//    TEST_F(ParserTest, FunctionCall) {
//        llvm::StringRef str = ("private int doSome() {return 1}\n"
//                               "public void doOther(int a, int b) {}\n"
//                               "int doNow() {return 0}"
//                               "int main(int a) {\n"
//                               "  int b = doSome()\n"
//                               "  b = doNow()\n"
//                               "  doOther(a, 1)\n"
//                               "  return b\n"
//                               "}\n");
//        ASTModule *Module = Parse("FunctionCall", str);
//
//        ASSERT_TRUE(Resolve());

//
//        // Get all functions
//        ASTFunction *doSome = *Module->getFunctions().find("doSome")->getValue().begin()->second.begin();
//        ASTFunction *doOther = *Module->getFunctions().find("doOther")->getValue().begin()->second.begin();
//        ASTFunction *doNow = *Module->getFunctions().find("doNow")->getValue().begin()->second.begin();
//        ASTFunction *main = *Module->getFunctions().find("main")->getValue().begin()->second.begin();
//
//        ASSERT_TRUE(doSome != nullptr);
//        ASSERT_TRUE(doOther != nullptr);
//        ASSERT_TRUE(main != nullptr);
//
//        // Get main() Body
//        const ASTBlockStmt *Body = main->getBody();
//
//        // Test: doSome()
//        ASTVarStmt *VarB = ((ASTVarStmt *) Body->getContent()[0]);
//        ASTCallExpr *doSomeCall = (ASTCallExpr *) VarB->getExpr();
//        EXPECT_EQ(doSomeCall->getCall()->getName(), "doSome");
//        EXPECT_EQ(doSomeCall->getExprKind(), ASTExprKind::EXPR_CALL);
//        ASSERT_FALSE(doSomeCall->getCall()->getDef() == nullptr);
//
//        // Test: doNow()
//        ASTVarStmt *B = ((ASTVarStmt *) Body->getContent()[1]);
//        ASTCallExpr *doNowCall = (ASTCallExpr *) B->getExpr();
//        EXPECT_EQ(doNowCall->getCall()->getName(), "doNow");
//        EXPECT_EQ(doNowCall->getExprKind(), ASTExprKind::EXPR_CALL);
//        ASSERT_FALSE(doNowCall->getCall()->getDef() == nullptr);
//
//        // Test: doOther(a, b)
//        ASTExprStmt *doOtherStmt = (ASTExprStmt *) Body->getContent()[2];
//        EXPECT_EQ(doOtherStmt->getKind(), ASTStmtKind::STMT_EXPR);
//        ASTCallExpr *doOtherCall = (ASTCallExpr *) doOtherStmt->getExpr();
//        EXPECT_EQ(doOtherCall->getCall()->getName(), "doOther");
//        ASTArg *Arg0 = doOtherCall->getCall()->getArgs()[0];
//        EXPECT_EQ(((ASTVarRefExpr *) Arg0->getExpr())->getVarRef()->getName(), "a");
//        ASTArg *Arg1 = doOtherCall->getCall()->getArgs()[1];
//        EXPECT_EQ(((ASTValueExpr *) Arg1->getExpr())->getValue()->print(), "1");
//
//        // return do()
//        ASTReturnStmt *Ret = (ASTReturnStmt *) Body->getContent()[3];
//        ASTVarRefExpr *RetExpr = (ASTVarRefExpr *) Ret->getExpr();
//        EXPECT_EQ(RetExpr->getVarRef()->getName(), "b");
//        EXPECT_FALSE(RetExpr->getVarRef()->getDef() == nullptr);
//    }
//
//    TEST_F(ParserTest, FunctionHandleFail) {
//        llvm::StringRef str = (
//                               "void err0() {\n"
//                               "  fail\n"
//                               "}\n"
//                               "int err1() {\n"
//                               "  fail 404\n"
//                               "}\n"
//                               "int err2() {\n"
//                               "  fail \"Error\"\n"
//                               "}\n"
//                               "void main() {\n"
//                               "  handle err0()\n"
//                               "  bool err0 = handle err0()\n"
//                               "  error err0 = handle err0()\n"
//                               "  int err1 = handle { err1() }\n"
//                               "  error err1 = handle err1()\n"
//                               "  string err2 = handle err2()\n"
//                               "  error err2 = handle { err2() }\n"
//                               "}\n");
//        ASTModule *Module = Parse("FunctionFail", str);
//        ASSERT_TRUE(Resolve());

//
//        // Get all functions
//        ASTFunction *err0 = *Module->getFunctions().find("err0")->getValue().begin()->second.begin();
//        ASTFunction *err1 = *Module->getFunctions().find("err1")->getValue().begin()->second.begin();
//        ASTFunction *err2 = *Module->getFunctions().find("err2")->getValue().begin()->second.begin();
//        ASTFunction *main = *Module->getFunctions().find("main")->getValue().begin()->second.begin();
//
//        ASSERT_TRUE(err0 != nullptr);
//        ASSERT_TRUE(err1 != nullptr);
//        ASSERT_TRUE(err2 != nullptr);
//        ASSERT_TRUE(main != nullptr);
//
//        ASTExprStmt *Stmt1 = (ASTExprStmt *) err0->getBody()->getContent()[0];
//        ASSERT_TRUE(((ASTCallExpr *) Stmt1->getExpr())->getCall()->getArgs().empty());
//
//        ASTExprStmt *Stmt2 = (ASTExprStmt *) err1->getBody()->getContent()[0];
//        ASTArg *Arg2 = ((ASTCallExpr *) Stmt2->getExpr())->getCall()->getArgs()[0];
//        ASTValue *Val2 = ((ASTValueExpr *) Arg2->getExpr())->getValue();
//        ASSERT_EQ(Val2->getTypeKind(), ASTTypeKind::TYPE_INTEGER);
//        ASSERT_EQ(((ASTIntegerValue *) Val2)->getValue(), 404);
//
//        ASTExprStmt *Stmt3 = (ASTExprStmt *) err2->getBody()->getContent()[0];
//        ASTArg *Arg3 = ((ASTCallExpr *) Stmt3->getExpr())->getCall()->getArgs()[0];
//        ASTValue *Val3 = ((ASTValueExpr *) Arg3->getExpr())->getValue();
//        ASSERT_EQ(Val3->getTypeKind(), ASTTypeKind::TYPE_STRING);
//        ASSERT_EQ(((ASTStringValue *) Val3)->getValue(), "Error");
//
//        // Get main() Body
//        ASTHandleStmt *HandleStmt = (ASTHandleStmt *) main->getBody()->getContent()[0];
//
//        ASTVarStmt *bool_err0 = (ASTVarStmt *) main->getBody()->getContent()[1];
//        ASSERT_TRUE(((ASTVarRefExpr *) bool_err0->getExpr())->getVarRef()->getDef()->getType()->isBool());
//        ASTVarStmt *error_err0 = (ASTVarStmt *) main->getBody()->getContent()[2];
//        ASSERT_TRUE(((ASTVarRefExpr *) error_err0->getExpr())->getVarRef()->getDef()->getType()->isError());
//
//        ASTVarStmt *int_err1 = (ASTVarStmt *) main->getBody()->getContent()[3];
//        ASSERT_TRUE(((ASTVarRefExpr *) int_err1->getExpr())->getVarRef()->getDef()->getType()->isInteger());
//        ASTVarStmt *error_err1 = (ASTVarStmt *) main->getBody()->getContent()[4];
//        ASSERT_TRUE(((ASTVarRefExpr *) error_err1->getExpr())->getVarRef()->getDef()->getType()->isError());
//
//        ASTVarStmt *string_err2 = (ASTVarStmt *) main->getBody()->getContent()[5];
//        ASSERT_TRUE(((ASTVarRefExpr *) string_err2->getExpr())->getVarRef()->getDef()->getType()->isString());
//        ASTVarStmt *error_err2 = (ASTVarStmt *) main->getBody()->getContent()[6];
//        ASSERT_TRUE(((ASTVarRefExpr *) error_err2->getExpr())->getVarRef()->getDef()->getType()->isError());
//    }
}