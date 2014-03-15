#include "FonctionsOpenGL.h"

/** Constructeur permettant le lancement de l'affichage d'OpenGL.
Ce constructeur fait appel � Aruco et OpenCV pour d�tecter les markers sur la vid�o. 
Il affiche ensuite les fichiers obj correctement. */
FonctionsOpenGL::FonctionsOpenGL(string TheInputVideo, string TheBoardConfigFile, string TheIntrinsicFile, float TheMarkerSize) 
{
	/* D�pendance � Aruco et OpenCv */
	//vid�o
	this->TheInputVideo=TheInputVideo;
	//Fichier config du board
	this->TheBoardConfigFile=TheBoardConfigFile;
	//Cam�ra calibration info
	this->TheIntrinsicFile=TheIntrinsicFile;
	//taille des marqueurs
	this->TheMarkerSize=TheMarkerSize;

	//Lecture des infos du board
	TheBoardConfig.readFromFile(TheBoardConfigFile);

	//Charge la vid�o donn�e en param�tre
	if (TheInputVideo=="live")  //Lecture depuis la premi�re webcam sur l'ordinateur
		TheVideoCapturer.open(0);
	else TheVideoCapturer.open(TheInputVideo);
	if (!TheVideoCapturer.isOpened())
	{
		cerr<<"Impossible de lire la video"<<endl;
	}
    
	TheVideoCapturer.set(3,1280);
	TheVideoCapturer.set(4,720);

	//Lecture de la premi�re image
	TheVideoCapturer>>TheInputImage;
	//Lit les param�tres de la cam�ra (pass�s en argument)
	TheCameraParams.readFromXMLFile(TheIntrinsicFile);
	TheCameraParams.resize(TheInputImage.size());

	//Taille de la fenetre adapt� � la vid�o / webcam
	screen_width = TheInputImage.size().width;
	screen_height = TheInputImage.size().height;

	/* Initialisation des autres attributs */
	xpos = 0, ypos = 0, zpos = 0, yrot = 0;
	TheGlWindowSize=Size(screen_width,screen_height);
	TheCaptureFlag=true; 
	facteurZoom = 0.125f; //valeur pour tuture.obj
    
}

/** Initialisation d'OpenGl (vue, etc.) */
void FonctionsOpenGL::initialisation (std::string objs)
{
     glClearColor(0.0, 0.0, 0.0, 0.0); // Met le fond en noir si aucune vid�o n'est charg� (cas limite)
	
    // Initialisation du viewport (f�netre d'affichage)
    glViewport(0,0,screen_width,screen_height);  

    // Passage en mode Projection 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Initialisation de la matrice de projection � l'identit�
    gluPerspective(70.0f,(GLfloat)screen_width/(GLfloat)screen_height,0.5f,100000000.0f);     


	//Initialisation et activation des lumi�res (utile)
   	GLfloat light_ambient[]= { 0.1f, 0.1f, 0.1f, 0.1f };
	GLfloat light_diffuse[]= { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat light_specular[]= { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat light_position[]= { 100.0f, 0.0f, -10.0f, 1.0f };
	glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, light_specular);
    glLightfv (GL_LIGHT1, GL_POSITION, light_position);    
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);

    //Initialisation et activitation des mat�riaux (utile ?)
	GLfloat mat_ambient[]= { 0.5f, 0.5f, 0.0f, 0.0f };
	GLfloat mat_diffuse[]= { 0.5f, 0.5f, 0.0f, 0.0f };
	GLfloat mat_specular[]= { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat mat_shininess[]= { 1.0f };
	glMaterialfv (GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_specular);
    glMaterialfv (GL_FRONT, GL_POSITION, mat_shininess);

	//Initialisations diverses d'OpenGl
    glShadeModel(GL_SMOOTH); // Type des Shaders utilis�s
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Correction de la perspective du texture mapping
    glEnable(GL_TEXTURE_2D); // Activation des textures
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL); // Les polygones sont "remplies"
	glEnable(GL_CULL_FACE); // Activation du back face culling

	//Chargement .obj
	string ligne;
	ifstream fichier (objs);
	int i=0;
	if (fichier.is_open())
	{
		while ( getline (fichier,ligne) )
		{
			objarray[i] = new (object_type);
			objarray[i]->objloader(ligne);
			i++;
		}
		fichier.close();
	}

	else cout << "Lecture impossible du fichier d'objets"; 

}


