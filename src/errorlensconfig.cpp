/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "errorlensconfig.h"

namespace ErrorLens
{

static const char *const KEY_SHOW_ERRORS = "ShowErrors";
static const char *const KEY_SHOW_WARNINGS = "ShowWarnings";
static const char *const KEY_SHOW_INFO = "ShowInfo";
static const char *const KEY_SHOW_HINTS = "ShowHints";
static const char *const KEY_FONT_SCALE = "FontSizeScale";
static const char *const KEY_MAX_MESSAGE_LENGTH = "MaxMessageLength";
static const char *const KEY_LINE_END_SPACING = "LineEndSpacing";
static const char *const KEY_ERROR_BG = "ErrorBackground";
static const char *const KEY_WARNING_BG = "WarningBackground";
static const char *const KEY_INFO_BG = "InfoBackground";
static const char *const KEY_HINT_BG = "HintBackground";
static const char *const KEY_ERROR_FG = "ErrorForeground";
static const char *const KEY_WARNING_FG = "WarningForeground";
static const char *const KEY_INFO_FG = "InfoForeground";
static const char *const KEY_HINT_FG = "HintForeground";
static const char *const KEY_SHOW_SOURCE = "ShowSource";
static const char *const KEY_DIAG_SEPARATOR = "DiagnosticSeparator";

ErrorLensConfig::ErrorLensConfig(QObject *parent)
    : QObject(parent)
{
}

// -- Visibility helpers ------------------------------------------------

bool ErrorLensConfig::showErrors() const { return m_showErrors; }
void ErrorLensConfig::setShowErrors(bool v)
{
    if (m_showErrors != v) {
        m_showErrors = v;
        Q_EMIT configChanged();
    }
}

bool ErrorLensConfig::showWarnings() const { return m_showWarnings; }
void ErrorLensConfig::setShowWarnings(bool v)
{
    if (m_showWarnings != v) {
        m_showWarnings = v;
        Q_EMIT configChanged();
    }
}

bool ErrorLensConfig::showInfo() const { return m_showInfo; }
void ErrorLensConfig::setShowInfo(bool v)
{
    if (m_showInfo != v) {
        m_showInfo = v;
        Q_EMIT configChanged();
    }
}

bool ErrorLensConfig::showHints() const { return m_showHints; }
void ErrorLensConfig::setShowHints(bool v)
{
    if (m_showHints != v) {
        m_showHints = v;
        Q_EMIT configChanged();
    }
}

bool ErrorLensConfig::isSeverityVisible(DiagnosticSeverity s) const
{
    switch (s) {
    case DiagnosticSeverity::Error:
        return m_showErrors;
    case DiagnosticSeverity::Warning:
        return m_showWarnings;
    case DiagnosticSeverity::Information:
        return m_showInfo;
    case DiagnosticSeverity::Hint:
        return m_showHints;
    }
    return false;
}

// -- Appearance --------------------------------------------------------

double ErrorLensConfig::fontSizeScale() const { return m_fontSizeScale; }
void ErrorLensConfig::setFontSizeScale(double v)
{
    const double clamped = qBound(0.3, v, 2.0);
    if (!qFuzzyCompare(m_fontSizeScale, clamped)) {
        m_fontSizeScale = clamped;
        Q_EMIT configChanged();
    }
}

int ErrorLensConfig::maxMessageLength() const { return m_maxMessageLength; }
void ErrorLensConfig::setMaxMessageLength(int v)
{
    const int clamped = qBound(10, v, 500);
    if (m_maxMessageLength != clamped) {
        m_maxMessageLength = clamped;
        Q_EMIT configChanged();
    }
}

int ErrorLensConfig::lineEndSpacing() const { return m_lineEndSpacing; }
void ErrorLensConfig::setLineEndSpacing(int v)
{
    const int clamped = qBound(0, v, 20);
    if (m_lineEndSpacing != clamped) {
        m_lineEndSpacing = clamped;
        Q_EMIT configChanged();
    }
}

bool ErrorLensConfig::showSource() const { return m_showSource; }
void ErrorLensConfig::setShowSource(bool v)
{
    if (m_showSource != v) {
        m_showSource = v;
        Q_EMIT configChanged();
    }
}

QString ErrorLensConfig::diagnosticSeparator() const { return m_diagnosticSeparator; }
void ErrorLensConfig::setDiagnosticSeparator(const QString &s)
{
    if (m_diagnosticSeparator != s) {
        m_diagnosticSeparator = s;
        Q_EMIT configChanged();
    }
}

// -- Colours ----------------------------------------------------------

QColor ErrorLensConfig::errorBackground() const { return m_errorBackground; }
void ErrorLensConfig::setErrorBackground(const QColor &c)
{
    if (m_errorBackground != c) {
        m_errorBackground = c;
        Q_EMIT configChanged();
    }
}

QColor ErrorLensConfig::warningBackground() const { return m_warningBackground; }
void ErrorLensConfig::setWarningBackground(const QColor &c)
{
    if (m_warningBackground != c) {
        m_warningBackground = c;
        Q_EMIT configChanged();
    }
}

QColor ErrorLensConfig::infoBackground() const { return m_infoBackground; }
void ErrorLensConfig::setInfoBackground(const QColor &c)
{
    if (m_infoBackground != c) {
        m_infoBackground = c;
        Q_EMIT configChanged();
    }
}

QColor ErrorLensConfig::hintBackground() const { return m_hintBackground; }
void ErrorLensConfig::setHintBackground(const QColor &c)
{
    if (m_hintBackground != c) {
        m_hintBackground = c;
        Q_EMIT configChanged();
    }
}

QColor ErrorLensConfig::errorForeground() const { return m_errorForeground; }
void ErrorLensConfig::setErrorForeground(const QColor &c)
{
    if (m_errorForeground != c) {
        m_errorForeground = c;
        Q_EMIT configChanged();
    }
}

QColor ErrorLensConfig::warningForeground() const { return m_warningForeground; }
void ErrorLensConfig::setWarningForeground(const QColor &c)
{
    if (m_warningForeground != c) {
        m_warningForeground = c;
        Q_EMIT configChanged();
    }
}

QColor ErrorLensConfig::infoForeground() const { return m_infoForeground; }
void ErrorLensConfig::setInfoForeground(const QColor &c)
{
    if (m_infoForeground != c) {
        m_infoForeground = c;
        Q_EMIT configChanged();
    }
}

QColor ErrorLensConfig::hintForeground() const { return m_hintForeground; }
void ErrorLensConfig::setHintForeground(const QColor &c)
{
    if (m_hintForeground != c) {
        m_hintForeground = c;
        Q_EMIT configChanged();
    }
}

// -- Persistence ------------------------------------------------------

void ErrorLensConfig::load(const KConfigGroup &group)
{
    m_showErrors = group.readEntry(KEY_SHOW_ERRORS, true);
    m_showWarnings = group.readEntry(KEY_SHOW_WARNINGS, true);
    m_showInfo = group.readEntry(KEY_SHOW_INFO, false);
    m_showHints = group.readEntry(KEY_SHOW_HINTS, false);

    m_fontSizeScale = group.readEntry(KEY_FONT_SCALE, 0.85);
    m_maxMessageLength = group.readEntry(KEY_MAX_MESSAGE_LENGTH, 120);
    m_lineEndSpacing = group.readEntry(KEY_LINE_END_SPACING, 2);

    m_errorBackground = group.readEntry(KEY_ERROR_BG, m_errorBackground);
    m_warningBackground = group.readEntry(KEY_WARNING_BG, m_warningBackground);
    m_infoBackground = group.readEntry(KEY_INFO_BG, m_infoBackground);
    m_hintBackground = group.readEntry(KEY_HINT_BG, m_hintBackground);

    m_errorForeground = group.readEntry(KEY_ERROR_FG, m_errorForeground);
    m_warningForeground = group.readEntry(KEY_WARNING_FG, m_warningForeground);
    m_infoForeground = group.readEntry(KEY_INFO_FG, m_infoForeground);
    m_hintForeground = group.readEntry(KEY_HINT_FG, m_hintForeground);

    m_showSource = group.readEntry(KEY_SHOW_SOURCE, true);
    m_diagnosticSeparator = group.readEntry(KEY_DIAG_SEPARATOR, QStringLiteral(" | "));
}

void ErrorLensConfig::save(KConfigGroup &group) const
{
    group.writeEntry(KEY_SHOW_ERRORS, m_showErrors);
    group.writeEntry(KEY_SHOW_WARNINGS, m_showWarnings);
    group.writeEntry(KEY_SHOW_INFO, m_showInfo);
    group.writeEntry(KEY_SHOW_HINTS, m_showHints);

    group.writeEntry(KEY_FONT_SCALE, m_fontSizeScale);
    group.writeEntry(KEY_MAX_MESSAGE_LENGTH, m_maxMessageLength);
    group.writeEntry(KEY_LINE_END_SPACING, m_lineEndSpacing);

    group.writeEntry(KEY_ERROR_BG, m_errorBackground);
    group.writeEntry(KEY_WARNING_BG, m_warningBackground);
    group.writeEntry(KEY_INFO_BG, m_infoBackground);
    group.writeEntry(KEY_HINT_BG, m_hintBackground);

    group.writeEntry(KEY_ERROR_FG, m_errorForeground);
    group.writeEntry(KEY_WARNING_FG, m_warningForeground);
    group.writeEntry(KEY_INFO_FG, m_infoForeground);
    group.writeEntry(KEY_HINT_FG, m_hintForeground);

    group.writeEntry(KEY_SHOW_SOURCE, m_showSource);
    group.writeEntry(KEY_DIAG_SEPARATOR, m_diagnosticSeparator);
}

void ErrorLensConfig::resetDefaults()
{
    m_showErrors = true;
    m_showWarnings = true;
    m_showInfo = false;
    m_showHints = false;
    m_fontSizeScale = 0.85;
    m_maxMessageLength = 120;
    m_lineEndSpacing = 2;
    m_showSource = true;
    m_diagnosticSeparator = QStringLiteral(" | ");
    m_errorBackground = QColor(220, 38, 38, 40);
    m_warningBackground = QColor(202, 138, 4, 40);
    m_infoBackground = QColor(37, 99, 235, 40);
    m_hintBackground = QColor(100, 116, 139, 40);
    m_errorForeground = QColor(239, 68, 68);
    m_warningForeground = QColor(234, 179, 8);
    m_infoForeground = QColor(96, 165, 250);
    m_hintForeground = QColor(148, 163, 184);
    Q_EMIT configChanged();
}

} // namespace ErrorLens