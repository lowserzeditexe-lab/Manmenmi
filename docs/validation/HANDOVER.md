# MANMENMI — handover report (M0 audit)

Date: 2026-09-18 UTC
Scope: audit of the current workspace snapshot and the GitHub remote repository, without modifying source implementation.

## Repository state

### Current commit
- The workspace checked out in this environment is not a Git worktree: there is no `.git` directory in the project root.
- The actual Git history was verified from the connected remote repository:
  - remote URL: https://github.com/lowserzeditexe-lab/Manmenmi.git
  - HEAD on `main`: `606b8625768ac8d253774e45990c02707a02ad4f` (`Auto-generated changes`)
  - previous relevant commit: `b1b9c22ac604791a8acb9ada488aabc0e6fca3e9` (`Auto-generated changes`)
  - earlier project-state commit: `45d7a9c` (`## M0 — READY FOR REMOTE CI VALIDATION ...`)
- The user statement about `b1b9c22` is historically valid in the remote repository, but it is not the active code baseline in the current GitHub remote state; the current remote HEAD is later and includes the M0 validation narrative.

### Git state
- Local workspace: Git status cannot be used because `.git` is absent.
- Remote repository: valid Git repository, remote origin present and accessible.
- Remote branch state observed from GitHub clone:
  - `main` exists
  - `origin/main` exists
  - `origin/HEAD` points to `origin/main`
- No evidence was found in this audit of a completed remote GitHub Actions run attached to a commit.

### Real structure
The workspace currently contains the following major areas, matching the project layout described by the repository:
- `core/` — runtime, config, log, result, backend abstractions
- `window/` — neutral window contract
- `graphics/` — neutral graphics contract, device/backend abstraction
- `backends/` — `dx12`, `opengl`, `vulkan` directories, all reserved as future backends
- `gx2/` — reserved for future Wii U API layer
- `cafe/` — reserved for future Cafe OS layer
- `input/`, `audio/`, `filesystem/` — reserved/planned areas, not implemented
- `tests/` — CTest suite with focused M0 foundation checks
- `tools/probe/` — `manmenmi_probe` CLI
- `examples/gx2_sandbox/` — not implemented; planned only
- `.github/workflows/ci.yml` — workflow definition for native CI
- `docs/` — research, validation, architecture, build docs

Notable fact: the `backends/{vulkan,dx12,opengl}` directories contain documentation, not implementations. There are no concrete backend source files under those folders.

### Dependencies
- Required for compilation in the project itself: C++20, CMake 3.25+, Ninja, Threads, Clang or compatible C++ compiler.
- Public, user-facing project requirements are intentionally minimal and do not require Vulkan, SDL3, DX12, OpenGL, Nintendo SDKs, or game dumps.
- CI bootstrap explicitly locks external toolchain archives and verifies SHA-256 before extraction; this is documented in `ci/toolchain-lock.json`, `docs/build/TOOLCHAIN.md`, and `docs/dependencies.md`.
- There is no runtime dependency on Splatoon data, GX2 dumps, or proprietary Nintendo files.

### Tools
- Project toolchain expectations documented:
  - CMake 3.25+
  - Ninja
  - Clang
  - Windows target path uses LLVM-MinGW + Clang 18.1.8 in the workflow
  - Linux target path uses Clang 18.1.8 in the workflow
- The local environment used for verification here includes:
  - CMake 4.4.3
  - Clang 22.1.8
  - Ninja from Chocolatey
- The project also declares `actionlint` for static workflow validation.

### Platforms
- Documented target constraints: Windows x64 and Linux x64 only.
- `CMakeLists.txt` enforces:
  - host OS must be Windows or Linux
  - processor must be x64 unless `MANMENMI_ALLOW_UNSUPPORTED_HOST=ON`
  - 64-bit toolchain required
- macOS is explicitly rejected.
- No platform support claims are made for PowerPC, Wii U hardware, or non-x64 hosts.

### CI workflows
- `.github/workflows/ci.yml` defines a six-job matrix:
  - Linux x64: debug, release, coverage, sanitizers
  - Windows x64: debug, release
- The workflow includes:
  - locked archive verification before extraction
  - static validation (`actionlint`)
  - build/test/artifact generation
  - artifact upload even on failure
- The workflow exists in repository. That is evidence of intended CI structure.
- The absence of actual GitHub Actions run results or uploaded artifacts prevents claiming success for the remote jobs.

## M0 verification

### VERIFIED
- The project builds on the local Windows host used for this audit.
- Fresh local evidence from command execution:
  - `cmake -S . -B build/windows-local -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug`
  - `cmake --build build/windows-local --parallel 2`
  - `ctest --test-dir build/windows-local --output-on-failure`
- Result: 26/26 tests passed.
- This is a real local test result from the repository source snapshot, not a claim by the previous agent alone.

### LOCALLY VERIFIED
- The M0 foundation is locally valid as a source-level contract:
  - compile succeeds
  - CTest suite passes
  - backend inventory reports all backends as `UNIMPLEMENTED`
  - CLI behavior correctly rejects unimplemented runtime requests with exit code 3
  - public header contract checks pass
- This remains M0-level validation only; it does not prove native remote GitHub Actions success.

### CI VERIFIED
- None.
- No GitHub Actions run URL, artifact bundle, or official workflow success evidence was observed in the repository or from the connected GitHub Actions context during this audit.

