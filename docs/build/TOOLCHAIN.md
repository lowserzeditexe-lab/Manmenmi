# Toolchain CI M0 — versions et empreintes verrouillées

**DOCUMENTED :** source de vérité exécutable : [`ci/toolchain-lock.json`](../../ci/toolchain-lock.json).
Chaque entrée contient version exacte, URL HTTPS, SHA-256 complet, format,
raison d'utilisation et provenance. Aucune résolution `latest`, HEAD, pip ou apt
n'a lieu dans les nouveaux workflows. Aucun secret/token à fournir ou stocker.

## Archives de construction

Les empreintes ont été **mesurées sur les octets téléchargés**, pas déduites du
nom de fichier. CMake et libtinfo ont en plus été comparés au manifeste SHA-256
publié par leurs distributeurs. Les signatures cryptographiques amont n'ont
pas été validées : ce verrouillage garantit la stabilité du contenu approuvé,
pas une certification de toute la chaîne d'approvisionnement.

| Outil / plateforme | Version exacte | SHA-256 |
|---|---|---|
| LLVM Linux x64 | 18.1.8 | `54ec30358afcc9fb8aa74307db3046f5187f9fb89fb37064cdde906e062ebf36` |
| LLVM-MinGW Windows x64 UCRT | 20240619 / Clang 18.1.8 | `810703594a7e3eea03385b5329c7ea3bd65f5e496b44cf1b68c17ff436d265e7` |
| CMake/CTest Linux x64 | 3.31.6 | `5a1133ff103c71eb5120e2cc3de922733e7d8a26a98ae716397e8676adb367bf` |
| CMake/CTest Windows x64 | 3.31.6 | `d163cd3ab4959b0a53fa8988f2ddbd2e6c501658201e6a154386bad9dbe4f836` |
| Ninja Linux x64 | 1.11.1 | `b901ba96e486dce377f9a070ed4ef3f79deb45f4ffe2938f8e7ddc69cfb3df77` |
| Ninja Windows x64 | 1.11.1 | `524b344a1a9a55005eaf868d991e090ab8ce07fa109f1820d40e74642e289abc` |
| libtinfo5 Linux amd64 | 6.3-2ubuntu0.3 | `4df4288404108f1a156d014e8764a064e977e34e6d44931ab60451694c03c90d` |
| actionlint Linux x64 | 1.7.7 | `023070a287cd8cccd71515fedc843f1985bf96c436b7effaecce67290e7e0757` |

URLs exactes et justifications, sans raccourci ambigu : voir chaque entrée du
[lock](../../ci/toolchain-lock.json). Sources des manifestes supplémentaires :
[Kitware](https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-SHA-256.txt),
[Ubuntu libtinfo5](https://packages.ubuntu.com/jammy/amd64/libtinfo5/download).

### Actions GitHub

`checkout v4.2.2` et `upload-artifact v4.6.2` sont épinglées aux commits complets
dans le workflow et le lock. Le lock documente aussi le SHA-256 de leurs archives
sources codeload à ces commits. **Distinction :** GitHub charge ces actions par
commit Git ; il n'exécute pas notre vérification SHA-256 sur son propre chargeur.
Les SHA codeload servent à l'audit de provenance, pas à une garantie imaginaire
d'exécution. L'ancienne action setup-python a été supprimée.

## Vérification avant utilisation

`ci/Bootstrap.cmake` s'exécute avec le CMake de l'image hôte (minimum 3.25) :

1. Refus d'une installation CI sur un hôte qui n'est pas nativement du bon OS x64.
2. Lecture du lock v1, URL HTTPS et SHA-256 de 64 caractères obligatoires.
3. Téléchargement exact avec vérification TLS et `EXPECTED_HASH`.
4. Vérification également des archives **déjà en cache**, avant toute extraction.
5. Divergence → erreur fatale ; aucun remplacement silencieux, aucun outil exécuté.
6. Extraction privée dans le dossier temporaire du runner. libtinfo5 n'est pas
   installé globalement : `dpkg-deb -x`, puis `LD_LIBRARY_PATH` privé.
7. CMake/CTest, Ninja, Clang, runtimes et actionlint proviennent ensuite du lock.
   Versions réellement exécutées contrôlées par `ci/VerifyTools.cmake`.

`VERIFY_ONLY=ON` ne fait que vérifier les archives, sans extraire ni exécuter les
outils ; cette option permet les contrôles locaux sur ARM64 sans prétendre à une CI native.
Le build CMake normal du projet reste entièrement hors ligne et indépendant de
ce bootstrap explicite de CI.

## Limites : PINNED n'est pas hermétique

Le bootstrap repose sur l'image GitHub (`ubuntu-22.04`, `windows-2022`), son
CMake initial, shell, Git, TLS/CA, `dpkg-deb` Linux et chargeur Node des actions.
Ces composants préinstallés ne sont **pas** des archives téléchargées par nos
scripts et ne sont **pas** figés par ce lock. Sous Linux, glibc, zlib, libgcc,
libstdc++/headers et outils système de liaison viennent de l'image ; sous Windows,
UCRT/OS viennent de Windows et libc++/winpthreads/LLD du paquet LLVM-MinGW.
La CI enregistre `ImageOS`, `ImageVersion`, versions exécutées et inventaire des
paquets Linux. Aucune dépendance de ce périmètre ne doit être présentée comme
entièrement épinglée ou hermétique.

### Définition de la reproductibilité mesurée

Deux configurations propres dans **deux répertoires distincts**, mêmes sources,
version de compilateur, outils, environnement et Debug/Release. Les six sorties
(librairie statique, CLI et quatre exécutables unitaires) doivent avoir les mêmes
SHA-256. Les chemins debug/source sont normalisés et le timestamp PE variable
est désactivé. Logs, cache CMake, JUnit et timings ne sont pas des sorties comparées.

`ci/CheckReproducibility.cmake` échoue à la moindre différence ou si le même
répertoire est fourni deux fois. **Cela ne prouve ni égalité inter-OS, ni égalité
entre images système différentes.** La validation native reste PENDING ; le
contrat de fermeture M0 est détaillé dans [M0.md](../validation/M0.md).

## Procédure de mise à jour

1. Choisir une **version précise**, identifier sa publication officielle et sa licence.
2. Télécharger chaque archive requise en zone temporaire, calculer SHA-256 ;
   comparer aux empreintes/signatures amont lorsqu'elles sont disponibles.
3. Ne jamais remplacer automatiquement une empreinte qui ne correspond plus.
   Déterminer d'abord la cause (artefact remplacé, erreur de version, compromission).
4. Mettre à jour ensemble lock, vérifications de version, workflow si nécessaire,
   ce tableau et THIRD_PARTY_NOTICES. Consigner la raison dans la modification.
5. Contrôler cache corrompu, téléchargement altéré, extraction, garde de versions,
   actionlint, 26 CTests, artefacts et deux builds propres.
6. Rejouer les matrices CI natives normales et un run injectant un échec. Conserver
   commits, URL de runs, images et manifests d'artefacts. Tant que ces étapes ne
   sont pas faites, ne pas publier « reproductibilité native VERIFIED ».