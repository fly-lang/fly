//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClass.h - Class declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASS_H
#define FLY_ASTCLASS_H

#include "ASTTopDef.h"
#include "Basic/Debuggable.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace fly {

    class ASTClassVar;
    class ASTClassFunction;
    class CodeGenClass;
    class ASTClassType;

    enum class ASTClassKind {
        STRUCT, // has only Fields
        CLASS, // has only Fields and Methods defined
        INTERFACE, // has only Methods declarations
        ENUM // has only Constants
    };

    enum class ASTClassVisibilityKind {
        CLASS_V_DEFAULT,
        CLASS_V_PUBLIC,
        CLASS_V_PRIVATE,
        CLASS_V_PROTECTED
    };

    class ASTClassScopes : public Debuggable {

        friend class SemaBuilder;

        // Visibility of the Fields or Methods
        ASTClassVisibilityKind Visibility = ASTClassVisibilityKind::CLASS_V_DEFAULT;

        // Constant Fields or Methods
        bool Constant = false;

        // Static Fields or Methods
        bool Static = false;

        ASTClassScopes(ASTClassVisibilityKind Visibility, bool Constant, bool Static = false);

    public:
        ASTClassVisibilityKind getVisibility() const;

        bool isConstant() const;

        bool isStatic() const;

        std::string str() const;
    };

    class ASTClass : public ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;

        llvm::StringRef Name;

        // Source Location
        SourceLocation Location;

        ASTClassType *Type = nullptr;

        ASTClassKind ClassKind;

        llvm::StringMap<ASTClass *> SuperClasses;

        // Class Fields
        llvm::StringMap<ASTClassVar *> Vars;

        // Class Constructors
        std::map <uint64_t, llvm::SmallVector <ASTClassFunction *, 4>> Constructors;

        bool autoDefaultConstructor = false;

        // Class Methods
        llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTClassFunction *, 4>>> Methods;

        CodeGenClass *CodeGen = nullptr;

        ASTClass(ASTNode *Node, ASTClassKind ClassKind, ASTTopScopes *Scopes,
                 const SourceLocation &Loc, const llvm::StringRef Name,
                 llvm::SmallVector<llvm::StringRef, 4> &ExtClasses);

    public:

        llvm::StringRef getName() const;

        const SourceLocation &getLocation() const;

        ASTClassType *getType() const;

        ASTClassKind getClassKind() const;

        llvm::StringMap<ASTClassVar *> getVars() const;

        std::map <uint64_t,llvm::SmallVector <ASTClassFunction *, 4>> getConstructors() const;

        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassFunction *, 4>>> getMethods() const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);

        std::string print() const;

        std::string str() const;

    };
}

#endif //FLY_ASTCLASS_H
