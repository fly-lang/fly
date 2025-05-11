//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/SemaModule.h - Symbolic Table of Module header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_MODULE_H
#define FLY_SEMA_MODULE_H

#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTModule;
	class SemaNameSpace;
    class SemaGlobalVar;
    class SemaFunction;
    class SemaType;

    class SemaModule {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTModule *AST;

    	SemaNameSpace *NameSpace;

    	llvm::StringMap<SemaNameSpace *> Imports;

        // Global Vars
        llvm::StringMap<SemaGlobalVar *> GlobalVars;

        // Functions
        llvm::StringMap<SemaFunction *> Functions;

        // Types
        llvm::StringMap<SemaType *> Types;

        SemaModule(ASTModule *Module);

    public:

        ~SemaModule();

        ASTModule* getAST() const;

		SemaNameSpace *getNameSpace() const;

		const llvm::StringMap<SemaNameSpace *> &getImports() const;

        const llvm::StringMap<SemaGlobalVar *> &getGlobalVars() const;

        const llvm::StringMap<SemaFunction *> &getFunctions() const;

        const llvm::StringMap<SemaType *> &getTypes() const;

    };
}

#endif //FLY_SEMA_MODULE_H
