/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    KTextEditor::InlineNoteProvider that renders LSP diagnostics inline
    at the end of affected code lines, mimicking VSCode's Error Lens.
*/

#ifndef ERRORLENS_ERRORLENSNOTEPROVIDER_H
#define ERRORLENS_ERRORLENSNOTEPROVIDER_H

#include "diagnosticdata.h"
#include "errorlensconfig.h"
#include "diagnosticprovider.h"

#include <KTextEditor/InlineNoteProvider>
#include <KTextEditor/Document>

namespace ErrorLens
{

/** InlineNoteProvider that draws diagnostics at line-ends.
 *
 *  For each line that has visible diagnostics, one InlineNote is placed
 *  at `lineLength + lineEndSpacing`.  Inside this note, all diagnostics
 *  for the line are rendered sequentially with severity-coloured text.
 *
 *  The provider subscribes to a DiagnosticProvider and updates its cache
 *  incrementally, emitting `inlineNotesChanged(line)` for single-line
 *  edits and `inlineNotesReset()` for full-document reloads.
 */
class ErrorLensNoteProvider : public KTextEditor::InlineNoteProvider
{
    Q_OBJECT

public:
    /** @param doc          Document whose diagnostics this provider renders.
     *  @param diagProvider Source of diagnostic data (e.g. KateDiagnosticBridge).
     *  @param config       User settings (severity filter, colours, sizing).
     */
    ErrorLensNoteProvider(KTextEditor::Document *doc,
                          DiagnosticProvider *diagProvider,
                          ErrorLensConfig *config);

    ~ErrorLensNoteProvider() override;

    // -- KTextEditor::InlineNoteProvider interface ------------------------

    QList<int> inlineNotes(int line) const override;
    QSize inlineNoteSize(const InlineNote &note) const override;
    void paintInlineNote(const InlineNote &note,
                         QPainter &painter,
                         Qt::LayoutDirection direction) const override;

    // -- Activation handlers ----------------------------------------------

    void inlineNoteActivated(const InlineNote &note,
                             Qt::MouseButtons buttons,
                             const QPoint &globalPos) override;

    void inlineNoteFocusInEvent(const InlineNote &note,
                                const QPoint &globalPos) override;

    void inlineNoteFocusOutEvent(const InlineNote &note) override;

private Q_SLOTS:
    /** Full document diagnostic update.  Rebuilds the whole cache. */
    void onDocumentDiagnosticsChanged(KTextEditor::Document *doc);

    /** Single-line diagnostic update. */
    void onLineDiagnosticsChanged(KTextEditor::Document *doc, int line);

    /** Reload when configuration changes. */
    void onConfigChanged();

private:
    KTextEditor::Document *m_doc;
    DiagnosticProvider *const m_diagProvider;
    ErrorLensConfig *const m_config;

    /** Cached visible diagnostics:  line → list of DiagnosticData.
     *  Mutable so const query/paint methods can read it.
     */
    mutable LineDiagnosticMap m_lineDiagnostics;

    // -- Internal helpers ------------------------------------------------

    /** Build the formatted display text for a single diagnostic. */
    QString formattedText(const DiagnosticData &d) const;

    /** Returns only diagnostics passing the severity filter. */
    DiagnosticList filteredDiagnostics(const DiagnosticList &list) const;

    /** Apply the severity filter and update m_lineDiagnostics from
     *  DiagnosticProvider's data for this document.
     *  Returns the set of line numbers that changed. */
    QSet<int> rebuildCache();

    /** Colour helpers — query config for the appropriate colour. */
    QColor bgColor(DiagnosticSeverity s) const;
    QColor fgColor(DiagnosticSeverity s) const;
    const char *severityPrefix(DiagnosticSeverity s) const;

    /** Internal padding in pixels around the rendered text. */
    static constexpr int kPaddingH = 4;
    static constexpr int kPaddingV = 1;
};

} // namespace ErrorLens

#endif // ERRORLENS_ERRORLENSNOTEPROVIDER_H