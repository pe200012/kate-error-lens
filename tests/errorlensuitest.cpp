/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Headless UI smoke test for Error Lens inline diagnostic rendering.
*/

#include "diagnosticdata.h"
#include "errorlensconfig.h"
#include "katediagnosticadapter.h"
#include "katediagnostictypes.h"
#include "errorlensnoteprovider.h"
#include "katediagnosticbridge.h"

#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QFont>
#include <QImage>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QVBoxLayout>
#include <QWidget>
#include <QtTest/QtTest>

namespace
{

QImage normalizedImage(QWidget &widget)
{
    return widget.grab().toImage().convertToFormat(QImage::Format_RGBA8888);
}

int changedPixelCount(const QImage &before, const QImage &after)
{
    if (before.size() != after.size()) {
        return before.width() * before.height() + after.width() * after.height();
    }

    int changed = 0;
    for (int y = 0; y < before.height(); ++y) {
        const auto *beforeLine = reinterpret_cast<const QRgb *>(before.constScanLine(y));
        const auto *afterLine = reinterpret_cast<const QRgb *>(after.constScanLine(y));
        for (int x = 0; x < before.width(); ++x) {
            const QColor a(beforeLine[x]);
            const QColor b(afterLine[x]);
            const int delta = qAbs(a.red() - b.red())
                + qAbs(a.green() - b.green())
                + qAbs(a.blue() - b.blue())
                + qAbs(a.alpha() - b.alpha());
            if (delta > 24) {
                ++changed;
            }
        }
    }
    return changed;
}

int magentaPixelCount(const QImage &image)
{
    int count = 0;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const QColor color(line[x]);
            if (color.red() > 150 && color.blue() > 150 && color.green() < 170) {
                ++count;
            }
        }
    }
    return count;
}

} // namespace

class FakeLspDiagnosticProvider : public QObject
{
    Q_OBJECT

public:
    explicit FakeLspDiagnosticProvider(QObject *parent = nullptr)
        : QObject(parent)
    {
        setObjectName(QStringLiteral("LSPDiagnosticProvider"));
    }

    void publish(const FileDiagnostics &diagnostics)
    {
        Q_EMIT diagnosticsAdded(diagnostics);
    }

Q_SIGNALS:
    void diagnosticsAdded(const FileDiagnostics &diagnostics);
};

class FakeKateMainWindowHost : public QObject
{
    Q_OBJECT

public:
    QList<KTextEditor::View *> viewsForWindow;
    QObject *lspPluginView = nullptr;

    Q_INVOKABLE QList<KTextEditor::View *> views()
    {
        return viewsForWindow;
    }

    Q_INVOKABLE QObject *pluginView(const QString &name)
    {
        if (name == QLatin1String("lspclientplugin")) {
            return lspPluginView;
        }
        return nullptr;
    }
};

class ErrorLensUiTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void inlineDiagnosticRendersInKTextEditorView();
    void lspDiagnosticsProviderFeedsBridge();
};