/** Fonction appel�e d�s qu'un redimensionnement de la taille de la fen�tre est effectu�.
 *
 * @param p_width : largeur (pixels) de la vue (fen�tre)
 * @param p_height : hauteur (pixels) de la vue (fen�tre)
 */
void FonctionsOpenGL::resize (int p_width, int p_height)
{
	if (screen_width==0 && screen_height==0) exit(0);

	screen_width=p_width; // On obtient la nouvelle taille de la fen�tre : ici largeur
	screen_height=p_height; // et l�, hauteur

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // On supprime les buffers de couleurs et de profondeurs pour la nouvelle image
	glViewport(0,0,screen_width,screen_height); // Transformation de la fen�tre

	glMatrixMode(GL_PROJECTION); // Transformation de la matrice
	glLoadIdentity(); // On r�initialise la matrice de projection � l'identit�
	gluPerspective(70.0f,(GLfloat)screen_width/(GLfloat)screen_height,0.1f,1000.0f);

	glutPostRedisplay (); // On re-rend la sc�ne
	TheGlWindowSize=Size(screen_width,screen_height); //on sauvegarde la taille de la fen�tre pour la suite
}

/** Gestion du clavier.
 Prise en charge de la taille de l'objet (=zoom) et des d�placements de l'objet. <br/>
 Touches � utiliser : <br/>
 "Page suivante" pour zoomer l'objet <br/>
 "Page pr�c�dente" pour d�zoomer l'objet <br/>
 Fl�ches directionnelles pour d�placer la voiture <br/>
 "Fin" pour r�initialiser la taille & la position de l'objet <br/>
*/
void FonctionsOpenGL::keyboard (int key, int x, int y) {

	/* Gestion du zoom */
	if (key==GLUT_KEY_PAGE_UP)
		facteurZoom=facteurZoom*1.25f;
	if (key==GLUT_KEY_PAGE_DOWN)
		facteurZoom=facteurZoom/1.25f;

	/* Fl�ches directionnelles */
	if (key==GLUT_KEY_UP) //avancer
	{
		float yrotrad = (yrot / 180 * 3.141592654f);
		zpos += TheMarkerSize/4 * cos(yrotrad);
		xpos += TheMarkerSize/4 * sin(yrotrad);
	}
	if (key==GLUT_KEY_DOWN) //reculer
	{
		float yrotrad = (yrot / 180 * 3.141592654f);
		zpos -= TheMarkerSize/4 * cos(yrotrad);
		xpos -= TheMarkerSize/4 * sin(yrotrad);
	}
    if (key==GLUT_KEY_LEFT) //tourner � gauche
    {
		yrot += 10; 
    }
    if (key==GLUT_KEY_RIGHT) //tourner � droite
    {
		yrot -= 10; 
    }
	
	/* Reinitialiser position/taille voiture */
    if (key==GLUT_KEY_END)
    {
		printf("facteurZoom: %f \n",facteurZoom);
		printf("xpos: %f \n",xpos);
		printf("zpos: %f \n",zpos);
		printf("yrot: %f\n",yrot);
		facteurZoom=0.125f;
		xpos = 0, ypos = 0, zpos = 0, yrot = 0;
    }
}

/** Support de la souris.
 * Mouvement et clic. Pas utilis� pour le moment
*/
void FonctionsOpenGL::mouseMovement(int x, int y) {
	
}


