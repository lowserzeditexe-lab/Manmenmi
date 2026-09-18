# Protocole distant M0 — PENDING, aucun dépôt connecté

**M1 reste interdit**, même après validation : attendre l'instruction utilisateur.
Pas de secret/token dans le projet. Les actions utilisent seulement les capacités
standards du runner, avec permissions minimales et `persist-credentials: false`.

## Run normal, une fois le dépôt disponible

Lancer `M0 native foundation` au commit à valider, `inject_failure=false`.
Six jobs natifs : Linux x64 Debug/Release/coverage/sanitizers et Windows x64
Debug/Release. Chaque job doit :

1. Valider SHA-256 avant extraction et versions réellement exécutées.
2. Enregistrer image/compilateur/commit ; refuser l'hôte non x64.
3. Exécuter les contrôles d'hygiène et de propagation d'échec isolée.
4. Compiler puis exécuter **exactement 26 tests, aucun ignoré**.
5. Vérifier formats ELF64 AMD64 ou PE AMD64, six binaires/librairie non vides,
   JUnit sans échec, puis créer `artifact-manifest.sha256`.
6. En Debug/Release, construire une seconde fois dans un répertoire propre distinct
   et comparer octet pour octet via SHA-256 les six artefacts.
7. Archiver répertoire de build/preuves, manifest de sources, lock et provenance.

Les artefacts sont des **preuves de build**, pas un paquet redistribuable autonome :
notamment les DLL LLVM-MinGW ne sont pas copiées dans ce paquet de preuve.

## Run volontairement rouge

Relancer **le même commit** manuellement avec `inject_failure=true`.
La suite normale reste inchangée ; seul ce run ajoute `sentinel.forced_failure`
(27e test). Les jobs doivent être **FAIL**, précisément à l'étape de tests,
avec les 26 tests M0 passés et la sentinelle échouée.

Pas de `continue-on-error`, pas de `|| true` ou filtre masquant cet échec.
Le contrôle négatif isolé inclus dans chaque job vérifie le wrapper, mais ne
remplace pas ce run rouge distant : les deux preuves doivent rester distinctes.
Un job échoué au téléchargement ou à la compilation ne prouve pas cette propagation.

## Contrôle après téléchargement des artefacts

Extraire le paquet de preuves, retrouver son dossier de build puis exécuter :

```sh
cmake -DBUILD_DIR=/chemin/du/build -DPLATFORM=linux-x64 \
  -DCHECK_MANIFEST=ON -P ci/VerifyArtifacts.cmake
# Pour les preuves Windows : PLATFORM=windows-x64 ; aucune exécution PE nécessaire.
```

Ce mode compare au manifest déjà enregistré sans le réécrire. Comparer aussi
le lock et `source-manifest.sha256` au commit étudié. Consigner le nom et l'ID
de l'artefact, pas seulement une capture d'écran verte. Vérifier les six sorties,
la provenance du compilateur et les rapports de reproductibilité.

## Dossier de clôture

Compléter les colonnes **Platform / Compiler / Configuration / Build result /
Test result / CI run/reference / Status** de M0.md, les URL normale et négative,
le commit, les références des artefacts et les restrictions de reproductibilité.
N'écrire `M0 — COMPLETE` qu'après toutes ces preuves. Aujourd'hui : **PENDING
faute de repository distant** ; aucune URL, exécution Windows ou Linux native inventée.