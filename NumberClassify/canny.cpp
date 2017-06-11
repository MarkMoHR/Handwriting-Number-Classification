#include "canny.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>

#define ffabs(x) ( (x) >= 0 ? (x) : -(x) ) 
#define GAUSSIAN_CUT_OFF 0.005f
#define MAGNITUDE_SCALE 100.0f
#define MAGNITUDE_LIMIT 1000.0f
#define MAGNITUDE_MAX ((int) (MAGNITUDE_SCALE * MAGNITUDE_LIMIT))

typedef struct
{
	CImg<int> data; /* input image */
	int width;
	int height;
	int *idata;          /* output for edges */
	int *magnitude;      /* edge magnitude as detected by Gaussians */
	float *xConv;        /* temporary for convolution in x direction */
	float *yConv;        /* temporary for convolution in y direction */
	float *xGradient;    /* gradients in x direction, as detected by Gaussians */
	float *yGradient;    /* gradients in x direction,a s detected by Gaussians */
} CANNY;


/*
buffers destructor
*/
void killbuffers(CANNY *can) {
	if (can) {
		delete(can->idata);
		delete(can->magnitude);
		delete(can->xConv);
		delete(can->yConv);
		delete(can->xGradient);
		delete(can->yGradient);
	}
}

/*
buffer allocation
*/
CANNY *allocatebuffers(const CImg<int> & grey, int width, int height)
{
	CANNY *answer;

	answer = new CANNY;
	if (!answer)
		goto error_exit;
	answer->data = CImg<int>(width, height, 1, 1, 0);
	answer->idata = new int[width * height];
	answer->magnitude = new int[width * height];
	answer->xConv = new float[width * height];
	answer->yConv = new float[width * height];
	answer->xGradient = new float[width * height];
	answer->yGradient = new float[width * height];

	if (!answer->data || !answer->idata || !answer->magnitude ||
		!answer->xConv || !answer->yConv ||
		!answer->xGradient || !answer->yGradient)
		goto error_exit;

	cimg_forXY(grey, x, y) {
		answer->data(x, y, 0) = grey(x, y, 0);
	}
	answer->width = width;
	answer->height = height;

	return answer;
error_exit:
	killbuffers(answer);
	return 0;
}


float gaussian(float x, float sigma)
{
	return (float)exp(-(x * x) / (2.0f * sigma * sigma));
}


float hypotenuse(float x, float y)
{
	return (float)sqrt(x*x + y*y);
}

/* NOTE: The elements of the method below (specifically the technique for
non-maximal suppression and the technique for gradient computation)
are derived from an implementation posted in the following forum (with the
clear intent of others using the code):
http://forum.java.sun.com/thread.jspa?threadID=546211&start=45&tstart=0
My code effectively mimics the algorithm exhibited above.
Since I don't know the providence of the code that was posted it is a
possibility (though I think a very remote one) that this code violates
someone's intellectual property rights. If this concerns you feel free to
contact me for an alternative, though less efficient, implementation.
*/

