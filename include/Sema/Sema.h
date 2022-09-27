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

namespace fly {

    class SemaBuilder;
    class SemaResolver;
    class SemaValidator;
    class DiagnosticsEngine;
    class DiagnosticBuilder;
    class SourceLocation;

    class Sema {

        friend class SemaBuilder;
        friend class SemaResolver;

        DiagnosticsEngine &Diags;

        SemaBuilder *Builder = nullptr;

        SemaResolver *Resolver = nullptr;

        SemaValidator *Validator = nullptr;

        Sema(DiagnosticsEngine &Diags);

    public:

        static SemaBuilder* Build(DiagnosticsEngine &Diags);

        DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const;

        DiagnosticBuilder Diag(unsigned DiagID) const;
    };

}  // end namespace fly

#endif