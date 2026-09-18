# Compiler M0

## Préconditions

C++20, CMake **≥ 3.25**, Ninja, Clang, bibliothèque standard C++ et SDK système.
Aucun SDK Nintendo, dump, SDK Vulkan, OpenGL, SDL ou service réseau n'est requis
pour configurer/compiler/tester MANMENMI. CMake ne télécharge **rien**.
La configuration CI épingle LLVM **18.1.8**, CMake **3.31.6**, Ninja Python
**1.11.1.3** (Ninja 1.11.1), Python **3.12.9**. Python ne fait pas partie du build local.
Clang **14.0.6** est également OBSERVED dans les essais locaux.

## Linux x64

Installer ces outils via les paquets du système ou les distributions officielles
listées dans [les notices](../THIRD_PARTY_NOTICES.md), puis :

```sh
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug --parallel 2
ctest --preset linux-clang-debug --output-junit results.xml
./build/linux-clang-debug/manmenmi_probe --check-config --config=configs/manmenmi.conf
./build/linux-clang-debug/manmenmi_probe --backend=vulkan
# Cette dernière commande DOIT sortir avec le code 3, UNIMPLEMENTED.
```

Remplacer `debug` par `release` pour tester les optimisations. Les assertions de
test restent actives en Release. `--list-backends` n'interroge aucun GPU.

## Windows x64

Extraire [LLVM-MinGW 20240619 UCRT x86_64](https://github.com/mstorsjo/llvm-mingw/releases/tag/20240619)
(Clang 18.1.8) et ajouter son dossier `bin` au PATH. Installer CMake et Ninja.
Un terminal Windows ordinaire suffit : **aucun SDK propriétaire ni Visual Studio requis**.
L'UCRT est le composant système de Windows 10/11, pas un fichier distribué dans ce dépôt.

```bat
cmake --preset windows-clang-debug
cmake --build --preset windows-clang-debug --parallel 2
ctest --preset windows-clang-debug --output-junit results.xml
build\windows-clang-debug\manmenmi_probe.exe --list-backends
```

Le preset choisit `x86_64-w64-mingw32-clang++`, pas `clang-cl` ni un Clang
MSVC récupéré accidentellement sur le PATH. Garder le `bin` de LLVM-MinGW sur
PATH pour les DLL de bibliothèque standard lors des tests. Le support clang-cl/MSVC
reste **UNKNOWN (non validé)** ; il n'est pas nécessaire à M0.

## Instrumentation (Linux Clang)

```sh
cmake --preset linux-clang-coverage
cmake --build --preset linux-clang-coverage
LLVM_PROFILE_FILE="$PWD/build/linux-clang-coverage/%m-%p.profraw" ctest --preset linux-clang-coverage
cmake -DBUILD_DIR=build/linux-clang-coverage -P cmake/Coverage.cmake

cmake --preset linux-clang-sanitizers
cmake --build --preset linux-clang-sanitizers
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --preset linux-clang-sanitizers
```

Installer les runtimes compiler-rt correspondant à Clang pour coverage/ASan.
Utiliser les versions correspondantes de llvm-profdata et llvm-cov sur PATH.
Supprimer les anciens `.profraw` après tout changement de binaire instrumenté.

## Vérification sur cet hôte ARM64 (ne signifie pas support ARM64)

Le garde x64 refuse les autres architectures par défaut. Dérogation explicite
pour les **seuls tests de fondation**, pas pour une plateforme de distribution :

```sh
cmake --preset linux-clang-debug -DMANMENMI_ALLOW_UNSUPPORTED_HOST=ON
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
```

Une seconde vérification produit de vrais ELF **x86-64** avec Clang :

```sh
# Outils de vérification Debian : g++-x86-64-linux-gnu et qemu-user.
cmake -S . -B build/linux-x64-cross -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64-cross.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build/linux-x64-cross
ctest --test-dir build/linux-x64-cross --output-on-failure
```

QEMU exécute ici les **tests hôtes x86-64**, pas du PowerPC ni du logiciel Wii U.
Il n'est ni lié, ni utilisé par MANMENMI. Cette mesure ne remplace pas les runners natifs x64.

## CI et reproductibilité

Workflow : [ci.yml](../.github/workflows/ci.yml). Matrice : Linux Debug/Release/
coverage/sanitizers et Windows Debug/Release. SHA complets pour les actions ;
versions exactes pour LLVM/CMake/Ninja/Python. Les archives LLVM viennent des
releases officielles ; leur SHA-256 est journalisé, **pas comparé à un hash prévalidé**.
Les images GitHub, paquets système, SDK et bibliothèques standard évoluent : la
construction n'est donc pas hermétique ni garantie bit-identique. Épingler les
hashes d'archives et l'image complète reste une amélioration de reproductibilité.

Un YAML présent n'est pas une CI passée. Voir [validation M0](validation/M0.md).