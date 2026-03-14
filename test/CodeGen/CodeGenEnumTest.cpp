//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "AST/ASTBinary.h"
#include "AST/ASTDeclStmt.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTMember.h"
#include "AST/ASTModule.h"
#include "AST/ASTName.h"
#include "AST/ASTType.h"
#include "AST/ASTValue.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGenTest.h"
#include "Sema/SemaBuilderModifiers.h"

#include <Sema/SemaNameSpace.h>

namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGEnum) {
        /**
         * Fly code:
         * enum TestEnum {
         *   A, B, C
         * }
         * void func() {
         *   TestEnum a = TestEnum.A
         *   TestEnum b = a
         * }
         */
        ASTModule *Module = CreateModule();

        // enum TestEnum {
        //   A
        //   B
        //   C
        // }
        llvm::SmallVector<ASTType *, 4> SuperEnums;
        ASTEnum *TestEnum = ASTBuilder::CreateEnum(Module, SourceLoc, "TestEnum", TopModifiers, SuperEnums);
        ASTEnumEntry *A = ASTBuilder::CreateEnumEntry(SourceLoc, TestEnum, "A", EmptyModifiers);
        ASTEnumEntry *B = ASTBuilder::CreateEnumEntry(SourceLoc, TestEnum, "B", EmptyModifiers);
        ASTEnumEntry *C = ASTBuilder::CreateEnumEntry(SourceLoc, TestEnum, "C", EmptyModifiers);

        // main() {
        //  TestEnum a = TestEnum.A;
        //  TestEnum b = a
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // Build a type reference to the enum
        ASTType *TestEnumType = CreateType(TestEnum);

        //  TestEnum a = TestEnum.A;
        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, TestEnumType, "a", EmptyModifiers);
        ASTIdentifier *aVarIdent = ASTBuilder::CreateIdentifier(aVar);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
        // Create TestEnum.A as member expression
        ASTIdentifier *TestEnumIdent = ASTBuilder::CreateIdentifier(SourceLoc, TestEnum->getName());
        ASTMember *Enum_AMember = ASTBuilder::CreateMember(SourceLoc, A->getName(), TestEnumIdent);
        ASTBinary *aAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aVarIdent, Enum_AMember);
        aDeclStmt->setExpr(aAssign);

        //  TestEnum b = a
        // ASTLocalVar *bVar = ASTBuilder::CreateLocalVar(SourceLoc, TestEnumType, "b", EmptyModifiers);
        // ASTIdentifier *bVarIdent = ASTBuilder::CreateIdentifier(bVar);
        // ASTDeclStmt *bDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, bVar);
        // ASTIdentifier *aIdent = ASTBuilder::CreateIdentifier(aVar);
        // ASTBinary *bAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, bVarIdent, aIdent);
        // bDeclStmt->setExpr(bAssign);

		// Generate Code
		Generate();
		llvm::Module * M = getModules()[0];
		std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGEnumUnset) {
        /**
         * Fly code:
         * enum TestEnum {
         *   A, B, C
         * }
         * void func() {
         *   TestEnum a = unset
         *   TestEnum b
         * }
         */
        ASTModule *Module = CreateModule();

        // enum TestEnum {
        //   A
        //   B
        //   C
        // }
        llvm::SmallVector<ASTType *, 4> SuperEnums;
        ASTEnum *TestEnum = ASTBuilder::CreateEnum(Module, SourceLoc, "TestEnum", TopModifiers, SuperEnums);
        ASTEnumEntry *A = ASTBuilder::CreateEnumEntry(SourceLoc, TestEnum, "A", EmptyModifiers);
        ASTEnumEntry *B = ASTBuilder::CreateEnumEntry(SourceLoc, TestEnum, "B", EmptyModifiers);
        ASTEnumEntry *C = ASTBuilder::CreateEnumEntry(SourceLoc, TestEnum, "C", EmptyModifiers);

        // func() {
        //   TestEnum a = unset
        //   TestEnum b
        // }
        ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
        ASTFunction *Func = ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

        // Build a type reference to the enum
        ASTType *TestEnumType = CreateType(TestEnum);

        // TestEnum a = unset
        ASTLocalVar *aVar = ASTBuilder::CreateLocalVar(SourceLoc, TestEnumType, "a", EmptyModifiers);
        ASTIdentifier *aVarIdent = ASTBuilder::CreateIdentifier(aVar);
        ASTDeclStmt *aDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, aVar);
        ASTUnsetValue *aUnsetValue = ASTBuilder::CreateUnsetValue(SourceLoc);
        ASTBinary *aAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, aVarIdent, aUnsetValue);
        aDeclStmt->setExpr(aAssign);

        // TestEnum b (declaration without initializer - implicitly unset)
        ASTLocalVar *bVar = ASTBuilder::CreateLocalVar(SourceLoc, TestEnumType, "b", EmptyModifiers);
        ASTIdentifier *bVarIdent = ASTBuilder::CreateIdentifier(bVar);
        ASTDeclStmt *bDeclStmt = ASTBuilder::CreateDeclStmt(Body, SourceLoc, bVar);
        ASTUnsetValue *bUnsetValue = ASTBuilder::CreateUnsetValue(SourceLoc);
        ASTBinary *bAssign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN, bVarIdent, bUnsetValue);
        bDeclStmt->setExpr(bAssign);

        // Generate Code
        Generate();
        llvm::Module *M = getModules()[0];
        std::string output = getOutput(M->getFunctionList());

        // Both a and b should be initialized to 0 (unset value for enums)
        EXPECT_EQ(output, "define void @_F4func(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 0, i32* %2, align 4\n"
                          "  store i32 0, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n");
    }
} // anonymous namespace
