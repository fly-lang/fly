#include "LspProtocol.h"

using namespace fly::lsp;
using namespace llvm;

json::Object fly::lsp::toJson(const LspPosition &p) {
    return json::Object{{"line", p.line}, {"character", p.character}};
}

json::Object fly::lsp::toJson(const LspRange &r) {
    return json::Object{{"start", toJson(r.start)}, {"end", toJson(r.end)}};
}

json::Object fly::lsp::toJson(const LspLocation &l) {
    return json::Object{{"uri", l.uri}, {"range", toJson(l.range)}};
}

json::Object fly::lsp::toJson(const LspDiagnostic &d) {
    return json::Object{
        {"range",    toJson(d.range)},
        {"severity", d.severity},
        {"message",  d.message},
        {"source",   "fly"},
    };
}

json::Object fly::lsp::toJson(const LspDocSymbol &s) {
    return json::Object{
        {"name",           s.name},
        {"kind",           (int)s.kind},
        {"range",          toJson(s.range)},
        {"selectionRange", toJson(s.selectionRange)},
    };
}

json::Object fly::lsp::toJson(const LspCompletionItem &c) {
    json::Object obj{{"label", c.label}, {"kind", (int)c.kind}};
    if (!c.detail.empty())     obj["detail"]     = c.detail;
    if (!c.insertText.empty()) obj["insertText"]  = c.insertText;
    return obj;
}

json::Object fly::lsp::toJson(const LspDocumentHighlight &h) {
    return json::Object{{"range", toJson(h.range)}, {"kind", h.kind}};
}

LspPosition fly::lsp::positionFromJson(const json::Object &obj) {
    return LspPosition{
        (int)obj.getInteger("line").value_or(0),
        (int)obj.getInteger("character").value_or(0),
    };
}
