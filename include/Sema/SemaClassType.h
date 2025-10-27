//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassType.h - SemaClassType
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CLASS_TYPE_H
#define FLY_SEMA_CLASS_TYPE_H

#include "SemaClassInstance.h"
#include "Sema/SemaType.h"
#include "AST/ASTClass.h"
#include "llvm/ADT/StringMap.h"
#include "SemaVisibilityKind.h"

namespace fly {

    class Sema;
    class ASTClass;
    class SemaComment;
    class SemaClassMethod;
    class SemaClassInstance;
    class CodeGenClass;
    class SemaModule;
    class SemaClassAttribute;
    class SemaVar;
    enum class SemaVisibilityKind;

    enum class SemaClassKind {
        CLASS, INTERFACE, STRUCT
    };

    class SemaClassType : public SemaType {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaResolverClass;
        friend class SemaValidator;

        // AST linked to this Class
        ASTClass &AST;

        // Class Module
        SemaModule *Module;

        // Class Visibility
        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        // Class is Constant (cannot be redefined)
        bool Constant;

        // Class Kind
        SemaClassKind ClassKind;

        // Super Classes
        llvm::SmallVector<SemaClassType *, 4> BaseClasses;

        // Class this Attribute
        SemaClassInstance *This;

        // Class Attributes
        llvm::StringMap<SemaClassAttribute *> Attributes;

        // Class Methods
        llvm::StringMap<SemaClassMethod *> Methods;

        // Class Constructors
        llvm::StringMap<SemaClassMethod *> Constructors;

        // Class Comment
        SemaComment *Comment = nullptr;

        // Class CodeGen
        CodeGenClass *CodeGen = nullptr;

        explicit SemaClassType(ASTClass &Class);

        SemaClassKind toClassKind(ASTClassKind ASTKind);

    public:

        ASTClass &getAST();

        SemaModule *getModule() const;

        SemaVisibilityKind getVisibility() const;

        bool isConstant() const;

        SemaClassKind getClassKind() const;

        SemaClassInstance *getThis() const;

        const llvm::SmallVector<SemaClassType *, 4> &getBaseClasses() const;

        const llvm::StringMap<SemaClassAttribute *> &getAttributes() const;

        const llvm::StringMap<SemaClassMethod *> &getMethods() const;

        const llvm::StringMap<SemaClassMethod *> &getConstructors() const;

        SemaComment *getComment() const;

        bool isDerivedOrEquals(const SemaClassType *BaseClassType) const;

        bool isDerived(const SemaClassType *BaseClassType) const;

        bool isBaseOrEquals(const SemaClassType *Derived) const;

        bool isBase(const SemaClassType *Derived) const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);
    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_TYPE_H