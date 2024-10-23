//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/FunctionParser.cpp - Function Declaration and Call Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ParserMethod.h"
#include "Parser/ParserClass.h"
#include "Parser/Parser.h"
#include "AST/ASTClass.h"
#include "Sema/SemaBuilder.h"

#include <vector>

using namespace fly;

ParserMethod::ParserMethod(ParserClass *PC) : ParserFunction(PC->P) {

}

ASTClassMethod *ParserMethod::Parse(ParserClass *PC, llvm::SmallVector<ASTScope *, 8> Scopes, ASTType *Type,
                                 const SourceLocation &Loc, llvm::StringRef Name) {

    ParserMethod *PF = new ParserMethod(PC);
    SmallVector<ASTParam *, 8> Params = PF->ParseParams();

    ASTClassMethod *Method = (Name == PC->Class->getName()) ?
                             PC->P->Builder.CreateClassConstructor(Loc, *PC->Class, Scopes, Params) :
                             PC->P->Builder.CreateClassMethod(Loc, *PC->Class, Type, Name, Scopes, Params);

    ASTBlockStmt *Body = PC->P->isBlockStart() ? PF->ParseBody((ASTFunctionBase *) Method) : nullptr;
    return Method;
}
