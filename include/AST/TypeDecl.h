//
// Created by marco on 4/18/21.
//

#ifndef FLY_TYPEDECL_H
#define FLY_TYPEDECL_H

#include "Decl.h"

namespace fly {

    /**
     * Abstract Base Type
     */
    class TypeDecl {

    protected:
        TypeDecl(TypeKind T) : Type(T) {}
        const TypeKind Type;

    public:
        const TypeKind &getKind() const {
            return Type;
        };

        ~TypeDecl() = default;
    };

    /**
     * Int Type
     */
    class IntTypeDecl : public TypeDecl {

        int *Value;

    public:

        IntTypeDecl(int *V) : TypeDecl(TypeKind::Int), Value(V) {}

        int *getValue() {
            return Value;
        }

        void setValue(int *V) {
            Value = V;
        }

        ~IntTypeDecl() {
            delete Value;
        }
    };

    /**
     * Float Type
     */
    class FloatTypeDecl : public TypeDecl {

        float *Value;

    public:

        FloatTypeDecl(float *V) : TypeDecl(TypeKind::Float), Value(V) {}

        float *getValue() {
            return Value;
        }

        void setValue(float *V) {
            Value = V;
        }

        virtual ~FloatTypeDecl() {
            delete Value;
        }
    };

    /**
     * Boolean Type
     */
    class BoolTypeDecl : public TypeDecl {

        bool *Value;

    public:

        BoolTypeDecl(bool *V) : TypeDecl(TypeKind::Boolean), Value(V) {}

        bool *getValue() {
            return Value;
        }

        void setValue(bool *V) {
            Value = V;
        }

        virtual ~BoolTypeDecl() {
            delete Value;
        }
    };
}

#endif //FLY_TYPEDECL_H
