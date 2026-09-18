# Politique de dépendances

**DOCUMENTED — décisions utilisateur :** pragmatique, limitée, remplaçable,
licences documentées, versions exactes, aucune dépendance propriétaire obligatoire.

## M0

**Zéro bibliothèque applicative tierce** : C++ standard et threads système
suffisent. Pas de FetchContent, submodule, git clone ou requête réseau depuis
la configuration/compilation CMake et les CTests ordinaires du projet. SDL3,
Vulkan, DX12, GL, wut, GoogleTest, JSON/TOML frameworks ne sont
pas nécessaires aux fonctions livrées. Format de config volontairement minimal.

Les outils CI et leurs sources sont dans [ci.yml](../.github/workflows/ci.yml)
et les [notices](../THIRD_PARTY_NOTICES.md). Versions exactes et SHA-256 vérifiés
avant extraction par le bootstrap CI **explicite et séparé**. Le verrou est dans
`ci/toolchain-lock.json`, la procédure dans [TOOLCHAIN](build/TOOLCHAIN.md).
Bibliothèques système de l'image non entièrement épinglées. Ce niveau
de reproductibilité est annoncé explicitement, pas présenté comme hermétique.
Le socle web préexistant de l'environnement a été sorti du dépôt source natif ;
il ne fait pas partie du graphe de construction MANMENMI.

## Toute proposition future doit préciser

1. Besoin mesurable et pourquoi la bibliothèque standard ne suffit pas.
2. Version exacte / commit immutable, URL d'origine, hash prévalidé de l'archive
   si téléchargée automatiquement, mécanisme hors ligne.
3. Licence, notices par fichier/composant, dépendances transitives et redistribution.
4. Cible CMake privée autant que possible ; aucune fuite de types dans API publique.
5. Coût de build/runtime, alternative spécialisée et voie raisonnable de remplacement.
6. Tests de construction/usage par plateforme et preuve des capacités annoncées.

Si CMake récupère une dépendance à l'avenir, prévoir une activation explicite,
pin/hash d'archive et origine documentée. Jamais `latest`, HEAD flottant ou tag
non résolu seul comme politique de dépendance source.

## SDL3 / GoogleTest

SDL3 sera étudié en M1 pour fenêtres/événements, pas utilisé comme modèle de l'API
graphique. GoogleTest est une option M2+ si le gain pour fixtures/parité justifie
la dépendance ; les labels et suites CTest restent la frontière d'exécution.