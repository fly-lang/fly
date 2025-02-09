//
// Created by marco on 05/02/25.
//

#ifndef SEMA_LOGGER_H
#define SEMA_LOGGER_H

#include "string"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

    class ASTBase;
	class SourceLocation;

	class Logger {

	     std::string Str;
	     bool isEmpty = true;
	     bool isClass = false;

		public:

		 static const char * OPEN;
		 static const char * SEP;
		 static const char * EQ;
		 static const char * CLOSE;
		 static const char * OPEN_LIST;
		 static const char * CLOSE_LIST;

		 Logger();

		 explicit Logger(std::string str);

		 std::string End();

		 Logger &Super(std::string val);

		 Logger &Attr(const char *key, std::string val);

		 Logger &Attr(const char *key, llvm::StringRef val);

		 Logger &Attr(const char *key, SourceLocation &val);

		 Logger &Attr(const char *key, bool val);

		 Logger &Attr(const char *key, size_t val);

		 Logger &Attr(const char *key, ASTBase *val);

		 template <typename T>
		 Logger &AttrList(const char *key, llvm::SmallVector<T *, 8> Vect);

	 };
}

#endif //SEMA_LOGGER_H
