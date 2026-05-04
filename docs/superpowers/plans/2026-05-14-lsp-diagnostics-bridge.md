# LSP Diagnostics Bridge Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Connect Kate Error Lens to Kate's real LSP diagnostics provider and keep the conversion covered by tests.

**Architecture:** A new `KateDiagnosticAdapter` locates the LSP plugin view, finds the `LSPDiagnosticProvider` child object, connects to `diagnosticsAdded(FileDiagnostics)`, and pushes converted diagnostics into `KateDiagnosticBridge`. Kate private diagnostic structs are mirrored in a single header so the standalone plugin keeps public KF6 build dependencies.

**Tech Stack:** C++17, Qt6 QObject/meta-object signals, QtTest, KF6 KTextEditor.

---

### Task 1: Add a failing adapter test

**Files:**
- Modify: `tests/errorlensuitest.cpp`

- [ ] Add mirrored Kate diagnostic includes once they exist: `#include "katediagnosticadapter.h"` and `#include "katediagnostictypes.h"`.
- [ ] Add a fake `KTextEditor::MainWindow` subclass inside the test file that returns a fake LSP plugin view for `pluginView("lspclientplugin")`.
- [ ] Add a child `QObject` named `LSPDiagnosticProvider` with signal `diagnosticsAdded(const FileDiagnostics &)`.
- [ ] Add test `lspDiagnosticsProviderFeedsBridge()` that connects the adapter, emits one `FileDiagnostics`, and asserts `KateDiagnosticBridge::diagnosticsForLine(document, 2)` contains the converted message and severity.
- [ ] Run `cmake --build build -j2 && ctest --test-dir build --output-on-failure -R errorlens_ui_test`.
- [ ] Expected RED result before implementation: compile failure for missing adapter/types or runtime failure because no diagnostics reach the bridge.

### Task 2: Add Kate diagnostic mirror types

**Files:**
- Create: `src/katediagnostictypes.h`

- [ ] Define global-scope structs matching Kate's private `diagnostic_types.h`: `SourceLocation`, `DiagnosticRelatedInformation`, `Diagnostic`, `FileDiagnostics`, and enum `DiagnosticSeverity` with values Unknown=0, Error=1, Warning=2, Information=3, Hint=4.
- [ ] Include `KTextEditor/Range`, `QList`, `QString`, and `QUrl`.
- [ ] Add module documentation explaining this is a Qt meta-object compatibility boundary for Kate's diagnostics provider.

### Task 3: Implement KateDiagnosticAdapter

**Files:**
- Create: `src/katediagnosticadapter.h`
- Create: `src/katediagnosticadapter.cpp`

- [ ] Constructor stores `KTextEditor::MainWindow *` and `KateDiagnosticBridge *`.
- [ ] `connectToKateLsp()` searches plugin names in this order: `lspclientplugin`, `kate-lspclient-plugin`, `kate-lsp-client-plugin`, `kate_lspclient_plugin`.
- [ ] `connectProvider(QObject *)` prefers `findChild<QObject *>("LSPDiagnosticProvider")` and connects `diagnosticsAdded(FileDiagnostics)` to slot `onDiagnosticsAdded(const FileDiagnostics &)`, using `Qt::UniqueConnection`.
- [ ] `onDiagnosticsAdded()` finds an open document whose `url()` equals `FileDiagnostics::uri`, converts each diagnostic, and calls `KateDiagnosticBridge::setDocumentDiagnostics()`.
- [ ] Unknown severity maps to `Information`; Error, Warning, Information, and Hint map directly.

### Task 4: Wire plugin view to adapter

**Files:**
- Modify: `src/errorlenspluginview.h`
- Modify: `src/errorlenspluginview.cpp`
- Modify: `CMakeLists.txt`

- [ ] Replace the current introspection body of `ErrorLensPluginView::connectToKateLsp()` with creation and invocation of `KateDiagnosticAdapter`.
- [ ] Add `KateDiagnosticAdapter *const m_adapter` as an owned child object of `ErrorLensPluginView`.
- [ ] Add `src/katediagnosticadapter.cpp` to `ERRORLENS_SOURCES` and to the `errorlens_ui_test` source list.
- [ ] Keep provider registration and inline-note rendering unchanged.

### Task 5: Verify

**Files:**
- No source edits.

- [x] Configure: `cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON`.
- [x] Build: `cmake --build build -j2`.
- [x] Test: `ctest --test-dir build --output-on-failure`.
- [x] Record exact test output and implementation notes in Serena memory `implementation/lsp_diagnostics_bridge`.

## Implementation Results
- Added adapter and Kate diagnostic mirror types.
- Added regression test for `LSPDiagnosticProvider::diagnosticsAdded(FileDiagnostics)` feeding `KateDiagnosticBridge`.
- Added runtime payload-size validation when Qt exposes the signal argument metatype.
- Hardened provider cleanup for view/document destruction.
- Verification output: `100% tests passed, 0 tests failed out of 2`.
