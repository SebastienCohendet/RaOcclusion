#ifndef FONCTIONSOPENGL
#define FONCTIONSOPENGL

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#ifdef __WIN32__
#include <windows.h>
#endif
#include <opencv/cv.h>
#include <fstream>
using namespace cv;
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <aruco/aruco.h>
#else
#include "GL\glut.h" 
#include "aruco\aruco.h"
#endif

//Aruco
using namespace aruco;

#include "objloader.h"

/** Objet contenant toutes les fonctions utilisées pour l'affichage avec OpenGL. */
class FonctionsOpenGL{
	private:
		/** Position de la voiture pour son déplacement dans la ville */
		float xpos, ypos, zpos, yrot;
		/** Contient les obj à charger/afficher */
		object_type *objarray[2];   
		/** Dimensions de la fenetre */
		int screen_width,screen_height;
		/** Variables nécessaires à la détection des marqueurs par Aruco */
		string TheInputVideo,TheIntrinsicFile,TheBoardConfigFile;
		float TheMarkerSize;
		VideoCapture TheVideoCapturer;
		Mat TheInputImage,TheUndInputImage,TheResizedImage;
		CameraParameters TheCameraParams;
 		Size TheGlWindowSize;
		bool TheCaptureFlag;
		MarkerDetector MDetector;
		vector<Marker> TheMarkers;
		/** la planche de marqueurs */
		BoardDetector TheBoardDetector;
		BoardConfiguration TheBoardConfig;
		pair<Board,float> TheBoardDetected; /**< la planche et ses probas */
		/** facteur de zoom pour la taille de l'Objet, modifiable au clavier */
		float facteurZoom; 

	public:
		FonctionsOpenGL(string TheInputVideo, string TheBoardConfigFile, string TheIntrinsicFile, float TheMarkerSize);
		void initialisation (std::string obj);
		void resize (int p_width, int p_height);
		void keyboard (unsigned char key, int x, int y);
		void mouseMovement(int x, int y);
		void display(void);
		void vIdle();
		int getWidth();
		int getHeight();
   
      void displayVirtualHiddenWorld();
      void displayVirtualWorld();
};

#endif
