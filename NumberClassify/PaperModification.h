#ifndef PAPERMODIFICATION_H
#define PAPERMODIFICATION_H

#include "CImg.h"
#include "HoughEdgeDetect.h"
#include <iostream>
#include <vector>
#include <set>
using namespace std;
using namespace cimg_library;

#define A4width 210    //A4纸宽
#define A4height 297   //A4纸高
#define Scalar 2       //A4纸矫正图像对于A4纸宽/高的倍数

enum PaperType { PaperHori, PaperVert, PaperDiag };

class PaperModification {
public:
	PaperModification(const CImg<int>& _SrcImg);
	~PaperModification();

	void setVertexSet(const vector<Vertex>& _vertexSet, const double DownSampledSize);    //设置顶点集
	CImg<int> getModifiedPaper();                           //获取A4纸矫正图主函数

private:
	CImg<int> SrcImg;
	int imgW, imgH;
	int paperType = -1;          //a4纸样式：0为横着、1为竖着、2为斜着
	vector<Vertex> vertexSet;    //顶点集

	void resetVertexSet();                                       //识别4个点的位置并按顺序放好
	int getPointNearestToOrigin();                               //获取距离最近的点A
	int getPointAtCrossline(int point);                          //获取点A的对角点C
	void computeThePaperType(int pointA, int pointC);            //计算纸是竖/横/斜
	int findPointB(int pointA, int pointC);                      //求点B
	vector<int> getInterpolationRGB(double scrX, double scrY);   //双线性插值获取RGB值
};


#endif