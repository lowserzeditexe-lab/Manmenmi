# Configuration, logs, erreurs, coverage

## Configuration (DOCUMENTED — contrat M0)

Fichier UTF-8 sans BOM, clés et valeurs ASCII, `key=value`, espaces/tabulations
autour autorisés, LF/CRLF, commentaires **de ligne entière** commençant par `#`.
Pas de commentaires de fin de ligne, de sections, de guillemets, ni d'expansion
de variables. Limite **65 536 octets** ; un NUL est rejeté. Fichier explicite
uniquement : aucun scan du dossier personnel ou chargement `.env`.

| Clé | Valeurs | Par défaut sans fichier |
|---|---|---|
| `schema_version` | `1`, obligatoire dans tout fichier | contrat v1 |
| `backend` | `auto`, `vulkan`, `dx12`, `opengl` | `auto` |
| `allow_fallback` | `true`, `false` | `false` |
| `log_level` | `debug`, `info`, `warning`, `error` | `info` |

Toute clé inconnue, dupliquée, invalide ou version non supportée est une erreur.
Priorité : **défauts < fichier < CLI**, indépendamment de la position de `--config`.
La CLI exige `--option=valeur`. Les doublons, options inconnues, commandes
concurrentes et `--allow-fallback` + `--no-fallback` sont refusés. Les options ne
masquent pas un fichier mal formé. `--help` avec une option invalide reste une erreur.

## Sélection

`auto` est une politique, jamais un backend réel. Sans autorisation, un seul
candidat : Vulkan pour `auto`, ou le backend explicitement demandé. Avec
`allow_fallback=true` / `--allow-fallback`, essayer le candidat initial puis les
autres dans l'ordre Vulkan → DX12 → OpenGL, sans doublon.

En M0 chaque candidat est **UNIMPLEMENTED**, même DX12 sous Windows. Tous les
essais échouent. Le plan n'est **pas une détection de GPU**. Chaque essai et
chaque repli autorisé est journalisé au niveau erreur pour ne pas disparaître
avec `--log-level=error`. Une option de backend ne prouve pas son support.

## Sorties et erreurs

`stdout` : informations demandées (aide, version, config, inventaire).
`stderr` : `[niveau][catégorie] message`, une ligne par événement. Catégories :
`core`, `config`, `window`, `graphics`, `test`. Retours ligne/CR/tabulations
échappés et caractères de contrôle remplacés ; pas d'horodatage non déterministe
en M0. Le niveau n'autorise jamais à transformer une erreur en succès.

| Code processus | Signification |
|---|---|
| 0 | Diagnostic réussi UNIQUEMENT (ne signifie jamais rendu en M0) |
| 2 | CLI ou fichier invalide |
| 3 | Fonction UNIMPLEMENTED ou ressource UNAVAILABLE |
| 4 | Lecture de fichier impossible |
| 70 | Erreur interne inattendue / invariant brisé |

`UNIMPLEMENTED` : aucun chemin d'implémentation. `UNAVAILABLE` : future
implémentation présente mais impossible dans l'environnement. Ne pas confondre.
Les erreurs attendues utilisent `Result<T>` ; jamais un booléen sans explication.

## Tests et coverage

CTests `unit.*`, `cli.*`, `architecture.*`, label `foundation`. Chaque CLI vérifie
**le code exact et le diagnostic**, pas `WILL_FAIL` qui masquerait un crash.
Assertions autonomes actives en Release. Aucun framework tiers ; GoogleTest
**PLANNED / option à justifier M2+**, pas ajouté à vide.

Coverage LLVM optionnelle sur les sources MANMENMI (tests et système exclus du
rapport). La CI archive la mesure sans objectif pourcentage artificiel.
Les chemins d'erreur du socle doivent être testés ; les écarts sont documentés.
**Aucune conversion** d'un pourcentage C++ en couverture GX2/API ou support de jeu.
Parité et golden tests sont **PLANNED M3/M5**, pas passés ni ignorés en faux vert.