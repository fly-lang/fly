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

#include "ASTIdentity.h"
#include "ASTClassType.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace fly {

    class ASTClassVar;
    class ASTClassFunction;
    class CodeGenClass;
    class ASTBlock;

    enum class ASTClassKind {
        STRUCT, // has only Fields
        CLASS, // has only Fields and Methods defined
        INTERFACE, // has only Methods declarations
    };

    class ASTClass : public ASTIdentity {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTClassType *Type = nullptr;

        ASTClassKind ClassKind;

        llvm::SmallVector<ASTClassType *, 4> SuperClasses;

        // Class Fields
        llvm::StringMap<ASTClassVar *> Vars;

        bool autoDefaultConstructor = false;

        // Class Constructors
        std::map <uint64_t, llvm::SmallVector <ASTClassFunction *, 4>> Constructors;

        // Class Methods
        llvm::StringMap<std::map <uint64_t, llvm::SmallVector <ASTClassFunction *, 4>>> Methods;

        CodeGenClass *CodeGen = nullptr;

        ASTClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *Scopes,
                 const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTClassType *getType() override;

        ASTClassKind getClassKind() const;

        llvm::SmallVector<ASTClassType *, 4> getSuperClasses() const;

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
