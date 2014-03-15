#include "FonctionsOpenGL.h"

/** Constructeur permettant le lancement de l'affichage d'OpenGL.
Ce constructeur fait appel à Aruco et OpenCV pour détecter les markers sur la vidéo. 
Il affiche ensuite les fichiers obj correctement. */
FonctionsOpenGL::FonctionsOpenGL(string TheInputVideo, string TheBoardConfigFile, string TheIntrinsicFile, float TheMarkerSize) 
{
	/* Dépendance à Aruco et OpenCv */
	//vidéo
	this->TheInputVideo=TheInputVideo;
	//Fichier config du board
	this->TheBoardConfigFile=TheBoardConfigFile;
	//Caméra calibration info
	this->TheIntrinsicFile=TheIntrinsicFile;
	//taille des marqueurs
	this->TheMarkerSize=TheMarkerSize;

	//Lecture des infos du board
	TheBoardConfig.readFromFile(TheBoardConfigFile);

	//Charge la vidéo donnée en paramètre
	if (TheInputVideo=="live")  //Lecture depuis la première webcam sur l'ordinateur
		TheVideoCapturer.open(0);
	else TheVideoCapturer.open(TheInputVideo);
	if (!TheVideoCapturer.isOpened())
	{
		cerr<<"Impossible de lire la video"<<endl;
	}
    
	TheVideoCapturer.set(3,1280);
	TheVideoCapturer.set(4,720);

	//Lecture de la première image
	TheVideoCapturer>>TheInputImage;
	//Lit les paramètres de la caméra (passés en argument)
	TheCameraParams.readFromXMLFile(TheIntrinsicFile);
	TheCameraParams.resize(TheInputImage.size());

	//Taille de la fenetre adapté à la vidéo / webcam
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
     glClearColor(0.0, 0.0, 0.0, 0.0); // Met le fond en noir si aucune vidéo n'est chargé (cas limite)
	
    // Initialisation du viewport (fênetre d'affichage)
    glViewport(0,0,screen_width,screen_height);  

    // Passage en mode Projection 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Initialisation de la matrice de projection à l'identité
    gluPerspective(70.0f,(GLfloat)screen_width/(GLfloat)screen_height,0.5f,100000000.0f);     


	//Initialisation et activation des lumières (utile)
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

    //Initialisation et activitation des matériaux (utile ?)
	GLfloat mat_ambient[]= { 0.5f, 0.5f, 0.0f, 0.0f };
	GLfloat mat_diffuse[]= { 0.5f, 0.5f, 0.0f, 0.0f };
	GLfloat mat_specular[]= { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat mat_shininess[]= { 1.0f };
	glMaterialfv (GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_specular);
    glMaterialfv (GL_FRONT, GL_POSITION, mat_shininess);

	//Initialisations diverses d'OpenGl
    glShadeModel(GL_SMOOTH); // Type des Shaders utilisés
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


/** Fonction appelée dès qu'un redimensionnement de la taille de la fenêtre est effectué.
 *
 * @param p_width : largeur (pixels) de la vue (fenêtre)
 * @param p_height : hauteur (pixels) de la vue (fenêtre)
 */
void FonctionsOpenGL::resize (int p_width, int p_height)
{
	if (screen_width==0 && screen_height==0) exit(0);

	screen_width=p_width; // On obtient la nouvelle taille de la fenêtre : ici largeur
	screen_height=p_height; // et là, hauteur

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // On supprime les buffers de couleurs et de profondeurs pour la nouvelle image
	glViewport(0,0,screen_width,screen_height); // Transformation de la fenêtre

	glMatrixMode(GL_PROJECTION); // Transformation de la matrice
	glLoadIdentity(); // On réinitialise la matrice de projection à l'identité
	gluPerspective(70.0f,(GLfloat)screen_width/(GLfloat)screen_height,0.1f,1000.0f);

	glutPostRedisplay (); // On re-rend la scène
	TheGlWindowSize=Size(screen_width,screen_height); //on sauvegarde la taille de la fenêtre pour la suite
}

/** Gestion du clavier.
 Prise en charge de la taille de l'objet (=zoom) et des déplacements de l'objet. <br/>
 Touches à utiliser : <br/>
 "Page suivante" pour zoomer l'objet <br/>
 "Page précédente" pour dézoomer l'objet <br/>
 Flèches directionnelles pour déplacer la voiture <br/>
 "Fin" pour réinitialiser la taille & la position de l'objet <br/>
*/
void FonctionsOpenGL::keyboard (int key, int x, int y) {

	/* Gestion du zoom */
	if (key==GLUT_KEY_PAGE_UP)
		facteurZoom=facteurZoom*1.25f;
	if (key==GLUT_KEY_PAGE_DOWN)
		facteurZoom=facteurZoom/1.25f;

	/* Flèches directionnelles */
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
    if (key==GLUT_KEY_LEFT) //tourner à gauche
    {
		yrot += 10; 
    }
    if (key==GLUT_KEY_RIGHT) //tourner à droite
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
 * Mouvement et clic. Pas utilisé pour le moment
*/
void FonctionsOpenGL::mouseMovement(int x, int y) {
	
}


/** Génère l'image en cours 
 * SUBROUTINE display(void)
 *
 * This is our main rendering subroutine, called each frame
 * 
 */
void FonctionsOpenGL::display(void)
{
    glDisable(GL_LIGHTING);
   
     
    if (TheResizedImage.rows==0) //On attend que l'image soit bien réinitialisée avant de continuer
        return;
    // C'est bon, image reinitialisee
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   
    // Déactivation Z-Buffer pour l'affichage de la video (en arrière plan)
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
		glDrawPixels ( TheGlWindowSize.width , TheGlWindowSize.height , GL_RGB , GL_UNSIGNED_BYTE , TheResizedImage.ptr(0) ); //rend la vidéo
    glPopMatrix();
   
    // Ré-Activation Z-Buffer pour occlusion entre éléments virtuels
    glEnable(GL_DEPTH_TEST);
   
    // On récupère la matrice de projection afin de faire nos rendus dans l'environnement comme si on filmait depuis la caméra
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
		glLoadIdentity();
		double proj_matrix[16];
		TheCameraParams.glGetProjectionMatrix(TheInputImage.size(),TheGlWindowSize,proj_matrix,0.05,10); 
		glLoadMatrixd(proj_matrix);
		glLineWidth(2);

		// Matrice utilisee pour chaque marqueur (démo plus)
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
       
				// Desactivation du color buffer et "dessiner" le monde virtuel caché = objet réel virtualisé
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

/** vIdle, détecte les marqueurs entre chaque itération **/
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
   // Modifier dimension monde virtuel caché
   float ratio = 0.65f;
   glScalef(ratio, ratio, ratio);
   
	// Affichage et positionnement du bâtiment central
    glPushMatrix();

	   // Translation de la voiture dans le plan (xOz)
	   glTranslated(0.292109f,0.0f,0.249397f);

	   // Grossir/réduire les éléments affichés à l'écran (+ pour zoomer, - pour dézoomer, 1 pour revenir à la taille d'origine)
	   glScalef(0.125f, 0.125f, 0.125f);
   
	   // Affichage de la voiture
	   glColor3f(0,1,1);

	   objarray[0]->render();
   glPopMatrix();

   ///////////////////////////

   	// Affichage et positionnement du bâtiment BG
    glPushMatrix();

	   // Translation de la voiture dans le plan (xOz)
	   glTranslated(0.111052f,0.0f,0.087257f);

	   // Grossir/réduire les éléments affichés à l'écran (+ pour zoomer, - pour dézoomer, 1 pour revenir à la taille d'origine)
	   glScalef(0.100f, 0.100f, 0.100f);
   
	   // Affichage de la voiture
	   glColor3f(1,1,1);

	   objarray[1]->render();
   glPopMatrix();

   ///////////////////////////

   	// Affichage et positionnement du bâtiment BD
    glPushMatrix();

	   // Translation de la voiture dans le plan (xOz)
	   glTranslated(0.078444f,0.0f,0.413195f);

	   // Grossir/réduire les éléments affichés à l'écran (+ pour zoomer, - pour dézoomer, 1 pour revenir à la taille d'origine)
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

	   // Grossir/réduire les éléments affichés à l'écran (+ pour zoomer, - pour dézoomer, 1 pour revenir à la taille d'origine)
	   glScalef(facteurZoom, facteurZoom, facteurZoom);
   
	   // Affichage de la voiture
	   glColor3f(1,0,0);

	   objarray[5]->render();
   glPopMatrix();
}

