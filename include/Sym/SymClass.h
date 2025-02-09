//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_CLASS_H
#define FLY_SYM_CLASS_H

#include "Sym/SymType.h"
#include "AST/ASTClass.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

#include <map>
#include <llvm/ADT/DenseMap.h>

#include "SymVisibilityKind.h"

namespace fly {

    class Sema;
    class ASTClass;
    class SymClassAttribute;
    class SymComment;
    class SymClassMethod;
    class CodeGenClass;
    class SymModule;
    enum class SymVisibilityKind;

    enum class SymClassKind {
        CLASS, INTERFACE, STRUCT
    };

    class SymClass : public SymType {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        // AST linked to this Class Symbol
        ASTClass *AST;

        // Class Module
        SymModule *Module;

        // Class Visibility
        SymVisibilityKind Visibility = SymVisibilityKind::DEFAULT;

        // Class is Constant (cannot be redefined)
        bool Constant;

        // Class Kind
        SymClassKind ClassKind;

        // Super Classes
        llvm::StringMap<SymClass *> SuperClasses;

        // Class Attributes
        llvm::StringMap<SymClassAttribute *> Attributes;

        // Class Methods
        llvm::DenseMap<size_t, SymClassMethod *> Methods;

        // Class Constructors
        llvm::DenseMap<size_t, SymClassMethod *> Constructors;

        // Class Comment
        SymComment *Comment = nullptr;

        // Class CodeGen
        CodeGenClass *CodeGen = nullptr;

        explicit SymClass(ASTClass *Class);

    public:

        ASTClass *getAST();

        SymModule *getModule() const;

        SymVisibilityKind getVisibility() const;

        bool isConstant() const;

        SymClassKind getClassKind() const;

        const llvm::StringMap<SymClass *> &getSuperClasses() const;

        const llvm::StringMap<SymClassAttribute *> &getAttributes() const;

        const llvm::DenseMap<size_t, SymClassMethod *> &getMethods() const;

        const llvm::DenseMap<size_t, SymClassMethod *> &getConstructors() const;

        SymComment *getComment() const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);
    };

}  // end namespace fly

#endif // FLY_SYM_CLASS_H