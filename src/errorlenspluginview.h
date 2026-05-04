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

class KateDiagnosticAdapter;
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

    /** Called while a KTextEditor::Document still has valid view state. */
    void onDocumentAboutToClose(KTextEditor::Document *doc);

private:
    /** Attempt to connect to Kate's internal LSP diagnostic infrastructure.
     *
     *  Delegates runtime discovery and signal conversion to KateDiagnosticAdapter.
     *
     *  If Kate's LSP client is unavailable, the adapter listens for a later
     *  pluginViewCreated signal and connects when the provider appears.
     */
    void connectToKateLsp();

    KTextEditor::MainWindow *const m_mainWindow;

    /** Shared configuration — owned by ErrorLensPlugin. */
    ErrorLensConfig *const m_config;

    /** Diagnostic data store — one per plugin view. */
    KateDiagnosticBridge *const m_bridge;

    /** Runtime connection from Kate's LSP diagnostics provider to m_bridge. */
    KateDiagnosticAdapter *const m_adapter;

    /** Document → its InlineNoteProvider (one provider per document). */
    QHash<const KTextEditor::Document *, ErrorLensNoteProvider *> m_providers;

    /** Tracks which views have an InlineNoteProvider registered. */
    QSet<KTextEditor::View *> m_registeredViews;
};

} // namespace ErrorLens

#endif // ERRORLENS_ERRORLENSPLUGINVIEW_H
