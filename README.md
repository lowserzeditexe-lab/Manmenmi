# MANMENMI

**Fondation native de compatibilité Wii U pour PC — C++20, indépendante de tout jeu.**

> **État courant : M8 VALIDATED — GX2 color-target descriptor bridge / DX12.** M1.3 à M7 restent validés. M8 accepts exactly documented GX2 `UNORM_R8_G8_B8_A8`, `COLOR_BUFFER`, and `LINEAR_ALIGNED` enum values; the semantic RGBA8 projection is INFERRED, and byte layout remains UNKNOWN. Vulkan/OpenGL are not implemented or validated.

> **Historique M0 — READY FOR REMOTE CI VALIDATION.** Aucune fenêtre, aucun
> rendu, aucun appel GX2/Cafe OS implémenté. Les backends sont **UNIMPLEMENTED**.
> Les tests locaux ne valent pas une CI Windows/Linux x64 passée.
> **Aucune compatibilité Wii U fonctionnelle n'est livrée à ce stade.**
> CI native Windows/Linux **PENDING faute de repository distant**. M1 n'était pas commencé dans ce snapshot historique.

MANMENMI prépare une couche runtime native pour des projets recompilés, de
reverse engineering/decompilation et des clients neutres. Ce n'est **ni un
émulateur PowerPC, ni un recompileur, ni un produit web**. Aucun dump n'est
nécessaire pour compiler. Aucun fichier propriétaire Nintendo n'est distribué.

## Ce qui est effectivement livré en M0

- Bibliothèque de fondation : erreurs typées `Result<T>`, journalisation
  catégorisée, configuration stricte versionnée, plan de sélection explicite.
- CLI `manmenmi_probe` : aide, version, validation de configuration, inventaire
  **non matériel** ; demande de runtime refusée avec `UNIMPLEMENTED`, code **3**.
- Interfaces abstraites runtime/fenêtre/graphique, sans SDL/Vulkan/DX12/OpenGL
  dans les types publics. Elles ne sont pas des implémentations.
- CMake/Ninja, presets Clang, CTest autonome, CI Windows/Linux x64 définie,
  outils verrouillés par SHA-256, contrôles d'échec/artefacts/reproductibilité,
  mesures de coverage et instrumentation Linux optionnelles.
- Recherche sourcée, frontières architecturales et matrice de compatibilité honnête.

## Démarrage (Linux x64)

```sh
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
./build/linux-clang-debug/manmenmi_probe --check-config --config=configs/manmenmi.conf
./build/linux-clang-debug/manmenmi_probe --backend=vulkan
# Attendu : code 3 UNIMPLEMENTED, aucun rendu.
```

[Instructions Windows, Release, coverage et hôte ARM64](docs/build.md).
Le chemin Windows recommandé utilise LLVM-MinGW, sans SDK propriétaire obligatoire.
Le build local ne télécharge aucune dépendance. MIT pour le code original ;
les [notices tierces](THIRD_PARTY_NOTICES.md) conservent les licences amont.

## Règles non négociables

1. Vulkan sera la référence initiale de **correction**, jamais la forme imposée de l'API.
2. `auto` sans `--allow-fallback` essaie uniquement la préférence Vulkan. Aucun
   repli silencieux ; fichier et CLI rendent l'autorisation explicite.
3. Une fonction absente retourne une erreur, jamais un faux succès.
4. SDL3 reste privé derrière la fenêtre ; il ne définit pas l'API graphique publique.
5. Aucune dépendance à Splatoon, Woody, Cemu ou Decaf. Aucun hack jeu dans le core.
6. Aucune promesse OpenGL/DX12/Vulkan sans tests du backend concerné.
7. Aucun support macOS ; les validations backend restent spécifiques à leur environnement.

## Arborescence

```text
core/                 erreurs, logs, configuration ; runtime concret UNIMPLEMENTED
window/               contrat neutre ; adaptateur SDL3 privé validé en M1.3
graphics/             contrat neutre ; présentation DX12 M1.3, ressources M2 et triangle M3 validés localement
backends/{vulkan,dx12,opengl}/    documentation/futures ; validations spécifiques dans graphics/
gx2/                  bridge GX2 minimal + one evidence-bounded texture descriptor validated on DX12
cafe/                 PLANNED (M8)
input/ audio/ filesystem/       PLANNED (M8)
tools/probe/          diagnostic M0 exécutable
tools/inspect/        PLANNED, outil séparé (M10)
tests/                CTest + exécutables autonomes
examples/gx2_sandbox/ PLANNED (M4)
cmake/ configs/ docs/ .github/workflows/
```

## Documentation à lire

| Sujet | Document |
|---|---|
| État des connaissances | [INITIAL_STATE](docs/research/INITIAL_STATE.md) |
| Fiches de recherche et provenance | [Sources](docs/research/SOURCES.md) |
| Architecture et interfaces | [Architecture](docs/architecture/README.md), [contrats](docs/architecture/INTERFACES.md) |
| CLI, erreurs et logging | [Conventions](docs/architecture/CONVENTIONS.md) |
| Compatibilité réellement établie | [Matrice](docs/compatibility.md) |
| Sous-ensemble GX2 réellement couvert | [Couverture GX2](docs/gx2/api-coverage.md) |
| Critères bloquants et preuves | [Validation M0](docs/validation/M0.md) |
| Versions / SHA-256 / procédure de mise à jour | [Toolchain CI](docs/build/TOOLCHAIN.md) |
| Protocole normal et volontairement rouge | [Validation distante](docs/validation/REMOTE_CI.md) |
| Dépendances et contributions | [Politique](docs/dependencies.md), [contribution](CONTRIBUTING.md) |

**DOCUMENTED / OBSERVED / INFERRED / UNKNOWN** qualifient les conclusions de
recherche. **PLANNED / UNIMPLEMENTED** décrivent l'état du produit ; ce ne sont
pas des preuves de fonctionnement. La recherche s'appuie uniquement sur des
sources publiques ; aucun audit juridique complet n'est revendiqué.
