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
		cerr<<"Impossible de lire la vidéo"<<endl;
	}
       
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
void FonctionsOpenGL::initialisation (std::string obj)
{
     glClearColor(0.0, 0.0, 0.0, 0.0); // Met le fond en noir si aucune vidéo n'est chargé (cas limite)
	
    // Initialisation du viewport (fênetre d'affichage)
    glViewport(0,0,screen_width,screen_height);  

    // Passage en mode Projection 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Initialisation de la matrice de projection à l'identité
    gluPerspective(45.0f,(GLfloat)screen_width/(GLfloat)screen_height,0.5f,100000000.0f);     


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
	for (int i=0;i<1;i++)
	{
		objarray[i] = new (object_type);
		objarray[i]->objloader(obj);
		objarray[i]->objdatadisplay();      
	}

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
	gluPerspective(45.0f,(GLfloat)screen_width/(GLfloat)screen_height,1.0f,100000000.0f);

	glutPostRedisplay (); // On re-rend la scène
	TheGlWindowSize=Size(screen_width,screen_height); //on sauvegarde la taille de la fenêtre pour la suite
}

/** Gestion du clavier.
 * Prise en charge de la taille de l'objet (=zoom) 
 * et des déplacements de l'objet. <br/>
 * Touches à utiliser : <br/>
 * "+" pour zoomer l'objet <br/>
 * "-" pour dézoomer l'objet <br/>
 * "0" pour réinitialiser la taille de l'objet <br/>
 * touches "zqsd" pour déplacer la voiture <br/>
 * "1" pour réinitialiser la position de la voiture  
 */
void FonctionsOpenGL::keyboard (unsigned char key, int x, int y) {

	/* Gestion du zoom */
	if (key=='+')
		facteurZoom=facteurZoom*2;
	if (key=='-')
		facteurZoom=facteurZoom/2;
	if (key=='0')
		facteurZoom=1;

	/* Flèches directionnelles */
	if (key=='z') //avancer
	{
		zpos += TheMarkerSize;
	}
	if (key=='s') //reculer
	{
		zpos -= TheMarkerSize;
	}
    if (key=='q') //tourner à gauche (en avancant)
    {
		float distanceAZero = sqrt (xpos*xpos + zpos*zpos) / TheMarkerSize;
		yrot += 40/distanceAZero; 
		
		float yrotrad = (yrot / 180 * 3.141592654f);
		xpos += float(sin(yrotrad))*TheMarkerSize;
		zpos += float(cos(yrotrad))*TheMarkerSize;
    }
    if (key=='d') //tourner à droite (en avancant)
    {
		float distanceAZero = sqrt (xpos*xpos + zpos*zpos) / TheMarkerSize;
		yrot -= 20/distanceAZero; 

		float yrotrad = (yrot / 180 * 3.141592654f);
		xpos -= float(sin(yrotrad))*TheMarkerSize;
		zpos += float(cos(yrotrad))*TheMarkerSize;
    }
	if (key=='1')  // reinitialiser position voiture
		xpos = 0, ypos = 0, zpos = 0, yrot = 0;

	/* Touche d'échappement (Echap = quitter) */
    if (key==27)
    {
		exit(0);
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
    if (TheResizedImage.rows==0) //On attend que l'image soit bien réinitialisée avant de continuer
        return;
    ///C'est bon, image réinitialisée
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    ///On rend l'image dans le buffer
    glMatrixMode(GL_MODELVIEW); //Positionnement de la caméra
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 5.0);
    glViewport(0, 0, TheGlWindowSize.width , TheGlWindowSize.height);
    glDisable(GL_TEXTURE_2D);
    glPixelZoom( 1, -1);
    glRasterPos3f( 0, TheGlWindowSize.height  - 0.5, -1.0f );
    glDrawPixels ( TheGlWindowSize.width , TheGlWindowSize.height , GL_RGB , GL_UNSIGNED_BYTE , TheResizedImage.ptr(0) ); //rend la vidéo
	
	glAccum(GL_LOAD, 0.5);

    ///On récupère la matrice de projection afin de faire nos rendus dans l'environnement comme si on filmait depuis la caméra
    glMatrixMode(GL_PROJECTION);
    double proj_matrix[16];

    TheCameraParams.glGetProjectionMatrix(TheInputImage.size(),TheGlWindowSize,proj_matrix,0.05,facteurZoom*10);
	
    glLoadIdentity();
    glLoadMatrixd(proj_matrix);
    glLineWidth(2);
    //Pour chaque marqueur (démo plus)
    double modelview_matrix[16];

	// Afficher un cube au dessus de chaque marker
    /*
	for (unsigned int m=0;m<TheMarkers.size();m++)
        {
            TheMarkers[m].glGetModelViewMatrix(modelview_matrix);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glLoadMatrixd(modelview_matrix);
    // 		axis(TheMarkerSize);
            glColor3f(1,0.4,0.4);
            glTranslatef(0, TheMarkerSize/2,0);
            glPushMatrix();
            glutWireCube( TheMarkerSize );

            glPopMatrix();
        }
	*/

    //Si la planche est détecté avec assez de probabilités, on affiche l'objet
    if (TheBoardDetected.second>0.1) {
        TheBoardDetected.first.glGetModelViewMatrix(modelview_matrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixd(modelview_matrix);
        glColor3f(0,1,0);
        glTranslatef(0, TheMarkerSize/2,0); //On est pile sur le plan des markers
        glPushMatrix();
		
		glEnable(GL_DEPTH_TEST); // Cache les éléments normalement cachés : c'est le Z-Buffer

		if (objarray[0]->id_texture!=-1) 
		{
			glBindTexture(GL_TEXTURE_2D, objarray[0]->id_texture); // On active les textures
			glEnable(GL_TEXTURE_2D); // Texture mapping ok
			//printf("Textures chargées");
		}
		else
			glDisable(GL_TEXTURE_2D); // Texture mapping OFF
		
		// Grossir/réduire les éléments affichés à l'écran (+ pour zoomer, - pour dézoomer, 1 pour revenir à la taille d'origine) 
		glScalef(facteurZoom, facteurZoom, facteurZoom);

		glClear( GL_COLOR_BUFFER_BIT);
		glutSolidCube(TheMarkerSize*10);
		glAccum(GL_ACCUM, 0);

		glClear(GL_COLOR_BUFFER_BIT);

		// Rotation de la voiture dans le plan (xOz)
		glRotatef(yrot,0.0,1.0,0.0);

		// Translation de la voiture dans le plan (xOz)
		glTranslated(xpos,0.0f,zpos);

		// Affichage de la voiture
		objarray[0]->render();
		glAccum(GL_ACCUM, 0.5);

        // Afficher théière de taille TheMarkerSize
		// glutWireTeapot( TheMarkerSize );

		glDisable(GL_DEPTH_TEST); // Cache les éléments normalement cachés : c'est le Z-Buffer
       
		glPopMatrix();
		glAccum(GL_RETURN, 1);

    }

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
        //by deafult, opencv works in BGR, so we must convert to RGB because OpenGL in windows preffer
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

