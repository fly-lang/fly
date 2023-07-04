//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClass.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTNameSpace.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, ASTClass *Class, bool isExternal) : CGM(CGM), AST(Class) {
    std::string TypeName = CodeGen::toIdentifier(Class->getName(), Class->getNameSpace()->getName());
    Type = llvm::StructType::create(CGM->LLVMCtx, TypeName);
    TypePtr = Type->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());

    // Generate VTable from Class and Interface
    if (Class->getClassKind() == ASTClassKind::CLASS || Class->getClassKind() == ASTClassKind::INTERFACE) {
        VTableType = llvm::StructType::create(CGM->LLVMCtx, TypeName + "_vtable");
    }
}

void CodeGenClass::Generate() {

    // Generate Constructors
    if (AST->getClassKind() == ASTClassKind::CLASS || AST->getClassKind() == ASTClassKind::STRUCT) {
        for (auto &Entry: AST->getConstructors()) {
            for (auto Constructor: Entry.second) {
                // Create Constructor CodeGen for Constructor
                CodeGenClassFunction *CGCF = new CodeGenClassFunction(CGM, Constructor, TypePtr);
                Constructor->setCodeGen(CGCF);
                Constructors.push_back(Constructor->getCodeGen());
            }
        }
    }

    // Create the Main StructType
    llvm::SmallVector<llvm::Type *, 4> TypeVector;

    // Generate Methods
    if (AST->getClassKind() == ASTClassKind::CLASS || AST->getClassKind() == ASTClassKind::INTERFACE) {
        llvm::SmallVector<llvm::Type *, 4> VTableVector;
        for (auto &Map: AST->getMethods()) {
            for (auto &Entry: Map.second) {
                for (auto Method: Entry.second) {
                    CodeGenClassFunction *CGCF = new CodeGenClassFunction(CGM, Method, TypePtr);
                    if (!Method->isStatic()) { // only instance methods
                        // Create the VTable Struct Type
                        // %vtable_type = type { i32(%Foo*)* }
                        VTableVector.push_back(CGCF->getFunctionType());
                    }
                    Method->setCodeGen(CGCF);
                    Functions.push_back(CGCF);
                }
            }
        }
        VTableType->setBody(VTableVector);

        // Add VTable as First element
        TypeVector.push_back(VTableType->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace()));
    }

    // Set CodeGen ClassVar
    if (!AST->getVars().empty()) {
        if (AST->getClassKind() == ASTClassKind::CLASS || AST->getClassKind() == ASTClassKind::STRUCT) {

            // Set var Index offset in the struct type
            uint32_t Index = AST->getClassKind() == ASTClassKind::CLASS ? 1 : 0;

            // add var to the type
            for (auto &Var: AST->getVars()) {
                llvm::Type *FieldType = CGM->GenType(Var.second->getType());
                TypeVector.push_back(FieldType);
                CodeGenClassVar *CGV = new CodeGenClassVar(CGM, Var.second, Type, Index++);
                Var.second->setCodeGen(CGV);
                Vars.push_back(CGV);
            }
        }
    }

    // %type = type { %fields_type, %vtable_type }
    Type->setBody(TypeVector);
}

llvm::StructType *CodeGenClass::getType() {
    return Type;
}

llvm::PointerType *CodeGenClass::getTypePtr() {
    return TypePtr;
}

llvm::StructType *CodeGenClass::getVTableType() {
    return VTableType;
}

const SmallVector<CodeGenClassVar *, 4> &CodeGenClass::getVars() const {
    return Vars;
}

const SmallVector<CodeGenClassFunction *, 4> &CodeGenClass::getConstructors() const {
    return Constructors;
}

const SmallVector<CodeGenClassFunction *, 4> &CodeGenClass::getFunctions() const {
    return Functions;
}