/** G�n�re l'image en cours 
 * SUBROUTINE display(void)
 *
 * This is our main rendering subroutine, called each frame
 * 
 */
void FonctionsOpenGL::display(void)
{
    glDisable(GL_LIGHTING);
   
     
    if (TheResizedImage.rows==0) //On attend que l'image soit bien r�initialis�e avant de continuer
        return;
    // C'est bon, image reinitialisee
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   
    // D�activation Z-Buffer pour l'affichage de la video (en arri�re plan)
    glDisable(GL_DEPTH_TEST);
   
    // On rend l'image (de la video) dans le buffer
    glMatrixMode(GL_MODELVIEW); //Positionnement de la camera
    glLoadIdentity();
    glPushMatrix(); 
	
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
		glLoadIdentity();
		glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 5.0);
		glViewport(0, 0, TheGlWindowSize.width , TheGlWindowSize.height);
		glDisable(GL_TEXTURE_2D);
		glPixelZoom( 1, -1);
		glRasterPos3f( 0, TheGlWindowSize.height  - 0.5, -1.0f );
		glDrawPixels ( TheGlWindowSize.width , TheGlWindowSize.height , GL_RGB , GL_UNSIGNED_BYTE , TheResizedImage.ptr(0) ); //rend la vid�o
    glPopMatrix();
   
    // R�-Activation Z-Buffer pour occlusion entre �l�ments virtuels
    glEnable(GL_DEPTH_TEST);
   
    // On r�cup�re la matrice de projection afin de faire nos rendus dans l'environnement comme si on filmait depuis la cam�ra
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
		glLoadIdentity();
		double proj_matrix[16];
		TheCameraParams.glGetProjectionMatrix(TheInputImage.size(),TheGlWindowSize,proj_matrix,0.05,10); 
		glLoadMatrixd(proj_matrix);
		glLineWidth(2);

		// Matrice utilisee pour chaque marqueur (d�mo plus)
		double modelview_matrix[16];

		// Afficher un cube au dessus de chaque marker
		/* 
		for (unsigned int m=0;m<TheMarkers.size();m++)
		{
			TheMarkers[m].glGetModelViewMatrix(modelview_matrix);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glLoadMatrixd(modelview_matrix);
			glColor3f(1,0.4,0.4);
			glTranslatef(0, TheMarkerSize/2,0);
			glutWireCube( TheMarkerSize );
		}
		*/

		//Si la planche est detectee avec assez de probabilites, on affiche l'objet
		if (TheBoardDetected.second>0.1) {
			TheBoardDetected.first.glGetModelViewMatrix(modelview_matrix);
			//TheMarkers[0].glGetModelViewMatrix(modelview_matrix);
			glMatrixMode(GL_MODELVIEW);

			glPushMatrix();
				glLoadIdentity();
				glLoadMatrixd(modelview_matrix);
				glColor3f(0,1,0);
				glTranslatef(0, TheMarkerSize/2,0); // on est exactement sur le plan des markers
       
				// Desactivation du color buffer et "dessiner" le monde virtuel cach� = objet r�el virtualis�
				glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
				glPushMatrix();
				displayVirtualHiddenWorld();
				glPopMatrix();
   
				// Reactivation du color buffer et dessiner l'objet virtuel
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
				glPushMatrix();
				displayVirtualWorld();
				glPopMatrix();
       
			glPopMatrix();
		}
		
   glPopMatrix(); // from the GL PROJECTION

   glutSwapBuffers();


}

