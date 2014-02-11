#ifndef _CAMERAPOS_
#define _CAMERAPOS_

#include <opencv/cv.h>
#include <fstream>
using namespace cv;

//Aruco
#include "aruco\aruco.h"

void axis(float); /**< Afficher des axes dans le rep�re courant */

/**Lire les matrices "intrinsics" et "distorsion" depuis un fichier.
 * <br /> Format du fichier :  
 * <br /> # comments
 * <br /> fx fy cx cy k1 k2 p1 p2 width height 1
 *
 * @param TheIntrinsicFile : chemin du fichier contenant les infos sur les caract�res intrins�ques
 * @param TheIntriscCameraMatrix matrices de sortie contenant les donn�es intrins�ques
 * @param TheDistorsionCameraParams vecteur de sortie avec les param�tres de distorsion
 * @param size taille des images film�es. 
 * Note : ces images peuvent �tre diff�rentes de celles utilis�es pour la calibration (qui sont dans le fichier de calibration).
 * Si c'est le cas, les donn�es intrins�ques doivent �tre adapt�es correctement. 
 * C'est pourquoi il est n�cessaire de passer ici la taille des images film�es.
 * @return true si param�tres lus correctement 
 */

bool readIntrinsicFile(string TheIntrinsicFile,Mat & TheIntriscCameraMatrix,Mat &TheDistorsionCameraParams,Size size)
{
	// ouvre le fichier
	ifstream InFile(TheIntrinsicFile.c_str());
	if (!InFile) return false;
	char line[1024];
	InFile.getline(line,1024);	 // ne pas lire la premiere ligne (qui contient que des commentaires normalement)
	InFile.getline(line,1024); // lire la ligne avec les infos pertinentes 

	// Transformation vers type stringstream 
	stringstream InLine;
	InLine<<line;
	// Cr�er les matrices
	TheDistorsionCameraParams.create(4,1,CV_32FC1);
	TheIntriscCameraMatrix=Mat::eye(3,3,CV_32FC1);
	

	// Lire la matrice "intrinsic"
	InLine>>TheIntriscCameraMatrix.at<float>(0,0);//fx								
	InLine>>TheIntriscCameraMatrix.at<float>(1,1); //fy								
	InLine>>TheIntriscCameraMatrix.at<float>(0,2); //cx								 
	InLine>>TheIntriscCameraMatrix.at<float>(1,2);//cy
	// Lire les param�tres de distorsion 
	for(int i=0;i<4;i++) InLine>>TheDistorsionCameraParams.at<float>(i,0);
	
	// Lire la taille de la cam�ra
	float width,height;
	InLine>>width>>height;
	// Redimensionner les parametres de la cam�ra pour correspondre � la taille de l'image
	float AxFactor= float(size.width)/ width;
	float AyFactor= float(size.height)/ height;
	TheIntriscCameraMatrix.at<float>(0,0)*=AxFactor;
	TheIntriscCameraMatrix.at<float>(0,2)*=AxFactor;
	TheIntriscCameraMatrix.at<float>(1,1)*=AyFactor;
	TheIntriscCameraMatrix.at<float>(1,2)*=AyFactor;

	// pour le debug
	/*
	cout<<"fx="<<TheIntriscCameraMatrix.at<float>(0,0)<<endl;
	cout<<"fy="<<TheIntriscCameraMatrix.at<float>(1,1)<<endl;
	cout<<"cx="<<TheIntriscCameraMatrix.at<float>(0,2)<<endl;
	cout<<"cy="<<TheIntriscCameraMatrix.at<float>(1,2)<<endl;
	cout<<"k1="<<TheDistorsionCameraParams.at<float>(0,0)<<endl;
	cout<<"k2="<<TheDistorsionCameraParams.at<float>(1,0)<<endl;
	cout<<"p1="<<TheDistorsionCameraParams.at<float>(2,0)<<endl;
	cout<<"p2="<<TheDistorsionCameraParams.at<float>(3,0)<<endl;
	*/

	return true;
} 
#endif
