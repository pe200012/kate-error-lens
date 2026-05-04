/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Abstract interface for diagnostic data sources.
*/

#ifndef ERRORLENS_DIAGNOSTICPROVIDER_H
#define ERRORLENS_DIAGNOSTICPROVIDER_H

#include "diagnosticdata.h"

#include <KTextEditor/Document>
#include <QObject>

namespace ErrorLens
{

/**
 * Abstract data source that supplies diagnostics from an external
 * system — typically Kate's built-in LSP Client plugin.
 *
 * The provider emits signals when diagnostics change so downstream
 * consumers (the InlineNoteProvider) can update incrementally.
 *
 * Concrete implementations must supply diagnostics on a per-document
 * basis.  The null implementation returns empty lists and never emits.
 */
class DiagnosticProvider : public QObject
{
    Q_OBJECT

public:
    explicit DiagnosticProvider(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~DiagnosticProvider() override = default;

    /** All diagnostics currently known for \a doc. */
    virtual DiagnosticList diagnosticsForDocument(KTextEditor::Document *doc) const = 0;

    /** Diagnostics on a single \a line of \a doc. */
    virtual DiagnosticList diagnosticsForLine(KTextEditor::Document *doc, int line) const = 0;

    /** Quick check for any diagnostics on a document. */
    virtual bool hasDiagnostics(KTextEditor::Document *doc) const = 0;

Q_SIGNALS:
    /** Emitted when the full diagnostic set for \a doc changes. */
    void documentDiagnosticsChanged(KTextEditor::Document *doc);

    /** Emitted when diagnostics on a single \a line of \a doc change. */
    void lineDiagnosticsChanged(KTextEditor::Document *doc, int line);
};

// ---------------------------------------------------------------------------
// Null implementation — used when no diagnostic source is available.
// ---------------------------------------------------------------------------

class NullDiagnosticProvider : public DiagnosticProvider
{
    Q_OBJECT

public:
    using DiagnosticProvider::DiagnosticProvider;

    DiagnosticList diagnosticsForDocument(KTextEditor::Document *) const override
    {
        return {};
    }
    DiagnosticList diagnosticsForLine(KTextEditor::Document *, int) const override
    {
        return {};
    }
    bool hasDiagnostics(KTextEditor::Document *) const override
    {
        return false;
    }
};

} // namespace ErrorLens

#endif // ERRORLENS_DIAGNOSTICPROVIDER_H