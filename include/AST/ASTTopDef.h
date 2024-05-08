//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTopDef.h - AST Top Definition header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_TOPDEF_H
#define FLY_AST_TOPDEF_H

namespace llvm {
    class StringRef;
}

namespace fly {

    class SourceLocation;
    class ASTModule;
    class ASTNameSpace;
    class ASTScopes;

    enum class ASTTopDefKind {
        DEF_NAMESPACE,
        DEF_IMPORT,
        DEF_GLOBALVAR,
        DEF_FUNCTION,
        DEF_CLASS,
        DEF_ENUM
    };

    class ASTTopDef {

    public:

        virtual ASTModule *getModule() const = 0;

        virtual llvm::StringRef getName() const = 0;

        virtual ASTNameSpace *getNameSpace() const = 0;

        virtual ASTTopDefKind getTopDefKind() const = 0;

    };
}

#endif //FLY_AST_TOPDEF_H
