//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTReference.h - Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_REFERENCE_H
#define FLY_AST_REFERENCE_H

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

namespace fly {

    class ASTVar;
    class ASTIdentifier;
    class CodeGenInstance;

    enum class ASTReferenceKind {
        REF_CALL, REF_VAR
    };

    class ASTReference : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTIdentifier *Identifier = nullptr;

        ASTReference *Instance = nullptr;

        ASTReferenceKind RefKind;

    protected:

        ASTReference(ASTIdentifier *Identifier, ASTReferenceKind RefKind);

    public:

        SourceLocation getLocation() const;

        llvm::StringRef getName() const;

        ASTIdentifier *getIdentifier() const;

        ASTReference *getInstance() const;

        ASTReferenceKind getRefKind() const;

        bool isCall() const;
    };
}

#endif //FLY_AST_REFERENCE_H
