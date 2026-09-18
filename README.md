# MANMENMI

**Fondation native de compatibilité Wii U pour PC — C++20, indépendante de tout jeu.**

> **M0 uniquement. Validation globale encore ouverte.** Aucune fenêtre, aucun
> rendu, aucun appel GX2/Cafe OS implémenté. Les backends sont **UNIMPLEMENTED**.
> Les tests locaux ne valent pas une CI Windows/Linux x64 passée.

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
4. SDL3 est **PLANNED** derrière la fenêtre, pas incorporé prématurément.
5. Aucune dépendance à Splatoon, Woody, Cemu ou Decaf. Aucun hack jeu dans le core.
6. Aucune promesse OpenGL/DX12/Vulkan sans tests du backend concerné.
7. Aucun passage à M1 avant toutes les preuves M0 ; aucun support macOS.

## Arborescence

```text
core/                 erreurs, logs, configuration ; runtime concret UNIMPLEMENTED
window/               contrat neutre ; adaptateur SDL3 PLANNED (M1)
graphics/             contrat neutre ; ressources et IR PLANNED (M2/M6)
backends/{vulkan,dx12,opengl}/    UNIMPLEMENTED
gx2/                  PLANNED (M4+)
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
| Aucun appel GX2 couvert en M0 | [Couverture GX2](docs/gx2/api-coverage.md) |
| Critères bloquants et preuves | [Validation M0](docs/validation/M0.md) |
| Dépendances et contributions | [Politique](docs/dependencies.md), [contribution](CONTRIBUTING.md) |

**DOCUMENTED / OBSERVED / INFERRED / UNKNOWN** qualifient les conclusions de
recherche. **PLANNED / UNIMPLEMENTED** décrivent l'état du produit ; ce ne sont
pas des preuves de fonctionnement. La recherche s'appuie uniquement sur des
sources publiques ; aucun audit juridique complet n'est revendiqué.
