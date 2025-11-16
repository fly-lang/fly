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

namespace fly {

    class SemaType;
    class SemaIntType;
    class SemaFloatType;
    class SemaErrorType;
    class SemaArrayType;
    class ASTExpr;

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

        static SemaArrayType * CreateArrayType(SemaType *Type, ASTExpr *SizeExpr = nullptr);

    };
}

#endif //FLY_SEMA_BUILTIN_H
