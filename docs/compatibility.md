# Compatibilité — état M3 et limites Wii U

Date de la première mesure : 2026-09-18. Les backends ne sont pas implémentés :
un test du parser `--backend` **n'est pas un test graphique**.

| Fonction | Vulkan | DX12 | OpenGL 4.6 |
|---|---|---|---|
| Nom / configuration / plan de sélection | OBSERVED (tests de fondation) | OBSERVED (tests de fondation) | OBSERVED (tests de fondation) |
| Adaptateur natif | MANMENMI non validé | DX12 validé localement en M1.3 | MANMENMI non implémenté |
| Fenêtre/device/swapchain/present | NON VALIDÉ | PASS local M1.3 | NON IMPLÉMENTÉ |
| Buffers / CommandBuffer / CopyBuffer | NON VALIDÉ | PASS M2.2 local | NON IMPLÉMENTÉ |
| Triangle / pipeline minimal | NON VALIDÉ | PASS M3 local | NON IMPLÉMENTÉ |
| GX2 minimal triangle bridge | NON VALIDÉ | PASS M4 local | NON IMPLÉMENTÉ |
| Private Shader IR / DX12 translation | NON VALIDÉ | PASS M6 local | NON IMPLÉMENTÉ |
| Texture RGBA8 / offscreen render target | NON VALIDÉ | PASS M7 local | NON IMPLÉMENTÉ |
| GX2 R8_G8_B8_A8 UNORM color-target descriptor mapping | NON VALIDÉ | PASS M8 local (descriptor bridge only) | NON IMPLÉMENTÉ |
| Ressources GX2 | PLANNED M4+ | PLANNED M4+ | PLANNED M4+ |
| Parité / golden images | PLANNED, aucune mesure | PLANNED, aucune mesure | PLANNED, aucune mesure |
| Rapport GPU/pilote/capacités | Runtime système observé, backend Mamenmi non validé | Intel UHD Graphics observé localement | Runtime système présent, backend Mamenmi absent |

M8 utilise les noms et valeurs documentés WUT pour `UNORM_R8_G8_B8_A8`,
`COLOR_BUFFER` et `LINEAR_ALIGNED`. La projection sémantique vers RGBA8 est
INFERRED, et le mode linear-aligned est uniquement métadonnée: ordre des octets,
pitch, alignement, tiling, swizzle et import de mémoire console restent UNKNOWN.

La validation DX12 locale couvre Window, SurfaceToken, Device, Queue, Swapchain,
Acquire, Submit, Present, synchronisation GPU, Resize/Recreate, le deuxième
frame, les buffers, CopyBuffer et le triangle minimal. Le test M3 vérifie aussi
des pixels non-clear dans le render target avant Present. Elle ne constitue pas
une preuve de compatibilité Wii U, GX2 ou cross-backend.

M1.4 ne change pas la baseline. M2.1/M2.2/M2.3 et M3 sont validés localement
uniquement sur DX12 ; Texture, Resource, Shader public, Sync public et parité
cross-backend restent hors périmètre. M4 ajoute seulement un bridge GX2
synthétique validé sur DX12 ; M6 ajoute un micro-sous-ensemble Shader IR privé
traduit vers HLSL/DX12. M7 ajoute une texture couleur RGBA8 hors écran validée
par readback. Aucun de ces incréments ne constitue une compatibilité GX2 console.

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
