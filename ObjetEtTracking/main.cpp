#include "objloader.h"
#include "fonctionsOpenGL.h"

// Création instance pour affichage OpenGL
FonctionsOpenGL * openGL; 

// Redéfinition des méthodes pour permettre à GlutDisplayFunc d'appeler les méthodes de openGL
void resize (int p_width, int p_height)
{
	openGL->resize(p_width, p_height);
}
void keyboard (int key, int x, int y) 
{
	openGL->keyboard(key, x, y);
}
void mouseMovement(int x, int y) 
{
	openGL->mouseMovement(x, y);
}
void display(void)
{   
	openGL->display();
}
void vIdle()
{
	openGL->vIdle();
}

/** La fonction main.
* @param argv : listing des paramètres nécessaire : l'obj à charger, la vidéo ou live, la configuration de la planche, les données de corrections de déformations de la caméra et la taille des marqueurs (en m)
*
*/
int main(int argc, char **argv)
{
	// Pour débugger plus vite !
	if (argc!=6) {
      cerr<<"Nombre d'arguments invalide"<<endl;
      cerr<<"Ordre demandé : objet (triangulaire) .obj (in.avi|live) boardConfig.yml  intrinsicsLogitech.yml   size "<<endl;
      return false;
   }
	

	// initialisation de l'instance avec les arguments passés à la fonction main
	openGL = new FonctionsOpenGL(argv[2],argv[3],argv[4],atof(argv[5]));

	// On utilise GLUT pour charger la fenêtre, gérer les entrées (clavier/souris) et interraction avec la fenêtre
	glutInit(&argc, argv);    
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(openGL->getWidth(),openGL->getHeight());    
	glutInitWindowPosition(0,0);
	glutCreateWindow("ObjetEtTracking");
	glutDisplayFunc(display);
	glutIdleFunc(vIdle);
	glutReshapeFunc (resize);
   



	// Prise en charge du clavier et souris 
	//glutPassiveMotionFunc(mouseMovement); 
	glutSpecialFunc (keyboard); 

	//initialisation de l'obj
	openGL->initialisation(argv[1]);
	glutMainLoop();
	
    return(0);    
}