/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later

    Plugin entry point — one instance per KTextEditor application.
*/

#ifndef ERRORLENS_ERRORLENSPLUGIN_H
#define ERRORLENS_ERRORLENSPLUGIN_H

#include "errorlensconfig.h"

#include <KTextEditor/Plugin>
#include <KTextEditor/SessionConfigInterface>

#include <QVariant>

namespace KTextEditor
{
class MainWindow;
}

namespace ErrorLens
{

/** Top-level plugin class.
 *
 *  Registered with KTextEditor via the JSON metadata file.  The host
 *  application calls createView() for each open MainWindow.  The
 *  returned view manages diagnostic bridging and InlineNoteProvider
 *  lifecycle.
 *
 *  This plugin also supports session-aware configuration through
 *  KTextEditor::SessionConfigInterface.
 */
class ErrorLensPlugin : public KTextEditor::Plugin,
                        public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

public:
    explicit ErrorLensPlugin(QObject *parent, const QVariantList &args);
    ~ErrorLensPlugin() override;

    /** Create a plugin view for the given \a mainWindow.
     *
     *  Called once per MainWindow.  The returned QObject is the
     *  ErrorLensPluginView for that window.
     */
    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    // -- SessionConfigInterface -----------------------------------------

    void readSessionConfig(const KConfigGroup &group) override;
    void writeSessionConfig(KConfigGroup &group) override;

    /** Shared configuration object — all plugin views reference this. */
    ErrorLensConfig *config() const;

private:
    ErrorLensConfig *const m_config;
};

} // namespace ErrorLens

#endif // ERRORLENS_ERRORLENSPLUGIN_H