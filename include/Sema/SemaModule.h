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

#include "SemaImport.h"
#include <llvm/ADT/SmallVector.h>
#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTModule;
	class SemaNameSpace;
    class SemaFunction;
    class SemaType;
    class SemaNode;

    class SemaModule {

        friend class Resolver;
        friend class SemaBuilder;

        ASTModule &AST;

    	SemaNameSpace *NameSpace;

    	llvm::SmallVector<SemaImport *, 8> Imports;

        llvm::SmallVector<SemaNode *, 8> Nodes;

    public:

        SemaModule(ASTModule &Module);

        ~SemaModule();

        ASTModule &getAST() const;

        llvm::StringRef getName() const;

		SemaNameSpace *getNameSpace() const;

        void setNameSpace(SemaNameSpace *NameSpace);

        const llvm::SmallVector<SemaImport *, 8> &getImports() const;

    	void addImport(SemaImport *Import);

        const llvm::SmallVector<SemaNode *, 8> &getNodes() const;

    	void addNode(SemaNode *Node);

    };
}

#endif //FLY_SEMA_MODULE_H
