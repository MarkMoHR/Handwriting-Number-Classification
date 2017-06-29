#ifndef HOUGHEDGEDETECT_H
#define HOUGHEDGEDETECT_H

#include "CImg.h"
#include <iostream>
#include <vector>
#include <set>
using namespace std;
using namespace cimg_library;

#define Pi 3.14
#define ThetaScale 10
#define RhoScale 1

#define TopK 13          //找线交叉最多的TopK个θ、ρ参数对
#define VertexGap 36     //直线交点，距离在VertexGap内，视作同一个点
#define DownSampledSquareSize 500.0

typedef multiset<int, less<int>> IntHeap;

struct Vertex {
	int x;
	int y;
	int crossTimes = 0;
	Vertex(int posX, int posY): x(posX), y(posY) {}
	void setXY(int _x, int _y) { 
		x = _x;
		y = _y;
	}
	void addCrossTimes() {
		crossTimes++;
	}
};

class HoughEdgeDetect {
public:
	HoughEdgeDetect(const CImg<int>& CannyImg);
	~HoughEdgeDetect();

	void HoughTransAndDetectEdge();               //边缘检测处理主要操作
	
	CImg<int> getHoughSpaceImage();               //获取HoughSpace的图像
	CImg<int> getCannyGrayImageWithEdge();        //获取画出边缘的灰度图

	CImg<int> getFinallyProcessedImage(const CImg<int>& SrcImg);  //获取最后处理完成的彩色原大小图像

	vector<Vertex> getTopFourVertexSet();
	double getDownSampledSize();

private:
	CImg<int> SrcCannyImg;
	int imgW, imgH;
	int **houghArray;

	IntHeap myHeap;
	double k_TopList[TopK]{ 0 };
	double b_TopList[TopK]{ 0 };
	vector<Vertex> vertexSet;
	vector<Vertex> top4vertexSet;

	double DownSampledSize = 1;

	void accumulateTheHoughArray(int x, int y);   //对HoughSpace矩阵做累加
	void findTheTopKParaPair();                   //找线交叉最多的K个rho/theta参数对
	void modifyTheParaArray(int i, int j);
	void findTheNearest4Points();                 //找到最接近的4个顶点
	void getValidCrossPointAndIncrement(double k1, double b1, double k2, double b2);
	void drawLinesBetweenVertex(CImg<int>& img);
};


#endif