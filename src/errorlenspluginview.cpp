/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "errorlenspluginview.h"
#include "errorlensnoteprovider.h"
#include "katediagnosticadapter.h"

#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <utility>

namespace ErrorLens
{

ErrorLensPluginView::ErrorLensPluginView(KTextEditor::MainWindow *mainWindow,
                                         ErrorLensConfig *config)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_config(config)
    , m_bridge(new KateDiagnosticBridge(this))
    , m_adapter(new KateDiagnosticAdapter(m_mainWindow, m_bridge, this))
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

    if (m_registeredViews.contains(view)) {
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
        // Track document lifetime while view/document pointers are still valid.
        connect(doc, &KTextEditor::Document::aboutToClose,
                this, &ErrorLensPluginView::onDocumentAboutToClose,
                Qt::UniqueConnection);
        connect(doc, &QObject::destroyed, this, [this, doc]() {
            if (auto *provider = m_providers.take(doc)) {
                delete provider;
            }
            m_bridge->clearDocument(doc);
        });
    }

    // Register provider on this view.
    view->registerInlineNoteProvider(provider);
    m_registeredViews.insert(view);

    // Clean up when view is destroyed.
    connect(view, &QObject::destroyed, this, [this, view]() {
        m_registeredViews.remove(view);
    });
}

void ErrorLensPluginView::onDocumentAboutToClose(KTextEditor::Document *doc)
{
    if (!doc) {
        return;
    }

    auto *provider = m_providers.take(doc);
    if (!provider) {
        m_bridge->clearDocument(doc);
        return;
    }

    for (auto *view : std::as_const(m_registeredViews)) {
        if (view && view->document() == doc) {
            view->unregisterInlineNoteProvider(provider);
        }
    }

    delete provider;
    m_bridge->clearDocument(doc);
}

// ---------------------------------------------------------------------------
// Kate LSP integration
// ---------------------------------------------------------------------------

void ErrorLensPluginView::connectToKateLsp()
{
    m_adapter->connectToKateLsp();
}

} // namespace ErrorLens
