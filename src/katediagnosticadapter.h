/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Runtime adapter from Kate's LSP DiagnosticsProvider signal to the
    Error Lens diagnostic bridge.
*/

#ifndef ERRORLENS_KATEDIAGNOSTICADAPTER_H
#define ERRORLENS_KATEDIAGNOSTICADAPTER_H

#include "diagnosticdata.h"
#include "katediagnostictypes.h"

#include <QObject>
#include <QPointer>
#include <QString>

namespace KTextEditor
{
class Document;
class MainWindow;
}

namespace ErrorLens
{

class KateDiagnosticBridge;

/** Connects Kate's LSP diagnostics provider to KateDiagnosticBridge.
 *
 *  Kate exposes LSP diagnostics through a DiagnosticsProvider child object
 *  named `LSPDiagnosticProvider`.  This adapter owns the Qt meta-object
 *  connection, converts Kate's FileDiagnostics payload into Error Lens data,
 *  and updates the bridge used by ErrorLensNoteProvider.
 */
class KateDiagnosticAdapter : public QObject
{
    Q_OBJECT

public:
    explicit KateDiagnosticAdapter(KTextEditor::MainWindow *mainWindow,
                                   KateDiagnosticBridge *bridge,
                                   QObject *parent = nullptr);

    /** Try to connect to an already-loaded Kate LSP Client plugin view.
     *
     *  The adapter also listens for future pluginViewCreated signals, so a
     *  later LSP plugin load can still activate diagnostics.
     *
     *  @return true when a provider is connected during this call.
     */
    bool connectToKateLsp();

private Q_SLOTS:
    void onPluginViewCreated(const QString &name, QObject *pluginView);
    void onDiagnosticsAdded(const FileDiagnostics &diagnostics);

private:
    bool connectProvider(QObject *pluginView);
    KTextEditor::Document *documentForUrl(const QUrl &url) const;

    static bool isLspPluginName(const QString &name);
    static bool hasDiagnosticsAddedSignal(const QObject *object);
    static ErrorLens::DiagnosticSeverity convertSeverity(::DiagnosticSeverity severity);

    KTextEditor::MainWindow *const m_mainWindow;
    KateDiagnosticBridge *const m_bridge;
    QPointer<QObject> m_provider;
};

} // namespace ErrorLens

#endif // ERRORLENS_KATEDIAGNOSTICADAPTER_H
