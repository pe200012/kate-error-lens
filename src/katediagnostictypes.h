/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Compatibility mirror for Kate's private diagnostics payload types.
*/

#ifndef ERRORLENS_KATEDIAGNOSTICTYPES_H
#define ERRORLENS_KATEDIAGNOSTICTYPES_H

#include <KTextEditor/Range>

#include <QList>
#include <QMetaType>
#include <QString>
#include <QUrl>

/** Severity values used by Kate's DiagnosticsProvider payload.
 *
 *  The names and field order in this header intentionally match Kate's
 *  private apps/lib/diagnostics/diagnostic_types.h types. Qt's string-based
 *  signal connection delivers diagnosticsAdded(FileDiagnostics) through this
 *  ABI boundary while the plugin remains buildable against public KF6 headers.
 */
enum class DiagnosticSeverity {
    Unknown = 0,
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
};

/** Location attached to related diagnostic information. */
struct SourceLocation {
    QUrl uri;
    KTextEditor::Range range;
};

/** Extra diagnostic context reported by an LSP server. */
struct DiagnosticRelatedInformation {
    SourceLocation location;
    QString message;
};

/** Single Kate diagnostic item as emitted by DiagnosticsProvider. */
struct Diagnostic {
    KTextEditor::Range range;
    DiagnosticSeverity severity = DiagnosticSeverity::Unknown;
    QString code;
    QString source;
    QString message;
    QList<DiagnosticRelatedInformation> relatedInformation;
};

/** Diagnostics for one file URL. */
struct FileDiagnostics {
    QUrl uri;
    QList<Diagnostic> diagnostics;
};

static_assert(sizeof(DiagnosticSeverity) == sizeof(int),
              "Kate diagnostic severity is stored as the default enum size");

Q_DECLARE_METATYPE(FileDiagnostics)

#endif // ERRORLENS_KATEDIAGNOSTICTYPES_H
