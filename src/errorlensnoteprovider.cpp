/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "errorlensnoteprovider.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QPainter>
#include <QToolTip>

namespace ErrorLens
{

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ErrorLensNoteProvider::ErrorLensNoteProvider(KTextEditor::Document *doc,
                                             DiagnosticProvider *diagProvider,
                                             ErrorLensConfig *config)
    : InlineNoteProvider()
    , m_doc(doc)
    , m_diagProvider(diagProvider)
    , m_config(config)
{
    // Subscribe to diagnostic updates.
    connect(m_diagProvider, &DiagnosticProvider::documentDiagnosticsChanged,
            this, &ErrorLensNoteProvider::onDocumentDiagnosticsChanged);
    connect(m_diagProvider, &DiagnosticProvider::lineDiagnosticsChanged,
            this, &ErrorLensNoteProvider::onLineDiagnosticsChanged);

    // Reload when user changes settings.
    connect(m_config, &ErrorLensConfig::configChanged,
            this, &ErrorLensNoteProvider::onConfigChanged);

    // Initial cache population.
    rebuildCache();
}

ErrorLensNoteProvider::~ErrorLensNoteProvider() = default;

// ---------------------------------------------------------------------------
// KTextEditor::InlineNoteProvider — query
// ---------------------------------------------------------------------------

QList<int> ErrorLensNoteProvider::inlineNotes(int line) const
{
    if (m_lineDiagnostics.contains(line) && !m_lineDiagnostics[line].isEmpty()) {
        const int col = m_doc->lineLength(line) + m_config->lineEndSpacing();
        return {col};
    }
    return {};
}

QSize ErrorLensNoteProvider::inlineNoteSize(const InlineNote &note) const
{
    const int line = note.position().line();
    const auto it = m_lineDiagnostics.constFind(line);
    if (it == m_lineDiagnostics.constEnd() || it->isEmpty()) {
        return {0, 0};
    }

    QFont font = note.font();
    if (!qFuzzyCompare(m_config->fontSizeScale(), 1.0)) {
        font.setPointSizeF(font.pointSizeF() * m_config->fontSizeScale());
    }
    const QFontMetrics fm(font);

    int totalWidth = kPaddingH * 2;
    const QString sep = m_config->diagnosticSeparator();
    const int sepWidth = fm.horizontalAdvance(sep);

    for (int i = 0; i < it->size(); ++i) {
        totalWidth += fm.horizontalAdvance(formattedText(it->at(i)));
        if (i < it->size() - 1) {
            totalWidth += sepWidth;
        }
    }

    // Clamp height to line height (framework clips paint beyond it anyway).
    return {totalWidth, qMin(note.lineHeight(), note.lineHeight())};
}

// ---------------------------------------------------------------------------
// KTextEditor::InlineNoteProvider — paint
// ---------------------------------------------------------------------------

void ErrorLensNoteProvider::paintInlineNote(const InlineNote &note,
                                            QPainter &painter,
                                            Qt::LayoutDirection direction) const
{
    Q_UNUSED(direction);

    const int line = note.position().line();
    const auto it = m_lineDiagnostics.constFind(line);
    if (it == m_lineDiagnostics.constEnd() || it->isEmpty()) {
        return;
    }

    const int w = static_cast<int>(note.width());
    const int h = note.lineHeight();

    if (w <= 0 || h <= 0) {
        return;
    }

    QFont font = note.font();
    if (!qFuzzyCompare(m_config->fontSizeScale(), 1.0)) {
        font.setPointSizeF(font.pointSizeF() * m_config->fontSizeScale());
    }
    painter.setFont(font);
    const QFontMetrics fm(font);

    const QString sep = m_config->diagnosticSeparator();
    const int sepWidth = fm.horizontalAdvance(sep);

    // Background — use the worst severity's colour for the note background.
    DiagnosticSeverity worstSeverity = DiagnosticSeverity::Hint;
    for (const auto &d : *it) {
        if (static_cast<quint8>(d.severity) < static_cast<quint8>(worstSeverity)) {
            worstSeverity = d.severity;
        }
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor(worstSeverity));

    const qreal radius = 3.0;
    const QRectF bgRect(kPaddingH, kPaddingV, w - kPaddingH * 2, h - kPaddingV * 2);
    painter.drawRoundedRect(bgRect, radius, radius);

    // Text rendering
    const int textY = (h + fm.ascent() - fm.descent()) / 2;
    const QColor dimSep(fgColor(worstSeverity).red(),
                        fgColor(worstSeverity).green(),
                        fgColor(worstSeverity).blue(),
                        80);

    int x = kPaddingH;
    for (int i = 0; i < it->size(); ++i) {
        const QString text = formattedText(it->at(i));
        painter.setPen(fgColor(it->at(i).severity));
        painter.drawText(x, textY, text);
        x += fm.horizontalAdvance(text);

        if (i < it->size() - 1) {
            painter.setPen(dimSep);
            painter.drawText(x, textY, sep);
            x += sepWidth;
        }
    }
}

// ---------------------------------------------------------------------------
// Interaction handlers
// ---------------------------------------------------------------------------

void ErrorLensNoteProvider::inlineNoteActivated(const InlineNote &note,
                                                Qt::MouseButtons buttons,
                                                const QPoint &globalPos)
{
    Q_UNUSED(buttons);
    Q_UNUSED(globalPos);

    // Navigate to the first diagnostic location on this line.
    const int line = note.position().line();
    const auto it = m_lineDiagnostics.constFind(line);
    if (it == m_lineDiagnostics.constEnd() || it->isEmpty()) {
        return;
    }

    // Move cursor to the first diagnostic's start position.
    const auto &first = it->first();
    const auto *view = note.view();
    if (view) {
        view->setCursorPosition({first.line, first.column});
    }
}

void ErrorLensNoteProvider::inlineNoteFocusInEvent(const InlineNote &note,
                                                   const QPoint &globalPos)
{
    Q_UNUSED(globalPos);

    // Show full diagnostic texts as a tooltip on hover.
    const int line = note.position().line();
    const auto it = m_lineDiagnostics.constFind(line);
    if (it == m_lineDiagnostics.constEnd() || it->isEmpty()) {
        return;
    }

    QStringList lines;
    for (const auto &d : *it) {
        QString entry = QStringLiteral("%1 %2")
                            .arg(QString::fromUtf8(severityPrefix(d.severity)),
                                 d.message);
        if (m_config->showSource() && !d.source.isEmpty()) {
            entry += QStringLiteral("  [%1]").arg(d.source);
        }
        if (!d.code.isEmpty()) {
            entry += QStringLiteral("  (%1)").arg(d.code);
        }
        lines.append(entry);
    }

    QToolTip::showText(globalPos, lines.join(QChar::LineFeed), note.view());
}

void ErrorLensNoteProvider::inlineNoteFocusOutEvent(const InlineNote &note)
{
    Q_UNUSED(note);
    QToolTip::hideText();
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void ErrorLensNoteProvider::onDocumentDiagnosticsChanged(KTextEditor::Document *doc)
{
    if (doc != m_doc) {
        return;
    }

    const QSet<int> changed = rebuildCache();

    if (changed.isEmpty()) {
        return;
    }

    if (changed.size() > 20) {
        Q_EMIT inlineNotesReset();
    } else {
        for (int line : changed) {
            Q_EMIT inlineNotesChanged(line);
        }
    }
}

void ErrorLensNoteProvider::onLineDiagnosticsChanged(KTextEditor::Document *doc, int line)
{
    if (doc != m_doc) {
        return;
    }

    const DiagnosticList diags = m_diagProvider->diagnosticsForLine(m_doc, line);
    const DiagnosticList filtered = filteredDiagnostics(diags);

    if (!filtered.isEmpty()) {
        m_lineDiagnostics[line] = filtered;
    } else {
        m_lineDiagnostics.remove(line);
    }

    Q_EMIT inlineNotesChanged(line);
}

void ErrorLensNoteProvider::onConfigChanged()
{
    rebuildCache();
    Q_EMIT inlineNotesReset();
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

QString ErrorLensNoteProvider::formattedText(const DiagnosticData &d) const
{
    const QString prefix = QString::fromUtf8(severityPrefix(d.severity));

    QString msg = d.message.simplified();
    const int maxLen = m_config->maxMessageLength();
    if (msg.length() > maxLen) {
        msg = msg.left(maxLen - 1) + QChar(0x2026); // ellipsis
    }

    QString text = prefix + msg;

    if (m_config->showSource() && !d.source.isEmpty()) {
        text += QStringLiteral(" [%1]").arg(d.source);
    }

    return text;
}

DiagnosticList ErrorLensNoteProvider::filteredDiagnostics(const DiagnosticList &list) const
{
    DiagnosticList result;
    result.reserve(list.size());
    for (const auto &d : list) {
        if (m_config->isSeverityVisible(d.severity)) {
            result.append(d);
        }
    }
    return result;
}

QSet<int> ErrorLensNoteProvider::rebuildCache()
{
    const DiagnosticList allDiags = m_diagProvider->diagnosticsForDocument(m_doc);

    // Build new map from flat list, filtered by visibility.
    LineDiagnosticMap newMap;
    for (const auto &d : allDiags) {
        if (m_config->isSeverityVisible(d.severity)) {
            newMap[d.line].append(d);
        }
    }

    // Diff old and new to find changed lines.
    QSet<int> changed;
    QSet<int> allLines;
    for (auto it = newMap.cbegin(); it != newMap.cend(); ++it) {
        allLines.insert(it.key());
    }
    for (auto it = m_lineDiagnostics.cbegin(); it != m_lineDiagnostics.cend(); ++it) {
        allLines.insert(it.key());
    }
    for (const int line : allLines) {
        if (newMap.value(line) != m_lineDiagnostics.value(line)) {
            changed.insert(line);
        }
    }

    m_lineDiagnostics = std::move(newMap);
    return changed;
}

QColor ErrorLensNoteProvider::bgColor(DiagnosticSeverity s) const
{
    switch (s) {
    case DiagnosticSeverity::Error:
        return m_config->errorBackground();
    case DiagnosticSeverity::Warning:
        return m_config->warningBackground();
    case DiagnosticSeverity::Information:
        return m_config->infoBackground();
    case DiagnosticSeverity::Hint:
        return m_config->hintBackground();
    }
    return m_config->hintBackground();
}

QColor ErrorLensNoteProvider::fgColor(DiagnosticSeverity s) const
{
    switch (s) {
    case DiagnosticSeverity::Error:
        return m_config->errorForeground();
    case DiagnosticSeverity::Warning:
        return m_config->warningForeground();
    case DiagnosticSeverity::Information:
        return m_config->infoForeground();
    case DiagnosticSeverity::Hint:
        return m_config->hintForeground();
    }
    return m_config->hintForeground();
}

const char *ErrorLensNoteProvider::severityPrefix(DiagnosticSeverity s) const
{
    switch (s) {
    case DiagnosticSeverity::Error:
        // HEAVY MULTIPLICATION X  ✘
        return "\xe2\x9c\x98 ";
    case DiagnosticSeverity::Warning:
        // WARNING SIGN  ⚠
        return "\xe2\x9a\xa0 ";
    case DiagnosticSeverity::Information:
        // INFORMATION SOURCE  ℹ
        return "\xe2\x84\xb9 ";
    case DiagnosticSeverity::Hint:
        // RIGHTWARDS ARROW  →
        return "\xe2\x86\x92 ";
    }
    return "  ";
}

} // namespace ErrorLens