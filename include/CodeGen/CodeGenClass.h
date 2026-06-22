//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenClass.h - class type code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_CLASS_H
#define FLY_CODEGEN_CLASS_H

#include "CodeGenType.h"
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

    class CodeGenClass : public CodeGenType {

        friend class CodeGenClassMethod;

        SemaClassType *Sema;

        bool IsExternal = false;

        std::string Id;

        llvm::StructType *Type = nullptr;

        llvm::PointerType *TypePtr = nullptr;

        llvm::SmallVector<llvm::Type *, 4> BodyType;

        llvm::Function *InitConstructor = nullptr;

        llvm::GlobalVariable * VTable = nullptr;

        llvm::SmallVector<llvm::GlobalVariable *, 4> VTableBases;

        // Per-base vtables for TRANSITIVE (non-direct) base subobjects that this
        // most-derived class overrides methods of — keyed by their byte offset
        // within this class. The constructor stores each at that offset so a
        // grandparent-typed reference dispatches to this class's override.
        llvm::SmallVector<std::pair<uint64_t, llvm::GlobalVariable *>, 4> ExtraBaseVTables;

        llvm::SmallVector<CodeGenClassMethod *, 4> Methods;

        // llvm::SmallVector<BaseType *, 4> BaseTypes;

        std::string toIdentifier(SemaClassType *ClassType);

        void CreateVTable();

        void CreateBaseVTables();

        // Build a per-base vtable global for `Base` whose subobject sits at
        // `byteOffset` within this most-derived class (offset-to-top = -byteOffset),
        // with this class's overrides routed through this-adjusting thunks.
        llvm::GlobalVariable *BuildPerBaseVTable(SemaClassType *Base, uint64_t byteOffset);

        // Recurse into Base's own bases (transitive w.r.t. this class), building a
        // per-base vtable for any whose methods this class overrides.
        void CollectTransitiveBaseVTables(SemaClassType *Base, uint64_t baseOffsetInDerived);

        llvm::Function *CreateThunk(SemaClassMethod *BaseMethod, SemaClassMethod *Override, uint64_t BaseOffset);

        SemaClassMethod *FindOverrideInDerived(SemaClassType *Derived, SemaClassMethod *BaseMethod);

        void CreateAttributes();

        void CreateBaseInfo(llvm::SmallVector<SemaClassType *, 4> BaseClasses);

        void CreateInitConstructor();

        void GenInitConstructorBody();

    public:

        CodeGenClass(CodeGenModule *CGM, SemaClassType *Sema, bool isExternal = false);

        void Build();

        // Offset-dependent build steps (per-base vtables + init-constructor body)
        // split out so they can be deferred to a post-pass for cyclic builds where
        // the struct layout is not yet available. See CodeGenClass::Build.
        void FinishBuild();

        llvm::StructType *getType();

        llvm::PointerType *getTypePtr();

        llvm::GlobalVariable * getVTable();

        llvm::Function *getInitConstructor();

        // const llvm::SmallVector<CodeGenClassMethod *, 4> &getConstructors() const;

        const llvm::SmallVector<CodeGenClassMethod *, 4> &getMethods() const;

        llvm::Value* getBaseInstance(llvm::Value* InstancePtr, SemaClassType* Base);

        llvm::Value *Downcast(llvm::Type *ToType, llvm::Value *InstancePtr);


    };
}

#endif //FLY_CODEGEN_CLASS_H
