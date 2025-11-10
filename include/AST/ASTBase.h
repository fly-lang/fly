//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBase.h - AST Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTBASE_H
#define FLY_ASTBASE_H

#include "Basic/SourceLocation.h"
#include <Basic/Logger.h>
#include "llvm/ADT/SmallVector.h"

namespace fly {

	class SourceLocation;

	enum class ASTKind {
		AST_MODULE,
		AST_NAMESPACE,
		AST_IMPORT,
		AST_VAR,
		AST_ARG,
		AST_STMT,
		AST_TYPE,
		AST_VALUE,
		AST_EXPR,
		AST_REF,
		AST_COMMENT,
		AST_FUNCTION,
		AST_MODIFIER,
		AST_STRUCT,
		AST_CLASS,
		AST_METHOD,
		AST_ATTRIBUTE,
		AST_INTERFACE,
		AST_ENUM,
		AST_ENUM_ENTRY,
	};

	class ASTBase {

		SourceLocation Loc;

		ASTKind Kind;

	protected:
		ASTBase(const SourceLocation &Loc, ASTKind Kind);

	public:

		virtual ~ASTBase() = default;

		virtual const SourceLocation &getLocation() const;

		virtual ASTKind getKind() const;

		virtual void setKind(ASTKind Kind);

		virtual std::string str() const;

		template <typename T>
		static const std::string &str(llvm::SmallVector<T *, 8> Vect) {
			std::string Str = Logger::OPEN_LIST;
			if(!Vect.empty()) {
				for (auto *V : Vect) {
					Str += (V ? V->str() : "") + Logger::SEP;
				}
				unsigned long end = Str.length()-std::string(Logger::SEP).length()-1;
				Str = Str.substr(0, end);
			}
			Str += Logger::CLOSE_LIST;
			return Str;
		}

	};

}

#endif //FLY_ASTBASE_H
