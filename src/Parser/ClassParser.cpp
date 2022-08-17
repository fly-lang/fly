//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ClassParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/ClassParser.h"
#include "AST/ASTClass.h"
#include "Sema/SemaBuilder.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 * @param Visibility
 * @param Constant
 */
ClassParser::ClassParser(Parser *P, VisibilityKind &Visibility, bool &Constant) : P(P) {
    assert(P->Tok.isAnyIdentifier() && "Tok must be an Identifier");

    IdentifierInfo *Id = P->Tok.getIdentifierInfo();
    llvm::StringRef Name = Id->getName();
    const SourceLocation Loc = P->Tok.getLocation();
    P->ConsumeToken();

    Class = P->Builder.CreateClass(P->Node, Loc, Name.str(), Visibility, Constant);

    // ParseAttributes() && ParseMethods()
}

/**
 * Parse Class Declaration
 * @return
 */
ASTClass *ClassParser::Parse(Parser *P, VisibilityKind &Visibility, bool &Constant) {
    ClassParser *CP = new ClassParser(P, Visibility, Constant);
    return CP->Class;
}
