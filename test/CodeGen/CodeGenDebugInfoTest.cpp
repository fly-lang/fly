//===--------------------------------------------------------------------------------------------------------------===//
// test/CodeGen/CodeGenDebugInfoTest.cpp - DWARF debug info unit tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTDeclStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTModule.h"
#include "AST/ASTParam.h"
#include "AST/ASTType.h"
#include "AST/ASTValue.h"
#include "AST/ASTBinary.h"
#include "CodeGenTest.h"

#include "llvm/IR/DebugInfoMetadata.h"

namespace {

using namespace fly;

// Test fixture that enables debug symbol generation
class CodeGenDebugInfoTest : public CodeGenTest {
protected:
    void SetUp() override {
        CodeGenTest::SetUp();
        CI->getCodeGenOptions().DebugSymbols = true;
        CG.setSourceManager(CI->getSourceManager());
    }

    void TearDown() override {
        CI->getCodeGenOptions().DebugSymbols = false;
        CodeGenTest::TearDown();
    }
};

// Verify DICompileUnit is emitted with correct language
TEST_F(CodeGenDebugInfoTest, CompileUnitPresent) {
    ASTModule *Module = CreateModule("dbg_cu");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("!DICompileUnit"), std::string::npos)
        << "Expected DICompileUnit in IR";
    EXPECT_NE(IR.find("DW_LANG_C"), std::string::npos)
        << "Expected DW_LANG_C language tag";
    EXPECT_NE(IR.find("producer: \"Fly Compiler\""), std::string::npos)
        << "Expected Fly Compiler producer string";
}

// Verify DIFile entry is emitted
TEST_F(CodeGenDebugInfoTest, FilePresent) {
    ASTModule *Module = CreateModule("dbg_file");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTBuilder::CreateFunction(Module, SourceLoc, "func", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("!DIFile("), std::string::npos)
        << "Expected DIFile in IR";
    EXPECT_NE(IR.find("dbg_file"), std::string::npos)
        << "Expected source filename in DIFile";
}

// Verify each function has a DISubprogram attached
TEST_F(CodeGenDebugInfoTest, FunctionHasSubprogram) {
    ASTModule *Module = CreateModule("dbg_fn");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTBuilder::CreateFunction(Module, SourceLoc, "myFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("!DISubprogram"), std::string::npos)
        << "Expected DISubprogram in IR";
    EXPECT_NE(IR.find("DISPFlagDefinition"), std::string::npos)
        << "Expected DISPFlagDefinition on subprogram";
}

// Verify that each function object has a non-null subprogram set
TEST_F(CodeGenDebugInfoTest, FunctionSubprogramNotNull) {
    ASTModule *Module = CreateModule("dbg_sp");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTBuilder::CreateFunction(Module, SourceLoc, "spFunc", TopModifiers, Params, Body);

    Generate();
    llvm::Module *M = Modules[0];

    bool found = false;
    for (auto &F : *M) {
        if (F.isDeclaration()) continue;
        EXPECT_NE(F.getSubprogram(), nullptr)
            << "Function " << F.getName().str() << " has no subprogram";
        found = true;
    }
    EXPECT_TRUE(found) << "No non-declaration functions found in module";
}

// Verify instructions have debug locations (every non-alloca instr has !dbg)
TEST_F(CodeGenDebugInfoTest, InstructionsHaveDebugLocation) {
    ASTModule *Module = CreateModule("dbg_loc");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);

    ASTLocalVar *Var = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "x", EmptyModifiers);
    ASTDeclStmt *Decl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, Var);
    ASTValue *Val = ASTBuilder::CreateNumberValue(SourceLoc, "5");
    ASTBinary *Assign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                  ASTBuilder::CreateIdentifier(Var), Val);
    Decl->setExpr(Assign);

    ASTBuilder::CreateFunction(Module, SourceLoc, "locFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("!dbg"), std::string::npos)
        << "Expected !dbg debug location metadata on instructions";
    EXPECT_NE(IR.find("!DILocation"), std::string::npos)
        << "Expected !DILocation entries in IR";
}

