# Super Moteur 64 - Projet HAI819I

## Description

Super Moteur 64 est un moteur de rendu 3D créé dans le cadre du projet de cours HAI819I. Il a pour but final le développement d'un jeu typé super mario 64 avec un rendu PBR améliorant la qualité.

## Fonctionnalités

- Affichage de scènes 3D avec objets, lumières et caméra dans un graphe de scène.
- Support des matériaux PBR (Physically Based Rendering) avec cartes de normal, métalliques, rugueuses, et d'occlusion ambiante.

## Prérequis

- **C++17** ou version supérieure
- **GLFW** pour la gestion de la fenêtre et des entrées.
- **GLM** pour les transformations mathématiques (matrices, vecteurs, etc.).
- **OpenGL** (version 3.3 ou supérieure) pour le rendu 3D.
- **GLEW** pour l'extension de la gestion d'OpenGL.

## Installation

1. **Cloner le projet :**

   ```bash
   git clone https://github.com/ton-repository/Super-Moteur-64.git
   cd Super-Moteur-64
   ```

2. **Créer un répertoire de build et compiler le projet :**

   Si vous utilisez `CMake` :

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

   Cela va générer un exécutable dans le répertoire `build`.

   Des executables sont disponibles :

   ```bash
   chmod u+x clean.sh recreate.sh execute.sh
   ./execute.sh
   ./clean.sh
   ./recreate.sh
   ```

   Chaque commande permet de générer, nettoyer ou régénérer le moteur, celui-ci ce lancera automatiquement après compilation.


## Utilisation

1. **Lancer l'exécutable :**

   Une fois la compilation terminée, Vous pouvez lancer le moteur avec la commande suivante :

   ```bash
   ./TP5
   ```

   Cela ouvrira une fenêtre 3D où vous pourrez voir les objets et les lumières dans la scène.

2. **Contrôles de la caméra :**
   - Utilisez la touche **C** pour changer de mode de caméra. 
   - Utilisez les touches **Z, Q, S, D** pour déplacer la caméra en mode orbital.
   - Utilisez les touches **I, J, K, L** pour déplacer le personnage jouable.

## Work in Progress
1. **Obj loader :**
    Un lecteur de fichiers obj est en cours de développement afin de pouvoir utiliser des structures de meshes, textures et animations plus complexes.
2. **Améliorations de l'éclairge PBR :**
    L'intégration de nouvelles fonctionnalités comme L'IBL sont en cours de développement pour améliorer le système d'illumination.
    De nombreuses modifications dont actuellement en cours afin de faciliter l'utilisation des éclairages.
3. **Prise en charge des input manette :**
    Intégration du support manette prévu.
4. **Sons :**
    Ajout d'un support des sons dans le moteur.
5. **Interface utilisateur :**
    Une interface utilisateur sera possiblement implémentée à l'aide d'une bibliothèque externe à déterminer.
6. **Super Mario :**
    Un jeu type super Mario 64 sera jouable sur ce moteur.

## Contributeurs

- **DUPUIS Thibaut** - Créateur
- **LANGOUET Bastian** - Créateur

## Licence

Ce projet est sous la licence [MIT](LICENSE).

---