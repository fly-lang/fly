//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTBase.cpp - AST Base implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTBase.h"

using namespace fly;

ASTBase::ASTBase(const SourceLocation &Loc) : Location(Loc) {

}

const SourceLocation &ASTBase::getLocation() const {
    return Location;
}

llvm::StringRef ASTBase::getComment() const {
    return Comment;
}

std::string ASTBase::str() const {
    return Logger("ASTBase").
            Attr("Location", (uint64_t) Location.getRawEncoding()).
            Attr("Comment", Comment).
            End();
}

const char *Logger::OPEN = "{";
const char *Logger::EQ = "=";
const char *Logger::SEP = ",";
const char *Logger::CLOSE = "}";
const char *Logger::OPEN_LIST = "[";
const char *Logger::CLOSE_LIST = "]";

bool DebugEnabled = false;

Logger::Logger() = default;

Logger::Logger(std::string str) : Str(str.append(OPEN)), isClass(true) {

}

std::string Logger::End() {
    if (isClass)
        Str.append(CLOSE);
    return Str;
}

Logger &Logger::Super(std::string val) {
    isEmpty ? Str.append(val) : Str.append(SEP).append(val);
    isEmpty = false;
    return *this;
}

Logger &Logger::Attr(const char *key, std::string val) {
    std::string entry = std::string(key).append(EQ).append(val);
    isEmpty ? Str.append(entry) : Str.append(SEP).append(" ").append(entry);
    isEmpty = false;
    return *this;
}

Logger &Logger::Attr(const char *key, llvm::StringRef val) {
    return Attr(key, val.str());
}

Logger &Logger::Attr(const char *key, SourceLocation &val) {
    return Attr(key, (uint64_t) val.getRawEncoding());
}

Logger &Logger::Attr(const char *key, bool val) {
    return Attr(key, std::to_string(val));
}

Logger &Logger::Attr(const char *key, uint64_t val) {
    return Attr(key, std::to_string(val));
}

Logger &Logger::Attr(const char *key, ASTBase *val) {
    return Attr(key, val ? val->str() : "");
}
