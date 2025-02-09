//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Logger.cpp - The Sema
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Logger.h"
#include "Basic/SourceLocation.h"
#include "AST/ASTBase.h"

using namespace fly;


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

template <typename T>
Logger &Logger::AttrList(const char *key, llvm::SmallVector<T *, 8> Vect) {
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
