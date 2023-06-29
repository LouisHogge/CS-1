#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "PNM.h"
#include "SeamCarving.h"

//shrinkImage prend en argument image et k : la réduction de pixels en largeur souhaitée et renvoie image_1 : une réduction de image de k pixels en largeur.
 
PNMImage *shrinkImage(PNMImage *image, int k)
{
    if ((unsigned long)k<(image->width))
    {
        PNMImage *image_1 = NULL;
        if (!(image_1 = createPNM(image->width,image->height)))
            return NULL;
        
        for (unsigned long a=0;a<(image_1->height);a++)
        {
            for (unsigned long b=0;b<(image_1->width);b++)
            {
                image_1->data[(a)*(image_1->width)+(b)]
                =
                image->data[(a)*(image->width)+(b)];
            }
        }
        
        PNMImage *image_2 = NULL;
        int *seam = NULL;
        
        for (int i=0;i<k;i++)
        {
            seam = computeMinEnergySeam(image_1, computePixelEnergyGradient);
            image_2 = cutImage(image_1, seam);
            freePNM (image_1);
            image_1 = image_2;
            image_2 = NULL;
            freePNM (image_2);
            free (seam);
        }
        return image_1;
    }
    else
    {
        fprintf(stderr, "\nk doit être plus petit que la largeur de l'image.\n");
        return NULL;
    }
}

//cutImage prend en argument image et seam : le tableau renvoyé par computeMinEnergySeam et renvoie img : l'image obtenue en retirant de image les pixels (i,j) donnés par seam de la manière suivante : numéro de case = i, contenu case correspondante = j.

PNMImage *cutImage(PNMImage *image, int *seam)
{
    
    //On crée le tableau img (c'est une image), de largeur inférieure d'une unité à celle du tableau image, dans lequel on copie le tableau image (c'est une image) excepté les positions (=pixels) présentes dans le tableau seam correspondant à la couture d’énergie minimale de image.
    
    PNMImage *img = NULL;
    if (!(img = createPNM((image->width)-1,image->height)))
        return NULL;
    
    for (unsigned long a=0;a<(img->height);a++)
    {
        for (unsigned long b=0;b<(img->width);b++)
        {
            if (b>=(unsigned long)seam[a])
            {
                img->data[(a)*(img->width)+(b)]
                =
                image->data[(a)*(image->width)+(b+1)];
            }
            else
            {
                img->data[(a)*(img->width)+(b)]
                =
                image->data[(a)*(image->width)+(b)];
            }
        }
    }
    return img;
}

//computeMinEnergySeam prend comme argument image et la fonction computePixelEnergyGradient permettant de calculer l’énergie d’un pixel et renvoie seam : un tableau d’entiers de la taille de la hauteur de image contenant les abscisses des pixels appartenant à la couture d’énergie minimale de image de telle sorte que : numéro de case = ordonnée-i, contenu case correspondante = abscisse-j.

int *computeMinEnergySeam(PNMImage *image, int (*computePixelEnergy)(PNMImage *, int, int))
{
    
    //On crée une table M de la même taille que image.
    
    int **M = NULL;
    if (!(M = malloc((image->height)*sizeof(unsigned long *))))
        return NULL;
    
    for (unsigned long i=0;i<(image->height);i++)
    {
        if (!(M[i] = malloc(image->width*sizeof(unsigned long))))
          {
            for (unsigned long j = 0; j < i; j++)
              {
                  free (M[j]);
              }
              
            free (M);
            return NULL;
          }
    }
    
    //On remplit M de la manière suivante : chaque élément (i,j) contient l’énergie minimale d’une couture partant de la première ligne et s’arrêtant au pixel (i,j).
    
    for (unsigned long s=0;s<(image->height);s++)
    {
        for (unsigned long t=0;t<(image->width);t++)
        {
            if (s==0)
            {
                M[s][t] = (computePixelEnergy(image, s, t));
            }
            else
            {
                int Mmin = M[s-1][t];
        
                for (int x=0;x<=2;x+=2)
                {
                    if (t==0)
                        x=x+2;
                
                    if (M[s-1][t-1+x] < Mmin)
                    {
                        Mmin = M[s-1][t-1+x];
                    }
                    
                    if (t==(image->width)-1)
                        x++;
                }
                M[s][t] = (computePixelEnergy(image, s, t)) + Mmin;
            }
        }
    }
    
    //On crée un tableau de la taille de la hauteur de image correspondant à la couture d’énergie minimale.
    
    int *seam = NULL;
    if (!(seam = malloc((image->height)*sizeof(unsigned long))))
        return NULL;
    
    //On cherche le pixel qui minimise la dernière ligne de la table M.
    
    int min = M[(image->height)-1][0];
    seam[(image->height)-1] = 0;
    
    for (unsigned long z=1;z<(image->width);z++)
    {
        if(M[(image->height)-1][z] < min)
        {
            min = M[(image->height)-1][z];
            seam[(image->height)-1] = z;
        }
    }
    
    //On remplit le tableau seam correspondant à la couture d’énergie minimale de image en commençant par la fin à partir du minimum trouvé à l'étape précédente
    
    for (int v=(image->height)-2;v>=0;v--)
    {
        int SeamMin = M[v][seam[v+1]];
        seam[v] = seam[v+1];

        for (int y=0;y<=2;y+=2)
        {
            if (seam[v+1]==0)
                y=y+2;
        
            if (M[v][seam[v+1]-1+y] < SeamMin)
            {
                SeamMin = M[v][seam[v+1]-1+y];
                seam[v] = seam[v+1]-1+y;
            }
        
            if ((unsigned long)seam[v+1]==(image->width)-1)
                y++;
        }
    }
    
    //On libère la table M.
    
    for (unsigned long p=0;p<image->height;p++)
    {
        free(M[p]);
    }
    free(M);

    return seam;
}

//computePixelEnergyGradient prend en argument image et la position (i,j) d'un pixel et renvoie Eij : l'énergie du pixel (i,j) de image.

int computePixelEnergyGradient(PNMImage *image, int i, int j)
{
    int Eij=0, a=i-1, c=j-1;
    unsigned long b=i+1, d=j+1;
    
    if (a<0)
        a++;
    else if (b>=image->height)
        b--;
    if (c<0)
        c++;
    else if (d>=image->width)
        d--;
    
    Eij =
      abs((image->data[(i)*(image->width)+(c)].red)-(image->data[(i)*(image->width)+(d)].red))
    + abs((image->data[(a)*(image->width)+(j)].red)-(image->data[(b)*(image->width)+(j)].red))
    + abs((image->data[(i)*(image->width)+(c)].green)-(image->data[(i)*image->width+(d)].green))
    + abs((image->data[(a)*(image->width)+(j)].green)-(image->data[(b)*(image->width)+(j)].green))
    + abs((image->data[(i)*(image->width)+(c)].blue)-(image->data[(i)*image->width+(d)].blue))
    + abs((image->data[(a)*(image->width)+(j)].blue)-(image->data[(b)*image->width+(j)].blue));
   
  return Eij;
}
