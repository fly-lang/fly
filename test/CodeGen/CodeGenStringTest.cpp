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
#include "AST/ASTOpExpr.h"

#include <Sema/SemaEnumType.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaNameSpace.h>


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGDefaultStringLocalVar) {
        ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

    	// default string k = ""
    	ASTVar *LocalVar_k = getASTBuilder().CreateLocalVar(Body, SourceLoc, StringTypeRef, "k", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_k = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_k);
    	VarStmt_k->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(DoubleTypeRef->getSym())));

    	// default char l = '\0'
    	ASTVar *LocalVar_l = getASTBuilder().CreateLocalVar(Body, SourceLoc, CharTypeRef, "l", EmptyModifiers);
    	SemaBuilderStmt *VarStmt_l = getASTBuilder().CreateAssignmentStmt(Body, LocalVar_l);
    	VarStmt_l->setExpr(getASTBuilder().CreateExpr(getASTBuilder().CreateDefaultValue(DoubleTypeRef->getSym())));

    	// Generate Code
    	llvm::Module * M = Generate();
    	std::string output = getOutput(M->getFunctionList());

    	EXPECT_EQ(output, "define void @_F0(%error* %0) {\n"
						  "entry:\n"
						  "  %1 = alloca %error*, align 8\n"
						  "  %12 = alloca [0 x i8], align 1\n"
						  "  %13 = alloca i8, align 1\n"
						  "  store %error* %0, %error** %1, align 8\n"
						  "  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), [0 x i8]* %12, align 8\n"
						  "  store i8 48, i8* %13, align 1\n"
						  "  ret void\n"
						  "}\n");
    }

    TEST_F(CodeGenTest, CGFuncStringParam) {
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTVar *, 8> Params;
    	Params.push_back(getASTBuilder().CreateParam(SourceLoc, StringTypeRef, "k", EmptyModifiers));
    	Params.push_back(getASTBuilder().CreateParam(SourceLoc, CharTypeRef, "l", EmptyModifiers));
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);

        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);
        // func(string k, char l) {
        // }

    	// Generate Code
    	llvm::Module * M = Generate();
    	std::string output = getOutput(M->getFunctionList());

        EXPECT_EQ(output, "define void @_F0_Ss_c(%error* %0, [0 x i8] %11, i8 %12) {\n"
                          "entry:\n"
                          "  %13 = alloca %error*, align 8\n"
                          "  %24 = alloca [0 x i8], align 1\n"
                          "  %25 = alloca i8, align 1\n"
                          "  store %error* %0, %error** %13, align 8\n"
                          "  store i8 %26, i8* %16, align 1\n"
                          "  store i8 %22, i8* %16, align 1\n"
                          "  store i64 %4, i64* %17, align 8\n"
                          "  store double %5, double* %18, align 8\n"
                          "  store i8 %6, i8* %19, align 1\n"
                          "  store i16 %7, i16* %20, align 2\n"
                          "  store i16 %8, i16* %21, align 2\n"
                          "  store i32 %9, i32* %22, align 4\n"
                          "  store i64 %10, i64* %23, align 8\n"
                          "  ret void\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, GCStringLocalVarAssignAfter) {
        ASTModule *Module = CreateModule();

        // func()
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

    	// float g
    	ASTVar *LocalVar_g = getASTBuilder().CreateLocalVar(Body, SourceLoc, FloatTypeRef, "g", EmptyModifiers);

        // g = 1.0
        ASTVarRef *VarRef_g = CreateVarRef(LocalVar_g);
        SemaBuilderStmt * GVarStmt = getASTBuilder().CreateAssignmentStmt(Body, VarRef_g);
        ASTExpr *ExprG = getASTBuilder().CreateExpr(getASTBuilder().CreateFloatingValue(SourceLoc, "1.0"));
        GVarStmt->setExpr(ExprG);

        // return g
        SemaBuilderStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        Return->setExpr(getASTBuilder().CreateExpr(CreateVarRef(LocalVar_g)));

    	// Generate Code
    	llvm::Module * M = Generate();
    	std::string output = getOutput(M);

        EXPECT_EQ(output, "define i32 @F_0(%error* %0) {\n"
                          "entry:\n"
                          "  %1 = alloca %error*, align 8\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store %error* %0, %error** %1, align 8\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  store float 1.000000e+00, float* @G, align 4\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  ret i32 %3\n"
                          "}\n");
    }

} // anonymous namespace
