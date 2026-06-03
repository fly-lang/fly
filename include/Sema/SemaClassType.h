//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassType.h - class type semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CLASS_TYPE_H
#define FLY_SEMA_CLASS_TYPE_H

#include "AST/ASTClass.h"
#include "CodeGen/CodeGenClass.h"
#include "Sema/SemaType.h"
#include "SemaClassInstance.h"
#include "SemaVisibilityKind.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SemaContext;
    class ASTClass;
    class SemaComment;
    class SemaClassMethod;
    class SemaClassInstance;
    class CodeGenClass;
    class SemaModule;
    class SemaClassAttribute;
    class SemaVar;
    class SymbolTable;
    class SemaTypeParam;
    enum class SemaVisibilityKind;

    enum class SemaClassKind {
        CLASS, INTERFACE, STRUCT, SUITE
    };

    class SemaClassType : public SemaType {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        // AST linked to this Class
        ASTClass &AST;

        // Class Module
        SemaModule &Module;

        // Class Symbol Table
        SymbolTable *Symbols;

        // Nodes: Attributes, Methods, Constructors
        llvm::SmallVector<SemaNode *, 8> Nodes;

        // Class Visibility
        SemaVisibilityKind Visibility;

        // Class is Constant (cannot be redefined)
        bool Constant = false;

        // Class is Abstract (cannot be instantiated; has at least one abstract method)
        bool Abstract = false;

        // Class is Final (cannot be subclassed)
        bool Final = false;

        // Tracks whether this class has already been resolved (guards against double-resolution)
        bool Resolved = false;

        // Class Kind
        SemaClassKind ClassKind;

        // Super Classes
        llvm::SmallVector<SemaClassType *, 4> BaseClasses;

        // Class this Attribute
        SemaClassInstance *This = nullptr;

        // Class Attributes
        llvm::StringMap<SemaClassAttribute *> Attributes;

        // Class Methods
        llvm::StringMap<SemaClassMethod *> Methods;

        // Class Constructors
        llvm::StringMap<SemaClassMethod *> Constructors;

    	SemaClassMethod *DefaultConstructor = nullptr;

        // Class Comment
        SemaComment *Comment = nullptr;

        // Class CodeGen
        CodeGenClass *CodeGen = nullptr;

        // Generic type parameters (non-empty for template classes like List<T>).
        llvm::SmallVector<SemaTypeParam *, 4> TypeParams;

        // Monomorphization cache: mangled key (e.g. "List_I") → specialized SemaClassType.
        llvm::StringMap<SemaClassType *> Specializations;

        // Back-pointer to the generic template this is a specialization of (null for templates).
        SemaClassType *GenericTemplate = nullptr;

        // Non-empty for specializations: overrides the base name in LLVM symbol generation.
        // E.g. "List_I" for List<int>.  CodeGenClass::toIdentifier() checks this.
        std::string MangledName;

        explicit SemaClassType(ASTClass &Class, SemaModule &Module, SymbolTable *Symbols);

        SemaClassKind toClassKind(ASTClassKind ASTKind);

    public:

        ~SemaClassType() override;

        ASTClass &getAST();

        SemaModule &getModule() const;

        SymbolTable *getSymbols() const;

        SemaComment *getComment() const;

        llvm::SmallVector<SemaNode *, 8> &getNodes();

        SemaVisibilityKind getVisibility() const;

        bool isConstant() const;

        bool isAbstract() const;

        bool isFinal() const;

        SemaClassKind getClassKind() const;

        SemaClassInstance *getThis() const;

        const llvm::SmallVector<SemaClassType *, 4> &getBaseClasses() const;

    	SemaClassMethod *getDefaultConstructor() const;

    	void setDefaultConstructor(SemaClassMethod *Method);

        SemaClassAttribute *LookupAttribute(llvm::StringRef Name) const;

        SemaClassMethod *LookupMethod(llvm::StringRef Name) const;

        SemaClassMethod *LookupConstructor(llvm::StringRef Name) const;

        const llvm::StringMap<SemaClassAttribute *> &getAttributes() const;

        const llvm::StringMap<SemaClassMethod *> &getMethods() const;

        const llvm::StringMap<SemaClassMethod *> &getConstructors() const;

        // Generics accessors
        const llvm::SmallVector<SemaTypeParam *, 4> &getTypeParams() const;
        bool isGeneric() const;
        SemaClassType *getGenericTemplate() const;
        llvm::StringMap<SemaClassType *> &getSpecializations();
        const std::string &getMangledName() const;

        void addAttribute(SemaClassAttribute *Attribute);

        void addMethod(SemaClassMethod *Method);

        bool isDerivedOrEquals(SemaClassType *BaseClassType) const;

        bool isDerived(SemaClassType *BaseClassType) const;

        bool isBaseOrEquals(SemaClassType *Derived) const;

        bool isBase(SemaClassType *Derived) const;

        CodeGenClass *getCodeGen() const override;

        void setCodeGen(CodeGenClass *CGC);

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SEMA_CLASS_TYPE_H