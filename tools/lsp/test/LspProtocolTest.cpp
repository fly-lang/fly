//===----------------------------------------------------------------------===//
// tools/lsp/test/LspProtocolTest.cpp — unit tests for LspProtocol
//===----------------------------------------------------------------------===//
#include <gtest/gtest.h>
#include "LspProtocol.h"
#include <llvm/Support/JSON.h>

using namespace fly::lsp;
using namespace llvm;

// ── URI helpers ──────────────────────────────────────────────────────────────

TEST(LspProtocol, FileUriToPath_WithPrefix) {
    EXPECT_EQ(fileUriToPath("file:///tmp/foo.fly"), "/tmp/foo.fly");
}

TEST(LspProtocol, FileUriToPath_StripsPrefixOnly) {
    EXPECT_EQ(fileUriToPath("file://foo.fly"), "foo.fly");
}

TEST(LspProtocol, FileUriToPath_NoPrefix_Passthrough) {
    EXPECT_EQ(fileUriToPath("/tmp/bar.fly"), "/tmp/bar.fly");
}

TEST(LspProtocol, FileUriToPath_Empty) {
    EXPECT_EQ(fileUriToPath(""), "");
}

TEST(LspProtocol, PathToFileUri_AddsPrefix) {
    EXPECT_EQ(pathToFileUri("/tmp/foo.fly"), "file:///tmp/foo.fly");
}

TEST(LspProtocol, PathToFileUri_Empty) {
    EXPECT_EQ(pathToFileUri(""), "file://");
}

TEST(LspProtocol, RoundTrip_UriPath) {
    std::string path = "/home/user/project/main.fly";
    EXPECT_EQ(fileUriToPath(pathToFileUri(path)), path);
}

// ── positionFromJson ─────────────────────────────────────────────────────────

TEST(LspProtocol, PositionFromJson_Normal) {
    json::Object obj{{"line", 5}, {"character", 12}};
    auto p = positionFromJson(obj);
    EXPECT_EQ(p.line, 5);
    EXPECT_EQ(p.character, 12);
}

TEST(LspProtocol, PositionFromJson_Zero) {
    json::Object obj{{"line", 0}, {"character", 0}};
    auto p = positionFromJson(obj);
    EXPECT_EQ(p.line, 0);
    EXPECT_EQ(p.character, 0);
}

TEST(LspProtocol, PositionFromJson_MissingKeys_DefaultsToZero) {
    json::Object obj{};
    auto p = positionFromJson(obj);
    EXPECT_EQ(p.line, 0);
    EXPECT_EQ(p.character, 0);
}

TEST(LspProtocol, PositionFromJson_MissingCharacter_DefaultsToZero) {
    json::Object obj{{"line", 3}};
    auto p = positionFromJson(obj);
    EXPECT_EQ(p.line, 3);
    EXPECT_EQ(p.character, 0);
}

TEST(LspProtocol, PositionFromJson_LargeValues) {
    json::Object obj{{"line", 9999}, {"character", 255}};
    auto p = positionFromJson(obj);
    EXPECT_EQ(p.line, 9999);
    EXPECT_EQ(p.character, 255);
}

// ── toJson(LspPosition) ──────────────────────────────────────────────────────

TEST(LspProtocol, ToJsonPosition_BasicValues) {
    LspPosition p{7, 3};
    auto obj = toJson(p);
    EXPECT_EQ(obj.getInteger("line").value_or(-1), 7);
    EXPECT_EQ(obj.getInteger("character").value_or(-1), 3);
}

TEST(LspProtocol, ToJsonPosition_Zero) {
    LspPosition p{0, 0};
    auto obj = toJson(p);
    EXPECT_EQ(obj.getInteger("line").value_or(-1), 0);
    EXPECT_EQ(obj.getInteger("character").value_or(-1), 0);
}

TEST(LspProtocol, ToJsonPosition_ExactKeys) {
    LspPosition p{1, 2};
    auto obj = toJson(p);
    EXPECT_NE(obj.get("line"), nullptr);
    EXPECT_NE(obj.get("character"), nullptr);
    // No extra keys
    EXPECT_EQ(obj.get("col"), nullptr);
    EXPECT_EQ(obj.get("column"), nullptr);
}

// ── toJson(LspRange) ─────────────────────────────────────────────────────────

TEST(LspProtocol, ToJsonRange_NestedPositions) {
    LspRange r{{1, 0}, {1, 10}};
    auto obj = toJson(r);
    auto *start = obj.getObject("start");
    auto *end   = obj.getObject("end");
    ASSERT_NE(start, nullptr);
    ASSERT_NE(end,   nullptr);
    EXPECT_EQ(start->getInteger("line").value_or(-1), 1);
    EXPECT_EQ(start->getInteger("character").value_or(-1), 0);
    EXPECT_EQ(end->getInteger("line").value_or(-1), 1);
    EXPECT_EQ(end->getInteger("character").value_or(-1), 10);
}

