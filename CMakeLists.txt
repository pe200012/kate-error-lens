cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Kate Error Lens — inline diagnostic display plugin for the KTextEditor framework.
#
# Dependencies:
#   Qt6 (Widgets, Gui)
#   KF6 (KTextEditor, CoreAddons, ConfigCore, I18n, XmlGui)
#   ECM  (Extra CMake Modules)
#
# Build:
#   cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
#   cmake --build build
#   cmake --install build
# ---------------------------------------------------------------------------

project(KateErrorLens VERSION 1.0.0 LANGUAGES CXX)

# -- C++ standard ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -- ECM (Extra CMake Modules) -----------------------------------------------
find_package(ECM 5.240.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)

# Tests are opt-in for packagers and regular local builds. KDECMakeSettings
# includes CTest and normally defaults BUILD_TESTING to ON, so seed the cache
# before including it while still respecting an explicit -DBUILD_TESTING=ON.
if(NOT DEFINED BUILD_TESTING)
    set(BUILD_TESTING OFF CACHE BOOL "Build automated tests")
endif()

include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupQtPluginMacroNames)

# -- Dependencies ------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
find_package(KF6 REQUIRED COMPONENTS
    TextEditor
    CoreAddons
    Config
    I18n
    XmlGui
)

# -- Source files ------------------------------------------------------------
set(ERRORLENS_SOURCES
    src/errorlensplugin.cpp
    src/errorlenspluginview.cpp
    src/errorlensnoteprovider.cpp
    src/katediagnosticadapter.cpp
    src/katediagnosticbridge.cpp
    src/errorlensconfig.cpp
    src/diagnosticprovider_moc.cpp
)

# -- Plugin target -----------------------------------------------------------
# Tell CMake's automoc that K_PLUGIN_CLASS_WITH_JSON injects
# Q_PLUGIN_METADATA, so it correctly tracks the .json dependency.
ecm_setup_qtplugin_macro_names(MACRO_NAMES K_PLUGIN_CLASS_WITH_JSON)

add_library(kateerrorlens MODULE ${ERRORLENS_SOURCES})

target_link_libraries(kateerrorlens PRIVATE
    Qt6::Widgets
    Qt6::Gui
    KF6::TextEditor
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::XmlGui
)

target_include_directories(kateerrorlens PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Mark the .json file as a dependency for automoc.
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/errorlens.json PROPERTIES HEADER_FILE_ONLY TRUE
)
target_sources(kateerrorlens PRIVATE errorlens.json)

# -- Tests -------------------------------------------------------------------
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_executable(errorlens_ui_test
        tests/errorlensuitest.cpp
        src/errorlensnoteprovider.cpp
        src/katediagnosticadapter.cpp
        src/katediagnosticbridge.cpp
        src/errorlensconfig.cpp
        src/diagnosticprovider_moc.cpp
    )

    target_include_directories(errorlens_ui_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(errorlens_ui_test PRIVATE
        Qt6::Test
        Qt6::Widgets
        Qt6::Gui
        KF6::TextEditor
        KF6::ConfigCore
        KF6::I18n
        KF6::XmlGui
    )

    add_test(NAME errorlens_ui_test COMMAND errorlens_ui_test)
    set_tests_properties(errorlens_ui_test PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=*.debug=false"
        TIMEOUT 30
    )
endif()

# -- Install -----------------------------------------------------------------
# KTextEditor plugins live under the kf6/ktexteditor subdirectory of the
# Qt plugin install path.
install(TARGETS kateerrorlens
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/ktexteditor
)

# -- Feature summary ---------------------------------------------------------
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
