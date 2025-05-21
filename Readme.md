# Super Moteur 64 - Projet HAI819I

## Description

Super Moteur 64 est un moteur de rendu 3D créé dans le cadre du projet de cours HAI819I. Il a pour but final le développement d'un jeu typé super mario 64 avec un rendu PBR améliorant la qualité.

## Prérequis

- **C++11** ou version supérieure
- **GLFW** pour la gestion de la fenêtre et des entrées.
- **GLM** pour les transformations mathématiques (matrices, vecteurs, etc.).
- **OpenGL** (version 3.3 ou supérieure) pour le rendu 3D.
- **GLEW** pour l'extension de la gestion d'OpenGL.
- **Assimp** pour le chargement des modèles 3D
- **OpenAL** pour le système audio

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
   ./execute.sh      # Compile et lance le moteur
   ./clean.sh        # Nettoie les fichiers de build
   ./recreate.sh     # Recrée le projet depuis zéro
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
   - Utilisez les touches **UP, DOWN, LEFT, RIGHT** pour déplacer la caméra en mode orbital.
   - Utilisez les touches **Z, Q, S, D** pour déplacer le personnage jouable.

## Réalisations
1. **Obj loader :**
    Un lecteur de fichiers obj est en cours de développement afin de pouvoir utiliser des structures de meshes, textures et animations plus complexes.
2. **Eclairge PBR :**
    L'éclairage PBR est disponible pour obtenir un Mario doré.
3. **Sons :**
    Un gestionnaire de son est disponible utilisant la bibliothèque OpenAL (Compatible avec C++17 et supérieur).
4. **Interface utilisateur :**
    Une interface utilisateur est implémentée.
5. **Super Mario :**
    Un jeu type super Mario 64 sera jouable sur ce moteur.
6. **Animation :**
    Un gestionnaire d'animations est également disponibles dans le moteur, il se base sur la bibliothèque Assimp pour charger les fichiers Collada.

## Contributeurs

- **DUPUIS Thibaut** - Créateur
- **LANGOUET Bastian** - Créateur

## Licence

Ce projet est sous la licence [MIT](LICENSE).

---