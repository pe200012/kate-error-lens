/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Diagnostic data structures shared across the plugin.
*/

#ifndef ERRORLENS_DIAGNOSTICDATA_H
#define ERRORLENS_DIAGNOSTICDATA_H

#include <QHash>
#include <QList>
#include <QString>

namespace ErrorLens
{

/** Severity levels matching the LSP DiagnosticSeverity enumeration.
 *
 *  Values align with:
 *  https://microsoft.github.io/language-server-protocol/specification#diagnosticSeverity
 */
enum class DiagnosticSeverity : quint8 {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
};

/** Human-readable label for a severity level. */
inline const char *severityLabel(DiagnosticSeverity s)
{
    switch (s) {
    case DiagnosticSeverity::Error:
        return "Error";
    case DiagnosticSeverity::Warning:
        return "Warning";
    case DiagnosticSeverity::Information:
        return "Info";
    case DiagnosticSeverity::Hint:
        return "Hint";
    }
    return "Unknown";
}

/** Single diagnostic item originating from a language server or build tool.
 *
 *  Mapped from LSP's Diagnostic structure.  Only the fields needed by
 *  the inline-rendering logic are kept.
 */
struct DiagnosticData {
    DiagnosticSeverity severity = DiagnosticSeverity::Error;

    /** 0-based line where the diagnostic range starts. */
    int line = 0;

    /** 0-based column where the diagnostic range starts. */
    int column = 0;

    /** 0-based line where the diagnostic range ends (>= line). */
    int endLine = 0;

    /** 0-based column where the diagnostic range ends. */
    int endColumn = 0;

    /** The diagnostic message text, trimmed. */
    QString message;

    /** Diagnostic source (e.g. "clangd", "pyright", "cmake"). */
    QString source;

    /** Error/warning code (e.g. "unused-variable").  May be empty. */
    QString code;

    /** Whether this diagnostic spans a range wider than a single cursor position. */
    bool isRange() const
    {
        return endLine != line || endColumn != column;
    }
};

using DiagnosticList = QList<DiagnosticData>;

/** Maps a zero-based line number to its list of diagnostics. */
using LineDiagnosticMap = QHash<int, DiagnosticList>;

} // namespace ErrorLens

#endif // ERRORLENS_DIAGNOSTICDATA_H