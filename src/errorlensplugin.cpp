/*
    SPDX-FileCopyrightText: 2026 Error Lens Plugin Contributors
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "errorlensplugin.h"
#include "errorlenspluginview.h"

#include <KTextEditor/MainWindow>
#include <KPluginFactory>

namespace ErrorLens
{

ErrorLensPlugin::ErrorLensPlugin(QObject *parent, const QVariantList & /*args*/)
    : KTextEditor::Plugin(parent)
    , m_config(new ErrorLensConfig(this))
{
}

ErrorLensPlugin::~ErrorLensPlugin() = default;

QObject *ErrorLensPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    return new ErrorLensPluginView(mainWindow, m_config);
}

ErrorLensConfig *ErrorLensPlugin::config() const
{
    return m_config;
}

void ErrorLensPlugin::readSessionConfig(const KConfigGroup &group)
{
    m_config->load(group);
}

void ErrorLensPlugin::writeSessionConfig(KConfigGroup &group)
{
    m_config->save(group);
}

} // namespace ErrorLens

// Register the plugin with KDE's plugin factory so KTextEditor can
// discover and load it from the JSON metadata file.
K_PLUGIN_CLASS_WITH_JSON(ErrorLens::ErrorLensPlugin, "errorlens.json")

#include "errorlensplugin.moc"