# Error Lens UI Smoke Test Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a headless automated UI test that verifies Error Lens inline diagnostics render inside a real KTextEditor view.

**Architecture:** CMake exposes a `BUILD_TESTING` path that builds a QtTest executable. The test instantiates KTextEditor editor/document/view objects, registers `ErrorLensNoteProvider`, feeds diagnostics through `KateDiagnosticBridge`, and validates screenshot changes.

**Tech Stack:** CMake/CTest, Qt6 Test/Widgets/Gui, KF6 KTextEditor/KConfig/KI18n/KXmlGui.

---

### Task 1: Add CMake test wiring

**Files:**
- Modify: `CMakeLists.txt`

- [ ] Add `include(CTest)` after ECM setup includes.
- [ ] Add `Qt6::Test` only when `BUILD_TESTING` is true.
- [ ] Add `errorlens_ui_test` executable under `BUILD_TESTING` with the production provider, bridge, config, and moc stub sources.
- [ ] Add a CTest entry named `errorlens_ui_test` with `QT_QPA_PLATFORM=offscreen`.

### Task 2: Add the UI smoke test

**Files:**
- Create: `tests/errorlensuitest.cpp`

- [ ] Create a QtTest class `ErrorLensUiTest`.
- [ ] Create a `KTextEditor::Document` and `KTextEditor::View`.
- [ ] Register `ErrorLensNoteProvider` on the view.
- [ ] Grab a baseline image.
- [ ] Inject an error diagnostic through `KateDiagnosticBridge::setDocumentDiagnostics`.
- [ ] Wait until the provider reports one inline note on the affected line.
- [ ] Grab another image and assert the screenshots differ.
- [ ] Assert the second screenshot contains a magenta-tinted diagnostic pixel.

### Task 3: Verify

**Files:**
- No source file changes.

- [ ] Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON`
- [ ] Build: `cmake --build build`
- [ ] Test: `ctest --test-dir build --output-on-failure -R errorlens_ui_test`
- [ ] Confirm the plugin target still builds.
