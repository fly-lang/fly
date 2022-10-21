//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBase.cpp - AST Base implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

#include "llvm/ADT/StringRef.h"

using namespace fly;

const char *Logger::OPEN = "{";
const char *Logger::EQ = "=";
const char *Logger::SEP = ",";
const char *Logger::CLOSE = "}";
const char *Logger::OPEN_LIST = "[";
const char *Logger::CLOSE_LIST = "]";

bool DebugEnabled = false;

Logger::Logger() {

}

Logger::Logger(const std::string str) : Str(std::string(str).append(OPEN)), isClass(true) {

}

std::string Logger::End() {
    if (isClass)
        Str.append(CLOSE);
    return Str;
}

Logger &Logger::Super(const std::string val) {
    isEmpty ? Str.append(val) : Str.append(SEP).append(val);
    isEmpty = false;
    return *this;
}

Logger &Logger::Attr(const char *key, const std::string val) {
    std::string Entry = Str.append(key).append(EQ).append(val);
    isEmpty ? Str.append(Entry) : Str.append(SEP).append(Entry);
    isEmpty = false;
    return *this;
}

Logger &Logger::Attr(const char *key, const llvm::StringRef val) {
    return Attr(key, val.str());
}

Logger &Logger::Attr(const char *key, const SourceLocation &val) {
    return Attr(key, std::to_string(val.getRawEncoding()));
}

Logger &Logger::Attr(const char *key, bool val) {
    return Attr(key, std::to_string(val));
}

Logger &Logger::Attr(const char *key, unsigned long val) {
    return Attr(key, std::to_string(val));
}

Logger &Logger::Attr(const char *key, unsigned int val) {
    return Attr(key, std::to_string(val));
}

Logger &Logger::Attr(const char *key, int val) {
    return Attr(key, std::to_string(val));
}

Logger &Logger::Attr(const char *key, Debuggable *val) {
    return Attr(key, val ? val->str() : "");
}
