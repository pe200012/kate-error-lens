/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Bridge between Kate's LSP diagnostic infrastructure and the Error Lens
    InlineNoteProvider.  Because this is a standalone plugin, we connect to
    Kate's internals at runtime through the MainWindow's plugin-view system.
*/

#ifndef ERRORLENS_KATEDIAGNOSTICBRIDGE_H
#define ERRORLENS_KATEDIAGNOSTICBRIDGE_H

#include "diagnosticdata.h"
#include "diagnosticprovider.h"

#include <KTextEditor/Document>
#include <QHash>
#include <QObject>
#include <QSet>

namespace ErrorLens
{

/** Concrete DiagnosticProvider backed by a cache populated from Kate's LSP
 *  client at runtime.
 *
 *  The owning plugin view calls setDocumentDiagnostics() whenever Kate's
 *  LSP system pushes new diagnostics.  The bridge then compares against its
 *  cache, emits the minimal change signals, and serves queries to the
 *  InlineNoteProvider.
 *
 *  If Kate's LSP client is not available, the bridge stays empty and
 *  produces no diagnostics.
 */
class KateDiagnosticBridge : public DiagnosticProvider
{
    Q_OBJECT

public:
    explicit KateDiagnosticBridge(QObject *parent = nullptr);
    ~KateDiagnosticBridge() override;

    /** Replace all diagnostics known for \a doc.
     *
     *  Called by the plugin view when Kate's LSP client emits updated
     *  diagnostics for a document.  The bridge diffs against its current
     *  cache and emits the narrowest-possible signals (line-level when
     *  feasible, document-level otherwise).
     */
    void setDocumentDiagnostics(KTextEditor::Document *doc, const DiagnosticList &diags);

    /** Remove all diagnostics for a closed document. */
    void clearDocument(KTextEditor::Document *doc);

    // -- DiagnosticProvider interface -----------------------------------

    DiagnosticList diagnosticsForDocument(KTextEditor::Document *doc) const override;
    DiagnosticList diagnosticsForLine(KTextEditor::Document *doc, int line) const override;
    bool hasDiagnostics(KTextEditor::Document *doc) const override;

    /** Whether the bridge has successfully connected to a diagnostic source. */
    bool isConnected() const;
    void setConnected(bool v);

private:
    /** Per-document diagnostic map:  line → list of diagnostics on that line. */
    QHash<const KTextEditor::Document *, LineDiagnosticMap> m_store;

    /** Tracks whether we have a live connection to Kate's LSP subsystem. */
    bool m_connected = false;
};

} // namespace ErrorLens

#endif // ERRORLENS_KATEDIAGNOSTICBRIDGE_H