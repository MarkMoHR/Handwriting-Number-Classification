#ifndef SIMPLEIMGPROCESS_H
#define SIMPLEIMGPROCESS_H

#include "CImg.h"
using namespace cimg_library;

//彩色图像转化为灰度图
CImg<int> grayTheImg(const CImg<int>& SrcImg);
//对灰度图进行下采样
CImg<int> downSampleTheImg(const CImg<int>& SrcGrayImg, float sampleSquareSize);


#endif