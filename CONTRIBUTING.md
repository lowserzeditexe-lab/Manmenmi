# Contribuer à MANMENMI

1. Lire README, INITIAL_STATE et conventions avant tout code de compatibilité.
2. **Ne pas dépasser M0 avant son gate.** Aucun backend à moitié implémenté pour
   remplir l'arborescence ; les README de modules réservés sont intentionnels.
3. Fournir du code original compatible MIT ; citer la provenance des idées.
   Ne pas soumettre de SDK/dump/asset Nintendo, code de jeu, clés ni secrets.
   `.gitignore` réduit les erreurs mais n'est pas une barrière de sécurité.
4. Qualifier chaque conclusion DOCUMENTED/OBSERVED/INFERRED/UNKNOWN, avec source
   ou protocole. Les décisions et états PLANNED/UNIMPLEMENTED restent explicites.
5. C++20, headers publics autonomes, erreurs explicites `Result<T>`, RAII,
   aucune fuite native, aucune dépendance backend depuis GX2.
6. Ajouter des CTests autonomes qui vérifient aussi erreurs et comportements
   Release ; pas d'`assert()` comme seule assertion. Warnings traités en erreurs.
7. Exécuter Debug/Release pour la plateforme concernée et consigner toute
   incapacité à tester ailleurs. Une CI non lancée n'est jamais « verte ».
8. Dépendances : [politique](docs/dependencies.md), pin et notice avant import.

Une proposition de compatibilité graphique doit inclure backend/OS/GPU/pilote,
scène synthétique redistribuable, divergence éventuelle et preuves. Ne pas
mettre de trace contenant données propriétaires dans des artefacts CI publics.