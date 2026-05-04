/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    User-configurable settings for the Error Lens plugin.
*/

#ifndef ERRORLENS_ERRORLENSCONFIG_H
#define ERRORLENS_ERRORLENSCONFIG_H

#include "diagnosticdata.h"

#include <KConfigGroup>
#include <QColor>
#include <QFont>
#include <QObject>
#include <QString>
#include <QStringList>

namespace ErrorLens
{

/** Persisted configuration, loaded from / written to KConfig.
 *
 *  Each instance is bound to a single KConfigGroup so settings can be
 *  scoped per-document or per-session if needed.
 */
class ErrorLensConfig : public QObject
{
    Q_OBJECT

public:
    explicit ErrorLensConfig(QObject *parent = nullptr);

    // -- Severity filter ----------------------------------------------------

    /** Whether Error-level diagnostics are shown inline. */
    bool showErrors() const;
    void setShowErrors(bool v);

    /** Whether Warning-level diagnostics are shown inline. */
    bool showWarnings() const;
    void setShowWarnings(bool v);

    /** Whether Info-level diagnostics are shown inline. */
    bool showInfo() const;
    void setShowInfo(bool v);

    /** Whether Hint-level diagnostics are shown inline. */
    bool showHints() const;
    void setShowHints(bool v);

    /** Convenience: checks if a given severity passes the filter. */
    bool isSeverityVisible(DiagnosticSeverity s) const;

    // -- Appearance ---------------------------------------------------------

    /** Font-size scale relative to the editor's base font (0.5 – 1.5). */
    double fontSizeScale() const;
    void setFontSizeScale(double v);

    /** Maximum number of characters to display from a diagnostic message. */
    int maxMessageLength() const;
    void setMaxMessageLength(int v);

    /** Number of virtual-spaces inserted between code end and the first
     *  inline note on a line.
     */
    int lineEndSpacing() const;
    void setLineEndSpacing(int v);

    /** Semi-transparent background colour for Error-level notes. */
    QColor errorBackground() const;
    void setErrorBackground(const QColor &c);

    /** Semi-transparent background colour for Warning-level notes. */
    QColor warningBackground() const;
    void setWarningBackground(const QColor &c);

    /** Semi-transparent background colour for Info-level notes. */
    QColor infoBackground() const;
    void setInfoBackground(const QColor &c);

    /** Semi-transparent background colour for Hint-level notes. */
    QColor hintBackground() const;
    void setHintBackground(const QColor &c);

    /** Foreground text colour for Error-level notes. */
    QColor errorForeground() const;
    void setErrorForeground(const QColor &c);

    /** Foreground text colour for Warning-level notes. */
    QColor warningForeground() const;
    void setWarningForeground(const QColor &c);

    /** Foreground text colour for Info-level notes. */
    QColor infoForeground() const;
    void setInfoForeground(const QColor &c);

    /** Foreground text colour for Hint-level notes. */
    QColor hintForeground() const;
    void setHintForeground(const QColor &c);

    /** Whether the diagnostic source tag (e.g. "[clangd]") is appended. */
    bool showSource() const;
    void setShowSource(bool v);

    /** Separator between multiple diagnostics on the same line. */
    QString diagnosticSeparator() const;
    void setDiagnosticSeparator(const QString &s);

    // -- Persistence --------------------------------------------------------

    /** Load settings from the given KConfigGroup. */
    void load(const KConfigGroup &group);

    /** Save current settings to the given KConfigGroup. */
    void save(KConfigGroup &group) const;

    /** Restore all settings to their defaults. */
    void resetDefaults();

Q_SIGNALS:
    /** Emitted whenever any setting changes (enables live preview). */
    void configChanged();

private:
    // Visibility
    bool m_showErrors = true;
    bool m_showWarnings = true;
    bool m_showInfo = false;
    bool m_showHints = false;

    // Appearance
    double m_fontSizeScale = 0.85;
    int m_maxMessageLength = 120;
    int m_lineEndSpacing = 2;
    bool m_showSource = true;
    QString m_diagnosticSeparator = QStringLiteral(" | ");

    // Colours — set sensible defaults that work with both light and dark themes.
    QColor m_errorBackground{220, 38, 38, 40};
    QColor m_warningBackground{202, 138, 4, 40};
    QColor m_infoBackground{37, 99, 235, 40};
    QColor m_hintBackground{100, 116, 139, 40};

    QColor m_errorForeground{239, 68, 68};
    QColor m_warningForeground{234, 179, 8};
    QColor m_infoForeground{96, 165, 250};
    QColor m_hintForeground{148, 163, 184};
};

} // namespace ErrorLens

#endif // ERRORLENS_ERRORLENSCONFIG_H