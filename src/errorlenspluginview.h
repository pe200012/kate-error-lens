/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Per-MainWindow plugin view.  Owns the diagnostic bridge and manages
    InlineNoteProvider lifecycle for each KTextEditor::View.
*/

#ifndef ERRORLENS_ERRORLENSPLUGINVIEW_H
#define ERRORLENS_ERRORLENSPLUGINVIEW_H

#include "diagnosticdata.h"
#include "errorlensconfig.h"
#include "katediagnosticbridge.h"

#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <KTextEditor/SessionConfigInterface>

#include <QHash>
#include <QObject>

namespace ErrorLens
{

class ErrorLensNoteProvider;

/** Plugin view — one instance per MainWindow.
 *
 *  Responsibilities:
 *  - Create and own the KateDiagnosticBridge.
 *  - Attempt runtime connection to Kate's LSP diagnostic infrastructure.
 *  - Watch for view-creation / view-destruction and register / unregister
 *    an ErrorLensNoteProvider on each relevant KTextEditor::View.
 *  - Persist and restore session configuration.
 */
class ErrorLensPluginView : public QObject,
                            public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    explicit ErrorLensPluginView(KTextEditor::MainWindow *mainWindow,
                                 ErrorLensConfig *config);
    ~ErrorLensPluginView() override;

    // -- SessionConfigInterface -----------------------------------------

    void readSessionConfig(const KConfigGroup &group) override;
    void writeSessionConfig(KConfigGroup &group) override;

    /** The bridge that pushes diagnostics to note providers. */
    KateDiagnosticBridge *bridge() const;

private Q_SLOTS:
    /** Called when the MainWindow creates a new KTextEditor::View. */
    void onViewCreated(KTextEditor::View *view);

    /** Called right before a KTextEditor::View is destroyed. */
    void onViewDestroyed(QObject *view);

private:
    /** Attempt to connect to Kate's internal LSP diagnostic infrastructure.
     *
     *  Uses MainWindow::pluginView() to locate the LSP client plugin and
     *  connect its diagnostic signals to the bridge.
     *
     *  If Kate's LSP client is not available or the interface has changed,
     *  the bridge stays unconnected and no inline diagnostics are shown.
     */
    void connectToKateLsp();

    KTextEditor::MainWindow *const m_mainWindow;

    /** Shared configuration — owned by ErrorLensPlugin. */
    ErrorLensConfig *const m_config;

    /** Diagnostic data store — one per plugin view. */
    KateDiagnosticBridge *const m_bridge;

    /** Document → its InlineNoteProvider (one provider per document). */
    QHash<const KTextEditor::Document *, ErrorLensNoteProvider *> m_providers;

    /** Tracks which views have an InlineNoteProvider registered. */
    QSet<KTextEditor::View *> m_registeredViews;
};

} // namespace ErrorLens

#endif // ERRORLENS_ERRORLENSPLUGINVIEW_H