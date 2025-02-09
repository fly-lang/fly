//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SymTable.h - AST Context header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYMTABLE_H
#define FLY_SYMTABLE_H

#include <AST/ASTTypeRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMap.h>

namespace fly {

    class SymModule;
    class SymNameSpace;
    class SymType;

    /**
     * AST Context
     */
    class SymTable {

        friend class SemaResolver;
        friend class SymBuilder;

        llvm::DenseMap<uint64_t, SymModule *> Modules;

        // The Default NameSpace
        SymNameSpace *DefaultNameSpace;

        // All NameSpaces in the Context
        llvm::StringMap<SymNameSpace *> NameSpaces;

        SymType *BoolType;

        SymType *ByteType;

        SymType *UShortType;

        SymType *ShortType;

        SymType *UIntType;

        SymType *IntType;

        SymType *ULongType;

        SymType *LongType;

        SymType *FloatType;

        SymType *DoubleType;

        SymType *VoidType;

        SymType *CharType;

        SymType *StringType;

        SymType *ErrorType;

        SymTable();

    public:

        ~SymTable();

        const llvm::DenseMap<uint64_t, SymModule *> &getModules() const;

        const SymNameSpace *getDefaultNameSpace() const;

        const llvm::StringMap<SymNameSpace*> &getNameSpaces() const;

        SymType *getBoolType() const;

        SymType * getByteType() const;

        SymType * getUShortType() const;

        SymType * getShortType() const;

        SymType * getUIntType() const;

        SymType * getIntType() const;

        SymType * getULongType() const;

        SymType * getLongType() const;

        SymType * getFloatType() const;

        SymType * getDoubleType() const;

        SymType * getVoidType() const;

        SymType * getCharType() const;

        SymType * getStringType() const;

        SymType * getErrorType() const;

    };
}

#endif //FLY_SYMTABLE_H