### UNVERIFIED
- Native Windows x64 job success on GitHub hosted runners.
- Native Linux x64 job success on GitHub hosted runners.
- Coverage and sanitizer jobs on remote runners.
- Artifact verification, provenance checks, and reproducibility across the exact six artifacts.
- Any claim of “six jobs succeeded” without linked artifacts or run URLs.

### PENDING
- The remote CI gate remains pending until the actual GitHub-hosted jobs run, pass, and the uploaded artifacts are inspected.
- The project is therefore considered `READY_FOR_REMOTE_CI`, not `COMPLETE`.

## Architecture assessment

### Separation core / graphics / backends
- The design is structurally clean: `core/`, `window/`, and `graphics/` are distinct API layers.
- This is a real and intentional separation, not a naming accident.
- The public contracts intentionally avoid SDL and Vulkan/DirectX/OpenGL types.
- The architecture is consistent with the repository’s stated M0 scope.

### Independence from Vulkan
- Public APIs do not expose Vulkan types.
- `core/backend.hpp` defines a neutral `Backend` enum and `backend_inventory()` without embedding Vulkan-specific types.
- Vulkan is treated as one backend candidate, not as the API model.
- This is well aligned with the project’s “Vulkan as correction reference, not forced API” rule.

### Independence from SDL
- The `window` API only defines `Descriptor`, `Extent`, and `Window` abstract methods.
- No `SDL_Window*`, `SDL_Event`, or SDL type is exposed in the public interface.
- This is an intentional abstraction boundary and matches the project goal to add SDL3 behind the window layer later, not as a public API requirement.

### Extensibility DX12 / Vulkan / OpenGL
- The backend inventory is explicitly modeled as `vulkan`, `dx12`, and `opengl`.
- This is a good architecture for future extensibility because the project can add providers behind the same abstraction.
- However, no concrete provider exists yet; this is a skeletal interface layer only.

### Error handling
- `core/result.hpp` implements `Result<T>` with typed `ErrorCode` and `Error`.
- The project uses explicit error propagation rather than implicit success or hidden fallbacks.
- `create_runtime()` consistently returns `ErrorCode::unimplemented` at M0.
- This is one of the stronger parts of the M0 baseline.

### Threading / memory / timing
- `core` depends on `Threads::Threads` and is explicit about no hidden global state.
- There is a runtime boundary and logging abstraction, but no actual thread model or memory ownership scheme beyond the interfaces.
- Timing functionality is not implemented; this is a known M0 gap.
- The project is not yet a complete runtime system, so these areas remain intentionally uncommitted.

### Testability
- The project is testable at the M0 level because it has small, independent contract tests and CLI-driven validation.
- Header compile checks and CLI checks are meaningful and not merely smoke tests.
- This is a good foundation for M1 and beyond.

### Capacity to receive GX2
- The repo is architecturally prepared to add a GX2 layer, but nothing is implemented.
- The documentation explicitly says GX2 is planned and not yet validated.
- There is no evidence of proprietary or incompatible code being copied.
- The generic neutral design is directionally correct, but GX2 support remains a large future effort.

### Capacity to receive Cafe OS
- Same assessment as GX2: the repo is prepared conceptually, but not implemented.
- The architecture separates the neutral core from later service layers, which is appropriate.
- There is still no runtime or OS service model validated for Cafe.

## Technical debt

- No `.git` metadata is present in the current workspace; the project is effectively distributed as a static snapshot.
- The repo history is not fully represented locally, so Git-based provenance must be checked against the remote clone.
- The real project is intentionally minimal and not yet a running rendering/runtime system.
- There are no actual `vulkan`/`dx12`/`opengl` provider implementations.
- There is no real windowing, input, audio, filesystem, or presentation layer.
- There is no remote GitHub validation proof for the six CI jobs and six artifact bundles.
- There are no GPU-backed runtime tests beyond the generic M0 contract suite.
- The project has good architectural discipline, but the implementation still has a large distance to a real Wii U-compatible runtime.
- The future GX2/Cafe design can easily become overly ambitious if it is started before a stable neutral runtime contract is proven.

## Remote validation observed
- Public repository confirmed: https://github.com/lowserzeditexe-lab/Manmenmi
- Workflow observed on commit `b1b9c22`: `M0 native foundation`
- Run observed: `35356188514`
- Status: `completed`, conclusion: `success`
- Jobs observed: six jobs all `success`
- Artifact list observed: six artifact entries exist for the run
- Artifact downloads: `401 Requires authentication` from this environment, so integrity verification remains `UNVERIFIED`
- Negative run (`inject_failure=true`): not executed from this environment; `UNVERIFIED`

This is sufficient evidence for the normal remote CI gate, but not for the negative workflow proof or artifact integrity checks without a token.

## M1.1 STATUS:
COMPLETE — READY FOR REVIEW

The M1.1 window stage has been implemented behind the public API boundary and remains consistent with the validated M1 contract.

### What was implemented
- Private native window adapter in `window/src/sdl_window.hpp` and `window/src/sdl_window.cpp`.
- SDL3 is restricted to the private implementation layer and is not exposed in any public header.
- The public contract remains limited to the neutral `Window` API and opaque `SurfaceToken` model.
- A headless fallback keeps window lifecycle behavior testable in CI without leaking native types into public code.

### WindowEvent contract note
- `WindowEvent` is part of the public contract but it is not directly consumable through a public polling API in the current M1 surface.
- In the current implementation, event state is reflected only via the observable window state: `should_close()` for a close request, `size()` for resizes, and `is_visible()` for minimize/restore transitions.
- The enum remains a contract-level type for future extension, but it is not a usable direct API surface until a public retrieval mechanism is added intentionally.

