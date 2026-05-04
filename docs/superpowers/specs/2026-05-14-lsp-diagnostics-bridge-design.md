# LSP Diagnostics Bridge Design

## Background
Kate Error Lens already renders inline notes when `KateDiagnosticBridge::setDocumentDiagnostics()` receives data. The failing path is the runtime connection from Kate's LSP Client plugin into that bridge.

## Problem
`ErrorLensPluginView::connectToKateLsp()` searches several plugin names and scans signals on the LSP plugin view object. Kate's current LSP plugin target is `lspclientplugin`, and its diagnostics are emitted by a child QObject named `LSPDiagnosticProvider` through `diagnosticsAdded(const FileDiagnostics &)`. The current implementation can build and pass the UI smoke test while the bridge receives zero real LSP diagnostics.

## Questions and Answers
- Q: Can this plugin include Kate private headers?
  - A: The standalone build should keep using public KF6/KTextEditor headers. The small diagnostic structs are mirrored with the same fields that Kate uses so the Qt meta-object signal can be consumed without a compile-time dependency on Kate private headers.
- Q: Which plugin object should be connected?
  - A: Prefer the child object named `LSPDiagnosticProvider` under the LSP plugin view. Keep a diagnostic-signal scan fallback for compatible provider-like objects.
- Q: How are diagnostics matched to a document?
  - A: Convert `FileDiagnostics::uri` to an open `KTextEditor::Document` by comparing document URLs across `MainWindow::views()`.

## Design
```mermaid
flowchart LR
  MainWindow --> PluginView[pluginView("lspclientplugin")]
  PluginView --> Provider[LSPDiagnosticProvider child]
  Provider -->|diagnosticsAdded(FileDiagnostics)| Adapter[KateDiagnosticAdapter]
  Adapter --> Bridge[KateDiagnosticBridge]
  Bridge --> Notes[ErrorLensNoteProvider]
```

Add a focused adapter module:
- `src/katediagnostictypes.h` mirrors Kate's `DiagnosticSeverity`, `Diagnostic`, `FileDiagnostics`, and related structs.
- `src/katediagnosticadapter.{h,cpp}` owns the runtime connection, locates the provider, converts diagnostics, and updates `KateDiagnosticBridge`.
- `ErrorLensPluginView` delegates all LSP connection work to the adapter.

## Implementation Plan
1. Add a failing QtTest that creates a fake LSP plugin view with a child `LSPDiagnosticProvider`, emits `diagnosticsAdded(FileDiagnostics)`, and expects `KateDiagnosticBridge` to contain one converted diagnostic.
2. Add `katediagnostictypes.h` and `katediagnosticadapter.{h,cpp}`.
3. Replace the current introspection-only `connectToKateLsp()` logic with adapter delegation.
4. Add the new sources to plugin and test targets.
5. Run the full build and CTest suite.

## Examples
✅ `mainWindow->pluginView("lspclientplugin")` then `findChild<QObject *>("LSPDiagnosticProvider")`.

✅ Convert severity `DiagnosticSeverity::Warning` to `ErrorLens::DiagnosticSeverity::Warning` and copy range start/end, message, source, and code.

❌ Marking the bridge connected after merely finding a diagnostic-like signal, because the bridge still receives no data.

## Trade-offs
Mirroring the small Kate diagnostic structs couples the plugin to Kate's private ABI layout. The adapter keeps that risk isolated in one module and keeps the main inline rendering logic independent. The public-header-only build remains easy to package.

## Implementation Results
- Added `src/katediagnostictypes.h` as the compatibility boundary for Kate's `FileDiagnostics` payload.
- Added `src/katediagnosticadapter.{h,cpp}` to discover `lspclientplugin`, connect `LSPDiagnosticProvider::diagnosticsAdded(FileDiagnostics)`, validate the payload metatype size when Qt exposes it, and convert diagnostics into `KateDiagnosticBridge`.
- Replaced the previous signal-name scan in `ErrorLensPluginView::connectToKateLsp()` with adapter delegation.
- Tightened view/document cleanup by removing destroyed views by pointer value, unregistering note providers on `Document::aboutToClose`, deleting providers, and clearing bridge state.
- Extended `tests/errorlensuitest.cpp` with a fake MainWindow host and fake LSP diagnostic provider regression test.
- Verification: `cmake --build build -j2 && ctest --test-dir build --output-on-failure` passed with 2/2 tests.

## Deviations
The adapter mirrors Kate private structs because the standalone package has no installed Kate private diagnostics header. Runtime payload-size checking adds a guard for compatible Qt metatypes; a full ABI compile-time check requires building inside Kate's source tree or packaging Kate private headers.