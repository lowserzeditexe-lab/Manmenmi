# Contrats préparés pour M1

**DOCUMENTED — API originale MANMENMI.** Les déclarations compilent ; les objets
fenêtre/device/runtime concrets sont **UNIMPLEMENTED**. Pas de contrat ABI public.

| Header | Contrat M0 | Ce qui n'est PAS fourni |
|---|---|---|
| `core/result.hpp` | `ErrorCode`, `Error`, `Result<T>`, `Status` | exceptions de backend transformées automatiquement |
| `core/log.hpp` | `Logger`, catégories/niveaux | fichier rotatif, télémétrie, horloge Cafe |
| `core/backend.hpp` | noms, inventaire d'implémentation, ordre de candidats | détection pilote, capacités ou device |
| `core/config.hpp` | parsing déterministe de fichier/CLI | configuration globale cachée |
| `core/runtime.hpp` | `Runtime::tick`, `request_stop`, frontière `create_runtime` | runtime concret ; toute création retourne une erreur |
| `window/window.hpp` | `Descriptor`, `Extent`, `Window` abstraite | création fenêtre, SDL3, surface native |
| `graphics/graphics.hpp` | `Device`, `Backend` abstraits | provider, shaders, buffers, swapchain |

Les fichiers sont sous `<module>/include/manmenmi/<module>/`.
`Queue`, `CommandBuffer`, `Buffer`, `Texture`, `Shader`, `Pipeline`, `Framebuffer`,
`Sync` et `Swapchain` sont **des noms déclarés seulement**. Impossible de les
instancier : cela évite de prétendre avoir livré M2.

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
- **PLANNED M1 :** init/destruction RAII, événement fermeture, règles d'affinité de
  thread et gestion resize. Aucun `tick` factice retournant succès n'existe en M0.

## Gardes de frontières

Les sept headers sont compilés chacun isolément. Un test CMake lexical rejette
des marqueurs natifs connus. **OBSERVED :** ces gardes passent dans les essais
locaux documentés. **UNKNOWN :** leur exhaustivité ; ce n'est pas une preuve
mathématique d'absence de toute fuite future. Les changements d'interface
nécessiteront tests et mise à jour de ce document.