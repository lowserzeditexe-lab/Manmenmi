# Fiches des sources publiques

Consultées le **2026-09-18 UTC**. Lecture documentaire initiale seulement ; aucun
projet tiers compilé ou exécuté, aucun code copié. Les SHA ci-dessous ont été
résolus via l'API GitHub lors de la consultation des pages de branche ; ils
figent un point de suivi. Ils ne constituent pas un audit complet de ces révisions.
Une URL de README de branche peut évoluer ; utiliser la révision liée pour
reproduire/approfondir. Une éventuelle course entre lectures reste possible.

## S1 — Aurora

Source : [encounter/aurora](https://github.com/encounter/aurora),
[révision 011e331](https://github.com/encounter/aurora/tree/011e331097fc81d7b271237b37cbe7828fd675b9).

- **DOCUMENTED — résumé :** couche source GameCube/Wii destinée aux projets de
  decompilation. Le README cite Dawn et SDL3. Licence déclarée MIT.
- **INFERRED — exploitable :** séparation API historique / rendu moderne,
  maintien de tests indépendants du jeu.
- **INFERRED — risques :** confondre GX/TEV avec GX2/Latte ; faire de WebGPU/Dawn
  le modèle obligatoire ; hériter d'un graphe de dépendances sans nécessité.
  MIT n'annule pas les obligations propres aux composants transitifs.
- **DOCUMENTED — inspiration retenue :** frontière de compatibilité source et
  responsabilités séparées, **aucun code**, aucune dépendance à Dawn/Aurora.
- **UNKNOWN :** pertinence des mécanismes internes pour les shaders Latte.

## S2 — WiiUBrew / Cafe OS

Source : [Cafe OS, révision oldid=4820](https://wiiubrew.org/w/index.php?title=Cafe_OS&oldid=4820),
[guide homebrew](https://wiiubrew.org/wiki/Homebrew_development_guide).

- **DOCUMENTED — résumé :** documentation communautaire sur l'OS PowerPC en mode
  Wii U, chargeur et bibliothèques RPX/RPL ; le pied de page indique CC BY-SA
  (version précise à recontrôler avant toute réutilisation textuelle).
- **INFERRED — exploitable :** vocabulaire et découpage des services sélectionnés.
- **INFERRED — risques :** incomplétude d'un wiki, révisions anciennes, précision
  variable ; attribution/share-alike à examiner si texte ou schéma repris.
- **DOCUMENTED — inspiration retenue :** catalogue de concepts pour futures
  questions Cafe, pas réimplémentation d'un OS entier ni copie des pages.
- **UNKNOWN :** comportement exhaustif des modules sur matériel, non observé ici.

## S3 — WiiUBrew / GX2 et Latte

Source : [Hardware/GX2](https://wiiubrew.org/wiki/Hardware/GX2).

- **DOCUMENTED — résumé :** page communautaire décrivant le processeur graphique
  GX2, famille Radeon R7xx, adresses MMIO et références matérielles connexes.
- **INFERRED — exploitable :** lexique matériel, distinction GX/GX2, index de
  questions sur formats/registers/ISA, pas des réponses garanties.
- **INFERRED — risques :** supposer que Latte est un Radeon desktop identique ;
  importer des documents de provenance incertaine depuis des liens externes.
  Aucun SDK Cafe Nintendo ni document confidentiel n'a été recherché/importé.
- **DOCUMENTED — inspiration retenue :** plan de validation par petits cas
  synthétiques. Pas de tables de registres ni d'ISA recopiées.
- **UNKNOWN :** révision exacte de cette page, exhaustivité des MMIO, licence
  précise des documents externes ; pas d'exploitation normative en M0.

## S4 — wut

Source canonique : [devkitPro/wut](https://github.com/devkitPro/wut)
(ancien lien `decaf-emu/wut` redirigé),
[révision 0a740dc](https://github.com/devkitPro/wut/tree/0a740dccd2bc038395dacc48e701a3228f63c245),
[headers GX2](https://github.com/devkitPro/wut/tree/0a740dccd2bc038395dacc48e701a3228f63c245/include/gx2).

- **DOCUMENTED — résumé :** toolchain/SDK homebrew pour RPX/RPL ; licence zlib
  déclarée. Familles publiques `include/gx2`, `include/coreinit`. GX2 comprend
  notamment state, shaders, surface, texture, draw, display, mem et event.
- **INFERRED — exploitable :** inventaire initial des catégories d'API ; noms à
  confronter à des tests et à la version système ciblée avant implémentation.
- **INFERRED — risques :** confondre déclaration d'ABI console et contrat hôte ;
  alignements/endianness et attribution par fichier à vérifier avant adaptation.
- **DOCUMENTED — inspiration retenue :** structure de couverture par familles,
  pas inclusion de headers wut, de stubs de liens ou de SDK dans MANMENMI.
- **UNKNOWN :** effets matériels non explicités par prototypes.

## S5 — libwiiu

Source : [wiiudev/libwiiu](https://github.com/wiiudev/libwiiu),
[révision 0e67eb1](https://github.com/wiiudev/libwiiu/tree/0e67eb1854b63c4f12b06237fbe175a8e71e211b).

- **DOCUMENTED — résumé :** dépôt historique d'exécution homebrew et accès aux
  services Cafe depuis l'environnement navigateur/console ; pas un runtime PC.
- **INFERRED — exploitable :** contexte historique et familles de services.
- **UNKNOWN — licence :** pas de licence globale identifiable dans les éléments
  consultés ; aucune permission de réutilisation supposée.
- **INFERRED — risques :** code lié à firmware/exploit et hypothèses d'adresses
  propres à la console, sans intérêt pour le cœur natif générique.
- **DOCUMENTED — inspiration retenue :** terminologie uniquement, aucun code,
  aucun mécanisme d'exploitation intégré.

## S6 — Cemu

Source : [cemu-project/Cemu](https://github.com/cemu-project/Cemu),
[révision 3310f3b](https://github.com/cemu-project/Cemu/tree/3310f3b8b184d64a62b89fd59088c799432badf5).

- **DOCUMENTED — résumé :** émulateur Wii U C/C++, licence principale MPL-2.0.
  Son annonce de compatibilité est celle du projet, pas un résultat MANMENMI.
- **INFERRED — exploitable :** index de sous-systèmes, classes de cas limites à
  rechercher ensuite sans adopter les solutions.
- **INFERRED — risques :** émulation CPU/GPU, correctifs par jeu et obligations
  MPL au niveau fichier lors d'une reprise ; dépendances possiblement distinctes.
- **DOCUMENTED — inspiration retenue :** idées de tests et séparation des
  responsabilités seulement ; ni code, ni liaison, ni usage requis de Cemu.
- **UNKNOWN :** validité matérielle de chaque comportement interne.

## S7 — Decaf

Source : [decaf-emu/decaf-emu](https://github.com/decaf-emu/decaf-emu),
[révision e6c528a](https://github.com/decaf-emu/decaf-emu/tree/e6c528a20a41c34e0f9eb91dd3da40f119db2dee),
[licence](https://github.com/decaf-emu/decaf-emu/blob/e6c528a20a41c34e0f9eb91dd3da40f119db2dee/LICENSE.md).

- **DOCUMENTED — résumé :** projet de recherche en émulation Wii U ; GPL v3,
  texte consulté indiquant « or later ».
- **INFERRED — exploitable :** classement des services et problèmes de
  compatibilité pour organiser une future recherche indépendante.
- **INFERRED — risques :** importer une architecture d'émulation ou du code GPL
  dans un cœur annoncé MIT sans traitement légal approprié.
- **DOCUMENTED — inspiration retenue :** discipline de recherche uniquement,
  aucun code réutilisé ; même politique pour les snippets/différences détaillées.
- **UNKNOWN :** fonctionnalités reproductibles et précision des modèles.

## S8 — RebrewU

Source identifiée : [ApfelTeeSaft/RebrewU](https://github.com/ApfelTeeSaft/RebrewU),
[révision 0b40c4f](https://github.com/ApfelTeeSaft/RebrewU/tree/0b40c4f810fdb41727cb8d6c979380001e29533f).

- **DOCUMENTED — résumé :** se décrit comme framework de recompilation statique
  Wii U, RPX/RPL vers C++ structuré. Le qualificatif amont « production-grade »
  n'est **pas** repris comme validation.
- **INFERRED — exploitable :** frontière entre génération de code et services hôtes.
- **UNKNOWN — licence/maturité :** aucune licence globale exploitable établie ;
  aucun titre, build ou résultat de runtime vérifié par MANMENMI.
- **INFERRED — risques :** dépendre d'une ABI ad hoc ou d'annonces non mesurées.
- **DOCUMENTED — inspiration retenue :** questions sur le contrat d'un client
  externe neutre ; aucun code ou adaptateur spécifique intégré.

## S9 — nWiiURecomp

Source : [BlackLineInteractive/nWiiURecomp](https://github.com/BlackLineInteractive/nWiiURecomp),
[révision d911856](https://github.com/BlackLineInteractive/nWiiURecomp/tree/d9118564dcedc45facf5adea06729799bfbc6774).

- **DOCUMENTED — résumé :** se décrit comme boîte à outils de recompilation et
  runtime Wii U. Le README présente des jalons Wind Waker HD EU v0 et
  l'initialisation Cafe ABI ; il ne fournit pas de preuve MANMENMI.
- **INFERRED — exploitable :** nécessité de séparer analyse d'exécutable,
  génération de code, profils de client et runtime neutre.
- **UNKNOWN — licence :** consultation de page évoquant GPL-3.0 mais métadonnée
  GitHub `NOASSERTION/Other` à cette révision ; texte non clarifié. Ne pas trancher
  arbitrairement : réutilisation interdite dans ce travail jusqu'à clarification.
- **INFERRED — risques :** contamination par profils/adresses/boot d'un titre et
  par conventions de recompileur supposées universelles.
- **DOCUMENTED — inspiration retenue :** séparation des profils hors core,
  aucun code repris et aucun support de ce client annoncé.

## S10 — WiiCompiled / Wiicompiled

Source résolue : [TimosCodd/Wiicompiled](https://github.com/TimosCodd/Wiicompiled),
[révision 8d5d93f](https://github.com/TimosCodd/Wiicompiled/tree/8d5d93f02d9806b0f0bbd03b154f3b14eea32292).

- **DOCUMENTED — résumé :** port natif par recompilation de Mario Kart **Wii**,
  utilisant Aurora pour le rendu ; GPL v3 déclarée.
- **INFERRED — exploitable :** séparation application / couche graphique / données
  utilisateur, mais la réussite de ce port ne préjuge pas de GX2.
- **INFERRED — risques :** assimiler Wii à Wii U, récupérer code/profils propres
  au jeu ou mélanger GPL et code original MIT sans démarche explicite.
- **DOCUMENTED — inspiration retenue :** ergonomie développeur et architecture
  d'un consommateur, pas code, assets, logique jeu ou promesse de compatibilité.
- **UNKNOWN :** compatibilité avec un futur MANMENMI.

## S11 — Skate 3 Recomp

Source choisie : [mchughalex/skate3recomp](https://github.com/mchughalex/skate3recomp),
[révision f6e0ae8](https://github.com/mchughalex/skate3recomp/tree/f6e0ae87fdfecbadb5c1e36c55d66a744187a3cd).
D'autres forks existent ; cette fiche ne les englobe pas.

- **DOCUMENTED — résumé :** recompilation native de Skate 3 **Xbox 360** ; README
  citant ReXGlue/fork spécifique et rendu DX12/Vulkan. Les données de jeu sont
  requises par ce projet amont, **pas par MANMENMI**.
- **INFERRED — exploitable :** jalons mesurables et distinction code généré /
  services natifs / renderer.
- **UNKNOWN — licence :** métadonnée de licence absente ; pas de permission
  présumée. Les licences ReXGlue et transitives nécessiteraient une étude propre.
- **INFERRED — risques :** importer une architecture Xbox 360/Xenos pour Latte,
  des hooks de titre et des affirmations de performances non reproduites.
- **DOCUMENTED — inspiration retenue :** discipline de validation et architecture
  de client externe uniquement ; aucune mesure amont revendiquée localement.

## S4-M8 — GX2 surface subset extraction

Rechecked on **2026-09-19 UTC**, without importing code or headers into the
build. Source: the existing pinned S4 revision,
[enum.h](https://raw.githubusercontent.com/devkitPro/wut/0a740dccd2bc038395dacc48e701a3228f63c245/include/gx2/enum.h)
and [surface.h](https://raw.githubusercontent.com/devkitPro/wut/0a740dccd2bc038395dacc48e701a3228f63c245/include/gx2/surface.h).

- **DOCUMENTED:** `GX2Surface` declares `width`, `height`, `format`, `use`,
  `tileMode`, `swizzle`, `alignment`, and `pitch`; it is a surface ABI record,
  not the MANMENMI public object.
- **DOCUMENTED:** `GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8` is `0x01a`;
  `GX2_SURFACE_USE_COLOR_BUFFER` is `1 << 1`; and
  `GX2_TILE_MODE_LINEAR_ALIGNED` is `1` in the pinned enum declaration.
- **INFERRED:** mapping the format's documented component/type spelling to
  MANMENMI `rgba8_unorm`, and the color-buffer use to `render_target`, is a
  small host semantic projection. It is not a copied GX2 ABI or a byte-layout
  claim.
- **OBSERVED:** MANMENMI M8 rejects every other represented enum value and
  validates the resulting DX12 render target with a real draw/readback test.
- **UNKNOWN:** data byte order, pitch and alignment derivation, swizzle,
  `LINEAR_ALIGNED` applicability beyond this metadata-only subset, all tiled
  modes, and CPU/GPU memory behavior. No such behavior is implemented.

## Règle transversale

**DOCUMENTED — politique MANMENMI :** conserver les faits publics sous forme de
notes attribuées, isoler les hypothèses et ne pas consulter de SDK divulgué.
**UNKNOWN :** validité juridique globale d'une future réimplémentation dans
toutes les juridictions. « Inspiration sans code » réduit certains risques,
mais ne constitue pas une certification clean-room ou un avis juridique.
