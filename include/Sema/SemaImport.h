//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaImport.h - AST Context header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_IMPORT_H
#define FLY_SEMA_IMPORT_H

namespace fly {

	class ASTImport;

    /**
     * AST Context
     */
    class SemaImport {

        ASTImport &AST;

    public:
        SemaImport(ASTImport &AST);

		ASTImport* getAST() const;
    };
}

#endif //FLY_SEMA_IMPORT_H