TEST(LspProtocol, ToJsonRange_MultiLine) {
    LspRange r{{3, 4}, {7, 0}};
    auto obj = toJson(r);
    auto *s = obj.getObject("start");
    auto *e = obj.getObject("end");
    ASSERT_NE(s, nullptr);
    ASSERT_NE(e, nullptr);
    EXPECT_EQ(s->getInteger("line").value_or(-1), 3);
    EXPECT_EQ(e->getInteger("line").value_or(-1), 7);
}

TEST(LspProtocol, ToJsonRange_ZeroRange) {
    LspRange r{{0, 0}, {0, 0}};
    auto obj = toJson(r);
    ASSERT_NE(obj.getObject("start"), nullptr);
    ASSERT_NE(obj.getObject("end"), nullptr);
}

// ── toJson(LspLocation) ──────────────────────────────────────────────────────

TEST(LspProtocol, ToJsonLocation_UriAndRange) {
    LspLocation loc{"file:///project/a.fly", {{0, 0}, {0, 5}}};
    auto obj = toJson(loc);
    EXPECT_EQ(obj.getString("uri").value_or(""), "file:///project/a.fly");
    auto *range = obj.getObject("range");
    ASSERT_NE(range, nullptr);
    auto *start = range->getObject("start");
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->getInteger("line").value_or(-1), 0);
    EXPECT_EQ(start->getInteger("character").value_or(-1), 0);
}

TEST(LspProtocol, ToJsonLocation_HasExactKeys) {
    LspLocation loc{"file:///a.fly", {{1, 2}, {3, 4}}};
    auto obj = toJson(loc);
    EXPECT_NE(obj.get("uri"), nullptr);
    EXPECT_NE(obj.get("range"), nullptr);
}

// ── toJson(LspDiagnostic) ────────────────────────────────────────────────────

TEST(LspProtocol, ToJsonDiagnostic_ErrorSeverity) {
    LspDiagnostic d;
    d.range    = {{2, 4}, {2, 9}};
    d.severity = 1;
    d.message  = "undefined variable 'x'";
    auto obj = toJson(d);
    EXPECT_EQ(obj.getInteger("severity").value_or(-1), 1);
    EXPECT_EQ(obj.getString("message").value_or(""), "undefined variable 'x'");
}

TEST(LspProtocol, ToJsonDiagnostic_SourceIsAlwaysFly) {
    LspDiagnostic d;
    d.message = "test";
    auto obj = toJson(d);
    EXPECT_EQ(obj.getString("source").value_or(""), "fly");
}

TEST(LspProtocol, ToJsonDiagnostic_InternalFileFieldNotExposed) {
    LspDiagnostic d;
    d.file = "/some/internal/path.fly";
    d.message = "msg";
    auto obj = toJson(d);
    EXPECT_EQ(obj.get("file"), nullptr);
}

TEST(LspProtocol, ToJsonDiagnostic_WarningSeverity) {
    LspDiagnostic d;
    d.severity = 2;
    d.message  = "unused variable";
    auto obj = toJson(d);
    EXPECT_EQ(obj.getInteger("severity").value_or(-1), 2);
}

TEST(LspProtocol, ToJsonDiagnostic_HintSeverity) {
    LspDiagnostic d;
    d.severity = 4;
    d.message  = "consider using const";
    auto obj = toJson(d);
    EXPECT_EQ(obj.getInteger("severity").value_or(-1), 4);
}

TEST(LspProtocol, ToJsonDiagnostic_RangeIsPresent) {
    LspDiagnostic d;
    d.range = {{5, 0}, {5, 15}};
    auto obj = toJson(d);
    auto *range = obj.getObject("range");
    ASSERT_NE(range, nullptr);
    auto *start = range->getObject("start");
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->getInteger("line").value_or(-1), 5);
}

// ── toJson(LspDocSymbol) ─────────────────────────────────────────────────────

TEST(LspProtocol, ToJsonDocSymbol_Function) {
    LspDocSymbol s{"doSomething", LspSymbolKind::Function, {{0,0},{0,10}}, {{0,0},{0,10}}};
    auto obj = toJson(s);
    EXPECT_EQ(obj.getString("name").value_or(""), "doSomething");
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 12); // Function = 12
    ASSERT_NE(obj.getObject("range"), nullptr);
    ASSERT_NE(obj.getObject("selectionRange"), nullptr);
}

TEST(LspProtocol, ToJsonDocSymbol_Class) {
    LspDocSymbol s{"MyClass", LspSymbolKind::Class, {{5,0},{5,7}}, {{5,0},{5,7}}};
    auto obj = toJson(s);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 5); // Class = 5
}

TEST(LspProtocol, ToJsonDocSymbol_Struct) {
    LspDocSymbol s{"Point", LspSymbolKind::Struct, {{0,0},{0,5}}, {{0,0},{0,5}}};
    auto obj = toJson(s);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 23); // Struct = 23
}

