#ifndef PALETTES_H
#define PALETTES_H

//Se definen los arreglos de las paletas de colores
extern const int colormap_rainbow[];
extern const int colormap_grayscale[];
extern const int colormap_ironblack[];
//Se declaran funciones para obtener el tamaño de cada paleta de colores, las cuales devuelven la cantidad de elementos que contiene cada uno.
extern int get_size_colormap_rainbow();
extern int get_size_colormap_grayscale();
extern int get_size_colormap_ironblack();

#endif
