//===----------------------------------------------------------------------===//
// src/Compiler/DiagnosticRenderer.cpp - Diagnostic Pretty-Printing
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Frontend/DiagnosticRenderer.h"
#include "Basic/Diagnostic.h"
#include "Basic/DiagnosticOptions.h"
#include "Basic/LLVM.h"
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "Lex/Lexer.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

using namespace fly;

DiagnosticRenderer::DiagnosticRenderer(DiagnosticOptions *DiagOpts)
    : DiagOpts(DiagOpts), LastLevel() {}

DiagnosticRenderer::~DiagnosticRenderer() = default;

void DiagnosticRenderer::emitDiagnostic(FullSourceLoc Loc,
                                        DiagnosticsEngine::Level Level,
                                        StringRef Message,
                                        ArrayRef<CharSourceRange> Ranges,
                                        ArrayRef<FixItHint> FixItHints,
                                        DiagOrStoredDiag D) {
  assert(Loc.hasManager() || Loc.isInvalid());

  beginDiagnostic(D, Level);

  if (!Loc.isValid())
    // If we have no source location, just emit the diagnostic message.
    emitDiagnosticMessage(Loc, PresumedLoc(), Level, Message, Ranges, D);
  else {
    // Get the ranges into a local array we can hack on.
    SmallVector<CharSourceRange, 20> MutableRanges(Ranges.begin(),
                                                   Ranges.end());

    for (const auto &Hint : FixItHints)
      if (Hint.RemoveRange.isValid())
        MutableRanges.push_back(Hint.RemoveRange);

    FullSourceLoc UnexpandedLoc = Loc;

    // Find the ultimate expansion location for the diagnostic.
    Loc = Loc.getFileLoc();

    PresumedLoc PLoc = Loc.getPresumedLoc(DiagOpts->ShowPresumedLoc);

    // First, if this diagnostic is not in the main file, print out the
    // "included from" lines.
    emitIncludeStack(Loc, PLoc, Level);

    // Next, emit the actual diagnostic message and caret.
    emitDiagnosticMessage(Loc, PLoc, Level, Message, Ranges, D);
    emitCaret(Loc, Level, MutableRanges, FixItHints);
  }

  LastLoc = Loc;
  LastLevel = Level;

  endDiagnostic(D, Level);
}

void DiagnosticRenderer::emitStoredDiagnostic(StoredDiagnostic &Diag) {
  emitDiagnostic(Diag.getLocation(), Diag.getLevel(), Diag.getMessage(),
                 Diag.getRanges(), Diag.getFixIts(),
                 &Diag);
}

void DiagnosticRenderer::emitBasicNote(StringRef Message) {
  emitDiagnosticMessage(FullSourceLoc(), PresumedLoc(), DiagnosticsEngine::Note,
                        Message, None, DiagOrStoredDiag());
}

/// Prints an include stack when appropriate for a particular
/// diagnostic level and location.
///
/// This routine handles all the logic of suppressing particular include
/// stacks (such as those for notes) and duplicate include stacks when
/// repeated warnings occur within the same file. It also handles the logic
/// of customizing the formatting and display of the include stack.
///
/// \param Loc   The diagnostic location.
/// \param PLoc  The presumed location of the diagnostic location.
/// \param Level The diagnostic level of the message this stack pertains to.
void DiagnosticRenderer::emitIncludeStack(FullSourceLoc Loc, PresumedLoc PLoc,
                                          DiagnosticsEngine::Level Level) {
  FullSourceLoc IncludeLoc =
      PLoc.isInvalid() ? FullSourceLoc()
                       : FullSourceLoc(PLoc.getIncludeLoc(), Loc.getManager());

  // Skip redundant include stacks altogether.
  if (LastIncludeLoc == IncludeLoc)
    return;

  LastIncludeLoc = IncludeLoc;

  if (!DiagOpts->ShowNoteIncludeStack && Level == DiagnosticsEngine::Note)
    return;

  if (IncludeLoc.isValid())
    emitIncludeStackRecursively(IncludeLoc);
  else {
    emitModuleBuildStack(Loc.getManager());
    emitImportStack(Loc);
  }
}

