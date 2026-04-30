//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenEnum.h - Code Generator of Enum
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_ENUM_H
#define FLY_CODEGEN_ENUM_H

#include "CodeGenType.h"
#include "llvm/ADT/StringMap.h"

namespace llvm {
    class Type;
}

namespace fly {

    class CodeGenModule;
    class SemaEnumType;
    class CodeGenEnumEntry;

    /**
     * CodeGenEnum - Code generator for enum types.
     *
     * Enums are represented as i32 integers in LLVM IR.
     * Each enum entry is a constant integer value based on its index.
     */
    class CodeGenEnum : public CodeGenType {

        SemaEnumType *Sema = nullptr;

        llvm::StringMap<CodeGenEnumEntry *> Entries;

    public:
        CodeGenEnum(CodeGenModule *CGM, SemaEnumType *Sema, bool isExternal = false);

        SemaEnumType *getSema() const;

        const llvm::StringMap<CodeGenEnumEntry *> &getEntries() const;
    };

}

#endif //FLY_CODEGEN_ENUM_H

