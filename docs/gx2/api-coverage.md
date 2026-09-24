# Couverture GX2 — M4 minimal

Le sous-ensemble M4 est volontairement synthétique et ne constitue pas une ABI
GX2 complète. Il exprime un triangle GX2 minimal via l'abstraction graphics.
Les noms de familles proviennent de la recherche documentaire wut [S4](../research/SOURCES.md).
La liste ci-dessous n'est pas exhaustive. **Dénominateur d'API exhaustif : UNKNOWN.**

| API / famille | Status | Evidence | Backend | Test | Notes |
|---|---|---|---|---|---|
| `gx2::Context` | IMPLEMENTED | DOCUMENTED + TESTED | DX12 | `unit.gx2`, `integration.gx2` | wrapper neutre qui emprunte graphics |
| Vertex GX2 minimal | IMPLEMENTED | DOCUMENTED + TESTED | DX12 | `runtime.integration_m4` | position 2D + couleur RGBA |
| Buffer/upload | IMPLEMENTED | TESTED via graphics | DX12 | `runtime.integration_m4` | upload opaque, pas de Map public |
| Pipeline/draw triangle | IMPLEMENTED | GPU RUNTIME TESTED | DX12 | `runtime.integration_m4` | réutilise le pipeline M3 |
| Texture descriptor mapping | PARTIAL | superseded by the evidence-backed M8 table below | DX12 | `unit.gx2_texture`, `runtime.integration_m8` | no normalized host-only format claim remains |
| Initialisation GX2 complète | NOT IMPLEMENTED | UNKNOWN | NOT IMPLEMENTED | N/A | hors M4 |
| Textures/surfaces/formats | NOT IMPLEMENTED | UNKNOWN | NOT IMPLEMENTED | N/A | hors M4 |
| Shaders GX2/Latte | NOT IMPLEMENTED | UNKNOWN | NOT IMPLEMENTED | N/A | M6 IR générique seulement, pas de parser GX2 |
| Tiling/swizzle/mips | NOT IMPLEMENTED | UNKNOWN | NOT IMPLEMENTED | N/A | aucune hypothèse introduite |
| Sync/events console | NOT IMPLEMENTED | UNKNOWN | NOT IMPLEMENTED | N/A | fence graphics privé seulement |
| Vulkan | NOT IMPLEMENTED | NOT VALIDATED | N/A | N/A | contrat conçu pour rester portable |
| OpenGL | NOT IMPLEMENTED | NOT VALIDATED | N/A | N/A | contrat conçu pour rester portable |

M5 ajoute une couverture d'erreurs : upload dépassant la taille du buffer et
triangle GX2 avec deux vertices sont refusés explicitement par
`runtime.integration_m4`; le triangle valide reste exécuté sur DX12.

**DOCUMENTED — règle :** toute ligne future par appel devra préciser provenance
de signature, sous-ensemble, données synthétiques, test, backend, OS, pilote/GPU,
résultat PASS/PARTIAL/FAIL et limites. Une assertion de parsing ou un mock ne
remplace pas une observation GPU. Ne jamais cocher OpenGL à partir d'un test Vulkan.

**OBSERVED / INFERRED :** les signatures et effets GX2 réels ne sont pas établis
par ce test synthétique. Le M4 ne revendique ni ABI console, ni mémoire Wii U,
ni shader translation. Pas de profils Splatoon/Woody, pas de hacks backend dans
`gx2/`.

M4 DX12 est validé localement ; Vulkan et OpenGL restent NOT IMPLEMENTED / NOT
VALIDATED. Cafe, PowerPC, Latte, console texture formats and translations remain
out of scope.

## M8 evidence-backed texture subset

| GX2 feature | Evidence level | MANMENMI mapping | Backend | Test | Status | Notes |
|---|---|---|---|---|---|---|
| `GX2Surface` width / height | DOCUMENTED [S4] | `TextureDescriptor::width` / `height` | neutral, DX12 exercised | `unit.gx2_texture`, `integration.gx2_texture`, `runtime.integration_m8` | VALIDATED | zero dimensions rejected |
| `GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8` (`0x01a`) | DOCUMENTED [S4]; semantic host mapping INFERRED | `rgba8_unorm` | DX12 | same M8 tests | VALIDATED for descriptor bridge | console byte order UNKNOWN |
| `GX2_SURFACE_USE_COLOR_BUFFER` | DOCUMENTED [S4] | `render_target` | DX12 | same M8 tests | VALIDATED for descriptor bridge | sampling unavailable |
| `GX2_TILE_MODE_LINEAR_ALIGNED` (`1`) | DOCUMENTED [S4]; physical semantics UNKNOWN | `opaque_render_target` | neutral, DX12 exercised | same M8 tests | PARTIAL / DESCRIPTIVE | no pitch/alignment/tile conversion |
| Opaque GX2 texture ownership | DOCUMENTED project contract + OBSERVED tests | `gx2::Texture` owns `manmenmi::Texture` | neutral, DX12 exercised | `integration.gx2_texture`, `runtime.integration_m8` | VALIDATED | no native handle in public GX2 API |
| DX12 render target / M6 draw / private readback | OBSERVED | mapped MANMENMI texture | DX12 | `runtime.integration_m8` | VALIDATED | verifies non-clear triangle pixel |
| Other formats, usages, tile modes | UNKNOWN / NOT IMPLEMENTED | none | none | N/A | NOT IMPLEMENTED | unknown represented enum values rejected |
| Tiling, swizzle, pitch, alignment, GX2 memory import | UNKNOWN | none | none | N/A | NOT IMPLEMENTED | no data-layout conversion claim |
| Vulkan / OpenGL | NOT VALIDATED | neutral descriptor contract only | Vulkan / OpenGL | N/A | NOT IMPLEMENTED | no backend path |

S4 is the pinned WUT evidence recorded in [SOURCES](../research/SOURCES.md).
