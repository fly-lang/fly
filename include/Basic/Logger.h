//
// include/Basic/Logger.h - debug string builder
//

#ifndef SEMA_LOGGER_H
#define SEMA_LOGGER_H

#include "string"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

namespace fly {
    class ASTNode;
    class SourceLocation;

    class Logger
    {
        std::string Str;
        bool isEmpty = true;

    public:
        static const char* OPEN;
        static const char* SEP;
        static const char* EQ;
        static const char* CLOSE;
        static const char* OPEN_LIST;
        static const char* CLOSE_LIST;

        Logger();

        explicit Logger(std::string str);

        std::string End();

        static std::string str(const char* key, std::string val);

        Logger& Attr(const char* key, std::string val);

        Logger& Attr(const char* key, llvm::StringRef val);

        Logger& Attr(const char* key, const SourceLocation& val);

        Logger& Attr(const char* key, bool val);

        // unsigned long and unsigned long long are distinct types everywhere, while
        // size_t and uint64_t are one type on Linux/Windows but two on macOS:
        // overloading on the former lets both size_t and uint64_t resolve.
        Logger& Attr(const char* key, unsigned long val);

        Logger& Attr(const char* key, unsigned long long val);

        // Null-safe pointer: calls val->str() or prints "null"
        template<typename T>
        Logger& Attr(const char* key, const T* val) {
            return Attr(key, val ? val->str() : std::string("null"));
        }

        // SmallVector of pointers: prints [e1,e2,...] using each element's str()
        template<typename T, unsigned N>
        Logger& Attr(const char* key, const llvm::SmallVector<T*, N>& vec) {
            std::string S = Logger::OPEN_LIST;
            bool first = true;
            for (auto* V : vec) {
                if (!first) S += Logger::SEP;
                S += V ? V->str() : "null";
                first = false;
            }
            S += Logger::CLOSE_LIST;
            return Attr(key, S);
        }
    };
}

#endif //SEMA_LOGGER_H
