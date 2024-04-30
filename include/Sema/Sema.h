//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_H
#define FLY_SEMA_H

namespace llvm {
    class StringRef;
}

namespace fly {

    class SemaBuilder;
    class SemaResolver;
    class SemaValidator;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;
    class ASTContext;
    class ASTNameSpace;
    class ASTNode;
    class ASTIdentity;
    class ASTClassAttribute;
    class ASTClassMethod;
    class ASTFunctionBase;
    class ASTFunction;
    class ASTImport;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTVar;
    class ASTBlock;
    class ASTIdentifier;
    class ASTIdentityType;

    class Sema {

        friend class SemaBuilder;
        friend class SemaResolver;

        DiagnosticsEngine &Diags;

        ASTContext *Context = nullptr;

        SemaBuilder *Builder = nullptr;

        SemaResolver *Resolver = nullptr;

        SemaValidator *Validator = nullptr;

        Sema(DiagnosticsEngine &Diags);

    public:

        static Sema* CreateSema(DiagnosticsEngine &Diags);

        SemaBuilder &getBuilder();

        DiagnosticsEngine &getDiags() const;

        SemaResolver &getResolver() const;

        SemaValidator &getValidator() const;

        ASTContext &getContext() const;

        bool Resolve();

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;

        ASTNameSpace *FindNameSpace(llvm::StringRef Name) const;

        ASTNode *FindNode(ASTFunctionBase *FunctionBase) const;

        ASTNode *FindNode(llvm::StringRef Name, ASTNameSpace *NameSpace) const;

        ASTIdentity *FindIdentity(llvm::StringRef Name, ASTNameSpace *NameSpace) const;

        ASTIdentityType *FindIdentityType(llvm::StringRef Name, ASTNameSpace *NameSpace) const;

        ASTVar *FindLocalVar(ASTBlock *Block, llvm::StringRef Name) const;

        ASTImport *FindImport(ASTNode *Node, llvm::StringRef Name);
    };

}  // end namespace fly

#endif // FLY_SEMA_H