//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBase.h - AST Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BASE_H
#define FLY_AST_BASE_H

#include "Basic/SourceLocation.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/StringRef.h"

#include <string>

namespace fly {

    class ASTComment;

    class ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SourceLocation Location;

        ASTComment *Comment = nullptr;

    public:

        explicit ASTBase(const SourceLocation &Loc);

        virtual const SourceLocation &getLocation() const;

        ASTComment *getComment() const;

        virtual std::string str() const = 0;
    };

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

        Logger &Attr(const char *key, uint64_t val);

        Logger &Attr(const char *key, ASTBase *val);

        template <typename T>
        Logger &AttrList(const char *key, llvm::SmallVector<T *, 8> Vect) {
            std::string Entry = OPEN_LIST;
            if(!Vect.empty()) {
                for (ASTBase *V : Vect) {
                    Entry += V->str() + SEP;
                }
                unsigned long end = Entry.length()-std::string(SEP).length()-1;
                Entry = Entry.substr(0, end);
            }
            Entry += CLOSE_LIST;
            Attr(key, Entry);
            isEmpty = false;
            return *this;
        }

    };
}

#endif //FLY_AST_BASE_H