// Verify a local variable has a dbg.declare intrinsic
TEST_F(CodeGenDebugInfoTest, LocalVarHasDeclareMeta) {
    ASTModule *Module = CreateModule("dbg_var");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);

    ASTLocalVar *Var = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "myVar", EmptyModifiers);
    ASTDeclStmt *Decl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, Var);
    ASTValue *Val = ASTBuilder::CreateNumberValue(SourceLoc, "99");
    ASTBinary *Assign = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                                  ASTBuilder::CreateIdentifier(Var), Val);
    Decl->setExpr(Assign);

    ASTBuilder::CreateFunction(Module, SourceLoc, "varFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("dbg_declare") != std::string::npos || IR.find("dbg.declare") != std::string::npos
              ? IR.find("dbg_declare") : IR.find("dbg.declare"), std::string::npos)
        << "Expected dbg.declare (or #dbg_declare) for local variable";
    EXPECT_NE(IR.find("!DILocalVariable"), std::string::npos)
        << "Expected DILocalVariable metadata for local variable";
    EXPECT_NE(IR.find("\"myVar\""), std::string::npos)
        << "Expected variable name 'myVar' in debug metadata";
}

// Verify DIBasicType for int
TEST_F(CodeGenDebugInfoTest, BasicTypeInt) {
    ASTModule *Module = CreateModule("dbg_int");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTLocalVar *Var = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "i", EmptyModifiers);
    ASTDeclStmt *Decl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, Var);
    ASTValue *Val = ASTBuilder::CreateNumberValue(SourceLoc, "1");
    Decl->setExpr(ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                            ASTBuilder::CreateIdentifier(Var), Val));
    ASTBuilder::CreateFunction(Module, SourceLoc, "intFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("DIBasicType(name: \"int\", size: 32, encoding: DW_ATE_signed)"), std::string::npos)
        << "Expected DIBasicType for int (32-bit signed)";
}

// Verify DIBasicType for bool
TEST_F(CodeGenDebugInfoTest, BasicTypeBool) {
    ASTModule *Module = CreateModule("dbg_bool");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTLocalVar *Var = ASTBuilder::CreateLocalVar(SourceLoc, BoolTypeRef, "b", EmptyModifiers);
    ASTDeclStmt *Decl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, Var);
    ASTValue *Val = ASTBuilder::CreateBoolValue(SourceLoc, true);
    Decl->setExpr(ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                            ASTBuilder::CreateIdentifier(Var), Val));
    ASTBuilder::CreateFunction(Module, SourceLoc, "boolFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("DIBasicType(name: \"bool\""), std::string::npos)
        << "Expected DIBasicType for bool";
    EXPECT_NE(IR.find("DW_ATE_boolean"), std::string::npos)
        << "Expected DW_ATE_boolean encoding for bool";
}

// Verify DIBasicType for float
TEST_F(CodeGenDebugInfoTest, BasicTypeFloat) {
    ASTModule *Module = CreateModule("dbg_float");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTLocalVar *Var = ASTBuilder::CreateLocalVar(SourceLoc, FloatTypeRef, "f", EmptyModifiers);
    ASTDeclStmt *Decl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, Var);
    ASTValue *Val = ASTBuilder::CreateNumberValue(SourceLoc, "1.0");
    Decl->setExpr(ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                            ASTBuilder::CreateIdentifier(Var), Val));
    ASTBuilder::CreateFunction(Module, SourceLoc, "floatFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("DIBasicType(name: \"float\", size: 32, encoding: DW_ATE_float)"), std::string::npos)
        << "Expected DIBasicType for float (32-bit)";
}

// Verify no debug info emitted when DebugSymbols is false
TEST_F(CodeGenDebugInfoTest, NoDebugInfoWhenDisabled) {
    CI->getCodeGenOptions().DebugSymbols = false;

    ASTModule *Module = CreateModule("dbg_nodebug");
    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTBuilder::CreateFunction(Module, SourceLoc, "noDbgFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_EQ(IR.find("!DISubprogram"), std::string::npos)
        << "Expected no DISubprogram when DebugSymbols is disabled";
    EXPECT_EQ(IR.find("!DICompileUnit"), std::string::npos)
        << "Expected no DICompileUnit when DebugSymbols is disabled";
}

} // namespace