### Validation evidence
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Result: 27/27 tests passed.

### Review gate
- Public API contract: verified.
- Native/window implementation: complete for M1.1.
- Remaining work: M1.2 and beyond are intentionally not started.

## M0 STATUS:
READY_FOR_REMOTE_CI

Reasons:
1. The six native GitHub Actions jobs were observed to pass on the public repository for commit `b1b9c22`.
2. The workflow definition exposes the negative test input, but the actual failure-propagation run was not triggered from this environment.
3. Artifact metadata exists, but archive download requires authentication, so direct integrity verification is not possible without a token.
4. The validation is therefore remote-run confirmed for normal CI, but not fully closed for the negative run and artifact integrity checks.

## M1.3 STATUS:
UNAVAILABLE

Verified runtime evidence from the dedicated integration binary on this machine:
- `VULKAN`: `UNAVAILABLE` because Vulkan headers are not available in this build.
- `DX12`: `UNAVAILABLE` because SDL window creation failed before any actual native DX12 swapchain cycle could start.
- Runtime binary output:
  - `[M1.3][VULKAN][REAL] result UNAVAILABLE reason: Vulkan headers are not available in this build`
  - `[M1.3][DX12][REAL] result UNAVAILABLE reason: SDL window creation unavailable: SDL3 window creation failed`
  - `M1.3 real runtime integration failures=0`

This is not a PASS and not a FAIL. It is the correct result for an environment that lacks a usable Vulkan runtime and cannot create a real DX12-backed SDL window. The project therefore remains in the honest runtime gate: backend runtime proof is absent until a compatible machine/driver configuration is available.

## M1.3 checkpoint — DX12 runtime lifecycle

Date: 2026-09-19 UTC
Scope: final local runtime validation of the existing M1.3 DX12 path.

### Validated results

| Check | Result |
| --- | --- |
| DX12 Resize | PASS |
| DX12 Recreate | PASS |
| GPU synchronization | VERIFIED |
| Backbuffer lifetime | PASS |
| Post-resize rendering | PASS |
| Post-recreate rendering | PASS |
| Runtime integration tests | PASS |
| Public contract | UNCHANGED |

### Previous blocker and correction

The original resize and recreate calls returned `DXGI_ERROR_INVALID_CALL (0x887A0001)`.
The validated DX12 lifecycle correction is limited to:

- real fence/event synchronization before swapchain resource replacement;
- releasing MANMENMI-held backbuffer references before `ResizeBuffers()`;
- reacquiring the new backbuffers after `ResizeBuffers()`;
- refreshing the swapchain image count and current backbuffer index.

The existing public `Window`, `GraphicsFactory`, `Device`, `Queue`, `Swapchain`, and `SurfaceToken` contracts were not changed. No Vulkan or OpenGL behavior was changed.

### Runtime evidence

The temporary runtime harness validated this sequence with a real SDL3 window and native DX12 objects:

```text
Window = 1280x720
Swapchain = 2 buffers
Acquire = PASS
Submit = PASS
Present = PASS
GPU_WAIT = PASS, fence synchronization verified
Resize(960,540) = PASS
Post-resize Acquire/Submit/Present/GPU_WAIT = PASS
Recreate = PASS
Post-recreate Acquire/Submit/Present/GPU_WAIT = PASS
```

The existing `manmenmi_runtime_integration_tests` executable completed with:

```text
M1.3 real runtime integration failures=0
```

Vulkan remains unavailable to the MANMENMI build because the development SDK headers/libraries are absent; this does not affect the validated DX12 result. The OpenGL MANMENMI backend remains unimplemented.

### Scope and repository checkpoint

The M1.3 correction is confined to the DX12 queue synchronization and swapchain backbuffer lifecycle, plus the necessary runtime diagnostics/tests. Vulkan, OpenGL, GX2, Cafe, shader IR, and unrelated subsystems were not changed by this checkpoint.

- Local `.git` directory: absent; no local history was created.
- Remote baseline recorded for integration: `606b8625768ac8d253774e45990c02707a02ad4f`.
- Files modified for the M1.3 implementation/checkpoint: `graphics/src/graphics.cpp`, `window/src/sdl_window.cpp`, `docs/validation/HANDOVER.md`.
- Files added: none.
- Diff summary: SDL3 boolean bootstrap correction; initial DX12 command-list state correction; real DX12 fence wait; pre-`ResizeBuffers()` backbuffer release and post-resize backbuffer/index refresh; validation checkpoint documentation.
- Documentation status: M1.3 runtime evidence and final lifecycle status recorded here.
- Push status: not attempted because this workspace has no Git metadata or configured local worktree.

M1.3 STATUS:
COMPLETE

## M1.4 — Contract Freeze & M2 Readiness

Statut de travail : documentation + spécification + architecture + validation.

Cette section est normative pour la transition après M1.3 ; les sections
historiques précédentes décrivent des états antérieurs et ne doivent pas être
interprétées comme l'état courant.

### M1.3 baseline

```text
M1.3 BASELINE: VALIDATED
M1.3 real runtime integration failures=0
```

La baseline couvre Window, SurfaceToken, GraphicsFactory, Device, Queue,
Swapchain, Acquire, Submit, Present, synchronisation DX12 réelle,
Resize/Recreate et le deuxième frame après recreate.

### M1.4 decision of work

