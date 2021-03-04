//===--------------------------------------------------------------------------------------------------------------===//
// test/LexerTest.cpp - Lexer tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Lex/Lexer.h"
#include "Basic/Diagnostic.h"
#include "Basic/DiagnosticOptions.h"
#include "Basic/FileManager.h"
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "Basic/TargetInfo.h"
#include "Basic/TargetOptions.h"
#include "Basic/TokenKinds.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <vector>

namespace {
    using namespace fly;
    using testing::ElementsAre;

    // The test fixture.
    class LexerTest : public ::testing::Test {
    protected:
        LexerTest()
                : FileMgr(FileMgrOpts),
                  DiagID(new DiagnosticIDs()),
                  Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
                  SourceMgr(Diags, FileMgr),
                  TargetOpts(new TargetOptions) {
            TargetOpts->Triple = "x86_64-apple-darwin11.1.0";
            Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
        }

        std::vector<Token> Lex(StringRef Source) {

            std::unique_ptr<llvm::MemoryBuffer> Buf =
                    llvm::MemoryBuffer::getMemBuffer(Source);
            llvm::MemoryBuffer *b = Buf.get();
            const FileID &FID = SourceMgr.createFileID(std::move(Buf));
            SourceMgr.setMainFileID(FID);


            // Create a lexer starting at the beginning of this token.
            Lexer TheLexer(FID, b, SourceMgr);

            // Lex comments
            TheLexer.SetCommentRetentionState(true);

            std::vector<Token> toks;
            while (1) {
                Token tok;
                TheLexer.Lex(tok);
                if (tok.is(tok::eof))
                    break;
                toks.push_back(tok);
            }

            return toks;
        }

        void CheckTokens(std::vector<Token> &toks, std::vector<std::pair<std::string, tok::TokenKind>> &exps) {
            EXPECT_EQ(exps.size(), toks.size());

            std::vector<std::string> expVStrs;
            expVStrs.reserve(exps.size());
            unsigned i = 0;
            for (auto & tok : toks) {
                std::pair<std::string, tok::TokenKind> exp = exps.at(i);
                std::string expStr = exp.first;
                EXPECT_EQ(expStr, getSourceText(tok, tok));
                tok::TokenKind expKind = exp.second;
                EXPECT_EQ(expKind, tok.getKind()); // Kind Verified directly
                expVStrs.push_back(expStr);
                i++;
            }
        }

        std::string getSourceText(Token Begin, Token End) {
            bool Invalid;
            StringRef Str =
                    Lexer::getSourceText(CharSourceRange::getTokenRange(SourceRange(
                            Begin.getLocation(), End.getLocation())),
                                         SourceMgr, &Invalid);
            if (Invalid)
                return "<INVALID>";
            return Str;
        }

        FileSystemOptions FileMgrOpts;
        FileManager FileMgr;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
        DiagnosticsEngine Diags;
        SourceManager SourceMgr;
        std::shared_ptr<TargetOptions> TargetOpts;
        IntrusiveRefCntPtr<TargetInfo> Target;
    };

    TEST_F(LexerTest, PunctuatorLex) {
        StringRef str = ("[]"
                         "()"
                         "{} "
                         ". "
                         "... "
                         "& "
                         "&& "
                         "&= "
                         "* "
                         "*= "
                         "+ "
                         "++ "
                         "+= "
                         "- "
                         "-- "
                         "-= "
                         "! "
                         "!= "
                         "/ "
                         "/= "
                         "% "
                         "%= "
                         "< "
                         "<< "
                         "<= "
                         "<<= "
                         "> "
                         ">> "
                         ">= "
                         ">>= "
                         "^ "
                         "^= "
                         "| "
                         "|| "
                         "|= "
                         "? "
                         ": "
                         "; "
                         "= "
                         "== "
                         ", "
                         );
        auto toks = Lex(str);

        std::vector<std::pair<std::string, tok::TokenKind>> exps;
        exps.push_back({"[", tok::l_square});
        exps.push_back({"]", tok::r_square});
        exps.push_back({"(", tok::l_paren});
        exps.push_back({")", tok::r_paren});;
        exps.push_back({"{", tok::l_brace});
        exps.push_back({"}", tok::r_brace});
        exps.push_back({".", tok::period});
        exps.push_back({"...", tok::ellipsis});
        exps.push_back({"&", tok::amp});
        exps.push_back({"&&", tok::ampamp});
        exps.push_back({"&=", tok::ampequal});
        exps.push_back({"*", tok::star});
        exps.push_back({"*=", tok::starequal});
        exps.push_back({"+", tok::plus});
        exps.push_back({"++", tok::plusplus});
        exps.push_back({"+=", tok::plusequal});
        exps.push_back({"-", tok::minus});
        exps.push_back({"--", tok::minusminus});
        exps.push_back({"-=", tok::minusequal});
        exps.push_back({"!", tok::exclaim});
        exps.push_back({"!=", tok::exclaimequal});
        exps.push_back({"/", tok::slash});
        exps.push_back({"/=", tok::slashequal});
        exps.push_back({"%", tok::percent});
        exps.push_back({"%=", tok::percentequal});
        exps.push_back({"<", tok::less});
        exps.push_back({"<<", tok::lessless});
        exps.push_back({"<=", tok::lessequal});
        exps.push_back({"<<=", tok::lesslessequal});
        exps.push_back({">", tok::greater});
        exps.push_back({">>", tok::greatergreater});
        exps.push_back({">=", tok::greaterequal});
        exps.push_back({">>=", tok::greatergreaterequal});
        exps.push_back({"^", tok::caret});
        exps.push_back({"^=", tok::caretequal});
        exps.push_back({"|", tok::pipe});
        exps.push_back({"||", tok::pipepipe});
        exps.push_back({"|=", tok::pipeequal});
        exps.push_back({"?", tok::question});
        exps.push_back({":", tok::colon});
        exps.push_back({";", tok::semi});
        exps.push_back({"=", tok::equal});
        exps.push_back({"==", tok::equalequal});
        exps.push_back({",", tok::comma});

        CheckTokens(toks, exps);
    }

    TEST_F(LexerTest, KeywordLex) {
        StringRef str = ("package");
        auto toks = Lex(str);

        std::vector<std::pair<std::string, tok::TokenKind>> exps;
        exps.push_back({"package", tok::kw_package});

        CheckTokens(toks, exps);
    }
} // anonymous namespace
