//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SymbolTable.h - AST Context header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILTIN_H
#define FLY_SEMA_BUILTIN_H

#include <cstdint>

namespace fly {

    class SemaType;
    class SemaIntType;
    class SemaFloatType;
    class SemaErrorType;
    class SemaArrayType;
    class SemaExpr;

    /**
     * AST Context
     */
    class SemaBuiltin {

        static SemaType *BoolType;

        static SemaIntType *ByteType;

        static SemaIntType *UShortType;

        static SemaIntType *ShortType;

        static SemaIntType *UIntType;

        static SemaIntType *IntType;

        static SemaIntType *ULongType;

        static SemaIntType *LongType;

        static SemaFloatType *FloatType;

        static SemaFloatType *DoubleType;

        static SemaType *VoidType;

        static SemaType *StringType;

        static SemaErrorType *ErrorType;

    public:
        SemaBuiltin() = delete;

        static SemaType *getBoolType();

        static SemaIntType * getByteType();

        static SemaIntType * getUShortType();

        static SemaIntType * getShortType();

        static SemaIntType * getUIntType();

        static SemaIntType * getIntType();

        static SemaIntType * getULongType();

        static SemaIntType * getLongType();

        static SemaFloatType * getFloatType();

        static SemaFloatType * getDoubleType();

        static SemaType * getVoidType();

        static SemaType * getStringType();

        static SemaErrorType * getErrorType();

        static SemaArrayType * CreateArrayType(SemaType *Type, SemaExpr *SizeExpr);

    	static SemaArrayType * CreateArrayType(SemaType *Type, std::uint64_t Size);

        /**
         * Resets the CodeGen pointers on all builtin types.
         * This is needed when running multiple tests with different LLVMContexts.
         */
        static void resetCodeGen();

    };
}

#endif //FLY_SEMA_BUILTIN_H
