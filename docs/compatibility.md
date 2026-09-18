# Compatibilité — aucune promesse de runtime Wii U en M0

Date de la première mesure : 2026-09-18. Les backends ne sont pas implémentés :
un test du parser `--backend` **n'est pas un test graphique**.

| Fonction | Vulkan | DX12 | OpenGL 4.6 |
|---|---|---|---|
| Nom / configuration / plan de sélection | OBSERVED (tests de fondation) | OBSERVED (tests de fondation) | OBSERVED (tests de fondation) |
| Adaptateur natif | UNIMPLEMENTED | UNIMPLEMENTED | UNIMPLEMENTED |
| Fenêtre/device/swapchain/present | UNIMPLEMENTED | UNIMPLEMENTED | UNIMPLEMENTED |
| Triangle / buffers / shaders minimaux | PLANNED M2/M3 | PLANNED M2/M3 | PLANNED M2/M3 |
| Ressources GX2 | PLANNED M4+ | PLANNED M4+ | PLANNED M4+ |
| Parité / golden images | PLANNED, aucune mesure | PLANNED, aucune mesure | PLANNED, aucune mesure |
| Rapport GPU/pilote/capacités | UNKNOWN, jamais interrogé | UNKNOWN, jamais interrogé | UNKNOWN, jamais interrogé |

## Plateformes

| Environnement | État réel |
|---|---|
| Linux x64 natif | Cible ; CI définie, validation native PENDING faute de repository distant |
| Linux x64 cross + QEMU sur ARM64 | OBSERVED : compilation Clang et CTests ; voir preuves |
| Windows x64 natif | Cible ; CI LLVM-MinGW définie, tests natifs PENDING faute de repository distant |
| Linux ARM64 | Hôte de vérification du socle avec dérogation explicite, **pas une plateforme supportée** |
| macOS | Hors périmètre ; configuration refusée |
| 32-bit | Hors périmètre ; configuration refusée |

DX12 sera une cible **Windows** ; aucune voie DX12 Linux promise. OpenGL 4.6
exigera un contexte/pilote adéquat. Vulkan 1.x reste un objectif de famille API :
version minimale et extensions exactes **UNKNOWN**, à choisir en M1 sur preuves.

## Interprétation future des résultats

- **PASS :** cas identifié exécuté sur le backend/OS/GPU/pilote indiqués.
- **PARTIAL :** sous-cas explicites seulement ; divergences et restrictions listées.
- **FAIL :** cas implémenté/exécuté dont le résultat ne respecte pas l'attendu.
- **UNIMPLEMENTED :** pas de chemin de code ; ne pas appeler cela PASS/SKIP de parité.
- **UNKNOWN :** non mesuré. **PLANNED :** backlog seulement.

La [validation M0](validation/M0.md) suit séparément build/CTest/CI et ne prouve
aucun comportement de matériel Wii U, aucun boot ou first frame.