//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/Debuggable.h - AST Base Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_BASIC_DEBUGGABLE_H
#define FLY_BASIC_DEBUGGABLE_H

#include <vector>
#include <string>

namespace llvm {
    class StringRef;
}

namespace fly {

    class SourceLocation;

    class Debuggable {

    public:

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

        Logger(const std::string str);

        std::string End();

        Logger &Super(const std::string val);

        Logger &Attr(const char *key, const std::string val);

        Logger &Attr(const char *key, const llvm::StringRef val);

        Logger &Attr(const char *key, const SourceLocation &val);

        Logger &Attr(const char *key, bool val);

        Logger &Attr(const char *key, unsigned long val);

        Logger &Attr(const char *key, unsigned int val);

        Logger &Attr(const char *key, int val);

        Logger &Attr(const char *key, Debuggable *val);

        template <typename T>
        Logger &AttrList(const char *key, std::vector<T *> Vect) {
            std::string Entry = OPEN_LIST;
            if(!Vect.empty()) {
                for (Debuggable *V : Vect) {
                    Entry += V->str() + SEP;
                }
                unsigned long end = Entry.length()-std::string(SEP).length()-1;
                Entry = Entry.substr(0, end);
            }
            Entry += CLOSE_LIST;
            isEmpty = false;
            return Attr(key, Entry);
        }

    };
}

#endif //FLY_BASIC_DEBUGGABLE_H