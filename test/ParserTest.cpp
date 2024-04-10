//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "Frontend/FrontendAction.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTImport.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarDefine.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParams.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTError.h"

#include "llvm/ADT/StringMap.h"

namespace {

    using namespace fly;

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
        ASTNode *Node1 = Parse("standard.fly", str1, false);

        llvm::StringRef str2 = ("import standard std");
        ASTNode *Node2 = Parse("default.fly", str2);

        ASSERT_TRUE(isSuccess());

        EXPECT_EQ(Node2->getNameSpace()->getName(), "default");
        ASTImport *Import = Node2->getImports().lookup("standard");
        EXPECT_EQ(Import->getName(), "standard");
        EXPECT_EQ(Import->getAlias(), "std");
        ASTImport *ImportAlias = Node2->getAliasImports().lookup("std");
        EXPECT_EQ(ImportAlias->getName(), "standard");
        EXPECT_EQ(ImportAlias->getAlias(), "std");
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
        EXPECT_EQ(((ASTIntegerType *) VerifyA->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");

        EXPECT_EQ(VerifyB->getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_FALSE(VerifyB->getScopes()->isConstant());
        EXPECT_EQ(((ASTFloatingPointType *) VerifyB->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");

        EXPECT_EQ(VerifyC->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyC->getScopes()->isConstant());
        EXPECT_EQ(VerifyC->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");

        EXPECT_EQ(VerifyD->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyD->getScopes()->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VerifyD->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
        EXPECT_EQ(VerifyD->getName(), "d");

        EXPECT_EQ(VerifyE->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyE->getScopes()->isConstant());
        EXPECT_EQ(((ASTFloatingPointType *) VerifyE->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
        EXPECT_EQ(VerifyE->getName(), "e");

        EXPECT_EQ(VerifyF->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyF->getScopes()->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VerifyF->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(VerifyF->getName(), "f");

        EXPECT_EQ(VerifyG->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyG->getScopes()->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VerifyG->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
        EXPECT_EQ(VerifyG->getName(), "g");

        EXPECT_EQ(VerifyH->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyH->getScopes()->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VerifyH->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
        EXPECT_EQ(VerifyH->getName(), "h");

        EXPECT_EQ(VerifyI->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyI->getScopes()->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VerifyI->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
        EXPECT_EQ(VerifyI->getName(), "i");

        EXPECT_EQ(VerifyJ->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyJ->getScopes()->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VerifyJ->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
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
        EXPECT_EQ(VerifyA->getType()->getKind(), ASTTypeKind::TYPE_INTEGER);
        EXPECT_EQ(VerifyA->getName(), "a");
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)VerifyA->getExpr())->getValue())->getValue(), 1);

        EXPECT_EQ(VerifyB->getScopes()->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_EQ(VerifyB->getScopes()->isConstant(), true);
        EXPECT_EQ(VerifyB->getType()->getKind(), ASTTypeKind::TYPE_FLOATING_POINT);
        EXPECT_EQ(VerifyB->getName(), "b");
        EXPECT_EQ(((ASTFloatingValue *) ((ASTValueExpr *) VerifyB->getExpr())->getValue())->getValue(), "2.0");

        EXPECT_EQ(VerifyC->getScopes()->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        EXPECT_EQ(VerifyC->getScopes()->isConstant(), true);
        EXPECT_EQ(VerifyC->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");
        EXPECT_EQ(((ASTBoolValue *) ((ASTValueExpr *)VerifyC->getExpr())->getValue())->getValue(), false);

    }

    TEST_F(ParserTest, GlobalArray) {
        llvm::StringRef str = (
                               "byte[] a = null\n" // array of zero bytes
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
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) a->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) a->getType())->getSize())->getValue())->getValue(), 0);
        EXPECT_EQ(((ASTValueExpr *) a->getExpr())->getValue()->print(), "null");

        // b
        EXPECT_EQ(b->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) b->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) b->getType())->getSize())->getValue())->getValue(), 0);
        EXPECT_NE(b->getExpr(), nullptr);
        ASTValueExpr *bExpr = (ASTValueExpr *) b->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) bExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) bExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
        EXPECT_EQ(((const ASTArrayValue *) bExpr->getValue())->print(), "zero");

        // c
        EXPECT_EQ(c->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) c->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) c->getType())->getSize())->getValue()).getValue(), 3); // FIXME?
        EXPECT_NE(c->getExpr(), nullptr);
        ASTValueExpr *cExpr = (ASTValueExpr *) c->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) cExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->size(), 3);
        EXPECT_FALSE(((const ASTArrayValue *) cExpr->getValue())->empty());
        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->getValues()[0]->print(), "1");
        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->getValues()[1]->print(), "2");
        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->getValues()[2]->print(), "3");

        // d
        EXPECT_EQ(d->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) d->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) d->getType())->getSize())->getValue())->getValue(), 3);
        EXPECT_EQ(d->getExpr(), nullptr);

        // e
        EXPECT_EQ(e->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) e->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) e->getType())->getSize())->getValue())->getValue(), 3);
        EXPECT_NE(e->getExpr(), nullptr);
        ASTValueExpr *eExpr = (ASTValueExpr *) c->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) eExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); // FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) eExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); // FIXME?
        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->size(), 3);
        EXPECT_FALSE(((const ASTArrayValue *) eExpr->getValue())->empty());
        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->getValues()[0]->print(), "1");
        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->getValues()[1]->print(), "2");
        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->getValues()[2]->print(), "3");

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
        EXPECT_EQ(((ASTIntegerType *) a->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_NE(a->getExpr(), nullptr);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)a->getExpr())->getValue())->getValue(), 0);

        // b
        EXPECT_EQ(((ASTIntegerType *) b->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_NE(b->getExpr(), nullptr);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)b->getExpr())->getValue())->getValue(), 'b');

        // c
        EXPECT_EQ(c->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) c->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
