RaOcclusion
===========

Le but de ce projet, mené dans le cadre de nos études à Centrale Nantes, est d'appréhender l'ensemble des tenants et aboutissants de la réalité augmentée. Cela passera par la réalisation d'une maquette carton/lego d'une ville (par exemple) qui, une fois filmée, sera agrémenter d'élements se déplaçant dans cette ville, en prenant en compte l'occlusion.

Le projet se base sur les librairies OpenCV et Aruco et est développé sous Visual Studio 2012, à des fins de compatibilités plus aisées avec OpenCv (version déjà compilée pour VS).

Ayant eu des difficultés à faire fonctionner Aruco sous Windows (et plus particulièrement sous Visual Studio), voici la démarche à suivre :

  - Il vous télécharger les fichiers sources d'aruco, disponible uniquement dans le package linux. Décompresser le package en .tgz (passez sous linux si nécessaire)
  - Créez une librairie statique avec les fichiers téléchargés (tutoriel disponible ici : http://msdn.microsoft.com/FR-FR/library/ms235627%28v=vs.90%29.aspx )
  - Il vous faudra ensuite créer des solutions dépendantes du projet ayant permis de créer la librairie statique Aruco.

=======

A noter qu'à la génération, une dll sera requise dans le dossier d'exécution du programme : freeglut.dll


L'ensemble du projet est censé fonctionner sous d'autres OS, à condition que les librairies OpenCv et Aruco soient disponibles, lors de la compilation des fichiers sources.


=======

Fichier config des objs :

Le fichier des objs (attendu dans les arguments du main) doit contenir un obj (=chemin complet de l'obj) par ligne.
Exemple de contenu de ce fichier : \n
C:\user\obj1 \n
C:\user\obj2 \n
C:\user\obj3

Le 7ème obj sera l'objet déplaçable et pouvant changer de taille (une voiture par exemple).
A noter que les autres objs sont placés précisément, dans le code, par des translations, rotations et mis à l'échelle un à un.

======