```text
M1.4 NAME: Contract Freeze & M2 Readiness
M1.4 TYPE: documentation + specification + architecture + validation
PUBLIC API CHANGE: NO
M2 IMPLEMENTATION: OUT OF SCOPE
```

M1.4 formalise uniquement le contrat existant, ses invariants, son lifecycle et
la frontière avec M2. Elle ne crée aucune abstraction GPU et ne modifie pas le
comportement M1.3.

### M1.4 scope

Required documentation/validation:

- Window et SurfaceToken opaques ;
- GraphicsFactory, Device, Queue et Swapchain ;
- Acquire, Submit, Present et synchronisation GPU ;
- Resize/Recreate avec l'ordre validé : GPU wait, libération des anciennes
  backbuffers, ResizeBuffers, récupération des nouvelles backbuffers,
  rafraîchissement du nombre d'images et de l'index courant ;
- séparation API publique/backend privé ;
- erreurs explicites et absence de fallback silencieux ;
- distinction entre validation build, runtime et API.

### M1.4 / M2 boundary

```text
M1.4 = contrat et préparation documentaire
M2   = ressources + commandes + IR minimale
```

Les éléments suivants restent **FUTURE / DECISION REQUIRED** et ne sont pas
résolus par M1.4 : ownership/lifetime des Buffer, Texture et Resource ;
lifetime des CommandBuffer et Pipeline ; Sync public ; Resource states ;
CommandBuffer states ; transitions de ressources ; destruction différée ;
synchronisation inter-ressources ; modèle shader ; IR détaillée.

M1.4 n'ajoute aucun type public `Buffer`, `Texture`, `Resource`,
`CommandBuffer`, `Command`, `Pipeline`, `Framebuffer` ou `Sync`.

### M1.4 exclusions

Vulkan, OpenGL, GX2, Cafe, shader translation, rendu visuel, triangle, golden
images, parité cross-backend et toute implémentation M2 sont hors périmètre.

### M1.4 validation

Le test M1.3 doit rester vert :

```text
M1.3 real runtime integration failures=0
```

Les invariants à vérifier restent : headers publics sans types natifs,
SurfaceToken opaque, registry privé, erreurs explicites, absence de fallback
silencieux, lifecycle DX12 réel, synchronisation GPU réelle, Resize/Recreate et
deuxième frame après recreate.

M1.4 STATUS: FROZEN / DOCUMENTED
M1.4 IMPLEMENTATION: NONE
M2 STATUS: NOT STARTED
PUBLIC API CHANGE: NONE

## M2 Contract Freeze — M2.1 Minimal Buffer

Statut : contrat M2 gelé pour le premier incrément d'implémentation.

### Public contract

M2.1 ajoute uniquement :

- `BufferUsage` avec `transfer_source` et `transfer_destination` ;
- `BufferDescriptor` avec `size` et `usage` ;
- `Buffer` opaque à destructeur virtuel ;
- `Device::create_buffer(const BufferDescriptor&)` retournant
  `Result<std::unique_ptr<Buffer>>`.

M2.1 n'ajoute pas `Resource`, `Texture`, `Pipeline`, `Framebuffer`, `Sync`,
`CommandBuffer`, `Command` ou une IR publique.

### Buffer contract

Le Buffer représente une ressource GPU linéaire neutre. `size` doit être non nul
et `usage` doit être l'un des deux usages de transfert. Les détails CPU access,
mapping, données initiales, alignement, placement mémoire, format, tiling,
swizzle et usages shader restent différés ou privés au backend.

### Ownership and lifetime

Le client possède le Buffer via `std::unique_ptr`. Le Device possède le contexte
backend privé et le Buffer possède son état logique/backend privé. M2.1 ne crée
aucune destruction différée ni ownership partagé. Le Buffer doit être détruit
après toute utilisation GPU ; les règles CommandBuffer/in-flight appartiennent
à M2.2 et ne sont pas implémentées par M2.1.

### Backend boundary

Sur DX12, le Buffer est traduit vers un `ID3D12Resource` privé créé sur un heap
par défaut avec une description de buffer neutre. Aucun type DX12, Vulkan, SDL ou
OpenGL ne figure dans l'API publique.

Vulkan et OpenGL restent différés. Aucun nouveau backend n'est ajouté.

### M2.1 validation

- descriptor valide : création réussie sur le Device DX12 réel ;
- taille nulle : erreur explicite ;
- usage invalide : erreur explicite ;
- création native : `ID3D12Resource` privé réellement non nul ;
- destruction : release RAII sans modification de la baseline M1.3 ;
- headers publics : aucun type natif.

M2.1 ne contient ni CommandBuffer, ni CopyBuffer, ni IR, ni Texture.

## M2.2 Checkpoint — CommandBuffer + CopyBuffer

M2.2 STATUS: VALIDATED
PUBLIC API CHANGE: NONE

### Validation state

- M1.3 BASELINE: VALIDATED
- M1.4: FROZEN / DOCUMENTED
- M2.1: VALIDATED
- M2.2: VALIDATED
- BUILD: PASS
- FULL CTEST: 30/30 PASS
- TARGETED TESTS: 4/4 PASS
- M2.2 DIRECT RUNTIME: `checks=32 failures=0`
- M1.3 REGRESSION: PASS
- ARCHITECTURE NATIVE LEAKAGE: PASS
- DX12 COPYBUFFER: PASS
- GPU COMPLETION: PASS
- CONTENT VALIDATION: PASS

### Validated implementation boundaries

