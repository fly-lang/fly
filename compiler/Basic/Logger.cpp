//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Basic/Logger.cpp - debug string builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Basic/Logger.h"
#include "Basic/SourceLocation.h"

using namespace fly;

class ASTBase;

const char *Logger::OPEN = "{";
const char *Logger::EQ = "=";
const char *Logger::SEP = ",";
const char *Logger::CLOSE = "}";
const char *Logger::OPEN_LIST = "[";
const char *Logger::CLOSE_LIST = "]";

bool DebugLog = false;
thread_local int DebugDepth = 0;

Logger::Logger() = default;

Logger::Logger(std::string str) : Str(str.append(OPEN)) {

}

std::string Logger::End() {
	Str.append(CLOSE);
	return Str;
}

std::string Logger::str(const char *key, std::string val) {
    return std::string(key).append(EQ).append(val);
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

Logger &Logger::Attr(const char *key, const SourceLocation &val) {
	return Attr(key, (uint64_t) val.getRawEncoding());
}

Logger &Logger::Attr(const char *key, bool val) {
	return Attr(key, std::to_string(val));
}

Logger &Logger::Attr(const char *key, uint64_t val) {
    return Attr(key, std::to_string(val));
}
