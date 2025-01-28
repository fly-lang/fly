//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "AST/ASTModule.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, GlobalVars) {
        llvm::StringRef str = ("private int a\n"
                                "public float b\n"
                                "bool c\n"
                                "const long d\n"
                                "double e\n"
                                "byte f\n"
                                "ushort g\n"
                                "short h\n"
                                "uint i\n"
                                "ulong j\n"
                                 );
        ASTModule *Module = Parse("GlobalVars", str);

        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarA = nullptr;
        ASTGlobalVar *VarB = nullptr;
        ASTGlobalVar *VarC = nullptr;
        ASTGlobalVar *VarD = nullptr;
        ASTGlobalVar *VarE = nullptr;
        ASTGlobalVar *VarF = nullptr;
        ASTGlobalVar *VarG = nullptr;
        ASTGlobalVar *VarH = nullptr;
        ASTGlobalVar *VarI = nullptr;
        ASTGlobalVar *VarJ = nullptr;

        for (ASTGlobalVar *GV : Module->getGlobalVars()) {
            if (GV->getName() == "a") {
                VarA = GV;
            } else if (GV->getName() == "b") {
                VarB = GV;
            } else if (GV->getName() == "c") {
                VarC = GV;
            } else if (GV->getName() == "d") {
                VarD = GV;
            } else if (GV->getName() == "e") {
                VarE = GV;
            } else if (GV->getName() == "f") {
                VarF = GV;
            } else if (GV->getName() == "g") {
                VarG = GV;
            } else if (GV->getName() == "h") {
                VarH = GV;
            } else if (GV->getName() == "i") {
                VarI = GV;
            } else if (GV->getName() == "j") {
                VarJ = GV;
            }
        }

        EXPECT_EQ(VarA->getVisibility(), ASTVisibilityKind::V_PRIVATE);
        EXPECT_FALSE(VarA->isConstant());
        EXPECT_EQ(((ASTIntegerType *) VarA->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_INT);
        EXPECT_EQ(VarA->getName(), "a");

        EXPECT_EQ(VarB->getVisibility(), ASTVisibilityKind::V_PUBLIC);
        EXPECT_FALSE(VarB->isConstant());
        EXPECT_EQ(((ASTFloatingPointType *) VarB->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_FLOAT);
        EXPECT_EQ(VarB->getName(), "b");

        EXPECT_EQ(VarC->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VarC->isConstant());
        EXPECT_EQ(VarC->getType()->getStmtKind(), ASTTypeKind::TYPE_BOOL);
        EXPECT_EQ(VarC->getName(), "c");

        EXPECT_EQ(VarD->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_TRUE(VarD->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VarD->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_LONG);
        EXPECT_EQ(VarD->getName(), "d");

        EXPECT_EQ(VarE->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VarE->isConstant());
        EXPECT_EQ(((ASTFloatingPointType *) VarE->getType())->getFloatingPointKind(), ASTFloatingPointTypeKind::TYPE_DOUBLE);
        EXPECT_EQ(VarE->getName(), "e");

        EXPECT_EQ(VarF->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VarF->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VarF->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(VarF->getName(), "f");

        EXPECT_EQ(VarG->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VarG->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VarG->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_USHORT);
        EXPECT_EQ(VarG->getName(), "g");

        EXPECT_EQ(VarH->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VarH->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VarH->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_SHORT);
        EXPECT_EQ(VarH->getName(), "h");

        EXPECT_EQ(VarI->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VarI->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VarI->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_UINT);
        EXPECT_EQ(VarI->getName(), "i");

        EXPECT_EQ(VarJ->getVisibility(), ASTVisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VarJ->isConstant());
        EXPECT_EQ(((ASTIntegerType *)VarJ->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_ULONG);
        EXPECT_EQ(VarJ->getName(), "j");
    }

    TEST_F(ParserTest, GlobalArrayNull) {
        llvm::StringRef str = (
                               "byte[] a = null\n" // array of zero bytes
                               );
        ASTModule *Module = Parse("GlobalArrayNull", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarA = *Module->getGlobalVars().begin();

        // a
        EXPECT_EQ(VarA->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) VarA->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ((ASTValueExpr *)((ASTArrayType *) VarA->getType())->getSize(), nullptr);
        EXPECT_NE(((ASTNullValue *) ((ASTValueExpr *) VarA->getExpr())->getValue()), nullptr);
    }

    TEST_F(ParserTest, GlobalArrayEmpty) {
        llvm::StringRef str = (
                "byte[] b = {}\n" // empty array
        );
        ASTModule *Module = Parse("GlobalArrayEmpty", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarB = *Module->getGlobalVars().begin();

        // b
        EXPECT_EQ(VarB->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) VarB->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ((ASTValueExpr *)((ASTArrayType *) VarB->getType())->getSize(), nullptr);
        EXPECT_NE(VarB->getExpr(), nullptr);
        ASTValueExpr *bExpr = (ASTValueExpr *) VarB->getExpr();
        EXPECT_EQ(((const ASTArrayValue *) bExpr->getValue())->size(), 0);
    }

    TEST_F(ParserTest, GlobalArraySet) {
        llvm::StringRef str = (
                "byte[] c = {1, 2, 3}\n"
        );
        ASTModule *Module = Parse("GlobalArraySet", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarC = *Module->getGlobalVars().begin();

        // c
        EXPECT_EQ(VarC->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) VarC->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ((ASTValueExpr *)((ASTArrayType *) VarC->getType())->getSize(), nullptr);
        EXPECT_NE(VarC->getExpr(), nullptr);
        ASTValueExpr *cExpr = (ASTValueExpr *) VarC->getExpr();
        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->size(), 3);
        EXPECT_FALSE(((const ASTArrayValue *) cExpr->getValue())->empty());
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[0])->getValue(), "1");
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[1])->getValue(), "2");
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[2])->getValue(), "3");
    }

    TEST_F(ParserTest, GlobalArrayInit) {
        llvm::StringRef str = (
                "byte[3] d\n" // array of 4 bytes without values
        );
        ASTModule *Module = Parse("GlobalArrayInit", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarD = *Module->getGlobalVars().begin();

        // d
        EXPECT_EQ(VarD->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) VarD->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) VarD->getType())->getSize())->getValue())->getValue(), "3");
        EXPECT_EQ(VarD->getExpr(), nullptr);
    }

    TEST_F(ParserTest, GlobalArrayInitSet) {
        llvm::StringRef str = (
                "byte[3] e = {1, 2, 3}\n"
        );
        ASTModule *Module = Parse("GlobalArrayInitSet", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarE = *Module->getGlobalVars().begin();

        // e
        EXPECT_EQ(VarE->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTIntegerType *) ((ASTArrayType *) VarE->getType())->getType())->getIntegerKind(), ASTIntegerTypeKind::TYPE_BYTE);
        EXPECT_EQ(((ASTIntegerValue *) ((ASTValueExpr *)((ASTArrayType *) VarE->getType())->getSize())->getValue())->getValue(), "3");
        EXPECT_NE(VarE->getExpr(), nullptr);
        ASTValueExpr *eExpr = (ASTValueExpr *) VarE->getExpr();
        EXPECT_EQ(((const ASTArrayValue *) eExpr->getValue())->size(), 3);
        EXPECT_FALSE(((const ASTArrayValue *) eExpr->getValue())->empty());
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) eExpr->getValue())->getValues()[0])->getValue(), "1");
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) eExpr->getValue())->getValues()[1])->getValue(), "2");
        EXPECT_EQ(((ASTIntegerValue *) ((const ASTArrayValue *) eExpr->getValue())->getValues()[2])->getValue(), "3");
    }

    TEST_F(ParserTest, GlobalChar) {
        llvm::StringRef str = (
               "char a = ''\n"
               "char b = 'b'\n"
        );
        ASTModule *Module = Parse("GlobalChar", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarA = nullptr;
        ASTGlobalVar *VarB = nullptr;
        for (ASTGlobalVar *GV : Module->getGlobalVars()) {
            if (GV->getName() == "a") {
                VarA = GV;
            } else if (GV->getName() == "b") {
                VarB = GV;
            }
        }

        // a
        EXPECT_EQ(VarA->getType()->getStmtKind(), ASTTypeKind::TYPE_CHAR);
        EXPECT_NE(VarA->getExpr(), nullptr);
        EXPECT_EQ(((ASTCharValue *) ((ASTValueExpr *)VarA->getExpr())->getValue())->getValue(), "");

        // b
        EXPECT_EQ(VarB->getType()->getStmtKind(), ASTTypeKind::TYPE_CHAR);
        EXPECT_NE(VarB->getExpr(), nullptr);
        EXPECT_EQ(((ASTCharValue *) ((ASTValueExpr *)VarB->getExpr())->getValue())->getValue(), "b");
    }

    TEST_F(ParserTest, GlobalCharArray) {
        llvm::StringRef str = (
                "char[] c = {'a', 'b', 'c', ''}\n"
                "char[2] d = {'', ''}\n" // Empty string
        );
        ASTModule *Module = Parse("GlobalCharArray", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarC = nullptr;
        ASTGlobalVar *VarD = nullptr;

        for (ASTGlobalVar *GV : Module->getGlobalVars()) {
            if (GV->getName() == "c") {
                VarC = GV;
            } else if (GV->getName() == "d") {
                VarD = GV;
            }
        }

        // c
        EXPECT_EQ(VarC->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        ASTArrayType *VarCType = (ASTArrayType *) VarC->getType();
        EXPECT_EQ(VarCType->getType()->getStmtKind(), ASTTypeKind::TYPE_CHAR);
        ASTValueExpr *cExpr = (ASTValueExpr *) VarC->getExpr();
        EXPECT_EQ(((const ASTArrayValue *) cExpr->getValue())->size(), 4);
        EXPECT_FALSE(((const ASTArrayValue *) cExpr->getValue())->empty());
        EXPECT_EQ(((ASTCharValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[0])->getValue(), "a");
        EXPECT_EQ(((ASTCharValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[1])->getValue(), "b");
        EXPECT_EQ(((ASTCharValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[2])->getValue(), "c");
        EXPECT_EQ(((ASTCharValue *) ((const ASTArrayValue *) cExpr->getValue())->getValues()[3])->getValue(), "");

        // d
        EXPECT_EQ(VarD->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) VarC->getType())->getType()->getStmtKind(), ASTTypeKind::TYPE_CHAR);
        EXPECT_EQ(((ASTCharValue *) ((ASTValueExpr *)((ASTArrayType *) VarD->getType())->getSize())->getValue())->getValue(), "2");
        EXPECT_NE(VarD->getExpr(), nullptr);
        ASTValueExpr *dExpr = (ASTValueExpr *) VarD->getExpr();
        EXPECT_EQ(((const ASTArrayValue *) dExpr->getValue())->size(), 2);
        EXPECT_FALSE(((const ASTArrayValue *) dExpr->getValue())->empty());
        EXPECT_EQ(((ASTCharValue *) ((const ASTArrayValue *) dExpr->getValue())->getValues()[0])->getValue(), "");
        EXPECT_EQ(((ASTCharValue *) ((const ASTArrayValue *) dExpr->getValue())->getValues()[1])->getValue(), "");

    }

    TEST_F(ParserTest, GlobalString) {
        llvm::StringRef str = (
               "string a = \"\"\n" // array of zero bytes
               "string b = \"abc\"\n" // string abc/0 -> array of 4 bytes
               "string c"
        );
        ASTModule *Module = Parse("GlobalString", str);
        ASSERT_TRUE(Resolve());

        ASTGlobalVar *VarA = nullptr;
        ASTGlobalVar *VarB = nullptr;
        ASTGlobalVar *VarC = nullptr;

        for (ASTGlobalVar *GV : Module->getGlobalVars()) {
            if (GV->getName() == "a") {
                VarA = GV;
            } else if (GV->getName() == "b") {
                VarB = GV;
            } else if (GV->getName() == "c") {
                VarC = GV;
            }
        }

        // a
        EXPECT_EQ(VarA->getType()->getStmtKind(), ASTTypeKind::TYPE_STRING);
        EXPECT_NE(VarA->getExpr(), nullptr);
        const ASTValue *ValA = ((ASTValueExpr *) VarA->getExpr())->getValue();
        EXPECT_EQ(((ASTArrayValue *) ValA)->getValues().size(), 0);

        // b
        EXPECT_EQ(VarB->getType()->getStmtKind(), ASTTypeKind::TYPE_STRING);
        StringRef Str = ((ASTStringValue *) ((ASTValueExpr *) VarB->getExpr())->getValue())->getValue();
        EXPECT_EQ(Str.size(), 3);
        EXPECT_EQ(Str.data()[0], 'a');
        EXPECT_EQ(Str.data()[1], 'b');
        EXPECT_EQ(Str.data()[2], 'c');

        // c
        EXPECT_EQ(VarC->getType()->getStmtKind(), ASTTypeKind::TYPE_STRING);
        EXPECT_EQ(VarC->getExpr(), nullptr);
    }
}