/** vIdle, d�tecte les marqueurs entre chaque it�ration **/
void FonctionsOpenGL::vIdle()
{
    if (TheCaptureFlag) {
        //capture image
        TheVideoCapturer.grab();
        TheVideoCapturer.retrieve( TheInputImage);
        TheUndInputImage.create(TheInputImage.size(),CV_8UC3);
        //by default, opencv works in BGR, so we must convert to RGB because OpenGL in windows preffer
        cv::cvtColor(TheInputImage,TheInputImage,CV_BGR2RGB);
        //remove distorion in image
        cv::undistort(TheInputImage,TheUndInputImage, TheCameraParams.CameraMatrix,TheCameraParams.Distorsion);
        //detect markers
        MDetector.detect(TheUndInputImage,TheMarkers,TheCameraParams.CameraMatrix,Mat(),TheMarkerSize);
        //Detection of the board
        TheBoardDetected.second=TheBoardDetector.detect( TheMarkers, TheBoardConfig,TheBoardDetected.first, TheCameraParams,TheMarkerSize);
        //chekc the speed by calculating the mean speed of all iterations
        //resize the image to the size of the GL window
        cv::resize(TheUndInputImage,TheResizedImage,TheGlWindowSize);
    }
    glutPostRedisplay();
}



int FonctionsOpenGL::getWidth() {
	return screen_width;
}


int FonctionsOpenGL::getHeight() {
	return screen_height;
}

void FonctionsOpenGL::displayVirtualHiddenWorld()
{
   // Modifier dimension monde virtuel cach�
   float ratio = 0.65f;
   glScalef(ratio, ratio, ratio);
   
	// Affichage et positionnement du b�timent central
    glPushMatrix();

	   // Translation de la voiture dans le plan (xOz)
	   glTranslated(0.292109f,0.0f,0.249397f);

	   // Grossir/r�duire les �l�ments affich�s � l'�cran (+ pour zoomer, - pour d�zoomer, 1 pour revenir � la taille d'origine)
	   glScalef(0.125f, 0.125f, 0.125f);
   
	   // Affichage de la voiture
	   glColor3f(0,1,1);

	   objarray[0]->render();
   glPopMatrix();

   ///////////////////////////

   	// Affichage et positionnement du b�timent BG
    glPushMatrix();

	   // Translation de la voiture dans le plan (xOz)
	   glTranslated(0.111052f,0.0f,0.087257f);

	   // Grossir/r�duire les �l�ments affich�s � l'�cran (+ pour zoomer, - pour d�zoomer, 1 pour revenir � la taille d'origine)
	   glScalef(0.100f, 0.100f, 0.100f);
   
	   // Affichage de la voiture
	   glColor3f(1,1,1);

	   objarray[1]->render();
   glPopMatrix();

   ///////////////////////////

   	// Affichage et positionnement du b�timent BD
    glPushMatrix();

	   // Translation de la voiture dans le plan (xOz)
	   glTranslated(0.078444f,0.0f,0.413195f);

	   // Grossir/r�duire les �l�ments affich�s � l'�cran (+ pour zoomer, - pour d�zoomer, 1 pour revenir � la taille d'origine)
	   glScalef(0.064f, 0.064f, 0.064f);
   
	   // Affichage de la voiture
	   glColor3f(1,1,1);

	   objarray[2]->render();
   glPopMatrix();

   ///////////////////////////
}

void FonctionsOpenGL::displayVirtualWorld()
{
    // Affichage objet
    if (objarray[0]->id_texture!=-1)
    {
		glBindTexture(GL_TEXTURE_2D, objarray[0]->id_texture); // On active les textures
		glEnable(GL_TEXTURE_2D); // Texture mapping ok
    }
    else
		glDisable(GL_TEXTURE_2D); // Texture mapping OFF

	glPushMatrix();

	   // Translation de la voiture dans le plan (xOz)
	   glTranslated(xpos,0.0f,zpos);
  
	   // Rotation de la voiture dans le plan (xOz)
	   glRotatef(yrot,0.0,1.0,0.0);

	   // Grossir/r�duire les �l�ments affich�s � l'�cran (+ pour zoomer, - pour d�zoomer, 1 pour revenir � la taille d'origine)
	   glScalef(facteurZoom, facteurZoom, facteurZoom);
   
	   // Affichage de la voiture
	   glColor3f(1,0,0);

	   objarray[5]->render();
   glPopMatrix();
}

