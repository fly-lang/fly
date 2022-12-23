//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNodeBase.h - Base AST Node
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTNODEBASE_H
#define FLY_ASTNODEBASE_H

#include "Basic/Debuggable.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"

#include <map>

namespace fly {

    class SourceLocation;
    class ASTContext;
    class ASTGlobalVar;
    class ASTClass;
    class ASTFunction;
    class ASTCall;

    class ASTNodeBase : public Debuggable {

        friend class Sema;
        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        ASTContext* Context = nullptr;

        // Node FileName
        const std::string Name;

        // Global Vars
        llvm::StringMap<ASTGlobalVar *> GlobalVars;

        // Functions
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> Functions;

        ASTNodeBase() = delete;

        ASTNodeBase(const std::string Name, ASTContext* Context);

    public:

        const std::string getName();

        ASTContext &getContext() const;

        const llvm::StringMap<ASTGlobalVar *> &getGlobalVars() const;

        const llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTFunction *, 4>>> &getFunctions() const;
    };
}

#endif //FLY_ASTNODEBASE_H