void ErrorLensUiTest::inlineDiagnosticRendersInKTextEditorView()
{
    QTemporaryDir configHome;
    QTemporaryDir dataHome;
    QVERIFY2(configHome.isValid(), "Temporary XDG config directory can be created");
    QVERIFY2(dataHome.isValid(), "Temporary XDG data directory can be created");
    qputenv("XDG_CONFIG_HOME", configHome.path().toUtf8());
    qputenv("XDG_DATA_HOME", dataHome.path().toUtf8());
    QApplication::setFont(QFont(QStringLiteral("DejaVu Sans Mono"), 11));

    KTextEditor::Editor *editor = KTextEditor::Editor::instance();
    QVERIFY2(editor, "KTextEditor editor singleton is available");

    QObject documentOwner;
    KTextEditor::Document *document = editor->createDocument(&documentOwner);
    QVERIFY2(document, "KTextEditor document can be created");
    QVERIFY2(document->setText(QStringLiteral("int main() {\n    return 0;\n}\n")), "Document text can be set");

    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    KTextEditor::View *view = document->createView(&window);
    QVERIFY2(view, "KTextEditor view can be created");
    layout->addWidget(view);

    ErrorLens::KateDiagnosticBridge bridge;
    ErrorLens::ErrorLensConfig config;
    config.setShowSource(false);
    config.setLineEndSpacing(1);
    config.setMaxMessageLength(80);
    config.setErrorForeground(QColor(255, 0, 255));
    config.setErrorBackground(QColor(255, 0, 255, 120));

    ErrorLens::ErrorLensNoteProvider provider(document, &bridge, &config);
    view->registerInlineNoteProvider(&provider);
    const auto unregisterProvider = qScopeGuard([&view, &provider]() {
        view->unregisterInlineNoteProvider(&provider);
    });

    window.resize(900, 260);
    window.show();
    QVERIFY2(QTest::qWaitForWindowExposed(&window), "KTextEditor test window is exposed");
    view->setFocus();
    QApplication::processEvents();

    const QImage before = normalizedImage(window);
    const int magentaBefore = magentaPixelCount(before);

    ErrorLens::DiagnosticData diagnostic;
    diagnostic.severity = ErrorLens::DiagnosticSeverity::Error;
    diagnostic.line = 1;
    diagnostic.column = 11;
    diagnostic.endLine = 1;
    diagnostic.endColumn = 12;
    diagnostic.message = QStringLiteral("missing return value rendered by Error Lens UI test");
    diagnostic.source = QStringLiteral("ui-test");

    bridge.setDocumentDiagnostics(document, ErrorLens::DiagnosticList{diagnostic});
    QTRY_COMPARE(provider.inlineNotes(1).size(), 1);

    QImage after;
    QTRY_VERIFY_WITH_TIMEOUT([&]() {
        view->update();
        window.update();
        QApplication::processEvents();
        after = normalizedImage(window);
        return changedPixelCount(before, after) > 20
            && magentaPixelCount(after) > magentaBefore + 5;
    }(), 3000);

    const int changedPixels = changedPixelCount(before, after);
    const int magentaAfter = magentaPixelCount(after);
    QVERIFY2(changedPixels > 20, qPrintable(QStringLiteral("Expected rendered view to change after diagnostic injection; changed pixels: %1").arg(changedPixels)));
    QVERIFY2(magentaAfter > magentaBefore + 5,
             qPrintable(QStringLiteral("Expected magenta diagnostic pixels to increase; before: %1 after: %2").arg(magentaBefore).arg(magentaAfter)));
}

void ErrorLensUiTest::lspDiagnosticsProviderFeedsBridge()
{
    QTemporaryFile sourceFile;
    QVERIFY2(sourceFile.open(), "Temporary source file can be created");
    sourceFile.write("int main() {\n    int value = 0;\n    return value;\n}\n");
    sourceFile.flush();

    KTextEditor::Editor *editor = KTextEditor::Editor::instance();
    QVERIFY2(editor, "KTextEditor editor singleton is available");

    QObject documentOwner;
    KTextEditor::Document *document = editor->createDocument(&documentOwner);
    QVERIFY2(document, "KTextEditor document can be created");
    QVERIFY2(document->openUrl(QUrl::fromLocalFile(sourceFile.fileName())), "Document can open the temporary source file");

    QWidget viewParent;
    KTextEditor::View *view = document->createView(&viewParent);
    QVERIFY2(view, "KTextEditor view can be created for adapter document lookup");

    QObject lspPluginView;
    FakeLspDiagnosticProvider provider(&lspPluginView);

    FakeKateMainWindowHost host;
    host.viewsForWindow = {view};
    host.lspPluginView = &lspPluginView;
    KTextEditor::MainWindow mainWindow(&host);

    ErrorLens::KateDiagnosticBridge bridge;
    ErrorLens::KateDiagnosticAdapter adapter(&mainWindow, &bridge);
    QVERIFY2(adapter.connectToKateLsp(), "Adapter connects to the LSPDiagnosticProvider child");
    QVERIFY2(bridge.isConnected(), "Bridge records a live diagnostic provider connection");

    FileDiagnostics fileDiagnostics;
    fileDiagnostics.uri = document->url();

    Diagnostic diagnostic;
    diagnostic.severity = DiagnosticSeverity::Warning;
    diagnostic.range = KTextEditor::Range(2, 4, 2, 16);
    diagnostic.message = QStringLiteral("return value should be explicit");
    diagnostic.source = QStringLiteral("fake-lsp");
    diagnostic.code = QStringLiteral("demo-warning");
    fileDiagnostics.diagnostics.append(diagnostic);

    provider.publish(fileDiagnostics);

    const ErrorLens::DiagnosticList converted = bridge.diagnosticsForLine(document, 2);
    QCOMPARE(converted.size(), 1);
    QCOMPARE(converted.first().severity, ErrorLens::DiagnosticSeverity::Warning);
    QCOMPARE(converted.first().line, 2);
    QCOMPARE(converted.first().column, 4);
    QCOMPARE(converted.first().endLine, 2);
    QCOMPARE(converted.first().endColumn, 16);
    QCOMPARE(converted.first().message, diagnostic.message);
    QCOMPARE(converted.first().source, diagnostic.source);
    QCOMPARE(converted.first().code, diagnostic.code);
}

QTEST_MAIN(ErrorLensUiTest)

#include "errorlensuitest.moc"