int computeGradients(CANNY *can, float kernelRadius, int kernelWidth)
{
	float *kernel;
	float *diffKernel;
	int kwidth;

	int width, height;

	int initX;
	int maxX;
	int initY;
	int maxY;

	int x, y;
	int i;
	int flag;

	width = can->width;
	height = can->height;

	kernel = new float[kernelWidth];
	diffKernel = new float[kernelWidth];

	if (!kernel || !diffKernel)
		goto error_exit;

	/* initialise the Gaussian kernel */
	for (kwidth = 0; kwidth < kernelWidth; kwidth++) {
		float g1, g2, g3;
		g1 = gaussian((float)kwidth, kernelRadius);
		if (g1 <= GAUSSIAN_CUT_OFF && kwidth >= 2)
			break;
		g2 = gaussian(kwidth - 0.5f, kernelRadius);
		g3 = gaussian(kwidth + 0.5f, kernelRadius);
		kernel[kwidth] = (g1 + g2 + g3) / 3.0f / (2.0f * (float) 3.14 * kernelRadius * kernelRadius);
		diffKernel[kwidth] = g3 - g2;
	}

	initX = kwidth - 1;
	maxX = width - (kwidth - 1);
	initY = width * (kwidth - 1);
	maxY = width * (height - (kwidth - 1));

	/* perform convolution in x and y directions */
	for (x = initX; x < maxX; x++)
	{
		for (y = initY; y < maxY; y += width)
		{
			int index = x + y;
			float sumX = can->data[index] * kernel[0];
			float sumY = sumX;
			int xOffset = 1;
			int yOffset = width;
			while (xOffset < kwidth)
			{
				sumY += kernel[xOffset] * (can->data[index - yOffset] + can->data[index + yOffset]);
				sumX += kernel[xOffset] * (can->data[index - xOffset] + can->data[index + xOffset]);
				yOffset += width;
				xOffset++;
			}

			can->yConv[index] = sumY;
			can->xConv[index] = sumX;
		}

	}

	for (x = initX; x < maxX; x++)
	{
		for (y = initY; y < maxY; y += width)
		{
			float sum = 0.0f;
			int index = x + y;
			for (i = 1; i < kwidth; i++)
				sum += diffKernel[i] * (can->yConv[index - i] - can->yConv[index + i]);

			can->xGradient[index] = sum;
		}

	}

	for (x = kwidth; x < width - kwidth; x++)
	{
		for (y = initY; y < maxY; y += width)
		{
			float sum = 0.0f;
			int index = x + y;
			int yOffset = width;
			for (i = 1; i < kwidth; i++)
			{
				sum += diffKernel[i] * (can->xConv[index - yOffset] - can->xConv[index + yOffset]);
				yOffset += width;
			}

			can->yGradient[index] = sum;
		}

	}

	initX = kwidth;
	maxX = width - kwidth;
	initY = width * kwidth;
	maxY = width * (height - kwidth);
	for (x = initX; x < maxX; x++)
	{
		for (y = initY; y < maxY; y += width)
		{
			int index = x + y;
			int indexN = index - width;
			int indexS = index + width;
			int indexW = index - 1;
			int indexE = index + 1;
			int indexNW = indexN - 1;
			int indexNE = indexN + 1;
			int indexSW = indexS - 1;
			int indexSE = indexS + 1;

			float xGrad = can->xGradient[index];
			float yGrad = can->yGradient[index];
			float gradMag = hypotenuse(xGrad, yGrad);

			/* perform non-maximal supression */
			float nMag = hypotenuse(can->xGradient[indexN], can->yGradient[indexN]);
			float sMag = hypotenuse(can->xGradient[indexS], can->yGradient[indexS]);
			float wMag = hypotenuse(can->xGradient[indexW], can->yGradient[indexW]);
			float eMag = hypotenuse(can->xGradient[indexE], can->yGradient[indexE]);
			float neMag = hypotenuse(can->xGradient[indexNE], can->yGradient[indexNE]);
			float seMag = hypotenuse(can->xGradient[indexSE], can->yGradient[indexSE]);
			float swMag = hypotenuse(can->xGradient[indexSW], can->yGradient[indexSW]);
			float nwMag = hypotenuse(can->xGradient[indexNW], can->yGradient[indexNW]);
			float tmp;
			/*
			* An explanation of what's happening here, for those who want
			* to understand the source: This performs the "non-maximal
			* supression" phase of the Canny edge detection in which we
			* need to compare the gradient magnitude to that in the
			* direction of the gradient; only if the value is a local
			* maximum do we consider the point as an edge candidate.
			*
			* We need to break the comparison into a number of different
			* cases depending on the gradient direction so that the
			* appropriate values can be used. To avoid computing the
			* gradient direction, we use two simple comparisons: first we
			* check that the partial derivatives have the same sign (1)
			* and then we check which is larger (2). As a consequence, we
			* have reduced the problem to one of four identical cases that
			* each test the central gradient magnitude against the values at
			* two points with 'identical support'; what this means is that
			* the geometry required to accurately interpolate the magnitude
			* of gradient function at those points has an identical
			* geometry (upto right-angled-rotation/reflection).
			*
			* When comparing the central gradient to the two interpolated
			* values, we avoid performing any divisions by multiplying both
			* sides of each inequality by the greater of the two partial
			* derivatives. The common comparand is stored in a temporary
			* variable (3) and reused in the mirror case (4).
			*
			*/
			flag = ((xGrad * yGrad <= 0.0f) /*(1)*/
				? ffabs(xGrad) >= ffabs(yGrad) /*(2)*/
				? (tmp = ffabs(xGrad * gradMag)) >= ffabs(yGrad * neMag - (xGrad + yGrad) * eMag) /*(3)*/
				&& tmp > fabs(yGrad * swMag - (xGrad + yGrad) * wMag) /*(4)*/
				: (tmp = ffabs(yGrad * gradMag)) >= ffabs(xGrad * neMag - (yGrad + xGrad) * nMag) /*(3)*/
				&& tmp > ffabs(xGrad * swMag - (yGrad + xGrad) * sMag) /*(4)*/
				: ffabs(xGrad) >= ffabs(yGrad) /*(2)*/
				? (tmp = ffabs(xGrad * gradMag)) >= ffabs(yGrad * seMag + (xGrad - yGrad) * eMag) /*(3)*/
				&& tmp > ffabs(yGrad * nwMag + (xGrad - yGrad) * wMag) /*(4)*/
				: (tmp = ffabs(yGrad * gradMag)) >= ffabs(xGrad * seMag + (yGrad - xGrad) * sMag) /*(3)*/
				&& tmp > ffabs(xGrad * nwMag + (yGrad - xGrad) * nMag) /*(4)*/
				);
			if (flag)
			{
				can->magnitude[index] = (gradMag >= MAGNITUDE_LIMIT) ? MAGNITUDE_MAX : (int)(MAGNITUDE_SCALE * gradMag);
				/*NOTE: The orientation of the edge is not employed by this
				implementation. It is a simple matter to compute it at
				this point as: Math.atan2(yGrad, xGrad); */
			}
			else
			{
				can->magnitude[index] = 0;
			}
		}
	}
	free(kernel);
	free(diffKernel);
	return 0;
error_exit:
	free(kernel);
	free(diffKernel);
	return -1;
}



