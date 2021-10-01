//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenFunction.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTFunc.h"
#include "AST/ASTVar.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTOperatorExpr.h"
#include "Basic/Diagnostic.h"
#include "Basic/DiagnosticOptions.h"
#include "Basic/SourceLocation.h"
#include "Basic/TargetOptions.h"
#include "Basic/Builtins.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <vector>


namespace {
    using namespace fly;

    // The test fixture.
    class CodeGenTest : public ::testing::Test {

        const char* testFile = "test.fly";

    public:
        const CompilerInstance CI;
        ASTContext *Context;
        CodeGen *CG;
        DiagnosticsEngine &Diags;
        SourceLocation SourceLoc;

        CodeGenTest() : CI(*TestUtils::CreateCompilerInstance()),
                   Context(new ASTContext(CI.getDiagnostics())),
                   CG(TestUtils::CreateCodeGen(CI)),
                   Diags(CI.getDiagnostics()) {
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmPrinters();
        }

        ASTNode *CreateAST() {
            EXPECT_TRUE(createTestFile());
            return CreateAST(testFile);
        }

        ASTNode *CreateAST(const llvm::StringRef Name, const StringRef NameSpace = "default") {
            auto *Node = new ASTNode(Name, Context, CG->CreateModule(Name));
            Node->setNameSpace(NameSpace);
            return Node;
        }

        virtual ~CodeGenTest() {
            deleteTestFile();
            llvm::outs().flush();
        }

        bool createTestFile() {
            std::fstream my_file;
            my_file.open(testFile, std::ios::out);
            if (my_file) {
                my_file.close();
            } else {
                std::cerr << "Error File " << testFile << " not created!\n";
            }
            return (bool) my_file;
        }

        void read() {
            std::ifstream reader(testFile) ;

            ASSERT_TRUE(reader && "Error opening input file");

            std::cout << testFile << " contains: \n";
            char letter ;
            for(int i = 0; ! reader.eof() ; i++ ) {
                reader.get( letter ) ;
                std::cout << letter ;
            }

            reader.close() ;
        }

        void deleteTestFile() {
            remove(testFile);
        }
    };

    TEST_F(CodeGenTest, CGGlobalVar) {
        ASTNode *Node = CreateAST();
        ASTGlobalVar *Var = new ASTGlobalVar(SourceLoc, Node, new ASTIntType(SourceLoc), "a");
        Node->AddGlobalVar(Var);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        GlobalVariable *GVar = (GlobalVariable *)CGM->GenGlobalVar(Var)->getPointer();
        testing::internal::CaptureStdout();
        GVar->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "@a = external global i32");
    }

    TEST_F(CodeGenTest, CGFunc) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc),
                                      "main");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "P1");
        MainFn->addParam(SourceLoc, new ASTFloatType(SourceLoc), "P2");
        MainFn->addParam(SourceLoc, new ASTBoolType(SourceLoc), "P3");
        Node->AddFunction(MainFn);

        ASTValueExpr *Expr = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        MainFn->getBody()->AddReturn(SourceLoc, Expr);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0, float %1, i1 %2) {\n"
                          "entry:\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca float, align 4\n"
                          "  %5 = alloca i1, align 1\n"
                          "  store i32 %0, i32* %3, align 4\n"
                          "  store float %1, float* %4, align 4\n"
                          "  store i1 %2, i1* %5, align 1\n"
                          "  ret i32 1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFuncRetVar) {
        ASTNode *Node = CreateAST();

        ASTGlobalVar *GVar = new ASTGlobalVar(SourceLoc, Node, new ASTFloatType(SourceLoc), "G");
        GVar->setVisibility(V_PRIVATE);
        Node->AddGlobalVar(GVar);

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->AddFunction(MainFn);

        // int A
        ASTLocalVar *VarA = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "A");
        MainFn->getBody()->AddVar(VarA);

        // A = 1
        ASTLocalVarRef * VarAAssign = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), VarA->getName());
        ASTExpr *Expr = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        VarAAssign->setExpr(Expr);
        MainFn->getBody()->AddVarRef(VarAAssign);

        // GlobalVar
        // G = 1
        ASTLocalVarRef * GVarAssign = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), GVar->getName(), "");
        GVarAssign->setExpr(Expr);
        MainFn->getBody()->AddVarRef(GVarAssign);

        // return A
        MainFn->getBody()->AddReturn(SourceLoc, new ASTVarRefExpr(SourceLoc,
                                                                  new ASTVarRef(SourceLoc, VarA->getName())));

        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        CGM->GenGlobalVar(GVar);
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  store i32 0, i32* %0, align 4\n"
                          "  store i32 1, i32* %0, align 4\n"
                          "  store float 1.000000e+00, float* @G, align 4\n"
                          "  %1 = load i32, i32* %0, align 4\n"
                          "  ret i32 %1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGFuncRetFn) {
        ASTNode *Node = CreateAST();

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->AddFunction(MainFn);

        // test()
        ASTFunc *TestFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "test");
        TestFn->setVisibility(V_PRIVATE);
        Node->AddFunction(TestFn);

        ASTFuncCall *TestCall = new ASTFuncCall(SourceLoc, "", TestFn->getName());