- Buffer DX12 réel créé et vérifié via un handle natif privé ;
- CommandBuffer opaque dans l'API publique ;
- lifecycle réel `Created -> Recording -> Closed -> Submitted -> GPU Complete -> Resettable` ;
- `CopyBuffer` enregistré et exécuté ;
- traduction DX12 par `CopyBufferRegion` ;
- soumission par `ExecuteCommandLists` ;
- fence privé et attente de complétion GPU ;
- validation du contenu de la destination après complétion ;
- absence de types natifs dans l'API publique ;
- absence de public mapping/readback/staging API.

M2.2 CHECKPOINT: COMPLETE

## M2.3 Contract Freeze — Minimal Private Transfer Command IR

M2.3 STATUS: VALIDATED
M2.3 IMPLEMENTATION: VALIDATED
M2.3 CONTRACT: FROZEN
PUBLIC API CHANGE: NONE

### Purpose

M2.3 sépare l'enregistrement d'intention de commande de la traduction DX12,
sans élargir le contrat public validé par M2.2. Le périmètre est limité à une
IR privée et typée pour `CopyBuffer`.

### Contract

- Le `CommandBuffer` possède une IR privée et ordonnée.
- `CopyBuffer` ajoute une commande validée pendant `Recording`.
- L'IR devient immuable après `close()`.
- La traduction vers DX12 s'effectue avant `ExecuteCommandLists`.
- Les transitions de ressources restent privées au backend.
- Le lifecycle reste `Created -> Recording -> Closed -> Submitted -> GPU Complete -> Resettable`.
- Le `CommandBuffer` emprunte les `Buffer` référencés ; il ne les possède pas.
- Aucun type DX12, Vulkan, SDL ou OpenGL n'entre dans l'IR ou les headers publics.

### Validation and exclusions

- Le backend de validation reste DX12 Windows.
- Les tests doivent couvrir plusieurs commandes `CopyBuffer` ordonnées et leur
  contenu final après fence et complétion GPU.
- Les erreurs de lifecycle, d'usage et de plage restent explicites.
- Vulkan/OpenGL, `Texture`, `Resource`, `Pipeline`, `Shader`, `Framebuffer`,
  draw, render pass, binding, `Sync` public, mapping/readback/staging public,
  GX2 et M3 sont hors périmètre.

### M2.3 checkpoint

- BUILD: PASS
- FULL CTEST: 30/30 PASS
- TARGETED REGRESSION: 4/4 PASS
- M2.3 DIRECT RUNTIME: `checks=33 failures=0`
- DX12 PRIVATE IR: VALIDATED
- ORDERED COPYBUFFER COMMANDS: VALIDATED
- M1.3 REGRESSION: PASS
- M2.1 REGRESSION: PASS
- M2.2 REGRESSION: PASS
- ARCHITECTURE NATIVE LEAKAGE: PASS
- VULKAN: NOT IMPLEMENTED / NOT VALIDATED FOR M2.3
- OPENGL: NOT IMPLEMENTED / NOT VALIDATED FOR M2.3

M2.3 CHECKPOINT: COMPLETE

## M3 Contract Freeze — Minimal DX12 Triangle

M3 NAME: Minimal DX12 Triangle
M3 TYPE: DX12 runtime rendering milestone
M3 STATUS: VALIDATED
M3 IMPLEMENTATION: VALIDATED
M3 CONTRACT: FROZEN
PUBLIC API CHANGE: MINIMAL / BACKEND-NEUTRAL

### Purpose

Prouver que l'abstraction graphique peut produire un draw GPU réel et une
présentation visible, au-delà du transfert de buffers validé par M2.

### Contract

- Ajouter uniquement `BufferUsage::vertex`, `Pipeline` et les commandes de
  rendu minimales nécessaires au triangle.
- `Device::create_triangle_pipeline()` crée un pipeline opaque minimal ; les
  shaders HLSL host-native restent un détail privé DX12 temporaire.
- `CommandBuffer` ajoute `begin_render_pass`, `set_pipeline`,
  `bind_vertex_buffer`, `draw` et `end_render_pass`.
- Le render target est le backbuffer de `Swapchain`, référencé sans handle natif.
- L'IR reste privée, typée, ordonnée et backend-neutral.
- Le lifecycle M2.2 reste inchangé ; un draw est enregistré entre `begin()` et
  `close()`, puis soumis par la queue existante avant `present()`.

### Backend and validation

- DX12 est le seul backend M3 implémenté et validé.
- La traduction utilise une RTV privée, un root signature vide, un pipeline
  graphique minimal et un draw triangle.
- La validation couvre build, API, enregistrement, soumission GPU et present.
- Vulkan et OpenGL restent non implémentés et non validés pour M3.
- Textures, shader translation GX2, descriptor systems, depth, MSAA, compute,
  golden images et parité cross-backend sont hors périmètre.

## M3 Checkpoint — Minimal DX12 Triangle

M3 CHECKPOINT: COMPLETE
M3 STATUS: VALIDATED
M3 BACKEND: DX12
M3 RUNTIME: PASS
M3 DIRECT RUNTIME: `checks=26 failures=0`
REGRESSION: PASS
FULL CTEST: PASS (`31/31`)

### Implemented and validated

- `BufferUsage::vertex` et buffer vertex DX12 réel ;
- `Pipeline` opaque public avec pipeline graphique DX12 minimal ;
- shaders HLSL host-native compilés en privé par `D3DCompile` ;
- IR privée ordonnée pour render pass, pipeline, vertex binding et draw ;
- traduction DX12 vers RTV, viewport, scissor et `DrawInstanced` ;
- soumission GPU, readback privé du render target et pixels non-clear validés ;
- `Present` réel après le draw.

