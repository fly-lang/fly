//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_CLASS_H
#define FLY_SYM_CLASS_H

#include "Sym/SymType.h"
#include "AST/ASTClass.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

#include <map>
#include <llvm/ADT/DenseMap.h>

namespace fly {

    class Sema;
    class ASTClass;
    class SymClassAttribute;
    class SymComment;
    class SymClassMethod;
    class CodeGenClass;
    class SymModule;
    enum class SymVisibilityKind;

    class SymClass : public SymType {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTClass *AST;

        SymModule *Module;

        // Class Attributes
        llvm::StringMap<SymClassAttribute *> Attributes;

        llvm::DenseMap<size_t, SymClassMethod *> Methods;

        llvm::DenseMap<size_t, SymClassMethod *> Constructors;

        // Class Functions
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <SymClassMethod *, 4>>> SearchFunctions;

        SymComment *Comment = nullptr;

        CodeGenClass *CodeGen = nullptr;

        explicit SymClass(ASTClass *Class);

    public:

        ASTClass *getAST();

        SymModule *getModule() const;

        llvm::StringMap<SymClassAttribute *> getAttributes() const;

        llvm::DenseMap<size_t, SymClassMethod *> getMethods() const;

        llvm::DenseMap<size_t, SymClassMethod *> getConstructors() const;

        SymComment *getComment() const;

        CodeGenClass *getCodeGen() const;

        void setCodeGen(CodeGenClass *CGC);
    };

}  // end namespace fly

#endif // FLY_SYM_CLASS_H