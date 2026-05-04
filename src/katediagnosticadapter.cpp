/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katediagnosticadapter.h"

#include "katediagnosticbridge.h"

#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaType>
#include <QStringList>

#include <utility>

namespace ErrorLens
{

KateDiagnosticAdapter::KateDiagnosticAdapter(KTextEditor::MainWindow *mainWindow,
                                             KateDiagnosticBridge *bridge,
                                             QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_bridge(bridge)
{
    qRegisterMetaType<FileDiagnostics>("FileDiagnostics");
}

bool KateDiagnosticAdapter::connectToKateLsp()
{
    connect(m_mainWindow, &KTextEditor::MainWindow::pluginViewCreated,
            this, &KateDiagnosticAdapter::onPluginViewCreated,
            Qt::UniqueConnection);

    static const QStringList kPluginNames = {
        QStringLiteral("lspclientplugin"),
        QStringLiteral("kate-lspclient-plugin"),
        QStringLiteral("kate-lsp-client-plugin"),
        QStringLiteral("kate_lspclient_plugin"),
    };

    for (const QString &name : kPluginNames) {
        if (QObject *pluginView = m_mainWindow->pluginView(name)) {
            if (connectProvider(pluginView)) {
                return true;
            }
        }
    }

    m_bridge->setConnected(false);
    return false;
}

void KateDiagnosticAdapter::onPluginViewCreated(const QString &name, QObject *pluginView)
{
    if (isLspPluginName(name)) {
        connectProvider(pluginView);
    }
}

void KateDiagnosticAdapter::onDiagnosticsAdded(const FileDiagnostics &diagnostics)
{
    KTextEditor::Document *document = documentForUrl(diagnostics.uri);
    if (!document) {
        return;
    }

    DiagnosticList converted;
    converted.reserve(diagnostics.diagnostics.size());

    for (const Diagnostic &diagnostic : diagnostics.diagnostics) {
        DiagnosticData item;
        item.severity = convertSeverity(diagnostic.severity);
        item.line = diagnostic.range.start().line();
        item.column = diagnostic.range.start().column();
        item.endLine = diagnostic.range.end().line();
        item.endColumn = diagnostic.range.end().column();
        item.message = diagnostic.message.simplified();
        item.source = diagnostic.source;
        item.code = diagnostic.code;
        converted.append(item);
    }

    m_bridge->setDocumentDiagnostics(document, converted);
}

bool KateDiagnosticAdapter::connectProvider(QObject *pluginView)
{
    if (!pluginView) {
        return false;
    }

    QList<QObject *> candidates;
    if (QObject *namedProvider = pluginView->findChild<QObject *>(QStringLiteral("LSPDiagnosticProvider"))) {
        candidates.append(namedProvider);
    }
    candidates.append(pluginView);
    candidates.append(pluginView->findChildren<QObject *>());

    for (QObject *candidate : std::as_const(candidates)) {
        if (!candidate || candidate == m_provider || !hasDiagnosticsAddedSignal(candidate)) {
            continue;
        }

        const bool connected = QObject::connect(candidate, SIGNAL(diagnosticsAdded(FileDiagnostics)),
                                                this, SLOT(onDiagnosticsAdded(FileDiagnostics)),
                                                Qt::UniqueConnection);
        if (connected) {
            m_provider = candidate;
            m_bridge->setConnected(true);
            return true;
        }
    }

    return false;
}

KTextEditor::Document *KateDiagnosticAdapter::documentForUrl(const QUrl &url) const
{
    if (!url.isValid()) {
        return nullptr;
    }

    const QList<KTextEditor::View *> views = m_mainWindow->views();
    for (KTextEditor::View *view : views) {
        if (!view) {
            continue;
        }

        KTextEditor::Document *document = view->document();
        if (document && document->url() == url) {
            return document;
        }
    }

    return nullptr;
}

bool KateDiagnosticAdapter::isLspPluginName(const QString &name)
{
    return name == QLatin1String("lspclientplugin")
        || name == QLatin1String("kate-lspclient-plugin")
        || name == QLatin1String("kate-lsp-client-plugin")
        || name == QLatin1String("kate_lspclient_plugin");
}

bool KateDiagnosticAdapter::hasDiagnosticsAddedSignal(const QObject *object)
{
    if (!object) {
        return false;
    }

    const QMetaObject *metaObject = object->metaObject();
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        const QMetaMethod method = metaObject->method(i);
        if (method.methodType() == QMetaMethod::Signal
            && method.methodSignature() == QByteArrayLiteral("diagnosticsAdded(FileDiagnostics)")) {
            const QMetaType payloadType = method.parameterMetaType(0);
            return !payloadType.isValid() || payloadType.sizeOf() == sizeof(FileDiagnostics);
        }
    }

    return false;
}

ErrorLens::DiagnosticSeverity KateDiagnosticAdapter::convertSeverity(::DiagnosticSeverity severity)
{
    switch (severity) {
    case ::DiagnosticSeverity::Error:
        return ErrorLens::DiagnosticSeverity::Error;
    case ::DiagnosticSeverity::Warning:
        return ErrorLens::DiagnosticSeverity::Warning;
    case ::DiagnosticSeverity::Information:
        return ErrorLens::DiagnosticSeverity::Information;
    case ::DiagnosticSeverity::Hint:
        return ErrorLens::DiagnosticSeverity::Hint;
    case ::DiagnosticSeverity::Unknown:
        return ErrorLens::DiagnosticSeverity::Information;
    }

    return ErrorLens::DiagnosticSeverity::Information;
}

} // namespace ErrorLens