/*
recursive portion of edge follower
*/

void follow(CANNY *can, int x1, int y1, int i1, int threshold)
{
	int x, y;
	int x0 = x1 == 0 ? x1 : x1 - 1;
	int x2 = x1 == can->width - 1 ? x1 : x1 + 1;
	int y0 = y1 == 0 ? y1 : y1 - 1;
	int y2 = y1 == can->height - 1 ? y1 : y1 + 1;

	can->idata[i1] = can->magnitude[i1];
	for (x = x0; x <= x2; x++)
	{
		for (y = y0; y <= y2; y++)
		{
			int i2 = x + y * can->width;
			if ((y != y1 || x != x1) && can->idata[i2] == 0 && can->magnitude[i2] >= threshold)
				follow(can, x, y, i2, threshold);
		}
	}
}

/*
we follow edges. high gives the parameter for starting an edge,
how the parameter for continuing it.
*/
void performHysteresis(CANNY *can, int low, int high)
{
	int offset = 0;
	int x, y;

	memset(can->idata, 0, can->width * can->height * sizeof(int));

	for (y = 0; y < can->height; y++) {
		for (x = 0; x < can->width; x++) {
			if (can->idata[offset] == 0 && can->magnitude[offset] >= high)
				follow(can, x, y, offset, low);
			offset++;
		}
	}
}


void normalizeContrast(CImg<int> & data, int width, int height)
{
	int histogram[256] = { 0 };
	int remap[256];
	int sum = 0;
	int j = 0;
	int k;
	int target;
	int i;

	cimg_forXY(data, x, y) {
		histogram[data(x, y, 0)]++;
	}


	for (i = 0; i < 256; i++) {
		sum += histogram[i];
		target = (sum * 255) / (width * height);
		for (k = j + 1; k <= target; k++)
			remap[k] = i;
		j = target;
	}

	for (i = 0; i < width * height; i++)
		data[i] = remap[data[i]];

	cimg_forXY(data, x, y) {
		data(x, y, 0) = remap[data(x, y, 0)];
	}
}

CImg<int> canny(CImg<int> grey, int width, int height) {
	return cannyparam(grey, width, height, 2.5f, 7.5f, 2.0f, 16, 0);
}

/*
Canny edge detection with parameters passed in by user
Params: grey - the greyscale image
width, height - image dimensions
lowthreshold - default 2.5
highthreshold - default 7.5
gaussiankernelradius - radius of edge detection Gaussian, in standard deviations
(default 2.0)
gaussiankernelwidth - width of Gaussian kernel, in pixels (default 16)
contrastnormalised - flag to normalise image before edge detection (defualt 0)
Returns: binary image with set pixels as edges

*/
CImg<int> cannyparam(CImg<int> grey, int width, int height,
	float lowthreshold, float highthreshold,
	float gaussiankernelradius, int gaussiankernelwidth,
	int contrastnormalised) {

	CANNY *can = 0;
	CImg<int> answer = CImg<int>(width, height, 1, 1, 0);
	int low, high;
	int err;
	int i;

	can = allocatebuffers(grey, width, height);

	if (!can)
		goto error_exit;
	if (contrastnormalised)
		normalizeContrast(can->data, width, height);

	err = computeGradients(can, gaussiankernelradius, gaussiankernelwidth);

	if (err < 0)
		goto error_exit;

	low = (int)(lowthreshold * MAGNITUDE_SCALE + 0.5f);
	high = (int)(highthreshold * MAGNITUDE_SCALE + 0.5f);
	performHysteresis(can, low, high);

	cimg_forXY(answer, x, y) {
		i = y * width + x;
		answer(x, y, 0) = can->idata[i] > 0 ? 255 : 0;
	}

	killbuffers(can);
	return answer;
error_exit:
	free(answer);
	killbuffers(can);
	return CImg<int>();
}


