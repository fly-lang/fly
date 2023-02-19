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
    class ASTNameSpace;
    class ASTNode;
    class ASTClass;
    class ASTClassVar;
    class ASTClassFunction;
    class ASTFunctionBase;
    class ASTFunction;
    class ASTImport;
    class ASTLocalVar;
    class ASTVarRef;
    class ASTVar;
    class ASTBlock;

    class Sema {

        friend class SemaBuilder;
        friend class SemaResolver;

        DiagnosticsEngine &Diags;

        SemaBuilder *Builder = nullptr;

        SemaResolver *Resolver = nullptr;

        SemaValidator *Validator = nullptr;

        Sema(DiagnosticsEngine &Diags);

    public:

        static SemaBuilder* CreateBuilder(DiagnosticsEngine &Diags);

        ASTNameSpace *FindNameSpace(llvm::StringRef Name) const;

        ASTNode *FindNode(ASTFunctionBase *FunctionBase) const;

        ASTNode *FindNode(llvm::StringRef Name, ASTNameSpace *NameSpace) const;

        ASTClass *FindClass(llvm::StringRef ClassName, ASTNameSpace *NameSpace) const;

        ASTLocalVar *FindVarDef(ASTBlock *Block, llvm::StringRef VarName) const;

        ASTImport *FindImport(ASTNode *Node, llvm::StringRef Name);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;
    };

}  // end namespace fly

#endif