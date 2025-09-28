//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/Class.h - Code Generator of Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASS_H
#define FLY_CODEGEN_CLASS_H

#include "CodeGenClassMethod.h"
#include "llvm/ADT/ArrayRef.h"

namespace llvm {
    class StructType;
    class AllocaInst;
    class ConstantStruct;
    class Constant;
    class GlobalVariable;
}

namespace fly {

    class SemaClassType;
    class ASTClass;

    struct BaseType {
		llvm::StructType *Type;
		llvm::SmallVector<llvm::Value *, 4> Index;
        llvm::Function *InitConstructor;
        llvm::SmallVector<BaseType *, 4> Bases;
	};

    class CodeGenClass {

        friend class CodeGenClassMethod;

        CodeGenModule * CGM;

        SemaClassType *Sema;

        llvm::StructType *Type = nullptr;

        llvm::PointerType *TypePtr = nullptr;

        llvm::Function *InitConstructor = nullptr;

        llvm::StructType *VTableType = nullptr;

        llvm::SmallVector<llvm::Type *, 4> VTableMethodTypes;

        llvm::SmallVector<llvm::Constant *, 4> VTableValues;

        llvm::GlobalVariable * VTable;

        // llvm::SmallVector<CodeGenClassMethod *, 4> Constructors;

        llvm::SmallVector<CodeGenClassMethod *, 4> Methods;

        llvm::SmallVector<BaseType *, 4> BaseTypes;

        llvm::SmallVector<llvm::Type *, 4> BodyTypes;

        void CreateVTableType();

        void CollectBaseTypesRecursive(CodeGenClass *CGC,
                                                llvm::SmallVector<llvm::Value *, 4> CurrentIdx,
                                                unsigned Idx);

        void CreateBaseTypes();

        void CreateAttributeTypes();

        void CreateVTable();

        void CreateInitConstructor();

        void GenInitConstructorBody();

    public:

        CodeGenClass(CodeGenModule *CGM, SemaClassType *Sema, bool isExternal = false);

        llvm::StructType *getType();

        llvm::PointerType *getTypePtr();

        llvm::StructType *getVTableType();

        llvm::GlobalVariable * getVTable();

        llvm::Function *getInitConstructor();

        // const llvm::SmallVector<CodeGenClassMethod *, 4> &getConstructors() const;

        const llvm::SmallVector<CodeGenClassMethod *, 4> &getMethods() const;

        llvm::Value* getBaseInstance(llvm::Value* InstancePtr, llvm::StructType* Base);

        llvm::Value *Downcast(llvm::Type *ToType, llvm::Value *InstancePtr);

        llvm::Value* NewInstance();

    private:
        llvm::Value* NewInstance(SemaClassType *ClassType);

    };
}

#endif //FLY_CODEGEN_CLASS_H
