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
#include "AST/ASTModule.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTNameSpace.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenClass::CodeGenClass(CodeGenModule *CGM, ASTClass *Class, bool isExternal) : CGM(CGM), AST(Class) {
    std::string TypeName = CodeGen::toIdentifier(Class->getName(), Class->getNameSpace()->getName());

    // Generate Class Type
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

        // Default Constructor
//        if (AST->getDefaultConstructor()) {
//            // Create Constructor CodeGen for Constructor
//            CodeGenClassFunction *CGCF = new CodeGenClassFunction(CGM, AST->getDefaultConstructor(), TypePtr);
//            AST->getDefaultConstructor()->setCodeGen(CGCF);
//            Constructors.push_back(CGCF);
//        }

        // Add Constructors
        for (auto &Constructor: AST->getConstructors()) {
            // Create Constructor CodeGen for Constructor
            CodeGenClassFunction *CGCF = new CodeGenClassFunction(CGM, Constructor, TypePtr);
            Constructor->setCodeGen(CGCF);
            Constructors.push_back(CGCF);
        }
    }

    // Create the Main StructType
    llvm::SmallVector<llvm::Type *, 4> TypeVector;

    // Set CodeGen Methods
    if (AST->getClassKind() == ASTClassKind::CLASS || AST->getClassKind() == ASTClassKind::INTERFACE) {
        llvm::SmallVector<llvm::Type *, 4> VTableVector;
        for (auto &Method: AST->getMethods()) {
            CodeGenClassFunction *CGCF = new CodeGenClassFunction(CGM, Method, TypePtr);
            if (!Method->isStatic()) { // only instance methods
                // Create the VTable Struct Type
                // %vtable_type = type { i32(%Foo*)* }
                VTableVector.push_back(CGCF->getFunctionType());
            }
            Method->setCodeGen(CGCF);
            Functions.push_back(CGCF);
        }
        VTableType->setBody(VTableVector);

        // Add VTable as First element
        TypeVector.push_back(VTableType->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace()));
    }

    // Set CodeGen Attributes
    if (!AST->getAttributes().empty()) {
        if (AST->getClassKind() == ASTClassKind::CLASS || AST->getClassKind() == ASTClassKind::STRUCT) {

            // add var to the type
            for (auto &Attribute: AST->getAttributes()) {
                llvm::Type *AttrType = CGM->GenType(Attribute->getType());
                TypeVector.push_back(AttrType);
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

//const SmallVector<CodeGenClassVar *, 4> &CodeGenClass::getAttributes() const {
//    return Attributes;
//}

const SmallVector<CodeGenClassFunction *, 4> &CodeGenClass::getConstructors() const {
    return Constructors;
}

const SmallVector<CodeGenClassFunction *, 4> &CodeGenClass::getFunctions() const {
    return Functions;
}
