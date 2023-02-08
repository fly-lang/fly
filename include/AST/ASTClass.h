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

    enum class ASTClassKind {
        CLASS_STRUCT,
        CLASS_STANDARD,
        CLASS_INTERFACE,
        CLASS_ABSTRACT
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

        // Constant of Fields or Methods
        bool Constant = false;

        ASTClassScopes(ASTClassVisibilityKind Visibility, bool Constant);

    public:
        ASTClassVisibilityKind getVisibility() const;

        bool isConstant() const;

        std::string str() const;
    };

    class ASTClass : public ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;

        llvm::StringRef Name;

        // Source Location
        const SourceLocation Location;

        ASTClassKind ClassKind;

        // Class Fields
        llvm::StringMap<ASTClassVar *> Vars;

        // Class Constructors
        std::map <uint64_t, llvm::SmallVector <ASTClassFunction *, 4>> Constructors;

        bool autoDefaultConstructor = false;

        // Class Methods
        llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTClassFunction *, 4>>> Methods;

        CodeGenClass *CodeGen = nullptr;

        ASTClass(const SourceLocation &Loc, ASTNode *Node, llvm::StringRef Name, ASTTopScopes *Scopes);

    public:

        llvm::StringRef getName() const;

        const SourceLocation &getLocation() const;

        ASTClassKind getClassKind() const;

        llvm::StringMap<ASTClassVar *> getVars() const;

        std::map <uint64_t,llvm::SmallVector <ASTClassFunction *, 4>> getConstructors() const;

        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassFunction *, 4>>> getMethods() const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);

        std::string str() const;

    };
}

#endif //FLY_ASTCLASS_H
