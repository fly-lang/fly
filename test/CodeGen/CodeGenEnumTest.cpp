//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "CodeGenTest.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaBuilderModifiers.h"
#include "AST/ASTModule.h"
#include <Sema/SemaNameSpace.h>
#include "AST/ASTName.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTType.h"
#include "AST/ASTOp.h"
#include "AST/ASTExprStmt.h"


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
        ASTEnum *TestEnum = getASTBuilder().CreateEnum(Module, SourceLoc, "TestEnum", TopModifiers, SuperEnums);
        ASTEnumEntry *A = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "A", EmptyModifiers);
        ASTEnumEntry *B = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "B", EmptyModifiers);
        ASTEnumEntry *C = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "C", EmptyModifiers);

        // int main() {
        //  TestEnum a = TestEnum.A;
        //  TestEnum b = a
        //  return 1
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        // Build a type reference to the enum by creating an ASTName and then an ASTType
        llvm::SmallVector<ASTName *, 4> EnumNames;
        EnumNames.push_back(getASTBuilder().CreateName(TestEnum->getName(), SourceLoc));
        ASTType *TestEnumType = getASTBuilder().CreateType(SourceLoc, EnumNames);

        //  TestEnum a = TestEnum.A;
        ASTLocalVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestEnumType, "a", EmptyModifiers);
        ASTIdentifier *aVarIdent = getASTBuilder().CreateIdentifier(aVar);
        ASTIdentifier *Enum_AIdent = getASTBuilder().CreateIdentifier(A);
        ASTExprStmt *aVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBinaryOp *aAssign = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, aVarIdent, Enum_AIdent);
        aVarStmt->setExpr(aAssign);

        //  TestEnum b = a
        ASTLocalVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestEnumType, "b", EmptyModifiers);
        ASTIdentifier *bVarIdent = getASTBuilder().CreateIdentifier(bVar);
        ASTIdentifier *aIdent = getASTBuilder().CreateIdentifier(aVar);
        ASTExprStmt *bVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBinaryOp *bAssign = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, bVarIdent, aIdent);
        bVarStmt->setExpr(bAssign);

		// Generate Code
		llvm::Module * M = Generate()[0];
		std::string output = getOutput(M);

        EXPECT_EQ(output, "%error = type { i8, i32, i8* }\n"
                          "\n"
                          "define void @F_0(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  %3 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  %4 = load i32, i32* %2, align 4\n"
                          "  store i32 %4, i32* %3, align 4\n"
                          "  ret void\n"
                          "}\n");
    }
} // anonymous namespace