### Backend status

- DX12 : API, backend, GPU execution, render target et present VALIDATED ;
- Vulkan : NOT IMPLEMENTED / NOT VALIDATED pour M3 ;
- OpenGL : NOT IMPLEMENTED / NOT VALIDATED pour M3.

M3 ne constitue pas une validation cross-backend, une implémentation GX2,
une traduction de shaders Wii U ou une preuve de compatibilité Wii U.

## M4 Contract Freeze — GX2 Minimal Triangle Bridge

M4 NAME: GX2 Minimal Triangle Bridge
M4 PURPOSE: Exprimer un draw GX2 minimal au-dessus de l'API graphics existante.
M4 STATUS: VALIDATED
M4 IMPLEMENTATION: VALIDATED
M4 CONTRACT: FROZEN

### GX2 FEATURES

- Un `GX2Context` emprunte `Device`, `Queue` et `Swapchain` graphics.
- Un vertex GX2 minimal contient position 2D et couleur RGBA.
- `draw_triangle()` crée un buffer vertex, l'alimente, enregistre le draw M3,
  soumet, présente et attend la complétion GPU.
- Cette surface est un sous-ensemble synthétique et non une reproduction ABI GX2.

### PUBLIC API

- Ajouter uniquement les headers `manmenmi/gx2/gx2.hpp` et les types opaques ou
  valeurs nécessaires au contexte et au vertex minimal.
- Ajouter à `Buffer` une opération d'upload de données, sans exposer mapping,
  readback, staging ou types natifs.
- Aucun type `ID3D12*`, `IDXGI*`, `Vk*`, `GLuint` ou `SDL_*` dans `gx2/`.

### LIFECYCLE AND OWNERSHIP

- `GX2Context` emprunte les objets graphics fournis par le client.
- Le contexte possède seulement les objets graphics créés pendant l'appel et
  les détruit après soumission/complétion.
- Aucun état GX2 global, mémoire Wii U, registre Latte ou synchronisation publique.

### MAPPING TO GRAPHICS

`GX2Context::draw_triangle()` -> vertex `Buffer` -> `Pipeline` opaque ->
`CommandBuffer` render pass/draw -> `Queue::submit` -> `Swapchain::present`.

### BACKEND STATUS

- DX12 : implémenter et valider sur le même GPU que M3.
- Vulkan : NOT IMPLEMENTED / NOT VALIDATED.
- OpenGL : NOT IMPLEMENTED / NOT VALIDATED.

### TESTS AND OUT OF SCOPE

- Test API GX2, test d'intégration GX2 -> graphics, test runtime GX2 -> DX12,
  avec readback privé des pixels M3.
- GX2 complet, registres, Latte, Cafe, PowerPC, shaders GX2, textures, tiling,
  swizzling, formats console, mémoire console, synchronisation console et
  compatibilité jeu sont hors périmètre.

## M4 Checkpoint — GX2 Minimal Triangle Bridge

M4 CHECKPOINT: COMPLETE
M4 STATUS: VALIDATED
M4 GX2 SUBSET: `Context`, vertex struct, buffer upload, triangle draw bridge
M4 BACKEND:
- DX12: VALIDATED
- Vulkan: NOT IMPLEMENTED / NOT VALIDATED
- OpenGL: NOT IMPLEMENTED / NOT VALIDATED
REGRESSION: PASS
FULL CTEST: PASS (`34/34`)

### Evidence

- API unit: `unit.gx2` PASS ;
- GX2 -> graphics integration: `integration.gx2` PASS ;
- GX2 -> DX12 GPU runtime: `runtime.integration_m4` PASS ;
- M3 regression remains `checks=26 failures=0` ;
- no native backend types in the GX2 public header ;
- no GX2 ABI, Cafe, PowerPC, Latte register, texture, tiling or shader
  translation claim is made.

## M5 Contract Freeze — GX2 Minimal Coverage

M5 NAME: GX2 Minimal Test Suite
M5 PURPOSE: Étendre la couverture des erreurs et frontières du sous-ensemble M4.
M5 STATUS: VALIDATED
M5 IMPLEMENTATION: VALIDATED
M5 CONTRACT: FROZEN

- Aucun changement d'API publique ni de backend.
- Couvrir les vertices de taille invalide, l'upload trop grand et la réussite
  du draw GX2 réel déjà validé par M4.
- Séparer les tests API, intégration graphics et runtime DX12.
- Conserver Vulkan/OpenGL, GX2 complet, Cafe, PowerPC et shader translation hors
  périmètre.

## M5 Checkpoint — GX2 Minimal Coverage

M5 CHECKPOINT: COMPLETE
M5 STATUS: VALIDATED
M5 REGRESSION: PASS
M5 FULL CTEST: PASS (`34/34`)
M5 TESTS: `unit.gx2`, `integration.gx2`, `runtime.integration_m4` PASS

La couverture M5 vérifie aussi explicitement le refus d'un upload trop grand et
d'un triangle GX2 avec un nombre de vertices invalide.

## M6 Contract Freeze — Minimal Shader IR

M6 NAME: Minimal Shader IR + First DX12 Translation
M6 PURPOSE: Remplacer les shaders HLSL codés en dur du triangle par une IR
structurée, validée et traduite vers HLSL/DXIL.
M6 STATUS: VALIDATED
M6 IMPLEMENTATION: VALIDATED
M6 CONTRACT: FROZEN

