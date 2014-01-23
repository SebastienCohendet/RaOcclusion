// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

void InitGL();
void Draw();
void Reshape(int w, int h);

int main(int argc, char** argv)
{
   //initialisation du FreeGlut
   glutInit(&argc, argv);

   // diff�rencier les mode RGBA et couleur index�, 
   // permet aussi de choisir le "double-buffering" et les buffers � utiliser (depth, stencil, accumulation)
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
  
   //cr�ation fenetre de taille donn�e
   glutInitWindowSize (800,600);
   glutCreateWindow ("EssaisOpenGL");
   //glutFullScreen();
   
   // pour d�finir les callbacks relatif aux actions de 
   // - affichage 
   // - redimensionnement
   // - interaction clavier/souris
   glutDisplayFunc (Draw);
   glutReshapeFunc (Reshape);
   /*
   glutIgnoreKeyRepeat(1);
   glutKeyboardFunc(keyboard);
   glutKeyboardUpFunc(keyboardup);
   glutSpecialFunc(special);
   glutSpecialUpFunc(specialup) ;
   glutMouseFunc(mouse);
   glutMotionFunc(motion);
   glutPassiveMotionFunc(motion);
   */

   //callback lanc� quand le syst�me n'a plus rien � faire => permet cr�ation d'animations 
   /*
   glutIdleFunc(void (*func)(void))
   */

   // Initialiser OpenGL
   InitGL();

   // boucle infinie => derniere ligne du programme pour permettre affichage en continu
   glutMainLoop();

   return 0;
}



/** 
Fonction d'affichage
*/
void Draw()             
{
	//Vide les buffers en param
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	
	//Choisit la matrice MODELVIEW
	glMatrixMode(GL_MODELVIEW);
	//R�initialise la matrice
	glLoadIdentity(); 	

	// Transformation de la vue (UTILISER CAMERA ARUCO)
	// http://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml
	gluLookAt(0,0,-10,0,0,0,0,1,0);
	
	// Tracer une figure (ENFIN!!!!!!!!!!)
	glBegin(GL_TRIANGLES);
		glVertex2i(0,1);
		glVertex2i(-1,0);
		glVertex2i(1,0);
	glEnd(); 

	//Echange les 2 frame buffers (celui qui est affich� et celui qu'on remplit en m�moire)
	glutSwapBuffers(); //Attention : pas SwapBuffers(DC), utiliser pour f�netre Windows native !

	//Recalculer la sc�ne
	glutPostRedisplay(); 

}



/**
Callback si fenetre redimentionn�e
Fonction utilis�e au moins une fois : � l'ouverture de la fenetre
*/
void Reshape(int w, int h)             
{
            // hauteur non null (pour �viter division par z�ro)
            if (h == 0) h = 1; 

			//mise � jour du Viewport
            glViewport(0,0,w,h);

			// chargement matrice de projection 
            glMatrixMode(GL_PROJECTION);
            // d�finition comme matrice identit�
			glLoadIdentity();
           
			/* gluPerspective modifie la matrice courante (c'est-�-dire ici la matrice de projection) 
			pour qu'OpenGL transforme les coordonn�es 3D au moment du rendu en coordonn�es 2D par rapport � l'�cran,
			de fa�on � ce qu'on ait l'impression de regarder � travers un objectif de 45 de focale, de largeur width et de hauteur height, 
			et dont le clipping va de 0.1 � 100 (attention la valeur du clipping near - ici 0.1 - doit �tre strictement sup�rieure � 0)
			*/
			gluPerspective(
			45,
			float(w)/float(h),
			0.1,
			100
			); 
				
			// chargement matrice de positionnement de la vue (optionnel)
            glMatrixMode(GL_MODELVIEW);
			// d�finition comme matrice identit�
			glLoadIdentity();

}


/** 
Fonction d'initialisation d'OpenGL
*/
void InitGL() 
{
}