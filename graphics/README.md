# Graphics — validation M3 DX12

API publique : `include/manmenmi/graphics/graphics.hpp`.
Le contrat reste neutral : aucune dépendance native n'est exposée dans les headers publics.

Le socle M1.2 apporte :
- sélection explicite ou automatique de backend via `BackendSelection` ;
- service de création de `Device` via `GraphicsFactory` ;
- détection privée de disponibilité Vulkan / DX12 / OpenGL ;
- propagation d'erreurs réelles (`unsupported_backend`, `unavailable`, `device_creation_failed`) ;
- les ressources et shaders restent opaques ; M6 fournit une IR shader privée
	partielle et aucun type de shader natif dans l'API publique.

M1.3 ajoute et valide localement, dans l'implémentation privée, le cycle DX12
Device → Queue → Swapchain → Acquire → Submit → Present → synchronisation →
Resize/Recreate. Cette validation ne crée aucune API publique de ressource ou
commande et ne constitue pas une compatibilité Wii U.

M2.1/M2.2/M2.3 ajoutent et valident localement Buffer, CommandBuffer, CopyBuffer
et une IR privée de transfert. M3 ajoute un Pipeline opaque minimal, les commandes
de render pass/draw et un triangle DX12 réel. Le test M3 vérifie le contenu du
render target par readback privé avant Present.

M6 remplace les shaders codés en dur du triangle par une micro-IR privée,
traduite en HLSL puis compilée par DX12. Le parser GX2 réel et la traduction
Latte restent hors périmètre.

M7 ajoute une texture couleur `rgba8_unorm` hors écran comme render target unique,
avec RTV DX12 privé et readback de validation. Les formats GX2, tiling, swizzle,
depth, MSAA et MRT ne sont pas implémentés.

M8 ajoute `TextureLayout::opaque_render_target`: il décrit une texture dont le
layout physique reste privé au backend. La conversion GX2 accepte un seul
descripteur couleur R8/G8/B8/A8 UNORM; `LINEAR_ALIGNED` est validé comme
métadonnée descriptive uniquement. Aucun upload ou décodage de mémoire GX2,
pitch, alignement, tiling ou swizzle n'est effectué. Le chemin DX12 réel reste
M6 shader -> render target -> readback; Vulkan et OpenGL restent non validés.

Vulkan et OpenGL ne sont pas validés pour M2/M3. Aucun type natif n'est exposé.

Le support des backends reste conditionné à leur disponibilité réelle dans
l'environnement et le compilateur.
L'implémentation privée se trouve dans `graphics/src/graphics.cpp`.
