//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SymTable.h - AST Context header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_TABLE_H
#define FLY_SYM_TABLE_H

#include <AST/ASTTypeRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMap.h>

namespace fly {

    class SemaModule;
    class SemaNameSpace;
    class SemaType;

    /**
     * AST Context
     */
    class SymTable {

        friend class SemaResolver;
        friend class SemaBuilder;

        llvm::DenseMap<uint64_t, SemaModule *> Modules;

        // The Default NameSpace
        SemaNameSpace *DefaultNameSpace;

        // All NameSpaces in the Context
        llvm::StringMap<SemaNameSpace *> NameSpaces;

        SemaType *BoolType;

        SemaType *ByteType;

        SemaType *UShortType;

        SemaType *ShortType;

        SemaType *UIntType;

        SemaType *IntType;

        SemaType *ULongType;

        SemaType *LongType;

        SemaType *FloatType;

        SemaType *DoubleType;

        SemaType *VoidType;

        SemaType *StringType;

        SemaType *ErrorType;

        SymTable();

    public:

        ~SymTable();

        const llvm::DenseMap<uint64_t, SemaModule *> &getModules() const;

        SemaNameSpace *getDefaultNameSpace() const;

        const llvm::StringMap<SemaNameSpace*> &getNameSpaces() const;

        SemaType *getBoolType() const;

        SemaType * getByteType() const;

        SemaType * getUShortType() const;

        SemaType * getShortType() const;

        SemaType * getUIntType() const;

        SemaType * getIntType() const;

        SemaType * getULongType() const;

        SemaType * getLongType() const;

        SemaType * getFloatType() const;

        SemaType * getDoubleType() const;

        SemaType * getVoidType() const;

        SemaType * getCharType() const;

        SemaType * getStringType() const;

        SemaType * getErrorType() const;

    };
}

#endif //FLY_SYM_TABLE_H
