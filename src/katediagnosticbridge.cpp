/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katediagnosticbridge.h"

#include <algorithm>

namespace ErrorLens
{

KateDiagnosticBridge::KateDiagnosticBridge(QObject *parent)
    : DiagnosticProvider(parent)
{
}

KateDiagnosticBridge::~KateDiagnosticBridge() = default;

void KateDiagnosticBridge::setDocumentDiagnostics(KTextEditor::Document *doc, const DiagnosticList &diags)
{
    if (!doc) {
        return;
    }

    // Build the new line→diagnostics map from the flat list.
    LineDiagnosticMap newMap;
    for (const auto &d : diags) {
        newMap[d.line].append(d);
    }

    // Sort each line's diagnostics by column for stable rendering order.
    for (auto &list : newMap) {
        std::sort(list.begin(), list.end(), [](const DiagnosticData &a, const DiagnosticData &b) {
            if (a.line != b.line) {
                return a.line < b.line;
            }
            return a.column < b.column;
        });
    }

    const bool previouslyHad = m_store.contains(doc);
    const LineDiagnosticMap &oldMap = m_store.value(doc);

    // Collect changed lines for incremental signals.
    QSet<int> changedLines;
    QSet<int> allLines;
    for (auto it = newMap.cbegin(); it != newMap.cend(); ++it) {
        allLines.insert(it.key());
    }
    for (auto it = oldMap.cbegin(); it != oldMap.cend(); ++it) {
        allLines.insert(it.key());
    }
    for (int line : allLines) {
        if (newMap.value(line) != oldMap.value(line)) {
            changedLines.insert(line);
        }
    }

    // Store new state.
    m_store[doc] = newMap;

    // Emit the narrowest signal possible.
    // If this is the first data or a large-scale change, send a document-level signal.
    // Otherwise emit per-line signals for only the changed lines.
    if (!previouslyHad || changedLines.size() > 10) {
        Q_EMIT documentDiagnosticsChanged(doc);
    } else {
        for (int line : changedLines) {
            Q_EMIT lineDiagnosticsChanged(doc, line);
        }
    }
}

void KateDiagnosticBridge::clearDocument(KTextEditor::Document *doc)
{
    if (doc && m_store.remove(doc)) {
        Q_EMIT documentDiagnosticsChanged(doc);
    }
}

DiagnosticList KateDiagnosticBridge::diagnosticsForDocument(KTextEditor::Document *doc) const
{
    DiagnosticList result;
    const auto it = m_store.find(doc);
    if (it != m_store.end()) {
        for (const auto &list : *it) {
            result.append(list);
        }
    }
    return result;
}

DiagnosticList KateDiagnosticBridge::diagnosticsForLine(KTextEditor::Document *doc, int line) const
{
    const auto docIt = m_store.find(doc);
    if (docIt == m_store.end()) {
        return {};
    }
    return docIt->value(line);
}

bool KateDiagnosticBridge::hasDiagnostics(KTextEditor::Document *doc) const
{
    return m_store.contains(doc) && !m_store[doc].isEmpty();
}

bool KateDiagnosticBridge::isConnected() const
{
    return m_connected;
}

void KateDiagnosticBridge::setConnected(bool v)
{
    m_connected = v;
}

} // namespace ErrorLens