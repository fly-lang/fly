#include "LspServer.h"

#include <llvm/Support/JSON.h>

using namespace fly::lsp;
using namespace llvm;

// ── Helper ────────────────────────────────────────────────────────────────────

std::string LspServer::extractPath(const json::Object &params) {
    if (auto *td = params.getObject("textDocument"))
        if (auto uri = td->getString("uri"))
            return fileUriToPath(*uri);
    return {};
}

static LspPosition extractPosition(const json::Object &params) {
    if (auto *pos = params.getObject("position"))
        return positionFromJson(*pos);
    return {0, 0};
}

// ── Main loop ─────────────────────────────────────────────────────────────────

void LspServer::run() {
    while (!shutdown_) {
        auto msg = transport_.readMessage();
        if (!msg) break;
        if (auto *obj = msg->getAsObject())
            dispatch(*obj);
    }
}

void LspServer::dispatch(const json::Object &msg) {
    auto method = msg.getString("method");
    if (!method) return;

    json::Value idVal = json::Value(nullptr);
    bool isRequest = false;
    if (const json::Value *id = msg.get("id")) {
        idVal     = *id;
        isRequest = true;
    }

    static const json::Object emptyObj;
    const json::Object *params = msg.getObject("params");
    if (!params) params = &emptyObj;

    if (!initialized_ && *method != "initialize") {
        if (isRequest)
            transport_.sendError(std::move(idVal), -32002, "Server not initialized");
        return;
    }

    if      (*method == "initialize")              onInitialize(std::move(idVal), *params);
    else if (*method == "initialized")             onInitialized();
    else if (*method == "shutdown")                onShutdown(std::move(idVal));
    else if (*method == "exit")                    onExit();
    else if (*method == "textDocument/didOpen")    onDidOpen(*params);
    else if (*method == "textDocument/didChange")  onDidChange(*params);
    else if (*method == "textDocument/didClose")   onDidClose(*params);
    else if (*method == "textDocument/hover")      onHover(std::move(idVal), *params);
    else if (*method == "textDocument/definition") onDefinition(std::move(idVal), *params);
    else if (*method == "textDocument/completion") onCompletion(std::move(idVal), *params);
    else if (*method == "textDocument/documentSymbol") onDocSymbols(std::move(idVal), *params);
    else if (*method == "textDocument/references") onReferences(std::move(idVal), *params);
    else if (isRequest)
        transport_.sendError(std::move(idVal), -32601,
                             ("Unknown method: " + *method).str());
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void LspServer::onInitialize(json::Value id, const json::Object & /*params*/) {
    initialized_ = true;
    transport_.sendResult(std::move(id), json::Object{
        {"capabilities", json::Object{
            {"textDocumentSync",       1},   // 1 = full document sync
            {"hoverProvider",          true},
            {"definitionProvider",     true},
            {"referencesProvider",     true},
            {"documentSymbolProvider", true},
            {"completionProvider", json::Object{
                {"triggerCharacters", json::Array{"."}},
            }},
        }},
        {"serverInfo", json::Object{
            {"name",    "fly-lsp"},
            {"version", "0.1.0"},
        }},
    });
}

void LspServer::onInitialized() {}

void LspServer::onShutdown(json::Value id) {
    shutdown_ = true;
    transport_.sendResult(std::move(id), json::Value(nullptr));
}

void LspServer::onExit() { shutdown_ = true; }

// ── Document sync ─────────────────────────────────────────────────────────────

void LspServer::onDidOpen(const json::Object &params) {
    std::string path = extractPath(params);
    if (path.empty()) return;
    openFiles_.insert(path);
    recompileAndPublish(path);
}

void LspServer::onDidChange(const json::Object &params) {
    std::string path = extractPath(params);
    if (path.empty()) return;
    recompileAndPublish(path);
}

void LspServer::onDidClose(const json::Object &params) {
    std::string path = extractPath(params);
    openFiles_.erase(path);
    transport_.sendNotification("textDocument/publishDiagnostics", json::Object{
        {"uri",         pathToFileUri(path)},
        {"diagnostics", json::Array{}},
    });
}

void LspServer::recompileAndPublish(const std::string &triggerFile) {
    std::vector<std::string> files(openFiles_.begin(), openFiles_.end());
    if (openFiles_.find(triggerFile) == openFiles_.end())
        files.push_back(triggerFile);

    auto diags = analyzer_.compile(files);

    json::Array lspDiags;
    for (const auto &d : diags)
        lspDiags.push_back(toJson(d));

    transport_.sendNotification("textDocument/publishDiagnostics", json::Object{
        {"uri",         pathToFileUri(triggerFile)},
        {"diagnostics", std::move(lspDiags)},
    });
}

// ── Language features ─────────────────────────────────────────────────────────

void LspServer::onHover(json::Value id, const json::Object &params) {
    std::string  path = extractPath(params);
    LspPosition  pos  = extractPosition(params);

    fly::Symbol *sym = analyzer_.findSymbolAt(path, pos.line, pos.character);
    if (!sym) {
        transport_.sendResult(std::move(id), json::Value(nullptr));
        return;
    }
    std::string md = analyzer_.getHoverMarkdown(sym);
    transport_.sendResult(std::move(id), json::Object{
        {"contents", json::Object{{"kind", "markdown"}, {"value", md}}},
    });
}

void LspServer::onDefinition(json::Value id, const json::Object &params) {
    std::string path = extractPath(params);
    LspPosition pos  = extractPosition(params);

    fly::Symbol *sym = analyzer_.findSymbolAt(path, pos.line, pos.character);
    auto loc = sym ? analyzer_.getDefinitionLocation(sym) : std::nullopt;
    if (!loc) {
        transport_.sendResult(std::move(id), json::Value(nullptr));
        return;
    }
    transport_.sendResult(std::move(id), toJson(*loc));
}

void LspServer::onCompletion(json::Value id, const json::Object &params) {
    std::string path = extractPath(params);
    LspPosition pos  = extractPosition(params);
    auto items = analyzer_.getCompletions(path, pos.line, pos.character, "");
    json::Array arr;
    for (const auto &c : items) arr.push_back(toJson(c));
    transport_.sendResult(std::move(id), std::move(arr));
}

void LspServer::onDocSymbols(json::Value id, const json::Object &params) {
    std::string path = extractPath(params);
    auto syms = analyzer_.getDocumentSymbols(path);
    json::Array arr;
    for (const auto &s : syms) arr.push_back(toJson(s));
    transport_.sendResult(std::move(id), std::move(arr));
}

void LspServer::onReferences(json::Value id, const json::Object & /*params*/) {
    // TODO: walk all modules for uses of the symbol at position
    transport_.sendResult(std::move(id), json::Array{});
}