/// Helper to recursively walk up the include stack and print each layer
/// on the way back down.
void DiagnosticRenderer::emitIncludeStackRecursively(FullSourceLoc Loc) {
  if (Loc.isInvalid()) {
    emitModuleBuildStack(Loc.getManager());
    return;
  }

  PresumedLoc PLoc = Loc.getPresumedLoc(DiagOpts->ShowPresumedLoc);
  if (PLoc.isInvalid())
    return;

  // If this source location was imported from a module, print the module
  // import stack rather than the
  // FIXME: We want submodule granularity here.
  std::pair<FullSourceLoc, StringRef> Imported = Loc.getModuleImportLoc();
  if (!Imported.second.empty()) {
    // This location was imported by a module. Emit the module import stack.
    emitImportStackRecursively(Imported.first, Imported.second);
    return;
  }

  // Emit the other include frames first.
  emitIncludeStackRecursively(
      FullSourceLoc(PLoc.getIncludeLoc(), Loc.getManager()));

  // Emit the inclusion text/note.
  emitIncludeLocation(Loc, PLoc);
}

/// Emit the module import stack associated with the current location.
void DiagnosticRenderer::emitImportStack(FullSourceLoc Loc) {
  if (Loc.isInvalid()) {
    emitModuleBuildStack(Loc.getManager());
    return;
  }

  std::pair<FullSourceLoc, StringRef> NextImportLoc = Loc.getModuleImportLoc();
  emitImportStackRecursively(NextImportLoc.first, NextImportLoc.second);
}

/// Helper to recursively walk up the import stack and print each layer
/// on the way back down.
void DiagnosticRenderer::emitImportStackRecursively(FullSourceLoc Loc,
                                                    StringRef ModuleName) {
  if (ModuleName.empty()) {
    return;
  }

  PresumedLoc PLoc = Loc.getPresumedLoc(DiagOpts->ShowPresumedLoc);

  // Emit the other import frames first.
  std::pair<FullSourceLoc, StringRef> NextImportLoc = Loc.getModuleImportLoc();
  emitImportStackRecursively(NextImportLoc.first, NextImportLoc.second);

  // Emit the inclusion text/note.
  emitImportLocation(Loc, PLoc, ModuleName);
}

/// Emit the module build stack, for cases where a module is (re-)built
/// on demand.
void DiagnosticRenderer::emitModuleBuildStack(const SourceManager &SM) {
  ModuleBuildStack Stack = SM.getModuleBuildStack();
  for (const auto &I : Stack) {
    emitBuildingModuleLocation(I.second, I.second.getPresumedLoc(
                                              DiagOpts->ShowPresumedLoc),
                               I.first);
  }
}

/// A recursive function to trace all possible backtrace locations
/// to match the \p CaretLocFileID.
static SourceLocation
retrieveMacroLocation(SourceLocation Loc, FileID MacroFileID,
                      FileID CaretFileID,
                      const SmallVectorImpl<FileID> &CommonArgExpansions,
                      bool IsBegin, const SourceManager *SM,
                      bool &IsTokenRange) {
  assert(SM->getFileID(Loc) == MacroFileID);
  if (MacroFileID == CaretFileID)
    return Loc;

  CharSourceRange MacroRange, MacroArgRange;


    MacroRange = SM->getImmediateExpansionRange(Loc);
    MacroArgRange =
        CharSourceRange(SM->getImmediateSpellingLoc(Loc), IsTokenRange);


  SourceLocation MacroLocation =
      IsBegin ? MacroRange.getBegin() : MacroRange.getEnd();
  if (MacroLocation.isValid()) {
    MacroFileID = SM->getFileID(MacroLocation);
    bool TokenRange = IsBegin ? IsTokenRange : MacroRange.isTokenRange();
    MacroLocation =
        retrieveMacroLocation(MacroLocation, MacroFileID, CaretFileID,
                              CommonArgExpansions, IsBegin, SM, TokenRange);
    if (MacroLocation.isValid()) {
      IsTokenRange = TokenRange;
      return MacroLocation;
    }
  }

  // If we moved the end of the range to an expansion location, we now have
  // a range of the same kind as the expansion range.
  if (!IsBegin)
    IsTokenRange = MacroArgRange.isTokenRange();

  SourceLocation MacroArgLocation =
      IsBegin ? MacroArgRange.getBegin() : MacroArgRange.getEnd();
  MacroFileID = SM->getFileID(MacroArgLocation);
  return retrieveMacroLocation(MacroArgLocation, MacroFileID, CaretFileID,
                               CommonArgExpansions, IsBegin, SM, IsTokenRange);
}

