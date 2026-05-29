//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Symbol.h - symbol implementation
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
		BUILTIN_TYPE,
		VAR,
		LOCAL_VAR,
		PARAM,
		ATTRIBUTE,
		ENUM_ENTRY,
		TYPE,
		VALUE,
		FUNCTION,
		CLASS,
		ENUM,
	};

	struct Symbol {

		std::string Name;

		SymbolKind Kind;

		SemaNode *Ref;  // riferimento all'oggetto semantico (Sema*)

		Symbol(std::string Name, SymbolKind Kind, SemaNode* Ref);

		Symbol(llvm::StringRef Name, SymbolKind Kind, SemaNode* Ref);

		std::string getName() const;

		SymbolKind getKind() const;

		SemaNode * getRef() const;

		/// Returns true if this Symbol represents any variable kind
		/// (VAR, LOCAL_VAR, PARAM, ATTRIBUTE)
		bool isVarKind() const;

		/// Typed access to the Sema reference
		template<typename T>
		T *getRefAs() const {
			return static_cast<T *>(Ref);
		}
	};
}

#endif //FLY_SYMBOL_H
