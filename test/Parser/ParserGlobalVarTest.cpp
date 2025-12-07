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

        EXPECT_TRUE(HasModifier(VarA->getModifiers(), ASTModifierKind::MOD_PRIVATE));
        EXPECT_FALSE(VarA->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarA->getType(), ASTBuiltinTypeKind::TYPE_INT));
        EXPECT_EQ(VarA->getName(), "a");

        EXPECT_TRUE(HasModifier(VarB->getModifiers(), ASTModifierKind::MOD_PUBLIC));
        EXPECT_FALSE(VarB->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarB->getType(), ASTBuiltinTypeKind::TYPE_FLOAT));
        EXPECT_EQ(VarB->getName(), "b");

        // VarC has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarC->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_FALSE(VarC->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarC->getType(), ASTBuiltinTypeKind::TYPE_BOOL));
        EXPECT_EQ(VarC->getName(), "c");

        // VarD has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarD->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_TRUE(VarD->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarD->getType(), ASTBuiltinTypeKind::TYPE_LONG));
        EXPECT_EQ(VarD->getName(), "d");

        // VarE has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarE->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_FALSE(VarE->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarE->getType(), ASTBuiltinTypeKind::TYPE_DOUBLE));
        EXPECT_EQ(VarE->getName(), "e");

        // VarF has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarF->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_FALSE(VarF->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarF->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(VarF->getName(), "f");

        // VarG has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarG->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_FALSE(VarG->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarG->getType(), ASTBuiltinTypeKind::TYPE_USHORT));
        EXPECT_EQ(VarG->getName(), "g");

        // VarH has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarH->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_FALSE(VarH->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarH->getType(), ASTBuiltinTypeKind::TYPE_SHORT));
        EXPECT_EQ(VarH->getName(), "h");

        // VarI has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarI->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_FALSE(VarI->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarI->getType(), ASTBuiltinTypeKind::TYPE_UINT));
        EXPECT_EQ(VarI->getName(), "i");

        // VarJ has default visibility -> check via modifiers below
        EXPECT_TRUE(HasModifier(VarJ->getModifiers(), ASTModifierKind::MOD_DEFAULT));
        ASSERT_FALSE(VarJ->isConstant());
        EXPECT_TRUE(HasBuiltinType(VarJ->getType(), ASTBuiltinTypeKind::TYPE_ULONG));
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
        EXPECT_TRUE(HasBuiltinType(((ASTArrayType *) VarA->getType())->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(((ASTArrayType *) VarA->getType())->getSizeExpr(), nullptr);
        EXPECT_NE(static_cast<ASTNullValue *>(VarA->getExpr()), nullptr);
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
        EXPECT_TRUE(HasBuiltinType(((ASTArrayType *) VarB->getType())->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(((ASTArrayType *) VarB->getType())->getSizeExpr(), nullptr);
        EXPECT_NE(VarB->getExpr(), nullptr);
        auto *bExpr = static_cast<ASTArrayValue *>(VarB->getExpr());
        EXPECT_EQ(bExpr->size(), 0);
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
        EXPECT_TRUE(HasBuiltinType(((ASTArrayType *) VarC->getType())->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(((ASTArrayType *) VarC->getType())->getSizeExpr(), nullptr);
        auto *cExpr = static_cast<ASTArrayValue *>(VarC->getExpr());
        EXPECT_EQ(cExpr->size(), 3);
        EXPECT_FALSE(cExpr->empty());
        EXPECT_EQ(static_cast<ASTNumberValue *>(cExpr->getValues()[0])->getValue(), "1");
        EXPECT_EQ(static_cast<ASTNumberValue *>(cExpr->getValues()[1])->getValue(), "2");
        EXPECT_EQ(static_cast<ASTNumberValue *>(cExpr->getValues()[2])->getValue(), "3");
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
        EXPECT_TRUE(HasBuiltinType(((ASTArrayType *) VarD->getType())->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(static_cast<ASTNumberValue *>(((ASTArrayType *) VarD->getType())->getSizeExpr())->getValue(), "3");
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
        EXPECT_TRUE(HasBuiltinType(((ASTArrayType *) VarE->getType())->getType(), ASTBuiltinTypeKind::TYPE_BYTE));
        EXPECT_EQ(static_cast<ASTNumberValue *>(((ASTArrayType *) VarE->getType())->getSizeExpr())->getValue(), "3");
        EXPECT_NE(VarE->getExpr(), nullptr);
        auto *eExpr = static_cast<ASTArrayValue *>(VarE->getExpr());
        EXPECT_EQ(eExpr->size(), 3);
        EXPECT_FALSE(eExpr->empty());
        EXPECT_EQ(static_cast<ASTNumberValue *>(eExpr->getValues()[0])->getValue(), "1");
        EXPECT_EQ(static_cast<ASTNumberValue *>(eExpr->getValues()[1])->getValue(), "2");
        EXPECT_EQ(static_cast<ASTNumberValue *>(eExpr->getValues()[2])->getValue(), "3");
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
        EXPECT_EQ(static_cast<ASTCharValue *>(VarA->getExpr())->getValue(), "");

        // b
        EXPECT_EQ(VarB->getType()->getStmtKind(), ASTTypeKind::TYPE_CHAR);
        EXPECT_NE(VarB->getExpr(), nullptr);
        EXPECT_EQ(static_cast<ASTCharValue *>(VarB->getExpr())->getValue(), "b");
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
        auto *cExpr = static_cast<ASTArrayValue *>(VarC->getExpr());
        EXPECT_EQ(cExpr->size(), 4);
        EXPECT_FALSE(cExpr->empty());
        EXPECT_EQ(static_cast<ASTCharValue *>(cExpr->getValues()[0])->getValue(), "a");
        EXPECT_EQ(static_cast<ASTCharValue *>(cExpr->getValues()[1])->getValue(), "b");
        EXPECT_EQ(static_cast<ASTCharValue *>(cExpr->getValues()[2])->getValue(), "c");
        EXPECT_EQ(static_cast<ASTCharValue *>(cExpr->getValues()[3])->getValue(), "");

        // d
        EXPECT_EQ(VarD->getType()->getStmtKind(), ASTTypeKind::TYPE_ARRAY);
        EXPECT_EQ(((ASTArrayType *) VarC->getType())->getType()->getStmtKind(), ASTTypeKind::TYPE_CHAR);
        EXPECT_EQ(static_cast<ASTCharValue *>(static_cast<ASTValue *>(((ASTArrayType *) VarD->getType())->getSizeExpr()))->getValue(), "2");
        auto *dExpr = static_cast<ASTArrayValue *>(VarD->getExpr());
        EXPECT_EQ(dExpr->size(), 2);
        EXPECT_FALSE(dExpr->empty());
        EXPECT_EQ(static_cast<ASTCharValue *>(dExpr->getValues()[0])->getValue(), "");
        EXPECT_EQ(static_cast<ASTCharValue *>(dExpr->getValues()[1])->getValue(), "");

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
        StringRef Str = static_cast<ASTStringValue *>(VarB->getExpr())->getValue();
        EXPECT_EQ(Str.size(), 3);
        EXPECT_EQ(Str.data()[0], 'a');
        EXPECT_EQ(Str.data()[1], 'b');
        EXPECT_EQ(Str.data()[2], 'c');

        // c
        EXPECT_EQ(VarC->getType()->getStmtKind(), ASTTypeKind::TYPE_STRING);
        EXPECT_EQ(VarC->getExpr(), nullptr);
    }
}