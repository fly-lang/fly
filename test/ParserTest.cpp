//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "AST/ASTContext.h"
#include <gtest/gtest.h>

namespace {
    using namespace fly;

    class ParserTest : public ::testing::Test {

    public:

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;
        ASTContext *Context;

        ParserTest(): FileMgr(FileMgrOpts),
                      DiagID(new DiagnosticIDs()),
                      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                      SourceMgr(Diags, FileMgr), Context(new ASTContext(Diags)) {
        }

        virtual ~ParserTest() {
            delete Context;
        }

        std::unique_ptr<Parser> Parse(llvm::StringRef FileName, StringRef Source) {

            // Set Source Manager file id
            std::unique_ptr<llvm::MemoryBuffer> Buf = llvm::MemoryBuffer::getMemBuffer(Source);
            llvm::MemoryBuffer *b = Buf.get();
            const FileID &FID = SourceMgr.createFileID(std::move(Buf));
            SourceMgr.setMainFileID(FID);

            // Create a lexer starting at the beginning of this token.
            Lexer TheLexer(FID, b, SourceMgr);
            std::unique_ptr<Parser> P = std::make_unique<Parser>(TheLexer, Diags);
            ASTNode *AST = new ASTNode(FileName, FID, Context);
            P->Parse(AST);
            AST->Finalize();
            return P;
        }

    };

