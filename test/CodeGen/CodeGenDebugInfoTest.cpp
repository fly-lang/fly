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
#include "AST/ASTBuilderIfStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
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

// Verify DIEnumerationType is emitted for an enum variable
TEST_F(CodeGenDebugInfoTest, EnumTypeHasDIEnumerationType) {
    ASTModule *Module = CreateModule("dbg_enum");

    llvm::SmallVector<ASTType *, 4> SuperEnums;
    ASTEnum *MyEnum = ASTBuilder::CreateEnum(Module, SourceLoc, "Color", TopModifiers, SuperEnums);
    ASTBuilder::CreateEnumEntry(SourceLoc, MyEnum, "Red",   EmptyModifiers);
    ASTBuilder::CreateEnumEntry(SourceLoc, MyEnum, "Green", EmptyModifiers);
    ASTBuilder::CreateEnumEntry(SourceLoc, MyEnum, "Blue",  EmptyModifiers);

    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTType *EnumTypeRef = CreateType(MyEnum);
    ASTLocalVar *Var = ASTBuilder::CreateLocalVar(SourceLoc, EnumTypeRef, "c", EmptyModifiers);
    ASTDeclStmt *Decl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, Var);
    ASTIdentifier *EnumIdent = ASTBuilder::CreateIdentifier(SourceLoc, MyEnum->getName());
    ASTMember *RedMember = ASTBuilder::CreateMember(SourceLoc, "Red", EnumIdent);
    Decl->setExpr(ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                           ASTBuilder::CreateIdentifier(Var), RedMember));
    ASTBuilder::CreateFunction(Module, SourceLoc, "enumFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("DW_TAG_enumeration_type"), std::string::npos)
        << "Expected DW_TAG_enumeration_type in IR";
    EXPECT_NE(IR.find("\"Color\""), std::string::npos)
        << "Expected enum name 'Color' in DWARF metadata";
    EXPECT_NE(IR.find("\"Red\""), std::string::npos)
        << "Expected enum entry 'Red' in DWARF enumerators";
}

// Verify a DIPointerType is emitted for an array-typed parameter
TEST_F(CodeGenDebugInfoTest, ArrayTypeHasDIPointerType) {
    ASTModule *Module = CreateModule("dbg_arr");

    llvm::SmallVector<ASTParam *, 8> FuncParams;
    ASTArrayType *ArrTy = ASTBuilder::CreateArrayType(SourceLoc, IntTypeRef, nullptr);
    FuncParams.push_back(ASTBuilder::CreateParam(SourceLoc, ArrTy, "arr", EmptyModifiers));

    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTBuilder::CreateFunction(Module, SourceLoc, "arrFunc", TopModifiers, FuncParams, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("DW_TAG_pointer_type"), std::string::npos)
        << "Expected DW_TAG_pointer_type for array parameter in DWARF metadata";
}

// Verify DIStructType is emitted for a class with an attribute
TEST_F(CodeGenDebugInfoTest, ClassTypeHasDIStructType) {
    ASTModule *Module = CreateModule("dbg_cls");

    llvm::SmallVector<ASTType *, 4> SuperClasses;
    ASTClass *MyClass = ASTBuilder::CreateClass(Module, SourceLoc, ASTClassKind::CLASS,
                                                "Point", TopModifiers, SuperClasses);
    ASTBuilder::CreateClassAttribute(SourceLoc, MyClass, IntTypeRef, "x", TopModifiers);
    ASTBuilder::CreateClassAttribute(SourceLoc, MyClass, IntTypeRef, "y", TopModifiers);

    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTType *ClassTypeRef = CreateType(MyClass);
    ASTLocalVar *Var = ASTBuilder::CreateLocalVar(SourceLoc, ClassTypeRef, "pt", EmptyModifiers);
    ASTDeclStmt *Decl = ASTBuilder::CreateDeclStmt(Body, SourceLoc, Var);
    ASTCall *CtorCall = ASTBuilder::CreateCall(SourceLoc, MyClass->getName(), Args, ASTCallKind::CALL_NEW);
    Decl->setExpr(ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                           ASTBuilder::CreateIdentifier(Var), CtorCall));
    ASTBuilder::CreateDeleteStmt(Body, SourceLoc, ASTBuilder::CreateIdentifier(Var));
    ASTBuilder::CreateFunction(Module, SourceLoc, "classFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("DW_TAG_structure_type"), std::string::npos)
        << "Expected DW_TAG_structure_type for class 'Point' in DWARF metadata";
    EXPECT_NE(IR.find("\"Point\""), std::string::npos)
        << "Expected class name 'Point' in DWARF metadata";
}

// Verify DILexicalBlock is emitted for an if-body block
TEST_F(CodeGenDebugInfoTest, LexicalBlockPresentInIfBody) {
    ASTModule *Module = CreateModule("dbg_lex");

    ASTBlockStmt *Body = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTLocalVar *A = ASTBuilder::CreateLocalVar(SourceLoc, IntTypeRef, "a", EmptyModifiers);
    ASTDeclStmt *DeclA = ASTBuilder::CreateDeclStmt(Body, SourceLoc, A);
    DeclA->setExpr(ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                            ASTBuilder::CreateIdentifier(A),
                                            ASTBuilder::CreateNumberValue(SourceLoc, "1")));

    ASTBinary *Cond = ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_COMPARE_EQ,
                                               ASTBuilder::CreateIdentifier(A),
                                               ASTBuilder::CreateNumberValue(SourceLoc, "1"));
    ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(SourceLoc);
    ASTExprStmt *ExprS = ASTBuilder::CreateExprStmt(IfBlock, SourceLoc);
    ExprS->setExpr(ASTBuilder::CreateBinary(SourceLoc, ASTBinaryKind::OP_BINARY_ASSIGN,
                                            ASTBuilder::CreateIdentifier(A),
                                            ASTBuilder::CreateNumberValue(SourceLoc, "2")));
    ASTBuilderIfStmt::Create(Body)->If(SourceLoc, Cond, IfBlock);

    ASTBuilder::CreateFunction(Module, SourceLoc, "lexFunc", TopModifiers, Params, Body);

    Generate();
    std::string IR = getOutput(Modules[0]);

    EXPECT_NE(IR.find("DILexicalBlock"), std::string::npos)
        << "Expected DILexicalBlock in IR for if-body block scope";
}

} // namespace