// Helper function to fix up source ranges.  It takes in an array of ranges,
// and outputs an array of ranges where we want to draw the range highlighting
// around the location specified by CaretLoc.
//
// To find locations which correspond to the caret, we crawl the macro caller
// chain for the beginning and end of each range.  If the caret location
// is in a macro expansion, we search each chain for a location
// in the same expansion as the caret; otherwise, we crawl to the top of
// each chain. Two locations are part of the same macro expansion
// iff the FileID is the same.
static void
mapDiagnosticRanges(FullSourceLoc CaretLoc, ArrayRef<CharSourceRange> Ranges,
                    SmallVectorImpl<CharSourceRange> &SpellingRanges) {
  FileID CaretLocFileID = CaretLoc.getFileID();

  const SourceManager *SM = &CaretLoc.getManager();

  for (const auto &Range : Ranges) {
    if (Range.isInvalid())
      continue;

    SourceLocation Begin = Range.getBegin(), End = Range.getEnd();
    bool IsTokenRange = Range.isTokenRange();

    FileID BeginFileID = SM->getFileID(Begin);
    FileID EndFileID = SM->getFileID(End);

    // Find the common parent for the beginning and end of the range.

    // First, crawl the expansion chain for the beginning of the range.
    llvm::SmallDenseMap<FileID, SourceLocation> BeginLocsMap;

    // Do the backtracking.
    SmallVector<FileID, 4> CommonArgExpansions;
    Begin = retrieveMacroLocation(Begin, BeginFileID, CaretLocFileID,
                                  CommonArgExpansions, /*IsBegin=*/true, SM,
                                  IsTokenRange);
    End = retrieveMacroLocation(End, BeginFileID, CaretLocFileID,
                                CommonArgExpansions, /*IsBegin=*/false, SM,
                                IsTokenRange);
    if (Begin.isInvalid() || End.isInvalid()) continue;

    // Return the spelling location of the beginning and end of the range.
    Begin = SM->getSpellingLoc(Begin);
    End = SM->getSpellingLoc(End);

    SpellingRanges.push_back(CharSourceRange(SourceRange(Begin, End),
                                             IsTokenRange));
  }
}

void DiagnosticRenderer::emitCaret(FullSourceLoc Loc,
                                   DiagnosticsEngine::Level Level,
                                   ArrayRef<CharSourceRange> Ranges,
                                   ArrayRef<FixItHint> Hints) {
  SmallVector<CharSourceRange, 4> SpellingRanges;
  mapDiagnosticRanges(Loc, Ranges, SpellingRanges);
  emitCodeContext(Loc, Level, SpellingRanges, Hints);
}

/// A helper function for emitMacroExpansion to print the
/// macro expansion message
void DiagnosticRenderer::emitSingleMacroExpansion(
    FullSourceLoc Loc, DiagnosticsEngine::Level Level,
    ArrayRef<CharSourceRange> Ranges) {
  // Find the spelling location for the macro definition. We must use the
  // spelling location here to avoid emitting a macro backtrace for the note.
  FullSourceLoc SpellingLoc = Loc.getSpellingLoc();

  // Map the ranges into the FileID of the diagnostic location.
  SmallVector<CharSourceRange, 4> SpellingRanges;
  mapDiagnosticRanges(Loc, Ranges, SpellingRanges);

  SmallString<100> MessageStorage;
  llvm::raw_svector_ostream Message(MessageStorage);

  emitDiagnostic(SpellingLoc, DiagnosticsEngine::Note, Message.str(),
                 SpellingRanges, None);
}

DiagnosticNoteRenderer::~DiagnosticNoteRenderer() = default;

void DiagnosticNoteRenderer::emitIncludeLocation(FullSourceLoc Loc,
                                                 PresumedLoc PLoc) {
  // Generate a note indicating the include location.
  SmallString<200> MessageStorage;
  llvm::raw_svector_ostream Message(MessageStorage);
  Message << "in file included from " << PLoc.getFilename() << ':'
          << PLoc.getLine() << ":";
  emitNote(Loc, Message.str());
}

void DiagnosticNoteRenderer::emitImportLocation(FullSourceLoc Loc,
                                                PresumedLoc PLoc,
                                                StringRef ModuleName) {
  // Generate a note indicating the include location.
  SmallString<200> MessageStorage;
  llvm::raw_svector_ostream Message(MessageStorage);
  Message << "in module '" << ModuleName;
  if (PLoc.isValid())
    Message << "' imported from " << PLoc.getFilename() << ':'
            << PLoc.getLine();
  Message << ":";
  emitNote(Loc, Message.str());
}

void DiagnosticNoteRenderer::emitBuildingModuleLocation(FullSourceLoc Loc,
                                                        PresumedLoc PLoc,
                                                        StringRef ModuleName) {
  // Generate a note indicating the include location.
  SmallString<200> MessageStorage;
  llvm::raw_svector_ostream Message(MessageStorage);
  if (PLoc.isValid())
    Message << "while building module '" << ModuleName << "' imported from "
            << PLoc.getFilename() << ':' << PLoc.getLine() << ":";
  else
    Message << "while building module '" << ModuleName << "':";
  emitNote(Loc, Message.str());
}