    TEST_F(ParserTest, SinglePackage) {
        StringRef str = ("namespace std");
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();

        EXPECT_EQ(AST->getFileName(), "package.fly");

        // verify AST contains package
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "std");
    }

    TEST_F(ParserTest, MultiPackageError) {
        StringRef str = ("namespace std\n"
                         "namespace bad");
        auto P = Parse("error.fly", str);

        EXPECT_TRUE(Diags.hasErrorOccurred());
    }

    TEST_F(ParserTest, SingleImport) {
        StringRef str = ("namespace std\n"
                         "import \"packageA\"");
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();

        ImportDecl* Verify = AST->getImports().lookup("packageA");

        EXPECT_EQ(Verify->getName(), "packageA");
        EXPECT_EQ(Verify->getAlias(), Verify->getName());
    }

    TEST_F(ParserTest, SingleImportAlias) {
        StringRef str = ("\n import  \"standard\" as \"std\"\n");
        auto P = Parse("package.fly", str);
        auto AST = P->getAST();
        EXPECT_EQ(AST->getNameSpace()->getNameSpace(), "default");
        EXPECT_EQ(AST->getImports().lookup("standard")->getName(), "standard");
        EXPECT_EQ(AST->getImports().lookup("standard")->getAlias(), "std");
    }

    TEST_F(ParserTest, MultiImports) {
        StringRef str = ("namespace std\n"
                         "import \"packageA\""
                         "import \"packageB\"");
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();

        ImportDecl* VerifyB = AST->getImports().lookup("packageB");
        ImportDecl* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
    }

    TEST_F(ParserTest, SingleParenImport) {
        StringRef str = ("namespace std\n"
                         "import (\"packageA\")");
        auto P = Parse("import.fly", str);
        auto AST = P->getAST();

        ImportDecl* Verify = AST->getImports().lookup("packageA");
        EXPECT_EQ(Verify->getName(), "packageA");
    }

    TEST_F(ParserTest, MultiParenImports) {
        StringRef str = ("namespace std\n"
                         "import (\"packageA\", \"packageB\")");
        auto P = Parse("imports.fly", str);
        auto AST = P->getAST();

        ImportDecl* VerifyB = AST->getImports().lookup("packageB");
        ImportDecl* VerifyA = AST->getImports().lookup("packageA");

        EXPECT_EQ(VerifyA->getName(), "packageA");
        EXPECT_EQ(VerifyB->getName(), "packageB");
    }

    TEST_F(ParserTest, GlobalVars) {
        StringRef str = ("namespace std\n"
                         "private int a\n"
                         "public float b\n"
                         "bool c\n"
                         );
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();

        GlobalVarDecl *VerifyA = AST->getVars().lookup("a");
        GlobalVarDecl *VerifyB = AST->getVars().lookup("b");
        GlobalVarDecl *VerifyC = AST->getVars().lookup("c");

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_FALSE(VerifyA->isConstant());
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_FALSE(VerifyB->isConstant());
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyC->isConstant());
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");

        delete AST;
    }

    TEST_F(ParserTest, GlobalConstants) {
        StringRef str = ("namespace std\n"
                         "private const int a = 1\n"
                         "const public float b = 2.0\n"
                         "const bool c = false\n");
        auto P = Parse("var.fly", str);
        auto AST = P->getAST();
        GlobalVarDecl *VerifyA = AST->getVars().lookup("a");
        GlobalVarDecl *VerifyB = AST->getVars().lookup("b");
        GlobalVarDecl *VerifyC = AST->getVars().lookup("c");

        EXPECT_EQ(VerifyA->getVisibility(), VisibilityKind::V_PRIVATE);
        EXPECT_EQ(VerifyA->isConstant(), true);
        EXPECT_EQ(VerifyA->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(VerifyA->getName(), "a");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyA->getExpr()->getGroup()[0])->getString(), "1");

        EXPECT_EQ(VerifyB->getVisibility(), VisibilityKind::V_PUBLIC);
        EXPECT_EQ(VerifyB->isConstant(), true);
        EXPECT_EQ(VerifyB->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(VerifyB->getName(), "b");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyB->getExpr()->getGroup()[0])->getString(), "2.0");

        EXPECT_EQ(VerifyC->getVisibility(), VisibilityKind::V_DEFAULT);
        EXPECT_EQ(VerifyC->isConstant(), true);
        EXPECT_EQ(VerifyC->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(VerifyC->getName(), "c");
        EXPECT_EQ(static_cast<ValueExpr*>(VerifyC->getExpr()->getGroup()[0])->getString(), "false");
    }

    TEST_F(ParserTest, FunctionDefaultVoidEmpty) {
        StringRef str = ("namespace std\n"
                         "void func() {}\n");
        auto P = Parse("function.fly", str);
        auto AST = P->getAST();

        FuncDecl *VerifyFunc = AST->getFunctions().lookup("func");
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_DEFAULT);
        ASSERT_FALSE(VerifyFunc->isConstant());
        ASSERT_TRUE(Context->getNameSpaces().lookup("std")->getFunctions().lookup("func"));
    }

    TEST_F(ParserTest, FunctionPrivateReturnParams) {
        StringRef str = ("namespace std\n"
                         "private int func(int a, const float b, bool c=false) {\n"
                         "  return 1"
                         "}\n");
        auto P = Parse("function.fly", str);
        auto AST = P->getAST();

        FuncDecl *VerifyFunc = AST->getFunctions().lookup("func");
        EXPECT_EQ(VerifyFunc->getVisibility(), VisibilityKind::V_PRIVATE);
        ASSERT_FALSE(VerifyFunc->isConstant());
        ASSERT_FALSE(Context->getNameSpaces().lookup("std")->getFunctions().lookup("func"));

        VarDecl *Arg0 = VerifyFunc->getParams()->getVars()[0];
        VarDecl *Arg1 = VerifyFunc->getParams()->getVars()[1];
        VarDecl *Arg2 = VerifyFunc->getParams()->getVars()[2];

        EXPECT_EQ(Arg0->getName(), "a");
        EXPECT_EQ(Arg0->getType()->getKind(), TypeKind::TYPE_INT);
        EXPECT_EQ(Arg0->isConstant(), false);

        EXPECT_EQ(Arg1->getName(), "b");
        EXPECT_EQ(Arg1->getType()->getKind(), TypeKind::TYPE_FLOAT);
        EXPECT_EQ(Arg1->isConstant(), true);

        EXPECT_EQ(Arg2->getName(), "c");
        EXPECT_EQ(Arg2->getType()->getKind(), TypeKind::TYPE_BOOL);
        EXPECT_EQ(Arg2->isConstant(), false);
        EXPECT_EQ(Arg2->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        ValueExpr *DefArg2 = static_cast<ValueExpr *>(Arg2->getExpr()->getGroup()[0]);
        EXPECT_EQ(DefArg2->getString(), "false");

        GroupExpr *Return = VerifyFunc->getBody()->getReturn()->getExpr();
        EXPECT_EQ(static_cast<ValueExpr*>(Return->getGroup()[0])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr*>(Return->getGroup()[0])->getString(), "1");
        EXPECT_EQ(Return->getKind(), ExprKind::EXPR_GROUP);
    }

    TEST_F(ParserTest, FunctionBodyVar) {
        StringRef str = ("namespace std\n"
                         "bool func(int a) {\n"
                         "  bool c"
                         "  Type t"
                         "  float b=2.0 + 1.0"
                         "  a += 2"
                         "  c = b == 1.0"
                         "  return c\n"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const StmtDecl *Body = F->getBody();

        // Test: bool c
        const VarDecl *cVar = static_cast<VarDecl *>(Body->getIstructions()[0]);
        EXPECT_EQ(cVar->getName(), "c");
        EXPECT_EQ(cVar->getType()->getKind(), TypeKind::TYPE_BOOL);
        ASSERT_FALSE(cVar->getExpr());

        // Test: Type t
        const VarDecl *typeVar = static_cast<VarDecl *>(Body->getIstructions()[1]);
        EXPECT_EQ(typeVar->getName(), "t");
        EXPECT_EQ(typeVar->getType()->getKind(), TypeKind::TYPE_CLASS);
        ClassTypeRef *TypeT = static_cast<ClassTypeRef *>(typeVar->getType());
        EXPECT_EQ(TypeT->getName(), "Type");
        ASSERT_FALSE(TypeT->getDecl());

        // Test: float b=2.0 + 1.0
        const VarDecl *bVar = static_cast<VarDecl *>(Body->getIstructions()[2]);
        EXPECT_EQ(bVar->getName(), "b");
        EXPECT_EQ(bVar->getType()->getKind(), TypeKind::TYPE_FLOAT);
        FloatPrimType *FloatType = static_cast<FloatPrimType *>(bVar->getType());
        EXPECT_EQ(bVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(bVar->getExpr()->getGroup()[0]->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(bVar->getExpr()->getGroup()[0])->getString(), "2.0");
        EXPECT_EQ(bVar->getExpr()->getGroup()[1]->getKind(), ExprKind::EXPR_OPERATOR);
        EXPECT_EQ(bVar->getExpr()->getGroup()[2]->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(bVar->getExpr()->getGroup()[2])->getString(), "1.0");

        // Test: a += 2
        const VarRefDecl *aVar = static_cast<VarRefDecl *>(Body->getIstructions()[3]);
        EXPECT_EQ(aVar->getName(), "a");
        EXPECT_EQ(aVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        VarRefExpr *aExpr = static_cast<VarRefExpr *>(aVar->getExpr()->getGroup()[0]);
        EXPECT_FALSE(aExpr->getRef()->getDecl());
        ArithExpr *opCExpr = static_cast<ArithExpr *>(aVar->getExpr()->getGroup()[1]);
        EXPECT_EQ(opCExpr->getArithKind(), ArithOpKind::ARITH_ADD);
        ValueExpr *opC1Expr = static_cast<ValueExpr *>(aVar->getExpr()->getGroup()[2]);
        EXPECT_EQ(opC1Expr->getString(), "2");

        // Test: c = b == 1.0
        const VarRefDecl *c2Var = static_cast<VarRefDecl *>(Body->getIstructions()[4]);
        EXPECT_EQ(c2Var->getName(), "c");
        EXPECT_EQ(c2Var->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        VarRefExpr *c2Expr = static_cast<VarRefExpr *>(c2Var->getExpr()->getGroup()[0]);
        EXPECT_EQ(static_cast<VarRef *>(c2Expr->getRef())->getName(), "b");
        LogicExpr *opC2Expr = static_cast<LogicExpr *>(c2Var->getExpr()->getGroup()[1]);
        EXPECT_EQ(opC2Expr->getLogicKind(), LogicOpKind::LOGIC_EQ);

        const ReturnDecl *Ret = static_cast<ReturnDecl *>(Body->getIstructions()[5]);
        VarRefExpr *RetRef = static_cast<VarRefExpr *>(Ret->getExpr()->getGroup()[0]);
        EXPECT_EQ(RetRef->getRef()->getName(), "c");
    }

    TEST_F(ParserTest, FunctionBodyCallFunc) {
        StringRef str = ("namespace std\n"
                         "void func(int a) {\n"
                         "  doSome()"
                         "  doOther(a, b)"
                         "  return do(false, 1, 2.0)"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const StmtDecl *Body = F->getBody();

        // Test: doSome()
        const FuncRefDecl *doSome = static_cast<FuncRefDecl *>(Body->getIstructions()[0]);
        //EXPECT_FALSE(F->getFuncRef().find("doSome")->getValue()->getRef());
        EXPECT_EQ(doSome->getName(), "doSome");
        EXPECT_EQ(doSome->getKind(), DeclKind::R_FUNCTION);
        ASSERT_FALSE(doSome->getDecl());

        // Test: doOther(a, b)
        const FuncRefDecl *doOther = static_cast<FuncRefDecl *>(Body->getIstructions()[1]);
        //EXPECT_FALSE(F->getFuncRef().find("doOther")->getValue()->getRef());
        EXPECT_EQ(doOther->getName(), "doOther");
        EXPECT_EQ(doOther->getKind(), DeclKind::R_FUNCTION);
        EXPECT_EQ(static_cast<VarRefExpr *>(doOther->getParams()->getArgs()[0])->getRef()->getName(),"a");
        EXPECT_EQ(static_cast<VarRefExpr *>(doOther->getParams()->getArgs()[1])->getRef()->getName(),"b");
        ASSERT_FALSE(doOther->getDecl());

        const ReturnDecl *Ret = static_cast<ReturnDecl *>(Body->getIstructions()[2]);
        FuncRefExpr *RetRef = static_cast<FuncRefExpr *>(Ret->getExpr()->getGroup()[0]);
        EXPECT_EQ(RetRef->getRef()->getName(), "do");
        EXPECT_EQ(RetRef->getKind(), DeclKind::R_FUNCTION);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[0])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[0])->getString(),"false");
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[1])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[1])->getString(), "1");
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[2])->getKind(), ExprKind::EXPR_VALUE);
        EXPECT_EQ(static_cast<ValueExpr *>(RetRef->getRef()->getParams()->getArgs()[2])->getString(), "2.0");
    }

    TEST_F(ParserTest, FunctionBodyIncDec) {
        StringRef str = ("namespace std\n"
                         "void func() {\n"
                         "  ++a"
                         "  b++"
                         "  --c"
                         "  d--"
                         "}\n");
        auto P = Parse("fbody.fly", str);
        auto AST = P->getAST();

        // Get Body
        FuncDecl *F = AST->getFunctions().lookup("func");
        const StmtDecl *Body = F->getBody();

        // ++a
        const VarRefDecl *aVar = static_cast<VarRefDecl *>(Body->getIstructions()[0]);
        EXPECT_EQ(aVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(aVar->getName(), "a");
        IncDecExpr *aExpr = static_cast<IncDecExpr *>(aVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(aExpr->getIncDecKind(), IncDecOpKind::PRE_INCREMENT);

        // b++
        const VarRefDecl *bVar = static_cast<VarRefDecl *>(Body->getIstructions()[1]);
        EXPECT_EQ(bVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(bVar->getName(), "b");
        IncDecExpr *bExpr = static_cast<IncDecExpr *>(bVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(bExpr->getIncDecKind(), IncDecOpKind::POST_INCREMENT);

        // ++c
        const VarRefDecl *cVar = static_cast<VarRefDecl *>(Body->getIstructions()[2]);
        EXPECT_EQ(cVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(cVar->getName(), "c");
        IncDecExpr *cExpr = static_cast<IncDecExpr *>(cVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(cExpr->getIncDecKind(), IncDecOpKind::PRE_DECREMENT);

        // d++
        const VarRefDecl *dVar = static_cast<VarRefDecl *>(Body->getIstructions()[3]);
        EXPECT_EQ(dVar->getExpr()->getKind(), ExprKind::EXPR_GROUP);
        EXPECT_EQ(dVar->getName(), "d");
        IncDecExpr *dExpr = static_cast<IncDecExpr *>(dVar->getExpr()->getGroup()[0]);
        EXPECT_EQ(dExpr->getIncDecKind(), IncDecOpKind::POST_DECREMENT);
    }
}