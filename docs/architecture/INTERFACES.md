# Contrats publics et frontière M1.4/M2

**DOCUMENTED — API publique MANMENMI.** Les headers restent neutres et ne
promettent pas d'ABI binaire stable. M1.3 ajoute une implémentation privée
validée pour Window, Device, Queue, Swapchain et présentation DX12 sans modifier
ces contrats publics.

| Header | Contrat M0 | Ce qui n'est PAS fourni |
|---|---|---|
| `core/result.hpp` | `ErrorCode`, `Error`, `Result<T>`, `Status` | exceptions de backend transformées automatiquement |
| `core/log.hpp` | `Logger`, catégories/niveaux | fichier rotatif, télémétrie, horloge Cafe |
| `core/backend.hpp` | noms, inventaire d'implémentation, ordre de candidats | détection pilote, capacités ou device |
| `core/config.hpp` | parsing déterministe de fichier/CLI | configuration globale cachée |
| `core/runtime.hpp` | `Runtime::tick`, `request_stop`, frontière `create_runtime` | runtime concret ; toute création retourne une erreur |
| `window/window.hpp` | `Window`, `WindowConfig`, `Extent`, `WindowEvent`, `SurfaceToken` opaque | handles SDL3 et surface native |
| `graphics/graphics.hpp` | `Device`, `Queue`, `Swapchain`, `Buffer`, `CommandBuffer`, `Pipeline`, `GraphicsFactory` | handles DX12/Vulkan/OpenGL, ressources natives, shaders publics |

Les fichiers sont sous `<module>/include/manmenmi/<module>/`.
`Buffer`, `CommandBuffer` et `Pipeline` font maintenant partie du contrat public
minimal M2/M3, mais restent opaques. La frontière `gx2/gx2.hpp` ne contient que
des valeurs GX2 synthétiques et emprunte l'API graphics ; elle ne connaît aucun
backend. `Texture`, `Shader`, `Framebuffer`, `Sync`, `Resource` et `Command` ne
font pas partie de l'API publique.

Le Shader IR M6 est interne à `graphics/` : il ne fait pas partie de l'API
publique et ne contient aucun bytecode ou type spécifique à DX12, Vulkan ou GL.
`Texture` et `TextureDescriptor` M7 restent opaques ; seul `rgba8_unorm` avec
usage `render_target` est validé sur DX12.
M8 ajoute une représentation GX2 descriptive au-dessus de cette API, sans
exposer de format/layout natif ni prétendre reproduire un layout console.
La révision M8 fondée sur preuve limite cette représentation à la projection
documentée `UNORM_R8_G8_B8_A8` / `COLOR_BUFFER` / `LINEAR_ALIGNED` et à un
`gx2::Texture` opaque qui possède le `Texture` graphics. `opaque_render_target`
est un contrat de layout MANMENMI possédé par le backend, pas une promesse de
pitch ou d'encodage GX2; ces propriétés restent UNKNOWN.
Le `Queue` et le `Swapchain` publics existants sont limités au contrat M1.3
validé ; leurs objets natifs restent privés.

## M1.4 — Contract Freeze & M2 Readiness

Statut de travail : documentation + spécification + architecture + validation.

- Aucun changement d'API publique.
- Aucun type natif dans les headers publics.
- M1.3 Window → SurfaceToken → Device → Queue → Swapchain → Acquire → Submit →
  Present → GPU synchronization → Resize/Recreate est la baseline à préserver.
- M1.4 formalise les invariants et la frontière avec M2 ; les incréments M2/M3
  ajoutent ensuite les ressources, commandes et le triangle DX12 minimal.
- Les ownership/lifetime/states des futures ressources et commandes restent
  **FUTURE / DECISION REQUIRED**.

```text
M1.3 : fenêtre, présentation et lifecycle DX12 validés
M1.4 : contrat, invariants et préparation documentaire
M2   : ressources, commandes et IR minimale
```

## Usage M0

```cpp
#include <manmenmi/core/runtime.hpp>
#include <iostream>

manmenmi::Logger logger{std::cerr};
manmenmi::Config config;
auto runtime = manmenmi::create_runtime(config, logger);
// M0 : !runtime, ErrorCode::unimplemented. Ne jamais appeler value() ici.
```

## Ownership et erreurs

- `Result<T>` porte **exactement** une valeur ou une erreur, sans succès par défaut.
  Une ressource future utilise `unique_ptr` ; destructeurs publics virtuels.
- Lire `value()` sur une erreur ou `error()` sur un succès est une faute de
  programmation (`std::bad_variant_access`), pas un contrôle de flux supporté.
- Le logger ne possède pas son `ostream`; celui-ci doit lui survivre. Un logger
  sérialise ses propres écritures ; deux loggers partageant un sink ne se
  coordonnent pas automatiquement. Minimum immutable.
- `Config`/`Options` sont des valeurs locales. Aucune variable globale mutable
  de sélection, aucun état GPU caché. Liste des backends immutable.
- **M1.3 VALIDATED :** init/destruction RAII, événement fermeture observable,
  règles de fenêtre et gestion DX12 du resize/recreate. Aucun `tick` factice ne
  constitue une preuve de rendu.

## Gardes de frontières

Les sept headers sont compilés chacun isolément. Un test CMake lexical rejette
des marqueurs natifs connus. **OBSERVED :** ces gardes passent dans les essais
locaux documentés. **UNKNOWN :** leur exhaustivité ; ce n'est pas une preuve
mathématique d'absence de toute fuite future. Les changements d'interface
nécessiteront tests et mise à jour de ce document.
