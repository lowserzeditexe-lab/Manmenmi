# Notices tierces — M0

Le code original MANMENMI est sous [MIT](LICENSE). **Aucun code tiers vendored,
aucun SDK Nintendo, dump ou asset de jeu dans cette livraison.** Les projets
étudiés ne sont ni liés ni redistribués ; leur licence ne devient pas MIT.

## Composants effectivement nécessaires

| Composant | Version / origine | Usage | Licence / remarque |
|---|---|---|---|
| Bibliothèque standard C++ / threads système | chaîne hôte documentée dans les preuves | Seules dépendances de la fondation | Licence propre à la chaîne ; aucune copie de sources ici |
| CMake / CTest | minimum 3.25 ; CI 3.31.6, [Kitware](https://cmake.org/) / paquet PyPI exact | Configuration, compilation et tests | BSD-3-Clause ; notices du paquet conservées par son installateur |
| Ninja | CI paquet `ninja==1.11.1.3`, binaire 1.11.1, [ninja-build](https://github.com/ninja-build/ninja) | Build | Apache-2.0 ; wrapper Python avec notices propres |
| LLVM / Clang / compiler-rt / LLVM tools | CI Linux 18.1.8, [release officielle](https://github.com/llvm/llvm-project/releases/tag/llvmorg-18.1.8) | Compilation, coverage, sanitizers | Apache-2.0 WITH LLVM-exception ; voir notices de la distribution |
| LLVM-MinGW | CI Windows **20240619 UCRT**, LLVM 18.1.8, [release](https://github.com/mstorsjo/llvm-mingw/releases/tag/20240619) | Chaîne Windows sans SDK MSVC | Distribution multi-licences : LLVM/Clang/libc++ Apache-2.0 WITH LLVM-exception ; mingw-w64 et winpthreads avec notices propres, à conserver si binaires redistribués |
| Python | CI 3.12.9, [python.org](https://www.python.org/) | Installation des outils CI, pas requis pour builder localement | PSF-2.0 et notices associées |
| libtinfo5 / libxml2 | paquets système de l'image ubuntu-22.04 | Exécution du Clang précompilé CI | MIT/X11 et notices propres ; pas liés par MANMENMI |
| GCC/libstdc++/glibc | version système, local GCC 12.2/glibc 2.36 | Headers/runtime Linux, y compris cross x64 | GCC GPL avec Runtime Library Exception applicable ; glibc LGPL, licences par composant |

`UCRT` est un composant système Windows, non distribué ici. Les exécutables
Windows peuvent nécessiter les DLL libc++/winpthreads de la même distribution
LLVM-MinGW sur PATH. **Aucun SDK propriétaire obligatoire** dans la chaîne retenue.
Avant de distribuer des binaires et leurs DLL, établir une nomenclature précise
des composants embarqués avec leurs textes de licence. M0 n'est pas un paquet binaire redistribuable.

## Actions CI (outils, pas dépendances du produit)

| Projet (MIT) | Version | Révision épinglée |
|---|---|---|
| [actions/checkout](https://github.com/actions/checkout) | v4.2.2 | `11bd71901bbe5b1630ceea73d27597364c9af683` |
| [actions/setup-python](https://github.com/actions/setup-python) | v5.6.0 | `a26af69be951a213d495a4c3e4e4022e16d87065` |
| [actions/upload-artifact](https://github.com/actions/upload-artifact) | v4.6.2 | `ea165f8d65b6e75b540449e92b4886f43607fa02` |

## Outils locaux supplémentaires de vérification

Clang/LLVM 14.0.6, CMake 3.25.1, Ninja 1.11.1 et compiler-rt 14 depuis Debian.
Cross GCC/libstdc++ x86-64 12.2.0, QEMU user 7.2 (GPL-2.0 et composants associés).
QEMU est uniquement un exécuteur de tests x64 sur l'hôte ARM64 : **aucune
émulation PowerPC dans MANMENMI**, aucune dépendance produit à QEMU.
Cross LLVM-MinGW 20240619 pour vérification de compilation Windows, même origine
et notices que la chaîne Windows CI. Rien n'est téléchargé par CMake.
Validation statique locale du workflow : [actionlint](https://github.com/rhysd/actionlint)
**1.7.7**, licence MIT, archive officielle `actionlint_1.7.7_linux_arm64.tar.gz`.
Outil de contrôle uniquement, pas dépendance du build ni du runtime.

## Références étudiées, NON incorporées

| Source | Licence déclarée ou état | Utilisation |
|---|---|---|
| Aurora | MIT | Recherche documentaire uniquement |
| WiiUBrew | CC BY-SA, version précise UNKNOWN dans cette lecture | Résumés originaux attribués et liens ; pas de reprise de texte/page |
| wut | zlib | Index de familles API, aucun header importé |
| libwiiu | UNKNOWN | Contexte historique, aucun code |
| Cemu | MPL-2.0 | Recherche, pas une dépendance |
| Decaf | GPL v3, or-later dans le texte consulté | Recherche, pas une dépendance |
| RebrewU | UNKNOWN | Déclarations amont uniquement |
| nWiiURecomp | UNKNOWN / incohérence des indications | Pas de reprise avant clarification |
| Wiicompiled | GPL v3 déclarée | Recherche, aucun élément de Mario Kart |
| Skate 3 Recomp | UNKNOWN | Recherche, aucun élément de Skate 3/ReXGlue |

Les [fiches](docs/research/SOURCES.md) fournissent URLs, révisions, limites et
risques de chaque source. Ni un README ni une métadonnée GitHub ne remplacent
une lecture légale par fichier lors d'une éventuelle réutilisation.

## Futur, non installé

SDL3 : **PLANNED**, zlib déclarée par le projet, version exacte à sélectionner en
M1. Vulkan headers/loader, OpenGL loader, outils shaders, éventuel GoogleTest :
**PLANNED / version UNKNOWN**, aucune dépendance ajoutée tant qu'inutile.