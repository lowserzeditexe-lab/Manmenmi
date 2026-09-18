# Graphics — contrat M0 uniquement

API publique : `include/manmenmi/graphics/graphics.hpp`. Interfaces abstraites
et noms de ressources réservés, sans provider ni ressource factice.
Les contrats device/queue/command buffer/buffer/texture/shader/pipeline/
framebuffer/sync/swapchain complets appartiennent à M2. Vulkan n'impose pas l'IR.