//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Symbol.h - AST Context header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYMBOL_H
#define FLY_SYMBOL_H

#include "SemaNode.h"
#include <string>
#include <llvm/ADT/StringRef.h>

namespace fly {

	enum class SymbolKind {
		NAMESPACE,
		TYPE,
		VAR,
		VALUE,
		FUNCTION,
		CLASS,
		ENUM,
	};

	struct Symbol {

		std::string Name;

		SymbolKind Kind;

		SemaNode *Ref;  // riferimento all’oggetto semantico (Sema*)

		Symbol(std::string Name, SymbolKind Kind, SemaNode* Ref);

		Symbol(llvm::StringRef Name, SymbolKind Kind, SemaNode* Ref);

		std::string getName() const;

		SymbolKind getKind() const;

		SemaNode * getRef() const;
	};
}

#endif //FLY_SYMBOL_H