TEST(LspProtocol, ToJsonDocSymbol_Interface) {
    LspDocSymbol s{"IReader", LspSymbolKind::Interface, {{0,0},{0,7}}, {{0,0},{0,7}}};
    auto obj = toJson(s);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 11); // Interface = 11
}

TEST(LspProtocol, ToJsonDocSymbol_Method) {
    LspDocSymbol s{"read", LspSymbolKind::Method, {{10,4},{10,8}}, {{10,4},{10,8}}};
    auto obj = toJson(s);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 6); // Method = 6
}

TEST(LspProtocol, ToJsonDocSymbol_RangeAndSelectionRange) {
    LspRange full{{2, 0}, {5, 1}};
    LspRange sel {{2, 9}, {2, 13}};
    LspDocSymbol s{"main", LspSymbolKind::Function, full, sel};
    auto obj = toJson(s);
    auto *range = obj.getObject("range");
    auto *sel2  = obj.getObject("selectionRange");
    ASSERT_NE(range, nullptr);
    ASSERT_NE(sel2, nullptr);
    EXPECT_EQ(range->getObject("end")->getInteger("line").value_or(-1), 5);
    EXPECT_EQ(sel2->getObject("start")->getInteger("character").value_or(-1), 9);
}

// ── toJson(LspCompletionItem) ────────────────────────────────────────────────

TEST(LspProtocol, ToJsonCompletion_MinimalFields) {
    LspCompletionItem c;
    c.label = "foo";
    c.kind  = LspCompletionKind::Function;
    auto obj = toJson(c);
    EXPECT_EQ(obj.getString("label").value_or(""), "foo");
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 3); // Function = 3
    // Optional fields must be absent when empty
    EXPECT_EQ(obj.get("detail"), nullptr);
    EXPECT_EQ(obj.get("insertText"), nullptr);
}

TEST(LspProtocol, ToJsonCompletion_AllFields) {
    LspCompletionItem c;
    c.label      = "myFunc";
    c.kind       = LspCompletionKind::Field;
    c.detail     = "int myFunc()";
    c.insertText = "myFunc()";
    auto obj = toJson(c);
    EXPECT_EQ(obj.getString("detail").value_or(""), "int myFunc()");
    EXPECT_EQ(obj.getString("insertText").value_or(""), "myFunc()");
}

TEST(LspProtocol, ToJsonCompletion_KeywordKind) {
    LspCompletionItem c;
    c.label = "class";
    c.kind  = LspCompletionKind::Keyword;
    auto obj = toJson(c);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 14); // Keyword = 14
}

TEST(LspProtocol, ToJsonCompletion_VariableKind) {
    LspCompletionItem c;
    c.label = "x";
    c.kind  = LspCompletionKind::Variable;
    auto obj = toJson(c);
    EXPECT_EQ(obj.getInteger("kind").value_or(-1), 6); // Variable = 6
}

TEST(LspProtocol, ToJsonCompletion_DetailWithoutInsertText) {
    LspCompletionItem c;
    c.label  = "pi";
    c.kind   = LspCompletionKind::Variable;
    c.detail = "float";
    auto obj = toJson(c);
    EXPECT_NE(obj.get("detail"), nullptr);
    EXPECT_EQ(obj.get("insertText"), nullptr);
}

// ── LSP spec constant values ─────────────────────────────────────────────────

TEST(LspProtocol, SymbolKindValues_MatchLspSpec) {
    EXPECT_EQ((int)LspSymbolKind::Module,    2);
    EXPECT_EQ((int)LspSymbolKind::Class,     5);
    EXPECT_EQ((int)LspSymbolKind::Method,    6);
    EXPECT_EQ((int)LspSymbolKind::Field,     8);
    EXPECT_EQ((int)LspSymbolKind::Enum,     10);
    EXPECT_EQ((int)LspSymbolKind::Interface,11);
    EXPECT_EQ((int)LspSymbolKind::Function, 12);
    EXPECT_EQ((int)LspSymbolKind::Variable, 13);
    EXPECT_EQ((int)LspSymbolKind::Struct,   23);
}

TEST(LspProtocol, CompletionKindValues_MatchLspSpec) {
    EXPECT_EQ((int)LspCompletionKind::Text,       1);
    EXPECT_EQ((int)LspCompletionKind::Function,   3);
    EXPECT_EQ((int)LspCompletionKind::Field,      5);
    EXPECT_EQ((int)LspCompletionKind::Variable,   6);
    EXPECT_EQ((int)LspCompletionKind::Class,      7);
    EXPECT_EQ((int)LspCompletionKind::Interface,  8);
    EXPECT_EQ((int)LspCompletionKind::Module,     9);
    EXPECT_EQ((int)LspCompletionKind::Keyword,   14);
}
