# Couverture GX2 — M0

**0 appel GX2 implémenté. 0 appel GX2 testé sur un backend.**
Les noms de familles proviennent de la recherche documentaire wut [S4](../research/SOURCES.md).
La liste ci-dessous est un plan de travail, pas une liste exhaustive des exports
GX2 ni une promesse d'ABI. **Dénominateur d'API exhaustif : UNKNOWN.**

| Famille future | Implémentation | Vulkan | DX12 | OpenGL | Milestone |
|---|---|---|---|---|---|
| Initialisation/device/état | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M4 |
| Mémoire/buffers | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M4/M5 |
| Shaders/pipeline | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M4/M6 |
| Textures/surfaces/formats | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M4/M7 |
| Draw/framebuffers | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M4/M5 |
| Sync/events/visibilité mémoire | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M5 |
| Tiling/swizzle/mips | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M7 |
| Depth/stencil/MRT/MSAA/resolve | UNIMPLEMENTED | UNKNOWN | UNKNOWN | UNKNOWN | M7 |

**DOCUMENTED — règle :** toute ligne future par appel devra préciser provenance
de signature, sous-ensemble, données synthétiques, test, backend, OS, pilote/GPU,
résultat PASS/PARTIAL/FAIL et limites. Une assertion de parsing ou un mock ne
remplace pas une observation GPU. Ne jamais cocher OpenGL à partir d'un test Vulkan.

**PLANNED :** sandbox neutre et petits tests réutilisant exclusivement la même
API portable. Pas de profils Splatoon/Woody, pas de hacks backend dans `gx2/`.