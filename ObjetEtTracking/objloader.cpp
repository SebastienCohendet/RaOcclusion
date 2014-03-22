#include "objloader.h"

using namespace std;

void object_type::render()
{
    glBegin(GL_TRIANGLES); // glBegin and glEnd délimitent les sommets définissant la forme primitive (Triangles ici)

    for (int j=0;j<polygons_qty;j++)
    {
        //----------------- Premier sommet -----------------
        glVertex3f( vertex[ polygon[j].v[0] - 1].x,
                vertex[ polygon[j].v[0] - 1].y,
                vertex[ polygon[j].v[0] - 1].z);

        //----------------- Deuxième sommet -----------------
        glNormal3f( normcoord[ polygon[j].n[1] - 1 ].i,
                normcoord[ polygon[j].n[1] - 1 ].j,
                normcoord[ polygon[j].n[1] - 1 ].k);

        glTexCoord2f( mapcoord[ polygon[j].t[1] - 1 ].u,
                  mapcoord[ polygon[j].t[1] - 1 ].v);

        glVertex3f( vertex[ polygon[j].v[1] - 1].x,
                vertex[ polygon[j].v[1] - 1].y,
                vertex[ polygon[j].v[1] - 1].z);

        //----------------- Troisième sommet -----------------
        glNormal3f( normcoord[ polygon[j].n[2] - 1 ].i,
                normcoord[ polygon[j].n[2] - 1 ].j,
                normcoord[ polygon[j].n[2] - 1 ].k);

        glTexCoord2f( mapcoord[ polygon[j].t[2] - 1 ].u,
                  mapcoord[ polygon[j].t[2] - 1 ].v);

        glVertex3f( vertex[ polygon[j].v[2] - 1].x,
                vertex[ polygon[j].v[2] - 1].y,
                vertex[ polygon[j].v[2] - 1].z);
    }

    glEnd();
}


int object_type::objdatadisplay()
{
    int i;

	printf("VERTICES: %d\n",vertices_qty); 
	for (i =0;i<vertices_qty;i++)
	{
		printf("%f %f %f\n",vertex[i].x,vertex[i].y,vertex[i].z);
	}
	 
	printf("NORMALS: %d\n",normcoord_qty); 
	for (i =0;i<normcoord_qty;i++)
	{
		printf("%f %f %f\n",normcoord[i].i,normcoord[i].j,normcoord[i].k);
	}

	printf("MAP COORDS: %d\n",mapcoord_qty);
	for (i =0;i<mapcoord_qty;i++)
	{
		printf("%f %f\n",mapcoord[i].u,mapcoord[i].v);
	}

	printf("POLYGONS: %d\n",polygons_qty); 
	for (i=0;i<polygons_qty;i++) 
	{
		for (int j = 0;j<3;j++) //pour chaque sommet du polygone (triangle)
		{   
			printf("%d::%d/%d/%d\n",i,polygon[i].v[j],polygon[i].t[j],polygon[i].n[j]);
		}
	}
	return 1;
}


int object_type::objloader(string p_filename)
{
	// Indexs pour les sommets, normales, polygones, maps
    int ivertex=0; 
    int inormal =0;
    int ipolygon=0;
    int imap=0;
    
	char string[256];
	FILE *l_file; // pointeur vers le fichier

	char l_char; // variable contenant le caractère lu

	if ((l_file=fopen (p_filename.c_str(), "rt"))== NULL) return 0; //ouvre le fichier

	while (!feof(l_file)) // boucle pour lire le fichier entier
	{
		fscanf(l_file,"%c",&l_char);
		if(l_char=='\n') //read char if'/n' -skip to next and read   
			fscanf(l_file,"%c",&l_char);
		switch (l_char) 
		{   
		default: fgets(string,256,l_file);
			break;
		case 'v':   // sommet, normal ou "text co-ord"
			fscanf(l_file,"%c",&l_char);
			switch (l_char)  
			{
			case ' ':   // sommet attendu => lire 3 floats
				  fscanf(l_file,"%f %f %f",&vertex[ivertex].x, &vertex[ivertex].y,&vertex[ivertex].z);
				  ivertex++;
				  break;
			case 'n': // normal attendue => lire 3 floats 
				  fscanf(l_file,"%f %f %f",&normcoord[inormal].i, &normcoord[inormal].j,&normcoord[inormal].k);
				  inormal++;
				  break;
			case 't': //"texture map" attendu => lire 2 floats 
				  fscanf(l_file,"%f %f",&mapcoord[imap].u, &mapcoord[imap].v);
				  imap++;
				  break;
			}  // fin switch
			break;
		case 'f': //correspond à une face => le format est "f 1/1/1 2/2/2 3/3/3"
			for (int i=0;i<3;i++) //pour chaque sommet du polygone (triangle)
			{
				 fscanf(l_file,"%c",&l_char); //ignore caractère espace 
				 fscanf(l_file,"%d",&polygon[ipolygon].v[i]); //lire sommet
				 fscanf(l_file,"%c",&l_char); //ignore caractère espace 
				 fscanf(l_file,"%d",&polygon[ipolygon].t[i]); //lire "textur coord"
				 fscanf(l_file,"%c",&l_char); //ignore caractère espace
				 fscanf(l_file,"%d",&polygon[ipolygon].n[i]); //lire normal
			}
			ipolygon++;
			break;

		} // fin switch case
	}

	fclose (l_file); // fermer fichier
	vertices_qty = ivertex;
	polygons_qty = ipolygon;
	mapcoord_qty = imap;
	normcoord_qty = inormal;

	return 1;  //si aucune erreur
}