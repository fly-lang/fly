//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SymNameSpace.h - AST Namespace header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_NAMESPACE_H
#define FLY_SYM_NAMESPACE_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTModule;
    class SymGlobalVar;
    class SymFunction;
    class SymClass;
    class SymEnum;
    class CodeGenModule;

    class SymNameSpace {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Name;

        llvm::SmallVector<ASTModule *, 8> Modules;

        // Global Vars
        llvm::StringMap<SymGlobalVar *> GlobalVars;

        // Functions
        llvm::SmallVector<SymFunction *, 8> Functions;

        // Classes
        llvm::StringMap<SymClass *> Classes;

        // Enumerations
        llvm::StringMap<SymEnum *> Enums;

        CodeGenModule* CodeGen = nullptr;

        SymNameSpace();

        SymNameSpace(llvm::StringRef Name);

    public:

        static std::string DEFAULT_NAMESPACE;

        ~SymNameSpace();

        llvm::StringRef getName() const;

        llvm::SmallVector<ASTModule *, 8> getModules() const;

        llvm::StringMap<SymGlobalVar *> getGlobalVars() const;

        llvm::SmallVector<SymFunction *, 8> getFunctions() const;

        llvm::StringMap<SymClass *> getClasses() const;

        llvm::StringMap<SymEnum *> getEnums() const;

        CodeGenModule* getCodeGen() const;

        void setCodeGen(CodeGenModule *CGM);

    };
}

#endif //FLY_SYM_NAMESPACE_H
