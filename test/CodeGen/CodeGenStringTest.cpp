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
#include "AST/ASTIdentifier.h"
#include "AST/ASTValue.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTOp.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTDeclStmt.h"

#include <Sema/SemaNameSpace.h>


namespace {

    using namespace fly;

    TEST_F(CodeGenTest, CGDefaultStringLocalVar) {
        /**
         * Fly code:
         * void func() {
         *   string k = default
         *   char l = default
         * }
         */
        ASTModule *Module = CreateModule();

    	ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
    	ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

    	// default string k = ""
    	ASTLocalVar *LocalVar_k = getASTBuilder().CreateLocalVar(SourceLoc, StringTypeRef, "k", EmptyModifiers);
    	ASTIdentifier *Ident_k = getASTBuilder().CreateIdentifier(LocalVar_k);
    	ASTDeclStmt *DeclStmt_k = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_k);
    	ASTBinaryOp *Assign_k = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Ident_k, getASTBuilder().CreateDefaultValue());
    	DeclStmt_k->setExpr(Assign_k);

    	// default char l = '\0'
    	ASTLocalVar *LocalVar_l = getASTBuilder().CreateLocalVar(SourceLoc, CharTypeRef, "l", EmptyModifiers);
    	ASTIdentifier *Ident_l = getASTBuilder().CreateIdentifier(LocalVar_l);
    	ASTDeclStmt *DeclStmt_l = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_l);
    	ASTBinaryOp *Assign_l = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, Ident_l, getASTBuilder().CreateDefaultValue());
    	DeclStmt_l->setExpr(Assign_l);

    	// Generate Code
    	llvm::Module * M = Generate()[0];
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
        /**
         * Fly code:
         * void func(string k, char l) {
         * }
         */
        ASTModule *Module = CreateModule();

        llvm::SmallVector<ASTParam *, 8> Params;
    	Params.push_back(getASTBuilder().CreateParam(SourceLoc, StringTypeRef, "k", EmptyModifiers));
    	Params.push_back(getASTBuilder().CreateParam(SourceLoc, CharTypeRef, "l", EmptyModifiers));
        ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);

        ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);
        // func(string k, char l) {
        // }

    	// Generate Code
    	llvm::Module * M = Generate()[0];
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
         /**
          * Fly code:
          * void func() {
          *   float g
          *   g = 1.0
          *   return g
          * }
          */
         ASTModule *Module = CreateModule();

         // func()
         ASTBlockStmt *Body = getASTBuilder().CreateBlockStmt(SourceLoc);
         ASTFunction *Func = getASTBuilder().CreateFunction(Module, SourceLoc, VoidTypeRef, "func", TopModifiers, Params, Body);

    	// float g
    	ASTLocalVar *LocalVar_g = getASTBuilder().CreateLocalVar(SourceLoc, FloatTypeRef, "g", EmptyModifiers);
    	ASTDeclStmt *DeclStmt_g = getASTBuilder().CreateDeclStmt(Body, SourceLoc, LocalVar_g);

        // g = 1.0
        ASTIdentifier *VarRef_g = getASTBuilder().CreateIdentifier(LocalVar_g);
        ASTNumberValue *ExprG = getASTBuilder().CreateNumberValue(SourceLoc, "1.0");
        ASTExprStmt *GVarStmt = getASTBuilder().CreateExprStmt(Body, SourceLoc);
        ASTBinaryOp *GAssign = getASTBuilder().CreateBinary(SourceLoc, ASTBinaryOpKind::OP_BINARY_ASSIGN, VarRef_g, ExprG);
        GVarStmt->setExpr(GAssign);

        // return g
        ASTReturnStmt *Return = getASTBuilder().CreateReturnStmt(Body, SourceLoc);
        Return->setExpr(getASTBuilder().CreateIdentifier(LocalVar_g));

    	// Generate Code
    	llvm::Module * M = Generate()[0];
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