### SHADER INPUT

- Corpus structuré interne `ShaderModule`, pas un blob GX2.
- Deux stages : vertex et pixel.
- Le parser/decoder d'un format shader Wii U/GX2 réel reste NON IMPLEMENTED,
  car aucune preuve de format exploitable n'est présente dans le repository.

### SHADER IR

- IR privée, typée, ordonnée, déterministe et validatable.
- Types minimaux `float2` et `float4`.
- Opérations : inputs position/couleur, constante `float4`, addition `float4`,
  construction de position, move et outputs position/couleur.
- Aucun type DX12/Vulkan/GLSL/DXIL dans l'IR.

### DX12 TRANSLATION AND VALIDATION

- Traduction IR -> HLSL source structurée -> `D3DCompile` -> pipeline DX12.
- Tests séparés pour construction/validation IR, traduction/compiler et GPU.
- Le readback M3 doit toujours confirmer des pixels triangle non-clear.
- Vulkan et OpenGL restent NOT IMPLEMENTED / NOT VALIDATED.

### ERROR MODEL AND OUT OF SCOPE

- Opcode inconnu, stage invalide, operand absent, type incompatible, output
  manquant et module mal formé retournent des erreurs explicites.
- Control flow complet, textures, dérivées, atomics, geometry/tessellation,
  compute, bindings complexes, shader cache, translation GX2/Latte et parité
  Vulkan/OpenGL sont hors périmètre.

## M6 Checkpoint — Minimal Shader IR + First DX12 Translation

M6 CHECKPOINT: COMPLETE
M6 STATUS: VALIDATED
M6 SHADER IR: VALIDATED
M6 DX12 TRANSLATION: VALIDATED
M6 GPU RUNTIME: PASS
VULKAN: NOT IMPLEMENTED / NOT VALIDATED
OPENGL: NOT IMPLEMENTED / NOT VALIDATED
REGRESSION: PASS
FULL CTEST: PASS (`36/36`)

### Evidence

- `unit.shader_ir`: IR construction, validation, deterministic translation and
  invalid opcode/stage/operand/output rejection PASS ;
- `integration.shader_translation`: IR -> HLSL -> `D3DCompile` PASS ;
- `runtime.integration_m3`: triangle readback with IR-generated shaders PASS ;
- M2.2, M4 and M5 runtime regressions PASS ;
- no real GX2 shader blob parser or Latte shader translation is claimed.

## M7 Contract Freeze — Minimal Texture / Render Target

M7 NAME: Minimal Texture / Render Target System
M7 PURPOSE: Rendre une texture couleur hors écran avec le pipeline M6 et valider
son contenu réel par readback DX12.
M7 STATUS: VALIDATED
M7 IMPLEMENTATION: VALIDATED
M7 CONTRACT: FROZEN

### TEXTURE API

- `Texture` opaque ; descriptor limité à `width`, `height`, `format`, `usage`.
- Format supporté : `rgba8_unorm` uniquement.
- Usage supporté : `render_target` uniquement pour M7.
- Largeur et hauteur non nulles ; aucun array, mip, cube, depth, compression ou
  layout GX2 n'est introduit.

### RENDER TARGET AND LIFECYCLE

- `Device::create_texture()` crée une texture possédée par le client via
  `unique_ptr` et libérée en RAII.
- `CommandBuffer` emprunte la texture pendant l'enregistrement et la soumission.
- L'IR existante réutilise `BeginRenderPass`, `SetPipeline`, vertex binding,
  `Draw`, `EndRenderPass`, avec texture ou swapchain comme cible.

### DX12 / READBACK

- Texture DX12 default heap, état `RENDER_TARGET`, RTV privée et transitions
  internes au backend.
- Le test copie la texture vers une ressource readback privée après soumission,
  vérifie le clear color et des pixels triangle non-clear.
- DX12 : IMPLEMENTED + VALIDATE ; Vulkan/OpenGL : NOT IMPLEMENTED / NOT VALIDATED.
- GX2 textures, tiling, swizzle, compression, depth, MSAA, MRT et
  `CopyBufferToTexture` restent hors périmètre.

## M7 Checkpoint — Minimal Texture / Render Target

M7 CHECKPOINT: COMPLETE
M7 STATUS: VALIDATED
M7 TEXTURE SUBSET: `TextureDescriptor`, `rgba8_unorm`, `render_target`
M7 RENDER TARGET SUBSET: one offscreen color texture with private RTV
DX12: VALIDATED
VULKAN: NOT IMPLEMENTED / NOT VALIDATED
OPENGL: NOT IMPLEMENTED / NOT VALIDATED
GX2 TEXTURES: NOT IMPLEMENTED
REGRESSION: PASS
FULL CTEST: PASS (`39/39`)

### Evidence

- `unit.texture`: descriptor/type coverage PASS ;
- `integration.texture`: valid creation and invalid dimensions/format/usage PASS ;
- `runtime.integration_m7`: offscreen clear, M6 triangle draw, GPU submission,
  private readback and non-clear pixels PASS ;
- existing M1.3 through M6 tests remain green.

## M8 Contract Freeze — GX2 Texture Representation Foundation

M8 NAME: GX2 Texture / Layout Foundation
M8 PURPOSE: Introduire une représentation GX2 minimale et stricte au-dessus de
M7, sans inventer un format ou layout console non établi.
M8 STATUS: VALIDATED — PARTIAL / INFERRED GX2 REPRESENTATION
M8 IMPLEMENTATION: VALIDATED
M8 CONTRACT: FROZEN

