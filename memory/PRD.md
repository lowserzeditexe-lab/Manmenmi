# MANMENMI — PRD et état de reprise

## Demande originale

« Plateforme native de compatibilité Wii U pour PC, générique, multi-backend,
destinée aux projets de recompilation et de reverse engineering — sans émulation
PowerPC, sans dépendance à Splatoon, Woody, Cemu ou Decaf. »

App native PC, pas web/mobile. C++20, CMake/Ninja, Clang, Windows x64 + Linux x64.
Runtime Cafe OS sélectionné, GX2 progressif, API graphique unique et backends
DX12/Vulkan/OpenGL 4.6 interchangeables **à terme**. Aucun dump requis au build,
aucun fichier propriétaire Nintendo dans Git. Conclusion de recherche toujours
DOCUMENTED/OBSERVED/INFERRED/UNKNOWN ; fonction annoncée testée sur son backend.

Flow futur : compiler → test `--backend=vulkan|dx12|opengl|auto` → fenêtre/device/
swapchain → triangle/buffers/shaders communs → client neutre/GX2 → séparément
outil inspect local vers Markdown/JSON. Vulkan référence de correction, API
non Vulkan-centric. Pas de fallback silencieux ni succès sur API non implémentée.
Pas macOS avant stabilité Windows/Linux ; pas de logique jeu dans le core.

Deliverables immédiats : README strict, INITIAL_STATE, architecture/diagrammes/
interfaces, compatibility, GX2 api-coverage, THIRD_PARTY_NOTICES, CMake/CI/test.
Recherche : Aurora, WiiUBrew, wut/libwiiu, Cafe OS/GX2/Latte, Cemu/Decaf, RebrewU,
nWiiURecomp, WiiCompiled, Skate 3 Recomp ; résumé, éléments exploitables, risques
architecturaux/légaux, inspiration sans code pour chaque source.

## Choix définitifs utilisateur

- MIT pour le code original ; licences tierces distinctes et notices.
- Dépendances pragmatiques mais limitées/remplaçables, versions exactes, origine
  et construction documentées, aucune dépendance propriétaire obligatoire.
- CTest + exécutables autonomes M0 ; GoogleTest facultatif M2+ si justifié.
- SDL3 derrière Manmenmi Window API ; ne pas intégrer prématurément en M0,
  aucun type SDL public. Fenêtre et graphics découplés.
- **Ne pas commencer GX2, Cafe OS, shaders ou backends pour remplir le dépôt.**
- M0 = recherche, architecture, repository/build/CI, logs/erreurs/config/tests,
  interfaces fondamentales. Toute absence PLANNED/UNIMPLEMENTED/UNKNOWN.
- **Ne pas commencer M1 avant builds/tests/CI natifs Windows/Linux réellement passés.**

## Architecture retenue / implémentée

Projet natif racine `/app`, aucune web app. Ancien scaffold déplacé dans
`/root/manmenmi-inherited-template`; .env d'environnement conservés hors build.
`manmenmi::core` bibliothèque statique + stdlib/Threads, zéro dépendance
applicative. `::window` et `::graphics` cibles INTERFACE. Sept headers publics
autonomes, aucune fuite de types SDL/Vulkan/DX12/GL. Aucun ABI binaire figé.

Core : Result<T>/Error/Status, logger sink ostream mutex niveaux/catégories,
config key=value schema v1 stricte (64KiB, rejet doublons/inconnus/NUL), CLI
priorité defaults<file<CLI, politique explicite de candidats. Runtime abstrait ;
create_runtime retourne toujours UNIMPLEMENTED en M0, logs des tentatives/replis.

CLI `manmenmi_probe` : help/version/check-config/list-backends diagnostics=0,
probe défaut ou demandé=3, entrée invalide=2, I/O=4, invariant interne=70.
auto seul → Vulkan uniquement. Fallback uniquement si config/CLI explicite,
journalisé même avec log-level=error. Aucun device réellement sondé.

