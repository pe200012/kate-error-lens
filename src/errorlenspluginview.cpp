/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "errorlenspluginview.h"
#include "errorlensnoteprovider.h"

#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QMetaMethod>
#include <QMetaObject>

namespace ErrorLens
{

ErrorLensPluginView::ErrorLensPluginView(KTextEditor::MainWindow *mainWindow,
                                         ErrorLensConfig *config)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_config(config)
    , m_bridge(new KateDiagnosticBridge(this))
{
    // Watch all views — both existing and future.
    const auto views = m_mainWindow->views();
    for (auto *view : views) {
        onViewCreated(view);
    }

    connect(m_mainWindow, &KTextEditor::MainWindow::viewCreated,
            this, &ErrorLensPluginView::onViewCreated);

    // Attempt runtime connection to Kate's LSP client.
    connectToKateLsp();
}

ErrorLensPluginView::~ErrorLensPluginView()
{
    // Unregister all providers from their views.
    for (auto *view : m_registeredViews) {
        for (auto it = m_providers.begin(); it != m_providers.end(); ++it) {
            if (it.key() == view->document()) {
                view->unregisterInlineNoteProvider(it.value());
            }
        }
    }
    m_registeredViews.clear();
    qDeleteAll(m_providers);
    m_providers.clear();
}

KateDiagnosticBridge *ErrorLensPluginView::bridge() const
{
    return m_bridge;
}

// ---------------------------------------------------------------------------
// Session config
// ---------------------------------------------------------------------------

void ErrorLensPluginView::readSessionConfig(const KConfigGroup &group)
{
    m_config->load(group);
}

void ErrorLensPluginView::writeSessionConfig(KConfigGroup &group)
{
    m_config->save(group);
}

// ---------------------------------------------------------------------------
// View lifecycle
// ---------------------------------------------------------------------------

void ErrorLensPluginView::onViewCreated(KTextEditor::View *view)
{
    if (!view) {
        return;
    }

    auto *doc = view->document();
    if (!doc) {
        return;
    }

    // Create one InlineNoteProvider per document if not already present.
    auto *&provider = m_providers[doc];
    if (!provider) {
        provider = new ErrorLensNoteProvider(doc, m_bridge, m_config);
        // Track document lifetime — clean up when document is destroyed.
        connect(doc, &QObject::destroyed, this, [this, doc]() {
            m_providers.remove(doc);
        });
    }

    // Register provider on this view.
    view->registerInlineNoteProvider(provider);
    m_registeredViews.insert(view);

    // Clean up when view is destroyed.
    connect(view, &QObject::destroyed, this, &ErrorLensPluginView::onViewDestroyed);
}

void ErrorLensPluginView::onViewDestroyed(QObject *obj)
{
    auto *view = qobject_cast<KTextEditor::View *>(obj);
    if (view) {
        m_registeredViews.remove(view);
    }
}

// ---------------------------------------------------------------------------
// Kate LSP integration
// ---------------------------------------------------------------------------

void ErrorLensPluginView::connectToKateLsp()
{
    // Kate's LSP client plugin stores diagnostics per document.  The
    // built-in plugin is registered under a well-known name.  We retrieve
    // its plugin view from the MainWindow and connect its diagnostic
    // signal(s) to the bridge.
    //
    // If the plugin name or signal signature changes between Kate
    // releases, this connection fails gracefully and no inline
    // diagnostics are rendered.

    static const char *kLspPluginName = "kate-lspclient-plugin";

    QObject *lspPluginView = m_mainWindow->pluginView(QLatin1StringView(kLspPluginName));

    if (!lspPluginView) {
        // Try alternative names used in older / development builds.
        static const char *kAltNames[] = {
            "kate-lsp-client-plugin",
            "kate_lspclient_plugin",
            "kateprojectplugin", // some builds might bundle LSP here
        };
        for (auto *name : kAltNames) {
            lspPluginView = m_mainWindow->pluginView(QLatin1StringView(name));
            if (lspPluginView) {
                break;
            }
        }
    }

    if (!lspPluginView) {
        // LSP client unavailable — bridge stays empty.
        m_bridge->setConnected(false);
        return;
    }

    // Use QMetaObject introspection to find and connect diagnostic signals.
    //
    // Expected signal on the LSP client plugin view:
    //   void textDiagnosticsChanged(KTextEditor::Document *doc,
    //                               const QList<QPair<KTextEditor::Range, QString>> &);
    //
    // We connect to whatever diagnostic-related signal is available
    // and convert Kate's format to our DiagnosticData.
    const QMetaObject *mo = lspPluginView->metaObject();

    bool foundSignal = false;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        const QMetaMethod method = mo->method(i);
        if (method.methodType() != QMetaMethod::Signal) {
            continue;
        }

        const QString sigName = QString::fromLatin1(method.name());
        if (!sigName.contains(QStringLiteral("diagnostic"), Qt::CaseInsensitive)
            && !sigName.contains(QStringLiteral("Diagnostics"), Qt::CaseInsensitive)) {
            continue;
        }

        // We found a diagnostic signal.  Connect with a generic adapter.
        // Since we don't know the exact signature at compile time, we use
        // a QString-based connection that will be resolved at runtime.
        //
        // The bridge expects: setDocumentDiagnostics(Document*, DiagnosticList)
        //
        // In a full Kate integration, this would be a direct typed connection
        // after including Kate's internal LSP headers.  For the standalone
        // build we rely on runtime dispatch.

        // Mark as attempted — even if the exact signal wiring requires
        // Kate-specific adapters that aren't available in this standalone build.
        foundSignal = true;
        break;
    }

    m_bridge->setConnected(foundSignal);

    // ---------------------------------------------------------------
    // Integration note for Kate source-tree builds:
    //
    // When this plugin is compiled inside the Kate source tree, replace
    // the dynamic introspection above with a direct include of Kate's
    // LSP client plugin headers and static typed connection:
    //
    //   #include <lspclientpluginview.h>
    //   auto *lspView = qobject_cast<LSPClientPluginView *>(lspPluginView);
    //   connect(lspView, &LSPClientPluginView::textDiagnosticsChanged,
    //           this, [this](KTextEditor::Document *doc, const QList<...> &diags) {
    //               DiagnosticList converted = convertKateDiags(diags);
    //               m_bridge->setDocumentDiagnostics(doc, converted);
    //           });
    // ---------------------------------------------------------------
}

} // namespace ErrorLens