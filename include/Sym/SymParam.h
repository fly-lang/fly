//===--------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymParam.h - Symbol Param
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_PARAM_H
#define FLY_SYM_PARAM_H

#include "SymVar.h"
#include "CodeGen/CodeGenVar.h"

namespace fly {

	class CodeGenVar;

    class SymParam : public SymVar {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

		CodeGenVar *CodeGen;

        explicit SymParam(ASTVar *AST);

    public:

    	CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase * CGC) override;
    };

}  // end namespace fly

#endif // FLY_SYM_ENUM_ENTRY_H