//        TestCall->addArg(new ValueExpr(SourceLoc, "1"));
        // call test()
        MainFn->getBody()->AddCall(TestCall);
        //return test()
        MainFn->getBody()->AddReturn(SourceLoc, new ASTFuncCallExpr(SourceLoc, TestCall));
        // Resolve Context for Resolutions of Call and Ref

        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        CGM->GenFunction(TestFn);
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = call i32 @test()\n"
                          "  %1 = call i32 @test()\n"
                          "  ret i32 %1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGGroupExpr) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "b");
        MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "c");
        Node->AddFunction(MainFn);

        // Create this expression: 1 + a * b / (c - 2)
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        Group->Add(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1",
                                                            new ASTIntType(SourceLoc))));
        Group->Add(new ASTArithExpr(SourceLoc, ARITH_ADD));
        Group->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(SourceLoc, "a")));
        Group->Add(new ASTArithExpr(SourceLoc, ARITH_MUL));
        Group->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(SourceLoc, "b")));
        Group->Add(new ASTArithExpr(SourceLoc, ARITH_DIV));

        ASTGroupExpr *SubGroup = new ASTGroupExpr(SourceLoc);
        SubGroup->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(SourceLoc, "c")));
        SubGroup->Add(new ASTArithExpr(SourceLoc, ARITH_SUB));
        SubGroup->Add(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2",
                                                               new ASTIntType(SourceLoc))));
        Group->Add(SubGroup);
        MainFn->getBody()->AddReturn(SourceLoc, Group);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0, i32 %1, i32 %2) {\n"
                          "entry:\n"
                          "  %3 = alloca i32, align 4\n"
                          "  %4 = alloca i32, align 4\n"
                          "  %5 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %3, align 4\n"
                          "  store i32 %1, i32* %4, align 4\n"
                          "  store i32 %2, i32* %5, align 4\n"
                          "  %6 = load i32, i32* %3, align 4\n"
                          "  %7 = load i32, i32* %4, align 4\n"
                          "  %8 = mul i32 %6, %7\n"
                          "  %9 = load i32, i32* %5, align 4\n"
                          "  %10 = sub i32 %9, 2\n"
                          "  %11 = sdiv i32 %8, %10\n"
                          "  %12 = add i32 1, %11\n"
                          "  ret i32 %12\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGArithOp) {
        ASTNode *Node = CreateAST();

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->AddFunction(MainFn);

        ASTLocalVar *A = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "A");
        ASTLocalVar *B = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "B");
        ASTLocalVar *C = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "C");
        MainFn->getBody()->AddVar(A);
        MainFn->getBody()->AddVar(B);

        // Operation Add
        ASTGroupExpr *GroupAdd = new ASTGroupExpr(SourceLoc);
        GroupAdd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupAdd->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_ADD));
        GroupAdd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        C->setExpr(GroupAdd);
        MainFn->getBody()->AddVar(C);

        // Operation Sub
        ASTLocalVarRef *Csub = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupSub = new ASTGroupExpr(SourceLoc);
        GroupSub->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupSub->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_SUB));
        GroupSub->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Csub->setExpr(GroupSub);
        MainFn->getBody()->AddVarRef(Csub);

        // Operation Mul
        ASTLocalVarRef *Cmul = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupMul = new ASTGroupExpr(SourceLoc);
        GroupMul->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupMul->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_MUL));
        GroupMul->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cmul->setExpr(GroupMul);
        MainFn->getBody()->AddVarRef(Cmul);

        // Operation Div
        ASTLocalVarRef *Cdiv = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupDiv = new ASTGroupExpr(SourceLoc);
        GroupDiv->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupDiv->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_DIV));
        GroupDiv->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cdiv->setExpr(GroupDiv);
        MainFn->getBody()->AddVarRef(Cdiv);

        // Operation Mod
        ASTLocalVarRef *Cmod = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupMod = new ASTGroupExpr(SourceLoc);
        GroupMod->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupMod->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_MOD));
        GroupMod->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cmod->setExpr(GroupMod);
        MainFn->getBody()->AddVarRef(Cmod);

        // Operation And
        ASTLocalVarRef *Cand = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupAnd = new ASTGroupExpr(SourceLoc);
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupAnd->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_AND));
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cand->setExpr(GroupAnd);
        MainFn->getBody()->AddVarRef(Cand);

        // Operation Or
        ASTLocalVarRef *Cor = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupOr = new ASTGroupExpr(SourceLoc);
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupOr->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_OR));
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cor->setExpr(GroupOr);
        MainFn->getBody()->AddVarRef(Cor);

        // Operation Xor
        ASTLocalVarRef *Cxor = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupXor = new ASTGroupExpr(SourceLoc);
        GroupXor->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupXor->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_XOR));
        GroupXor->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cxor->setExpr(GroupXor);
        MainFn->getBody()->AddVarRef(Cxor);

        // Operation Shl
        ASTLocalVarRef *Cshl = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupShl = new ASTGroupExpr(SourceLoc);
        GroupShl->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupShl->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_SHIFT_L));
        GroupShl->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cshl->setExpr(GroupShl);
        MainFn->getBody()->AddVarRef(Cshl);

        // Operation Shr
        ASTLocalVarRef *Cshr = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupShr = new ASTGroupExpr(SourceLoc);
        GroupShr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupShr->Add(new ASTArithExpr(SourceLoc, ArithOpKind::ARITH_SHIFT_R));
        GroupShr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cshr->setExpr(GroupShr);
        MainFn->getBody()->AddVarRef(Cshr);

        // Pre-Increment
        ASTArithExpr *PreIncr = new ASTArithExpr(SourceLoc, ARITH_INCR);
        ASTUnaryExpr *PreIncrExpr = new ASTUnaryExpr(SourceLoc, PreIncr, new ASTVarRef(A), UnaryOpKind::UNARY_PRE);
        ASTExprStmt *PreIncrExprStmt = new ASTExprStmt(SourceLoc, MainFn->getBody());
        PreIncrExprStmt->setExpr(PreIncrExpr);
        MainFn->getBody()->AddExprStmt(PreIncrExprStmt);

        // Post-Increment
        ASTArithExpr *PostIncr = new ASTArithExpr(SourceLoc, ARITH_INCR);
        ASTUnaryExpr *PostIncrExpr = new ASTUnaryExpr(SourceLoc, PostIncr, new ASTVarRef(A), UnaryOpKind::UNARY_POST);
        ASTExprStmt *PostIncrExprStmt = new ASTExprStmt(SourceLoc, MainFn->getBody());
        PostIncrExprStmt->setExpr(PostIncrExpr);
        MainFn->getBody()->AddExprStmt(PostIncrExprStmt);

        // Pre-Decrement
        ASTArithExpr *PreDecr = new ASTArithExpr(SourceLoc, ARITH_DECR);
        ASTUnaryExpr *PreDecrExpr = new ASTUnaryExpr(SourceLoc, PreDecr, new ASTVarRef(A), UnaryOpKind::UNARY_PRE);
        ASTExprStmt *PreDecrExprStmt = new ASTExprStmt(SourceLoc, MainFn->getBody());
        PreDecrExprStmt->setExpr(PreDecrExpr);
        MainFn->getBody()->AddExprStmt(PreDecrExprStmt);

        // Post-Decrement
        ASTArithExpr *PostDecr = new ASTArithExpr(SourceLoc, ARITH_DECR);
        ASTUnaryExpr *PostDecrExpr = new ASTUnaryExpr(SourceLoc, PostDecr, new ASTVarRef(A), UnaryOpKind::UNARY_POST);
        ASTExprStmt *PostDecrExprStmt = new ASTExprStmt(SourceLoc, MainFn->getBody());
        PostDecrExprStmt->setExpr(PostDecrExpr);
        MainFn->getBody()->AddExprStmt(PostDecrExprStmt);

        //return test()
        MainFn->getBody()->AddReturn(SourceLoc, new ASTVarRefExpr(SourceLoc, new ASTVarRef(C)));

        // Resolve Context for Resolutions of Call and Ref
        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  %1 = alloca i32, align 4\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 0, i32* %0, align 4\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  %3 = load i32, i32* %0, align 4\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = add i32 %3, %4\n"
                          "  store i32 %5, i32* %2, align 4\n"
                          "  %6 = sub i32 %3, %4\n"
                          "  store i32 %6, i32* %2, align 4\n"
                          "  %7 = mul i32 %3, %4\n"
                          "  store i32 %7, i32* %2, align 4\n"
                          "  %8 = sdiv i32 %3, %4\n"
                          "  store i32 %8, i32* %2, align 4\n"
                          "  %9 = srem i32 %3, %4\n"
                          "  store i32 %9, i32* %2, align 4\n"
                          "  %10 = and i32 %3, %4\n"
                          "  store i32 %10, i32* %2, align 4\n"
                          "  %11 = or i32 %3, %4\n"
                          "  store i32 %11, i32* %2, align 4\n"
                          "  %12 = xor i32 %3, %4\n"
                          "  store i32 %12, i32* %2, align 4\n"
                          "  %13 = shl i32 %3, %4\n"
                          "  store i32 %13, i32* %2, align 4\n"
                          "  %14 = ashr i32 %3, %4\n"
                          "  store i32 %14, i32* %2, align 4\n"
                          "  %15 = add nsw i32 %3, 1\n"
                          "  store i32 %15, i32* %0, align 4\n"
                          "  %16 = load i32, i32* %0, align 4\n"
                          "  %17 = add i32 %16, 1\n"
                          "  store i32 %16, i32* %0, align 4\n"
                          "  %18 = load i32, i32* %0, align 4\n"
                          "  %19 = add nsw i32 %18, -1\n"
                          "  store i32 %19, i32* %0, align 4\n"
                          "  %20 = load i32, i32* %0, align 4\n"
                          "  %21 = add i32 %20, -1\n"
                          "  store i32 %20, i32* %0, align 4\n"
                          "  %22 = load i32, i32* %2, align 4\n"
                          "  ret i32 %22\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGComparatorOp) {
        ASTNode *Node = CreateAST();

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->AddFunction(MainFn);

        ASTLocalVar *A = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "A");
        ASTLocalVar *B = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTIntType(SourceLoc), "B");
        ASTLocalVar *C = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "C");
        MainFn->getBody()->AddVar(A);
        MainFn->getBody()->AddVar(B);

        // Operation Equal
        ASTGroupExpr *GroupEq = new ASTGroupExpr(SourceLoc);
        GroupEq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupEq->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_EQ));
        GroupEq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        C->setExpr(GroupEq);
        MainFn->getBody()->AddVar(C);

        // Operation Not Equal
        ASTLocalVarRef *Cneq = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupNeq = new ASTGroupExpr(SourceLoc);
        GroupNeq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupNeq->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_NE));
        GroupNeq->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cneq->setExpr(GroupNeq);
        MainFn->getBody()->AddVarRef(Cneq);

        // Operation Greater Than
        ASTLocalVarRef *Cgt = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupGt = new ASTGroupExpr(SourceLoc);
        GroupGt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupGt->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_GT));
        GroupGt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cgt->setExpr(GroupGt);
        MainFn->getBody()->AddVarRef(Cgt);

        // Operation Greater Than or Equal
        ASTLocalVarRef *Cgte = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupGte = new ASTGroupExpr(SourceLoc);
        GroupGte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupGte->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_GTE));
        GroupGte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cgte->setExpr(GroupGte);
        MainFn->getBody()->AddVarRef(Cgte);

        // Operation Less Than
        ASTLocalVarRef *Clt = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupLt = new ASTGroupExpr(SourceLoc);
        GroupLt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupLt->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_LT));
        GroupLt->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Clt->setExpr(GroupLt);
        MainFn->getBody()->AddVarRef(Clt);

        // Operation Less Than or Equal
        ASTLocalVarRef *Clte = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupLte = new ASTGroupExpr(SourceLoc);
        GroupLte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupLte->Add(new ASTComparisonExpr(SourceLoc, ComparisonOpKind::COMP_LTE));
        GroupLte->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Clte->setExpr(GroupLte);
        MainFn->getBody()->AddVarRef(Clte);

        //return test()
        MainFn->getBody()->AddReturn(SourceLoc, new ASTVarRefExpr(SourceLoc, new ASTVarRef(C)));
        // Resolve Context for Resolutions of Call and Ref
        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i32, align 4\n"
                          "  %1 = alloca i32, align 4\n"
                          "  %2 = alloca i1, align 1\n"
                          "  store i32 0, i32* %0, align 4\n"
                          "  store i32 0, i32* %1, align 4\n"
                          "  %3 = load i32, i32* %0, align 4\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = icmp eq i32 %3, %4\n"
                          "  store i1 %5, i1* %2, align 1\n"
                          "  %6 = icmp ne i32 %3, %4\n"
                          "  store i1 %6, i1* %2, align 1\n"
                          "  %7 = icmp sgt i32 %3, %4\n"
                          "  store i1 %7, i1* %2, align 1\n"
                          "  %8 = icmp sge i32 %3, %4\n"
                          "  store i1 %8, i1* %2, align 1\n"
                          "  %9 = icmp slt i32 %3, %4\n"
                          "  store i1 %9, i1* %2, align 1\n"
                          "  %10 = icmp sle i32 %3, %4\n"
                          "  store i1 %10, i1* %2, align 1\n"
                          "  %11 = load i1, i1* %2, align 1\n"
                          "  ret i1 %11\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGLogicOp) {
        ASTNode *Node = CreateAST();

        // main()
        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        Node->AddFunction(MainFn);

        ASTLocalVar *A = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "A");
        ASTLocalVar *B = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "B");
        ASTLocalVar *C = new ASTLocalVar(SourceLoc, MainFn->getBody(), new ASTBoolType(SourceLoc), "C");
        MainFn->getBody()->AddVar(A);
        MainFn->getBody()->AddVar(B);
        MainFn->getBody()->AddVar(C);

        // Operation And Logic
        ASTGroupExpr *GroupAnd = new ASTGroupExpr(SourceLoc);
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupAnd->Add(new ASTLogicExpr(SourceLoc, LogicOpKind::LOGIC_AND));
        GroupAnd->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        C->setExpr(GroupAnd);

        // Operation Or Logic
        ASTLocalVarRef *Cor = new ASTLocalVarRef(SourceLoc, MainFn->getBody(), C);
        ASTGroupExpr *GroupOr = new ASTGroupExpr(SourceLoc);
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(A)));
        GroupOr->Add(new ASTLogicExpr(SourceLoc, LogicOpKind::LOGIC_OR));
        GroupOr->Add(new ASTVarRefExpr(SourceLoc, new ASTVarRef(B)));
        Cor->setExpr(GroupOr);
        MainFn->getBody()->AddVarRef(Cor);

        //return test()
        MainFn->getBody()->AddReturn(SourceLoc, new ASTVarRefExpr(SourceLoc, new ASTVarRef(C)));
        // Resolve Context for Resolutions of Call and Ref
        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main() {\n"
                          "entry:\n"
                          "  %0 = alloca i1, align 1\n"
                          "  %1 = alloca i1, align 1\n"
                          "  %2 = alloca i1, align 1\n"
                          "  store i1 false, i1* %0, align 1\n"
                          "  store i1 false, i1* %1, align 1\n"
                          "  %3 = load i1, i1* %0, align 1\n"
                          "  br i1 %3, label %and, label %and1\n"
                          "\n"
                          "and:                                              ; preds = %entry\n"
                          "  %4 = load i1, i1* %1, align 1\n"
                          "  br label %and1\n"
                          "\n"
                          "and1:                                             ; preds = %and, %entry\n"
                          "  %5 = phi i1 [ false, %entry ], [ %4, %and ]\n"
                          "  %6 = zext i1 %5 to i8\n"
                          "  store i8 %6, i1* %2, align 1\n"
                          "  %7 = load i1, i1* %0, align 1\n"
                          "  br i1 %7, label %or2, label %or\n"
                          "\n"
                          "or:                                               ; preds = %and1\n"
                          "  %8 = load i1, i1* %1, align 1\n"
                          "  br label %or2\n"
                          "\n"
                          "or2:                                              ; preds = %or, %and1\n"
                          "  %9 = phi i1 [ true, %and1 ], [ %8, %or ]\n"
                          "  %10 = zext i1 %9 to i8\n"
                          "  store i8 %10, i1* %2, align 1\n"
                          "  %11 = load i1, i1* %2, align 1\n"
                          "  ret i1 %11\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        // Compare
        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Group->Add(ARef);
        Group->Add(Comp);
        Group->Add(OneCost);

        // First If
        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Group);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, IfBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        IfBlock->AddVarRef(A2);
        MainFn->getBody()->AddBlock(SourceLoc, IfBlock);
        MainFn->getBody()->AddReturn(SourceLoc, ARef);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %endif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %ifthen, %entry\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  ret i32 %4\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElseBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Cond = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Cond->Add(ARef);
        Cond->Add(Comp);
        Cond->Add(OneCost);

        // if (a == 1) { a = 1 }
        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Cond);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, IfBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        IfBlock->AddVarRef(A2);
        MainFn->getBody()->AddBlock(SourceLoc, IfBlock);

        // else {a == 2}
        ASTElseBlock *ElseBlock = new ASTElseBlock(SourceLoc, MainFn->getBody());
        ASTLocalVarRef *A3 = new ASTLocalVarRef(SourceLoc, ElseBlock, Param);
        A3->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2", new ASTIntType(SourceLoc))));
        ElseBlock->AddVarRef(A3);
        IfBlock->AddBranch(MainFn->getBody(), ElseBlock);

        MainFn->getBody()->AddReturn(SourceLoc, ARef);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %else\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %entry\n"
                          "  store i32 2, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %else, %ifthen\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  ret i32 %4\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsifElseBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Cond = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Cond->Add(ARef);
        Cond->Add(Comp);
        Cond->Add(OneCost);

        // if (a == 1) { a = 11 }
        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Cond);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, IfBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "11", new ASTIntType(SourceLoc))));
        IfBlock->AddVarRef(A2);
        MainFn->getBody()->AddBlock(SourceLoc, IfBlock);

        // elsif (a == 1) { a = 22 }
        ASTElsifBlock *ElsifBlock = new ASTElsifBlock(SourceLoc, MainFn->getBody(), Cond);
        ASTLocalVarRef *A3 = new ASTLocalVarRef(SourceLoc, ElsifBlock, Param);
        A3->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "22", new ASTIntType(SourceLoc))));
        ElsifBlock->AddVarRef(A3);
        IfBlock->AddBranch(MainFn->getBody(), ElsifBlock);

        // elsif (a == 1) { a = 33 }
        ASTElsifBlock *Elsif2Block = new ASTElsifBlock(SourceLoc, MainFn->getBody(), Cond);
        ASTLocalVarRef *A4 = new ASTLocalVarRef(SourceLoc, Elsif2Block, Param);
        A4->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "33", new ASTIntType(SourceLoc))));
        Elsif2Block->AddVarRef(A4);
        IfBlock->AddBranch(MainFn->getBody(), Elsif2Block);

        // else { a = 44 }
        ASTElseBlock *ElseBlock = new ASTElseBlock(SourceLoc, MainFn->getBody());
        ASTLocalVarRef *A5 = new ASTLocalVarRef(SourceLoc, ElseBlock, Param);
        A5->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "44", new ASTIntType(SourceLoc))));
        ElseBlock->AddVarRef(A5);
        IfBlock->AddBranch(MainFn->getBody(), ElseBlock);

        MainFn->getBody()->AddReturn(SourceLoc, ARef);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif:                                            ; preds = %entry\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = icmp eq i32 %4, 1\n"
                          "  br i1 %5, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %6 = load i32, i32* %1, align 4\n"
                          "  %7 = icmp eq i32 %6, 1\n"
                          "  br i1 %7, label %elsifthen2, label %else\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "else:                                             ; preds = %elsif1\n"
                          "  store i32 44, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %else, %elsifthen2, %elsifthen, %ifthen\n"
                          "  %8 = load i32, i32* %1, align 4\n"
                          "  ret i32 %8\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGIfElsifBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Cond = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Cond->Add(ARef);
        Cond->Add(Comp);
        Cond->Add(OneCost);

        // if (a == 1) { a = 11 }
        ASTIfBlock *IfBlock = new ASTIfBlock(SourceLoc, MainFn->getBody(), Cond);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, IfBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "11", new ASTIntType(SourceLoc))));
        IfBlock->AddVarRef(A2);
        MainFn->getBody()->AddBlock(SourceLoc, IfBlock);

        // elsif (a == 1) { a = 22 }
        ASTElsifBlock *ElsifBlock = new ASTElsifBlock(SourceLoc, MainFn->getBody(), Cond);
        ASTLocalVarRef *A3 = new ASTLocalVarRef(SourceLoc, ElsifBlock, Param);
        A3->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "22", new ASTIntType(SourceLoc))));
        ElsifBlock->AddVarRef(A3);
        IfBlock->AddBranch(MainFn->getBody(), ElsifBlock);

        // elsif (a == 1) { a = 33 }
        ASTElsifBlock *Elsif2Block = new ASTElsifBlock(SourceLoc, MainFn->getBody(), Cond);
        ASTLocalVarRef *A4 = new ASTLocalVarRef(SourceLoc, Elsif2Block, Param);
        A4->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "33", new ASTIntType(SourceLoc))));
        Elsif2Block->AddVarRef(A4);
        IfBlock->AddBranch(MainFn->getBody(), Elsif2Block);

        MainFn->getBody()->AddReturn(SourceLoc, ARef);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %ifthen, label %elsif\n"
                          "\n"
                          "ifthen:                                           ; preds = %entry\n"
                          "  store i32 11, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif:                                            ; preds = %entry\n"
                          "  %4 = load i32, i32* %1, align 4\n"
                          "  %5 = icmp eq i32 %4, 1\n"
                          "  br i1 %5, label %elsifthen, label %elsif1\n"
                          "\n"
                          "elsifthen:                                        ; preds = %elsif\n"
                          "  store i32 22, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "elsif1:                                           ; preds = %elsif\n"
                          "  %6 = load i32, i32* %1, align 4\n"
                          "  %7 = icmp eq i32 %6, 1\n"
                          "  br i1 %7, label %elsifthen2, label %endif\n"
                          "\n"
                          "elsifthen2:                                       ; preds = %elsif1\n"
                          "  store i32 33, i32* %1, align 4\n"
                          "  br label %endif\n"
                          "\n"
                          "endif:                                            ; preds = %elsifthen2, %elsif1, %elsifthen, %ifthen\n"
                          "  %8 = load i32, i32* %1, align 4\n"
                          "  ret i32 %8\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGSwitchBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTExpr *Cost1 = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTExpr *Cost2 = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2", new ASTIntType(SourceLoc)));

        ASTVarRefExpr *SwitchExpr = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        ASTSwitchBlock *SwitchBlock = new ASTSwitchBlock(SourceLoc, MainFn->getBody(), SwitchExpr);

        ASTSwitchCaseBlock *Case1Block = SwitchBlock->AddCase(SourceLoc, Cost1);
        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, Case1Block, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        Case1Block->AddVarRef(A2);

        ASTSwitchCaseBlock *Case2Block = SwitchBlock->AddCase(SourceLoc, Cost2);
        ASTLocalVarRef *A3 = new ASTLocalVarRef(SourceLoc, Case2Block, Param);
        A3->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "2", new ASTIntType(SourceLoc))));
        Case2Block->AddVarRef(A3);
        Case2Block->AddBreak(SourceLoc);

        ASTBlock *DefaultBlock = SwitchBlock->setDefault(SourceLoc);
        ASTLocalVarRef *A4 = new ASTLocalVarRef(SourceLoc, DefaultBlock, Param);
        A4->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "3", new ASTIntType(SourceLoc))));
        DefaultBlock->AddVarRef(A4);
        DefaultBlock->AddBreak(SourceLoc);

        MainFn->getBody()->AddBlock(SourceLoc, SwitchBlock);

        MainFn->getBody()->AddReturn(SourceLoc, Cost1);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  switch i32 %2, label %default [\n"
                          "    i32 1, label %case\n"
                          "    i32 2, label %case1\n"
                          "  ]\n"
                          "\n"
                          "case:                                             ; preds = %entry\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %case1\n"
                          "\n"
                          "case1:                                            ; preds = %entry, %case\n"
                          "  store i32 2, i32* %1, align 4\n"
                          "  br label %endswitch\n"
                          "\n"
                          "default:                                          ; preds = %entry\n"
                          "  store i32 3, i32* %1, align 4\n"
                          "  br label %endswitch\n"
                          "\n"
                          "endswitch:                                        ; preds = %default, %case1\n"
                          "  ret i32 1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGWhileBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));
        ASTGroupExpr *Group = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_EQ);
        ASTVarRefExpr *ARef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Group->Add(ARef);
        Group->Add(Comp);
        Group->Add(OneCost);
        ASTWhileBlock *WhileBlock = new ASTWhileBlock(SourceLoc, MainFn->getBody(), Group);

        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, WhileBlock, Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        WhileBlock->AddVarRef(A2);
        WhileBlock->AddContinue(SourceLoc);
        MainFn->getBody()->AddBlock(SourceLoc, WhileBlock);

        MainFn->getBody()->AddReturn(SourceLoc, OneCost);

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  br label %whilecond\n"
                          "\n"
                          "whilecond:                                        ; preds = %whileloop, %entry\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp eq i32 %2, 1\n"
                          "  br i1 %3, label %whileloop, label %whileend\n"
                          "\n"
                          "whileloop:                                        ; preds = %whilecond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %whilecond\n"
                          "\n"
                          "whileend:                                         ; preds = %whilecond\n"
                          "  ret i32 1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForInitCondPostBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        MainFn->setVisibility(V_PRIVATE);
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTForBlock *ForBlock = new ASTForBlock(SourceLoc, MainFn->getBody());
        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));

        // Init
        ASTLocalVar *InitVar = new ASTLocalVar(SourceLoc, ForBlock, new ASTIntType(SourceLoc), "i");
        InitVar->setExpr(OneCost);
        ForBlock->AddVar(InitVar);

        //Cond
        ASTGroupExpr *Cond = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_LTE);
        ASTVarRefExpr *InitVarRef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(InitVar));
        Cond->Add(InitVarRef);
        Cond->Add(Comp);
        Cond->Add(OneCost);
        ForBlock->setCond(Cond);

        // Post
        ASTBlock *PostBlock = ForBlock->getPost();
        ASTArithExpr *Incr = new ASTArithExpr(SourceLoc, ARITH_INCR);
        ASTUnaryExpr *IncrExpr = new ASTUnaryExpr(SourceLoc, Incr, new ASTVarRef(InitVar), UnaryOpKind::UNARY_PRE);
        ASTExprStmt *ExprStmt = new ASTExprStmt(SourceLoc, PostBlock);
        ExprStmt->setExpr(IncrExpr);
        PostBlock->AddExprStmt(ExprStmt);

        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, ForBlock->getLoop(), Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        ForBlock->getLoop()->AddVarRef(A2);
        ForBlock->getLoop()->AddContinue(SourceLoc);
        MainFn->getBody()->AddBlock(SourceLoc, ForBlock);

        MainFn->getBody()->AddReturn(SourceLoc, OneCost);
        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  %2 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  store i32 1, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forpost, %entry\n"
                          "  %3 = load i32, i32* %2, align 4\n"
                          "  %4 = icmp sle i32 %3, 1\n"
                          "  br i1 %4, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %forpost\n"
                          "\n"
                          "forpost:                                          ; preds = %forloop\n"
                          "  %5 = load i32, i32* %2, align 4\n"
                          "  %6 = add nsw i32 %5, 1\n"
                          "  store i32 %6, i32* %2, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  ret i32 1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForCondBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        MainFn->setVisibility(V_PRIVATE);
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTForBlock *ForBlock = new ASTForBlock(SourceLoc, MainFn->getBody());
        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));

        //Cond
        ASTGroupExpr *Cond = new ASTGroupExpr(SourceLoc);
        ASTComparisonExpr *Comp = new ASTComparisonExpr(SourceLoc, COMP_LTE);
        ASTVarRefExpr *InitVarRef = new ASTVarRefExpr(SourceLoc, new ASTVarRef(Param));
        Cond->Add(InitVarRef);
        Cond->Add(Comp);
        Cond->Add(OneCost);
        ForBlock->setCond(Cond);

        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, ForBlock->getLoop(), Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        ForBlock->getLoop()->AddVarRef(A2);
        ForBlock->getLoop()->AddContinue(SourceLoc);
        MainFn->getBody()->AddBlock(SourceLoc, ForBlock);

        MainFn->getBody()->AddReturn(SourceLoc, OneCost);
        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "forcond:                                          ; preds = %forloop, %entry\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = icmp sle i32 %2, 1\n"
                          "  br i1 %3, label %forloop, label %endfor\n"
                          "\n"
                          "forloop:                                          ; preds = %forcond\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %forcond\n"
                          "\n"
                          "endfor:                                           ; preds = %forcond\n"
                          "  ret i32 1\n"
                          "}\n");
    }

    TEST_F(CodeGenTest, CGForPostBlock) {
        ASTNode *Node = CreateAST();

        ASTFunc *MainFn = new ASTFunc(Node, SourceLoc, new ASTIntType(SourceLoc), "main");
        MainFn->setVisibility(V_PRIVATE);
        ASTFuncParam *Param = MainFn->addParam(SourceLoc, new ASTIntType(SourceLoc), "a");
        Node->AddFunction(MainFn);

        ASTForBlock *ForBlock = new ASTForBlock(SourceLoc, MainFn->getBody());
        ASTValueExpr *OneCost = new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc)));

        // Post
        ASTBlock *PostBlock = ForBlock->getPost();
        ASTArithExpr *Incr = new ASTArithExpr(SourceLoc, ARITH_INCR);
        ASTUnaryExpr *IncrExpr = new ASTUnaryExpr(SourceLoc, Incr, new ASTVarRef(Param), UnaryOpKind::UNARY_PRE);
        ASTExprStmt *ExprStmt = new ASTExprStmt(SourceLoc, PostBlock);
        ExprStmt->setExpr(IncrExpr);
        PostBlock->AddExprStmt(ExprStmt);

        ASTLocalVarRef *A2 = new ASTLocalVarRef(SourceLoc, ForBlock->getLoop(), Param);
        A2->setExpr(new ASTValueExpr(SourceLoc, new ASTValue(SourceLoc, "1", new ASTIntType(SourceLoc))));
        ForBlock->getLoop()->AddVarRef(A2);
        ForBlock->getLoop()->AddContinue(SourceLoc);
        MainFn->getBody()->AddBlock(SourceLoc, ForBlock);

        MainFn->getBody()->AddReturn(SourceLoc, OneCost);
        Node->Resolve();

        // Generate Code
        CodeGenModule *CGM = Node->getCodeGen();
        Function *F = CGM->GenFunction(MainFn)->getFunction();
        testing::internal::CaptureStdout();
        F->print(llvm::outs());
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(output, "define i32 @main(i32 %0) {\n"
                          "entry:\n"
                          "  %1 = alloca i32, align 4\n"
                          "  store i32 %0, i32* %1, align 4\n"
                          "  br label %forloop\n"
                          "\n"
                          "forloop:                                          ; preds = %forpost, %entry\n"
                          "  store i32 1, i32* %1, align 4\n"
                          "  br label %forpost\n"
                          "\n"
                          "forpost:                                          ; preds = %forloop\n"
                          "  %2 = load i32, i32* %1, align 4\n"
                          "  %3 = add nsw i32 %2, 1\n"
                          "  store i32 %3, i32* %1, align 4\n"
                          "  br label %forloop\n"
                          "\n"
                          "endfor:                                           ; No predecessors!\n"
                          "  ret i32 1\n"
                          "}\n");
    }

} // anonymous namespace
