#ifndef canny_h
#define canny_h

#include "CImg.h"
using namespace cimg_library;

CImg<int> canny(CImg<int> grey, int width, int height);
CImg<int> cannyparam(CImg<int> grey, int width, int height,
						  float lowthreshold, float highthreshold,
						  float gaussiankernelradius, int gaussiankernelwidth,  
						  int contrastnormalised);

#endif