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
#include <Basic/Debug.h>

// third party
#include <AST/ASTBuilder.h>
#include <AST/ASTType.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

#ifndef FLY_PARSERTEST_H
#define FLY_PARSERTEST_H

using namespace fly;

class ParserTest : public ::testing::Test {

public:
    std::shared_ptr<CompilerInstance> CI;
    // Sema *S;
    DiagnosticsEngine &Diags;
    ASTBuilder *Builder;
    llvm::SmallVector<ASTModule *, 8> ASTModules;
    // llvm::SmallVector<SemaModule*, 8> SemaModules;

    // Store the last parsed source text so tests can compute expected token positions
    std::string LastParseSource;

    // Enable debug messages for all tests in this suite
    //static void SetUpTestCase() {
    //    DebugLog = true;
    //}

    // Disable debug messages after all tests complete
    //static void TearDownTestCase() {
    //    DebugLog = false;
    //}

    ParserTest() : CI(TestUtils::CreateCompilerInstance()),
        Diags(CI->getDiagnostics()), Builder(new ASTBuilder(Diags))
        // ,S(new Sema(CI.getDiagnostics()))
    {

    }

    virtual ~ParserTest() {
        delete Builder;
    }

protected:

    ASTModule *Parse(std::string Name, llvm::StringRef Source) {
        Diags.getClient()->BeginSourceFile();
        auto FID = new InputFile(Diags, CI->getSourceManager(), Name);
        // Load the provided source into the InputFile so the Parser can access the buffer
        FID->Load(Source);
        // Keep a copy of the source text so tests can search tokens and compute expected locations
        LastParseSource = Source.str();
        Parser *P = new Parser(FID, CI->getSourceManager(), Diags, *Builder);
        ASTModule *AST = P->ParseModule();
        Diags.getClient()->EndSourceFile();
        ASTModules.push_back(AST);
        return AST;
    }

    bool HasErrorOccurred() {
        return Diags.hasErrorOccurred();
    }

    // bool Resolve() {
    //     Diags.getClient()->BeginSourceFile();
    //     SemaModules = S->Resolve(ASTModules);
    //     Diags.getClient()->EndSourceFile();
    //     return !Diags.hasErrorOccurred();
    // }

    template<typename T, typename U>
    static T *As(U *Ptr) {
        return static_cast<T *>(Ptr);
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

    void TearDown() override {
        ASTModules.clear();
    }

    // Helper: convert a SourceLocation to (line, column) using the SourceManager (1-based)
    std::pair<unsigned, unsigned> LocToLineCol(const SourceLocation &L) {
        bool Invalid = false;
        unsigned Line = CI->getSourceManager().getSpellingLineNumber(L, &Invalid);
        unsigned Col = CI->getSourceManager().getSpellingColumnNumber(L, &Invalid);
        return { Line, Col };
    }

    // Helper: find the N-th occurrence of Token in the LastParseSource and return (line, column) 1-based.
    // Matches whole tokens only (checks identifier boundaries). If not found, returns {0,0}.
    std::pair<unsigned, unsigned> FindTokenLineCol(llvm::StringRef Token, unsigned Occurrence = 1) {
        if (LastParseSource.empty())
            return {0,0};

        auto isIdentChar = [](char c) -> bool {
            return (std::isalnum(static_cast<unsigned char>(c)) || c == '_');
        };

        size_t pos = 0;
        size_t from = 0;
        unsigned found = 0;
        const std::string &S = LastParseSource;
        const size_t TokLen = Token.size();
        while (true) {
            pos = S.find(std::string(Token), from);
            if (pos == std::string::npos) {
                break;
            }

            // Check left boundary
            bool left_ok = (pos == 0) || !isIdentChar(S[pos - 1]);
            // Check right boundary
            size_t right_pos = pos + TokLen;
            bool right_ok = (right_pos >= S.size()) || !isIdentChar(S[right_pos]);

            if (left_ok && right_ok) {
                found++;
                if (found == Occurrence) {
                    // compute line and column from pos
                    unsigned line = 1;
                    unsigned col = 1;
                    for (size_t i = 0; i < pos; ++i) {
                        if (S[i] == '\n') {
                            ++line;
                            col = 1;
                        } else {
                            ++col;
                        }
                    }
                    return {line, col};
                }
            }

            from = pos + 1;
        }

        return {0,0};
    }

    // Expect the SourceLocation L to match expected (line,col)
    void EXPECT_LOC_EQ(const SourceLocation &L, unsigned ExpectLine, unsigned ExpectCol) {
        auto P = LocToLineCol(L);
        unsigned LLine = P.first;
        unsigned LCol = P.second;
        EXPECT_EQ(LLine, ExpectLine);
        EXPECT_EQ(LCol, ExpectCol);
    }

    // Convenience: assert that node's location matches the N-th occurrence of Token in the last parsed source
    template<typename NodeT>
    void EXPECT_NODE_LOC(NodeT *Node, llvm::StringRef Token, unsigned Occurrence = 1) {
        auto Expected = FindTokenLineCol(Token, Occurrence);
        if (Expected.first == 0 && Expected.second == 0) {
            FAIL() << "Token '" << Token.str() << "' not found in last parsed source";
            return;
        }
        EXPECT_LOC_EQ(Node->getLocation(), Expected.first, Expected.second);
    }

};

#endif //FLY_PARSERTEST_H
