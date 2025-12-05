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
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "Sema/SemaBuilderModifiers.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTVar.h"
#include "AST/ASTFunction.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTVar.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTOp.h"

#include <Sema/SemaEnumType.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaNameSpace.h>


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGEnum) {
        ASTModule *Module = CreateModule();

        // enum TestEnum {
        //   A
        //   B
        //   C
        // }
        llvm::SmallVector<ASTType *, 4> SuperEnums;
        ASTEnum *TestEnum = getASTBuilder().CreateEnum(Module, SourceLoc, "TestEnum", TopModifiers, SuperEnums);
        ASTVar *A = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "A", EmptyModifiers);
        ASTVar *B = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "B", EmptyModifiers);
        ASTVar *C = getASTBuilder().CreateEnumEntry(SourceLoc, TestEnum, "C", EmptyModifiers);

        // int main() {
        //  TestEnum a = TestEnum.A;
        //  TestEnum b = a
        //  return 1
        // }
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

        ASTType *TestEnumType = getASTBuilder().CreateTypeRef(TestEnum);

        //  TestEnum a = TestEnum.A;
        ASTVar *aVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestEnumType, "a", EmptyModifiers);
        ASTVarRef *Enum_AVarRef = CreateVarRef(A, getASTBuilder().CreateRef(SourceLoc, TestEnumType->getName()));
        ASTVarRefExpr *Enum_ARefExpr = getASTBuilder().CreateExpr(Enum_AVarRef);
        SemaBuilderStmt *aVarStmt = getASTBuilder().CreateAssignmentStmt(Body, aVar);
        aVarStmt->setExpr(Enum_ARefExpr);

        //  TestEnum b = a
        ASTVar *bVar = getASTBuilder().CreateLocalVar(Body, SourceLoc, TestEnumType, "b", EmptyModifiers);
        SemaBuilderStmt *bVarStmt = getASTBuilder().CreateAssignmentStmt(Body, bVar);
        ASTVarRefExpr *aRefExpr = getASTBuilder().CreateExpr(CreateVarRef(aVar));
        bVarStmt->setExpr(aRefExpr);

		// Generate Code
		llvm::Module * M = Generate();
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
