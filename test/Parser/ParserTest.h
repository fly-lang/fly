//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

// fly
#include "../TestUtils.h"
#include "Parser/Parser.h"
#include "Sema/Sema.h"

// third party
#include <AST/ASTBuilder.h>
#include <AST/ASTType.h>
#include <gtest/gtest.h>

#ifndef FLY_PARSERTEST_H
#define FLY_PARSERTEST_H

using namespace fly;

class ParserTest : public ::testing::Test {

public:
    const CompilerInstance CI;
    Sema *S;
    DiagnosticsEngine &Diags;
    ASTBuilder *Builder;
    llvm::SmallVector<ASTModule *, 8> ASTModules;
    llvm::SmallVector<SemaModule*, 8> SemaModules;

    ParserTest() : CI(*TestUtils::CreateCompilerInstance()),
        Diags(CI.getDiagnostics()), Builder(new ASTBuilder(Diags)),
        S(new Sema(CI.getDiagnostics())) {

    }

    virtual ~ParserTest() {
        delete S;
    }

    ASTModule *Parse(std::string Name, llvm::StringRef Source) {
        Diags.getClient()->BeginSourceFile();
        auto Buffer = llvm::MemoryBuffer::getMemBuffer("", Name);
        auto FID = new InputFile(Diags, CI.getSourceManager(), Name);
        Parser *P = new Parser(FID, CI.getSourceManager(), Diags, *Builder);
        ASTModule *AST = P->ParseModule();
        Diags.getClient()->EndSourceFile();

        return AST;
    }

    bool Resolve() {
        Diags.getClient()->BeginSourceFile();
        SemaModules = S->Resolve(ASTModules);
        Diags.getClient()->EndSourceFile();
        return !Diags.hasErrorOccurred();
    }

    static bool HasModifier(const llvm::SmallVector<ASTModifier *, 8> &Modifiers, ASTModifierKind Kind) {;
        for (auto &Mod : Modifiers) {
            if (Mod->getModifierKind() == Kind) {
                return true;
            }
        }
        return false;
    }

    static bool HasBuiltinType(ASTType *Type, ASTBuiltinTypeKind Kind) {
		if (Type->getTypeKind() != ASTTypeKind::TYPE_BUILTIN) {
			return false;
		}
		ASTBuiltinType *BuiltinType = static_cast<ASTBuiltinType *>(Type);
		return BuiltinType->getBuiltinKind() == Kind;
	}

};

#endif //FLY_PARSERTEST_H