CMake≥3.25, Ninja, Clang14.0.6 local ; CI LLVM18.1.8 Linux, Windows LLVM-MinGW
20240619 UCRT/Clang18.1.8 sans SDK propriétaire obligatoire. CMake3.31.6 et
Ninja1.11.1 archives officielles ; aucun Python/pip/setup-python en CI désormais.
Actions checkout/upload-artifact SHA immutables. Les huit archives du bootstrap
(LLVM×2, CMake×2, Ninja×2, libtinfo5 6.3-2ubuntu0.3, actionlint1.7.7) sont verrouillées
par version/URL/SHA256 avec contrôle obligatoire avant extraction/cache.
Source : ci/toolchain-lock.json. Les images et composants bootstrap/système
restent non hermétiques, limites et mise à jour dans docs/build/TOOLCHAIN.md.

## Livré

- Build et presets Debug/Release, coverage et ASan/UBSan Linux.
- Workflow six jobs de matrice avec rapports JUnit/log/cache/coverage.
- 26 CTests : unit/core/config/log/contracts +21 CLI +1 garde architecture.
  Checks actifs Release, exits CLI exacts, pas WILL_FAIL trompeur.
- 11 fiches de sources avec liens/révisions et risques ; ambiguïtés de licence
  libwiiu/RebrewU/nWiiURecomp/Skate3 restent UNKNOWN. Aucun code source tiers copié.
- Toutes les docs demandées, MIT, conventions/contribution/dépendances.
- Arborescence finale réservée par README PLANNED ; zéro backend/GX2/Cafe,
  shader translator, sandbox, inspect ou service timing/memory invité implémenté.

## Résultats de vérification — 2026-09-18

Hôte Linux ARM64 Debian12, Clang/LLVM14.0.6 CMake3.25.1 Ninja1.11.1 GCC12.2.
Dérogation MANMENMI_ALLOW_UNSUPPORTED_HOST=ON uniquement pour les fondations.

- ARM64 Debug/Release/coverage/ASan+UBSan : 26/26 chacun, rebuilt après derniers changements.
- Linux x64 cross Clang14 + GCC12, exécution qemu-x86_64 7.2 : 26/26 Debug/Release.
- Windows x64 cross LLVM-MinGW18.1.8 Debug/Release : compilé, PE AMD64 vérifié.
  **Aucun test exécuté sous Windows.**
- LLVM coverage propre : 96.25% lignes,94.50% branches, aucun diagnostic.
  Défaut initial hash-null inline error_name/success éliminé avec result.cpp,
  pas de filtre masquant les avertissements. Coverage.cmake échoue sur diagnostic.
- Testing agent `/app/test_reports/iteration_1.json` confirme ces résultats,
  aucun défaut produit ; ne modifie que son rapport (git diff vide).
- Après ses réserves : actionlint1.7.7 installé, workflow valide exit0.
  i686 LLVM-MinGW complète confirme rejet CMake «requires a 64-bit toolchain».
- **CI native Linux/Windows non exécutée : UNKNOWN, M0 gate OPEN, M1 bloqué.**