### GX2 TEXTURE SUBSET

- `gx2::TextureDescriptor` avec width, height, format et layout.
- Format normalisé `rgba8_unorm` et layout descriptif `linear` uniquement.
- Ces noms sont une représentation hôte **INFERRED**, pas une affirmation de
  correspondance avec un enum GX2/Wii U réel.
- Les recherches du dépôt établissent seulement les familles GX2 surface/texture
  (`DOCUMENTED [S4]`) ; format et layout concrets restent `UNKNOWN`.

### GX2 -> MANMENMI MAPPING

- Conversion explicite et déterministe vers `TextureDescriptor` M7.
- Dimensions non nulles, format connu et layout `linear` requis.
- Toute autre valeur retourne `invalid_argument` ; aucun fallback.
- Le résultat est la `manmenmi::Texture` existante, sans second système de
  ressource et sans handle natif dans `gx2/`.

### VALIDATION AND BACKENDS

- Unité : conversion valide, erreurs de dimensions/format/layout et déterminisme.
- Intégration : descripteur GX2 vers texture MANMENMI.
- DX12 : render target M7, shader M6, draw et readback GPU réel.
- Vulkan/OpenGL : NOT IMPLEMENTED / NOT VALIDATED.
- Tiling, swizzle, compression, formats console, mipmaps, arrays, mémoire Wii U,
  Cafe et compatibilité jeu restent hors périmètre.

## M8 Checkpoint — GX2 Texture / Layout Foundation

M8 CHECKPOINT: COMPLETE
M8 STATUS: VALIDATED — PARTIAL / INFERRED GX2 REPRESENTATION
GX2 FORMAT SUBSET: normalized `rgba8_unorm` descriptor, evidence INFERRED
GX2 LAYOUT SUBSET: descriptive `linear`, evidence INFERRED
GX2 -> MANMENMI: VALIDATED for this normalized subset
DX12: VALIDATED
VULKAN: NOT IMPLEMENTED / NOT VALIDATED
OPENGL: NOT IMPLEMENTED / NOT VALIDATED
GPU TEST: PASS
REGRESSION: PASS
FULL CTEST: PASS (`42/42`)

### Evidence and limits

- `unit.gx2_texture`: deterministic conversion and invalid input rejection PASS ;
- `integration.gx2_texture`: GX2 descriptor to M7 Texture PASS ;
- `runtime.integration_m8`: M6 draw into mapped M7 texture and private readback PASS ;
- repository research documents GX2 surface/texture families, but no concrete
  format enum or layout semantics sufficient to claim real console compatibility;
- no GX2 tiling, swizzle, compression, memory or ABI behavior is implemented.

## M8 Contract Freeze — Evidence-backed revision

This revision supersedes the earlier normalized `rgba8_unorm` / `linear` M8
wording above.  It is frozen before the implementation below and keeps the
scope deliberately smaller than a GX2 surface implementation.

M8 NAME: GX2 Texture / Layout Foundation

M8 PURPOSE: map one documented GX2 2D color-render-target descriptor to the
existing M7 texture abstraction, then validate the resulting DX12 render target.

GX2 TEXTURE SUBSET: width, height, one `GX2SurfaceFormat` projection, one
`GX2SurfaceUse` projection, and one tile-mode metadata value.  This is not a
GX2 ABI structure and contains no console address or image pointer.

GX2 FORMAT SUBSET: `GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8` (`0x01a`) is
DOCUMENTED by WUT S4.  The semantic mapping to MANMENMI `rgba8_unorm` is
INFERRED from the documented component/type name and is runtime-tested; its
console byte order is UNKNOWN.

GX2 LAYOUT SUBSET: `GX2_TILE_MODE_LINEAR_ALIGNED` (`1`) is DOCUMENTED by WUT
S4.  It is preserved only as accepted descriptor metadata.  Pitch, alignment,
swizzle, physical placement, and whether this mode is valid for every GX2
color buffer are UNKNOWN.  M8 performs no console-memory or pixel-layout
conversion.

GX2 → MANMENMI MAPPING: non-zero width/height + the sole format +
`COLOR_BUFFER` usage + `LINEAR_ALIGNED` metadata map deterministically to
`TextureDescriptor{width, height, rgba8_unorm, render_target,
opaque_render_target}`.  Every other format, layout, usage, dimension, or
combination fails with `invalid_argument`; there is no fallback.

PUBLIC API: `gx2::TextureDescriptor`, opaque move-only `gx2::Texture`,
`map_texture_descriptor`, and `Context::create_texture`.  Public GX2 headers
contain no native backend types.

OWNERSHIP: `gx2::Texture` owns the mapped `manmenmi::Texture` by RAII;
`Context` only borrows graphics objects while creating or drawing.

VALIDATION: `unit.gx2_texture` covers deterministic conversion and rejection;
`integration.gx2_texture` uses a recording backend-neutral Device to verify
the descriptor preserved at the graphics boundary; `runtime.integration_m8`
draws through M6 and verifies DX12 private readback.

DX12: IMPLEMENTED + VALIDATED only after the runtime test passes.

VULKAN: NOT IMPLEMENTED / NOT VALIDATED.

OPENGL: NOT IMPLEMENTED / NOT VALIDATED.

OUT OF SCOPE: every other format, tile mode, tiling/swizzle algorithm, raw
texture upload, mipmaps, arrays, cubes, depth/stencil, MSAA, MRT, GX2 memory,
Cafe heap, cache, Latte registers, and game compatibility.
