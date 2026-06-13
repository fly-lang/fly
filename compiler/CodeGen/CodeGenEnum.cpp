//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenEnum.cpp - enum type code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnum.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaEnumEntry.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>

#include <vector>

using namespace fly;

CodeGenEnum::CodeGenEnum(CodeGenModule *CGM, SemaEnumType *Sema, bool isExternal)
        : CodeGenType(CGM), Sema(Sema) {
    // Enums are represented as i32 integers
    T = CodeGen::Int32Ty;

    // Generate CodeGen for each enum entry
    for (auto &Entry : Sema->getEntries()) {
        SemaEnumEntry *SemaEntry = Entry.getValue();
        CodeGenEnumEntry *CGE = new CodeGenEnumEntry(CGM, SemaEntry);
        SemaEntry->setCodeGen(CGE);
        Entries.insert(std::make_pair(Entry.getKey(), CGE));
    }
}

SemaEnumType *CodeGenEnum::getSema() const {
    return Sema;
}

const llvm::StringMap<CodeGenEnumEntry *> &CodeGenEnum::getEntries() const {
    return Entries;
}

llvm::GlobalVariable *CodeGenEnum::getNamesTable() {
    if (NamesTable)
        return NamesTable;

    llvm::IRBuilder<> *Builder = CGM->getBuilder();

    // Entry values are 1-based (index 0 is the undefined/default entry), so the
    // table has one extra slot at the front.
    size_t Count = Sema->getEntries().size() + 1;

    // Slot 0 and any gaps hold an empty {null, 0} String struct.
    llvm::Constant *EmptyStr = llvm::ConstantStruct::get(
        CodeGen::StringTy,
        {llvm::Constant::getNullValue(CodeGen::Int8PtrTy), CodeGen::Zero});

    std::vector<llvm::Constant *> Elems(Count, EmptyStr);
    for (const auto &E : Sema->getEntries()) {
        size_t Idx = E.getValue()->getIndex();
        if (Idx >= Count)
            continue; // defensive — should not happen
        llvm::StringRef Name = E.getKey();
        llvm::Constant *Ptr = Builder->CreateGlobalStringPtr(Name);
        llvm::Constant *Size = llvm::ConstantInt::get(CodeGen::Int32Ty, Name.size());
        Elems[Idx] = llvm::ConstantStruct::get(CodeGen::StringTy, {Ptr, Size});
    }

    llvm::ArrayType *ArrTy = llvm::ArrayType::get(CodeGen::StringTy, Count);
    llvm::Constant *Init = llvm::ConstantArray::get(ArrTy, Elems);
    NamesTable = new llvm::GlobalVariable(
        *CGM->getModule(), ArrTy, /*isConstant=*/true,
        llvm::GlobalValue::PrivateLinkage, Init,
        "enum.names." + Sema->getName());
    return NamesTable;
}


