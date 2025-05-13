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

#include "Sema/SemaType.h"
#include "AST/ASTClass.h"
#include "llvm/ADT/StringMap.h"
#include "SemaVisibilityKind.h"

namespace fly {

    class Sema;
    class ASTClass;
    class SemaComment;
    class SemaClassMethod;
    class CodeGenClass;
    class SemaModule;
    class SemaClassAttribute;
    enum class SemaVisibilityKind;

    enum class SemaClassKind {
        CLASS, INTERFACE, STRUCT
    };

    class SemaClassType : public SemaType {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaResolverClass;
        friend class SemaValidator;

        // AST linked to this Class
        ASTClass *AST;

        // Class Module
        SemaModule *Module;

        // Class Visibility
        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        // Class is Constant (cannot be redefined)
        bool Constant;

        // Class Kind
        SemaClassKind ClassKind;

        // Super Classes
        llvm::StringMap<SemaClassType *> SuperClasses;

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

        explicit SemaClassType(ASTClass *Class);

        SemaClassKind toClassKind(ASTClassKind ASTKind);

    public:

        ASTClass *getAST();

        SemaModule *getModule() const;

        SemaVisibilityKind getVisibility() const;

        bool isConstant() const;

        SemaClassKind getClassKind() const;

        const llvm::StringMap<SemaClassType *> &getSuperClasses() const;

        const llvm::StringMap<SemaClassAttribute *> &getAttributes() const;

        const llvm::StringMap<SemaClassMethod *> &getMethods() const;

        const llvm::StringMap<SemaClassMethod *> &getConstructors() const;

        SemaComment *getComment() const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);
    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_TYPE_H