Preuves et protocoles : docs/validation/M0.md + m0-status.json ; artefacts locaux
sous build/*/results.xml, Testing/Temporary/LastTest.log, coverage.txt. Build
artefacts ignorés Git, pas d'URL de CI inventée. Aucune auth/aucun credential.

## Backlog priorisé

### P0 — avant toute suite
1. Exécuter vraie CI Linux x64 et Windows x64 sur le dépôt du projet.
2. Collecter commit, run URL, artefacts des six configurations.
3. Corriger tout échec ; consigner preuves avant de fermer M0.
4. Ne pas substituer cross build/QEMU/actionlint à ces preuves natives.

### P1 — ensuite seulement
1. Hashes des archives CI verrouillés et vérifiés dans la deuxième itération.
   Un environnement système plus hermétique reste une évolution distincte.
2. M1 : recherche/pin SDL3, pont privé fenêtre-présentation, création/destroy/
   resize/device/present, tests par backend présent localement, sélection explicite.
3. M2 : contrats complets device/queue/command buffer/buffer/texture/shader/
   pipeline/framebuffer/sync/swapchain, IR minimale, erreurs, ressources réelles.
4. M3 : triangle commun, parité de scène/paramètres, capacités, golden si stables.

### P2 — roadmap à ne pas anticiper
M4 GX2 minimal + sandbox ; M5 tests/parité GX2 ; M6 parser/IR shaders/sous-ensemble
traduit/cache ; M7 formats/layout/tiling/depth/stencil/MRT/MSAA selon preuves ;
M8 coreinit/filesystem/timing/threading/input/audio ciblés avec trace unimplemented ;
M9 client externe neutre ; M10 inspect séparé, dump local facultatif, Markdown/JSON ;
M11+ Woody puis Splatoon hors core après fondations validées. Jamais full shader
support ni compatibilité large proclamés sans mesure.

## Prochaine tâche

Statut actuel : **M0 — READY FOR REMOTE CI VALIDATION**.
Compléter les preuves natives M0 quand l'utilisateur aura créé/connecté son dépôt.
Même après M0 COMPLETE, ne commencer M1 que sur nouvelle instruction explicite.
Ne pas construire une UI web ou ajouter de dépendances graphiques à cette reprise.

## Deuxième demande / choix définitif (2026-09-18)

L'utilisateur a demandé la fermeture M0 avec preuves natives, tests/artefacts/
propagation d'échec, verrou SHA256 et audit final. Puis a précisé **aucun dépôt
GitHub encore créé** : conserver le repo local, préparer les workflows, vérifier
localement, indiquer CI native **PENDING faute de repository distant**, ne demander
ou ajouter aucun secret/token. Arrêt demandé au statut READY FOR REMOTE CI VALIDATION.

### Ajouts de cette itération

- ci/Bootstrap.cmake : archives du lock vérifiées avant extraction, cache corrompu
  refusé, mode VERIFY_ONLY sans installation, garde hôte natif x64.
- CMake bootstrap préinstallé minimum3.25 puis CMake/CTest verrouillé3.31.6 ;
  libtinfo5 extrait localement avec dpkg-deb, pas apt/sudo. Versions vérifiées.
- ci/RunTests.cmake : inventaire strict26 et erreur CTest propagée.
- MANMENMI_CI_INJECT_FAILURE opt-in : 27e sentinelle pour runs négatifs ; défautOFF.
- Contrôle isolé de propagation + inputworkflow inject_failure pour vraie preuve
  distante rouge ultérieure (ne pas confondre les deux).
- Artefacts : magic/arch ELF/PE, six sorties, JUnit26 sans skip/fail,
  manifestSHA256, relecture CHECK_MANIFEST sans écrasement.
- Reproductibilité : deux répertoires propres distincts, six artefacts comparés,
  source/debug prefix maps et timestampPE désactivé, pas de promesse inter-image.
- Orchestration commune BuildAndVerify, auditborné source avec inventaireSha,
  enregistrement image/versions/commit et upload même en cas d'échec.
- TOOLCHAIN.md, REMOTE_CI.md, M0.md avec vocabulaire six statuts +PENDING,
  README explicitement «aucune compatibilité Wii U fonctionnelle» et statut READY.

### Preuves indépendantes

/app/test_reports/iteration_2.json : aucun bug critique/mineur ; seul blocage
restant = absence de repo/runs natifs. Ne pas transformer son retest_needed
(preuve distante à venir) en CI locale prétendument native.

- 26/26 ARM Debug/Release/Coverage/ASan+UBSan, 26/26 Linuxx64 cross Debug/Release QEMU.
- 8 empreintes de cache confirmées, manifesteCMake comparé ; modesVERIFY_ONLY,
  hôteARM refusé, mauvaiseSHA/lock refusés sans extraction.
- Suiteinjectée26PASS+1FAIL CTestexit8, wrappernonzero ; nombreincorrect/aucuntestrejetés.
- Artefacts manquants/tronqués/archincorrecte/altérés, manifestsaltérés et
  JUnitmalformé/incomplet/fail/skip rejetés (9contrôlesnégatifs).
- Reproindépendante ARM Debug/Release et Linuxx64crossDebug : 6SHAidentiques ;
  mêmesrépertoires et artefactmuté rejetés. Autrescomparaisonsnatives PENDING.
- actionlint1.7.7 et auditborné PASS, pas decontinue-on-error oufauxWILL_FAIL.
- Tous les caches debuildnormaux ont INJECT_FAILURE=OFF après tests.
- L'agent de test n'a changé que son rapport, pas les sources.

### Arrêt et suite autorisée

M0 non COMPLETE. Aucun M1. Aucun secret. Attendre le dépôt distant puis effectuer
les six jobs natifs normaux + runrouge au mêmecommit, télécharger/vérifier les
artefacts, consigner URL/commit/IDs/image. La reproductibilité VERIFIED sera toujours
qualifiée par son périmètre (deuxbuilds mêmesoutils/environnement), pas hermétique.