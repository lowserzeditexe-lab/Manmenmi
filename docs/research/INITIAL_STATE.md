# INITIAL_STATE — recherche M0

Consultation initiale : **2026-09-18 UTC**. Périmètre : documentation publique,
README, métadonnées de dépôts/licences et organisation de headers. Aucun dump,
SDK propriétaire, binaire de jeu ou corpus de shaders consulté. Aucune source
de recherche n'est une dépendance du build. Fiches complètes : [SOURCES](SOURCES.md).

## Taxonomie obligatoire

| Marqueur | Signification |
|---|---|
| DOCUMENTED | Énoncé attribué à une source précise ou décision explicite du projet ; pas une preuve d'exécution |
| OBSERVED | Mesure reproduite avec protocole/environnement/preuve identifiables |
| INFERRED | Hypothèse argumentée, à réfuter ou vérifier |
| UNKNOWN | Non établi, information inaccessible ou contradictoire |

`PLANNED` / `UNIMPLEMENTED` qualifient séparément la livraison. Une déclaration
« production-grade » dans un README reste DOCUMENTED **comme déclaration**,
pas OBSERVED comme fait. Les métadonnées de licence ne constituent pas un audit.

## Ce qui est établi au niveau documentaire

- **DOCUMENTED [S1] :** Aurora se présente comme couche **GameCube/Wii GX**,
  avec Dawn ; ce n'est pas une spécification Wii U/GX2. Ne pas assimiler GX à GX2.
- **DOCUMENTED [S2, S3] :** WiiUBrew décrit Cafe OS et les modules RPX/RPL ainsi
  qu'une parenté matérielle GX2/Latte avec la famille Radeon R7xx. Cela ne prouve
  pas l'équivalence des layouts, instructions ou conventions aux GPU desktop.
- **DOCUMENTED [S4] :** wut expose des familles de headers GX2/coreinit pour le
  développement Wii U. Un prototype n'établit pas tous les effets de bord,
  règles de cache, barrières ni comportements matériels.
- **DOCUMENTED [S6, S7] :** Cemu et Decaf sont des projets d'émulation Wii U.
  MANMENMI n'en adopte ni l'exécution PowerPC, ni les correctifs de jeux.
- **DOCUMENTED [S8, S9] :** RebrewU et nWiiURecomp annoncent une chaîne de
  recompilation Wii U ; leurs annonces de maturité ne sont pas vérifiées ici.
- **DOCUMENTED [S10, S11] :** Wiicompiled concerne Mario Kart **Wii**, Skate 3
  Recomp la version **Xbox 360**. Ni l'un ni l'autre n'est preuve de support GX2.

## Décisions retenues

- **DOCUMENTED — choix utilisateur :** MIT, C++20, Clang, CMake/Ninja, CTest
  autonome ; dépendances limitées/remplaçables, versions exactes et licences.
- **DOCUMENTED — choix utilisateur :** SDL3 derrière API fenêtre neutre en M1,
  Vulkan référence de correction, DX12/OpenGL progressifs et conditionnés aux tests.
- **INFERRED :** séparer état GX2 → représentation portable → backend devrait
  contenir les spécificités de GPU ; doit être testé par le même client neutre.
- **INFERRED :** isoler les ponts de présentation dans des adaptateurs privés
  évitera d'exposer des handles SDL/Vulkan ; faisabilité à valider sur les trois API.
- **OBSERVED :** le socle actuel compile et ses 26 CTests passent en local,
  dans les environnements identifiés par [M0.md](../validation/M0.md).

## Inconnues qui bloquent toute promesse graphique

- **UNKNOWN :** sous-ensemble exact GX2 nécessaire à un client neutre et ses
  signatures/ABI stables ; contrats de mémoire et synchronisation observés.
- **UNKNOWN :** translation Latte, décodage complet ISA, shader IR, comportement
  des dérivées, précision numérique et corpus libre redistribuable.
- **UNKNOWN :** tiling/swizzle, alignements, mipmaps, depth/stencil, MRT, MSAA,
  resolve et divergences par API/pilote dans MANMENMI.
- **UNKNOWN :** modèle de threads, input/audio et services Cafe compatibles.
- **UNKNOWN :** capacité à booter un jeu quelconque. Woody/Splatoon sont hors core.
- **UNKNOWN :** résultats natifs Windows/Linux x64 de la CI tant qu'aucun run
  réel avec artefacts n'est consigné. Aucun badge « passing » inventé.

## Prochain protocole de recherche, avant M1/M2 complexes

1. Épingler sources et licences de SDL3/chargeurs graphiques avant ajout.
2. Définir un test sans jeu : créer/détruire fenêtre, device et present ; journal
   de pilote/API, resize/minimisation, erreurs et sélection explicite.
3. Documenter chaque observation avec OS, GPU, pilote, backend, commit et commande.
4. Pour GX2/Latte, partir de cas synthétiques publics et d'hypothèses réfutables,
   pas de fichiers SDK ni de conventions copiées depuis un émulateur.
5. Tenir les résultats séparés par backend. Jamais une preuve Vulkan recyclée
   comme preuve OpenGL ou DX12.