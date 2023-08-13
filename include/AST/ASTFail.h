//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFail.h - Fail declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_AST_FAIL_H
#define FLY_AST_FAIL_H

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

namespace fly {

    enum class ASTErrorKind {
        ERR_NONE = 0,
        ERR_UNKNOWN = 1
    };

    class ASTFail : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const SourceLocation Loc;

        const llvm::StringRef Message;

        ASTErrorKind Kind = ASTErrorKind::ERR_NONE;

        ASTFail(const SourceLocation &Loc, ASTErrorKind ErrorKind);

        ASTFail(const SourceLocation &Loc, ASTErrorKind ErrorKind, llvm::StringRef Description);

        ~ASTFail();

    public:

        const SourceLocation &getLocation() const;

        llvm::StringRef getMessage() const;

        ASTErrorKind getKind() const;

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_AST_FAIL_H