//        EXPECT_EQ(((ASTIntegerValue &) ((ASTValueExpr *)((ASTArrayType *) c->getType())->getSize())->getValue()).getValue(), 4); FIXME?
        EXPECT_NE(c->getExpr(), nullptr);
        ASTValueExpr *cExpr = (ASTValueExpr *) c->getExpr();
//        EXPECT_EQ(((const ASTArrayValue &) cExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue &) cExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); FIXME?
        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->size(), 4);
        EXPECT_FALSE(((const ASTArrayValue *) cExpr->getValue())->empty());
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[0])->getValue(), (uint64_t)'a');
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[1])->getValue(), 'b');
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[2])->getValue(), 'c');
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[3])->getValue(), 0);

        // d
        EXPECT_EQ(d->getType()->getKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) c->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) d->getType())->getSize())->getValue())->getValue(), 2);
        EXPECT_NE(d->getExpr(), nullptr);
        ASTValueExpr *dExpr = (ASTValueExpr *) d->getExpr();
//        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue()).getType()->getKind(), TypeKind::TYPE_ARRAY); FIXME?
//        EXPECT_EQ(((ASTArrayType *) ((const ASTArrayValue *) dExpr->getValue()).getType())->getType()->getKind(), TypeKind::TYPE_BYTE); FIXME?
        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue())->size(), 2);
        EXPECT_FALSE(((const ASTArrayValue *) dExpr->getValue())->empty());
        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue())->getValues()[0]->print(), "0");
        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue())->getValues()[1]->print(), "0");

    }

    TEST_F(ParserTest, GlobalString) {
        llvm::StringRef str = (
               "string a = \"\"\n" // array of zero bytes
               "string b = \"abc\"\n" // string abc/0 -> array of 4 bytes
        );
        ASTNode *Node = Parse("GlobalString", str);
        ASSERT_TRUE(isSuccess());

        ASTGlobalVar *a = Node->getGlobalVars().find("a")->getValue();
        ASTGlobalVar *b = Node->getGlobalVars().find("b")->getValue();

        // a
        EXPECT_EQ(a->getType()->getKind(), ASTTypeKind::TYPE_STRING);
        EXPECT_NE(a->getExpr(), nullptr);
        const ASTValue *Val = ((ASTValueExpr *) a->getExpr())->getValue();
        EXPECT_EQ(((ASTArrayValue *) Val)->getValues().size(), 0);

        // b
        EXPECT_EQ(b->getType()->getKind(), ASTTypeKind::TYPE_STRING);
        StringRef Str = ((ASTStringValue *) ((ASTValueExpr *) b->getExpr())->getValue())->getValue();
        EXPECT_EQ(Str.size(), 3);
        EXPECT_EQ(Str.data()[0], 'a');
        EXPECT_EQ(Str.data()[1], 'b');
        EXPECT_EQ(Str.data()[2], 'c');

    }

    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
        llvm::StringRef str = ("void func() {}\n");
        ASTNode *AST = Parse("FunctionDefaultVoidEmpty", str);
        ASSERT_TRUE(isSuccess());

        EXPECT_TRUE(AST->getFunctions().size() == 1); // Func has DEFAULT Visibility
        EXPECT_TRUE(AST->getNameSpace()->getFunctions().size() == 2); // Default contains also fail() function
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
        EXPECT_EQ(Par0->getType()->getKind(), ASTTypeKind::TYPE_INTEGER);
        EXPECT_EQ(Par0->isConstant(), false);

        EXPECT_EQ(Par1->getName(), "b");
        EXPECT_EQ(Par1->getType()->getKind(), ASTTypeKind::TYPE_FLOATING_POINT);
        EXPECT_EQ(Par1->isConstant(), true);

        EXPECT_EQ(Par2->getName(), "c");
        EXPECT_EQ(Par2->getType()->getKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(Par2->isConstant(), false);
        EXPECT_EQ(Par2->getExpr()->getExprKind(), ASTExprKind::EXPR_VALUE);
        ASTValueExpr *DefArg2 = (ASTValueExpr *)Par2->getExpr();
        EXPECT_EQ(((ASTBoolValue *) DefArg2->getValue())->getValue(), false);
        EXPECT_EQ(DefArg2->getValue()->print(), "false");

        ASTReturn *Return = (ASTReturn *) VerifyFunc->getBody()->getContent()[0];
        EXPECT_EQ(((ASTValueExpr *) Return->getExpr())->getExprKind(), ASTExprKind::EXPR_VALUE);
        EXPECT_EQ(((ASTValueExpr*) Return->getExpr())->getValue()->print(), "1");
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
        ASTFunction *F = *Node->getFunctions().begin()->getValue().begin()->second.begin();
        EXPECT_EQ(F->getType()->getKind(), ASTTypeKind::TYPE_IDENTITY);
        const ASTBlock *Body = F->getBody();

        // Test: Type t
        ASTLocalVar *typeVar = (ASTLocalVar *) Body->getContent()[0];
        EXPECT_EQ(typeVar->getName(), "t");
        EXPECT_EQ(typeVar->getType()->getKind(), ASTTypeKind::TYPE_IDENTITY);
        ASTClassType *ClassType = (ASTClassType *) typeVar->getType();
        EXPECT_EQ(ClassType->getName(), "Type");
        ASSERT_EQ(((ASTNullValue *)((ASTValueExpr *) typeVar->getExpr())->getValue())->print(), "null");

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
        ASTFunction *F = *Node->getFunctions().begin()->getValue().begin()->second.begin();
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
        EXPECT_EQ(((ASTIntegerType *) bVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        ASSERT_EQ(bVar->getExpr(), nullptr);

        // Test: short c
        ASTLocalVar *cVar = (ASTLocalVar *) Body->getContent()[2];
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(((ASTIntegerType *) cVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
        ASSERT_EQ(cVar->getExpr(), nullptr);

        // Test: ushort d
        ASTLocalVar *dVar = (ASTLocalVar *) Body->getContent()[3];
        EXPECT_EQ(dVar->getName(), "d");
        EXPECT_EQ(((ASTIntegerType *) dVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
        ASSERT_EQ(dVar->getExpr(), nullptr);

        // Test: int e
        ASTLocalVar *eVar = (ASTLocalVar *) Body->getContent()[4];
        EXPECT_EQ(eVar->getName(), "e");
        EXPECT_EQ(((ASTIntegerType *) eVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        ASSERT_EQ(eVar->getExpr(), nullptr);

        // Test: uint f
        ASTLocalVar *fVar = (ASTLocalVar *) Body->getContent()[5];
        EXPECT_EQ(fVar->getName(), "f");
        EXPECT_EQ(((ASTIntegerType *) fVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
        ASSERT_EQ(fVar->getExpr(), nullptr);

        // Test: long g
        ASTLocalVar *gVar = (ASTLocalVar *) Body->getContent()[6];
        EXPECT_EQ(gVar->getName(), "g");
        EXPECT_EQ(((ASTIntegerType *) gVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
        ASSERT_EQ(gVar->getExpr(), nullptr);

        // Test: ulong h
        ASTLocalVar *hVar = (ASTLocalVar *) Body->getContent()[7];
        EXPECT_EQ(hVar->getName(), "h");
        EXPECT_EQ(((ASTIntegerType *) hVar->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
        ASSERT_EQ(hVar->getExpr(), nullptr);

        // Test: float i
        ASTLocalVar *iVar = (ASTLocalVar *) Body->getContent()[8];
        EXPECT_EQ(iVar->getName(), "i");
        EXPECT_EQ(((ASTFloatingPointType *) iVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
        ASSERT_EQ(iVar->getExpr(), nullptr);

        // Test: double j
        ASTLocalVar *jVar = (ASTLocalVar *) Body->getContent()[9];
        EXPECT_EQ(jVar->getName(), "j");
        EXPECT_EQ(((ASTFloatingPointType *) jVar->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
        ASSERT_EQ(jVar->getExpr(), nullptr);

        // Test: Type t
        ASTLocalVar *tVar = (ASTLocalVar *) Body->getContent()[10];
        EXPECT_EQ(tVar->getName(), "t");
        EXPECT_EQ(((ASTIdentityType *) tVar->getType())->getIdentityKind(), ASTIdentityTypeKind::TYPE_NONE);
        ASSERT_EQ(tVar->getExpr(), nullptr);

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
        ASTVarDefine *B = ((ASTVarDefine *) Body->getContent()[1]);
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
        EXPECT_EQ(((ASTValueExpr *) Arg1->getExpr())->getValue()->print(), "1");

        // return do()
        ASTReturn *Ret = (ASTReturn *) Body->getContent()[3];
        ASTVarRefExpr *RetExpr = (ASTVarRefExpr *) Ret->getExpr();
        EXPECT_EQ(RetExpr->getVarRef()->getName(), "b");
        EXPECT_FALSE(RetExpr->getVarRef()->getDef() == nullptr);
    }

    TEST_F(ParserTest, FunctionHandleFail) {
        llvm::StringRef str = (
                               "void err0() {\n"
                               "  fail()\n"
                               "}\n"
                               "int err1() {\n"
                               "  fail(404)"
                               "}\n"
                               "int err2() {\n"
                               "  fail(\"Error\")"
                               "}\n"
                               "void main() {\n"
                               "  handle err0()\n"
                               "  bool err0 = handle err0()\n"
                               "  error err0 = handle err0()\n"
                               "  int err1 = handle { err1() }\n"
                               "  error err1 = handle err1()\n"
                               "  string err2 = handle err2()\n"
                               "  error err2 = handle { err2() }\n"
                               "}\n");
        ASTNode *Node = Parse("FunctionFail", str);
        ASSERT_TRUE(isSuccess());

        // Get all functions
        ASTFunction *err0 = *Node->getFunctions().find("err0")->getValue().begin()->second.begin();
        ASTFunction *err1 = *Node->getFunctions().find("err1")->getValue().begin()->second.begin();
        ASTFunction *err2 = *Node->getFunctions().find("err2")->getValue().begin()->second.begin();
        ASTFunction *main = *Node->getFunctions().find("main")->getValue().begin()->second.begin();

        ASSERT_TRUE(err0 != nullptr);
        ASSERT_TRUE(err1 != nullptr);
        ASSERT_TRUE(err2 != nullptr);
        ASSERT_TRUE(main != nullptr);

        ASTExprStmt *Stmt1 = (ASTExprStmt *) err0->getBody()->getContent()[0];
        ASSERT_TRUE(((ASTCallExpr *) Stmt1->getExpr())->getCall()->getArgs().empty());

        ASTExprStmt *Stmt2 = (ASTExprStmt *) err1->getBody()->getContent()[0];
        ASTArg *Arg2 = ((ASTCallExpr *) Stmt2->getExpr())->getCall()->getArgs()[0];
        ASTValue *Val2 = ((ASTValueExpr *) Arg2->getExpr())->getValue();
        ASSERT_EQ(Val2->getTypeKind(), ASTTypeKind::TYPE_INTEGER);
        ASSERT_EQ(((ASTIntegerValue *) Val2)->getValue(), 404);

        ASTExprStmt *Stmt3 = (ASTExprStmt *) err2->getBody()->getContent()[0];
        ASTArg *Arg3 = ((ASTCallExpr *) Stmt3->getExpr())->getCall()->getArgs()[0];
        ASTValue *Val3 = ((ASTValueExpr *) Arg3->getExpr())->getValue();
        ASSERT_EQ(Val3->getTypeKind(), ASTTypeKind::TYPE_STRING);
        ASSERT_EQ(((ASTStringValue *) Val3)->getValue(), "Error");

        // Get main() Body
        ASTHandleBlock *HandleStmt = (ASTHandleBlock *) main->getBody()->getContent()[0];

        ASTLocalVar *bool_err0 = (ASTLocalVar *) main->getBody()->getContent()[1];
        ASSERT_TRUE(((ASTVarRefExpr *) bool_err0->getExpr())->getVarRef()->getDef()->getType()->isBool());
        ASTLocalVar *error_err0 = (ASTLocalVar *) main->getBody()->getContent()[2];
        ASSERT_TRUE(((ASTVarRefExpr *) error_err0->getExpr())->getVarRef()->getDef()->getType()->isError());

        ASTLocalVar *int_err1 = (ASTLocalVar *) main->getBody()->getContent()[3];
        ASSERT_TRUE(((ASTVarRefExpr *) int_err1->getExpr())->getVarRef()->getDef()->getType()->isInteger());
        ASTLocalVar *error_err1 = (ASTLocalVar *) main->getBody()->getContent()[4];
        ASSERT_TRUE(((ASTVarRefExpr *) error_err1->getExpr())->getVarRef()->getDef()->getType()->isError());

        ASTLocalVar *string_err2 = (ASTLocalVar *) main->getBody()->getContent()[5];
        ASSERT_TRUE(((ASTVarRefExpr *) string_err2->getExpr())->getVarRef()->getDef()->getType()->isString());
        ASTLocalVar *error_err2 = (ASTLocalVar *) main->getBody()->getContent()[6];
        ASSERT_TRUE(((ASTVarRefExpr *) error_err2->getExpr())->getVarRef()->getDef()->getType()->isError());
    }
}