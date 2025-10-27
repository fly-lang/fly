//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Registry.h - Module Manager
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_NAMESPACE_REGISTRY_H
#define FLY_NAMESPACE_REGISTRY_H

#include <string>
#include <unordered_map>
#include <llvm/ADT/SmallVector.h>

#include "Resolver.h"

namespace fly {

	class SemaNameSpace;
	class SemaModule;
	class ASTModule;
	class ASTBlockStmt;

	class Registry {

		static std::string DEFAULT_NAMESPACE;

		llvm::SmallVector<SemaModule *, 8> Modules;
		
		SemaNameSpace *DefaultNameSpace;

		std::unordered_map<std::string, SemaNameSpace *> NameSpaces;

		llvm::SmallVector<ASTBlockStmt *, 4> Bodies;

	public:

		Registry();

		~Registry() = default;

		void addModule(SemaModule* Module);

		llvm::SmallVector<SemaModule *, 8> &getModules();

		SemaNameSpace * getDefaultNameSpace();

		SemaNameSpace * getOrAddFQNameSpace(const std::string& FQName, SemaNameSpace *Parent = nullptr);

		SemaNameSpace *getFQNameSpace(const std::string& FQName);

		llvm::SmallVector<ASTBlockStmt *, 4> getBodies();


	};

}

#endif //FLY_NAMESPACE_REGISTRY_H
