//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SymModule.h - Symbolic Table of Module header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_MODULE_H
#define FLY_SYM_MODULE_H

#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTModule;
	class SymNameSpace;
    class SymGlobalVar;
    class SymFunction;
    class SymEnum;
    class SymClass;

    class SymModule {

        friend class SymBuilder;
        friend class SemaResolver;

        ASTModule *AST;

    	SymNameSpace *NameSpace;

    	llvm::StringMap<SymNameSpace *> Imports;

        // Global Vars
        llvm::StringMap<SymGlobalVar *> GlobalVars;

        // Functions
        llvm::StringMap<SymFunction *> Functions;

        // Classes
        llvm::StringMap<SymClass *> Classes;

        // Enumerations
        llvm::StringMap<SymEnum *> Enums;

        SymModule(ASTModule *Module);

    public:

        ~SymModule();

        ASTModule* getAST() const;

		SymNameSpace *getNameSpace() const;

		const llvm::StringMap<SymNameSpace *> &getImports() const;

        llvm::StringMap<SymGlobalVar *> getGlobalVars() const;

        llvm::StringMap<SymFunction *> getFunctions() const;

        llvm::StringMap<SymClass *> getClasses() const;

        llvm::StringMap<SymEnum *> getEnums() const;

    };
}

#endif //FLY_SYM_MODULE_H
