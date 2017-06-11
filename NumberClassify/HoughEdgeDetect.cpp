#include "HoughEdgeDetect.h"
#include <math.h>

HoughEdgeDetect::HoughEdgeDetect(const CImg<int>& CannyImg) {
	SrcCannyImg = CImg<int>(CannyImg._width, CannyImg._height, 1, 1, 0);
	imgW = SrcCannyImg._width;
	imgH = SrcCannyImg._height;
	cimg_forXY(CannyImg, x, y) {
		SrcCannyImg(x, y, 0, 0) = CannyImg(x, y, 0, 0);
	}

	houghArray = new int*[(imgW + imgH) * 2 / RhoScale];
	for (int i = 0; i < (imgW + imgH) * 2 / RhoScale; i++) {
		houghArray[i] = new int[360]{ 0 };
	}
}

HoughEdgeDetect::~HoughEdgeDetect() {
	for (int j = 0; j < (imgW + imgH) * 2 / RhoScale; j++) {
		delete[]houghArray[j];
	}
	delete[]houghArray;
}

void HoughEdgeDetect::HoughTransAndDetectEdge() {
	cimg_forXY(SrcCannyImg, x, y) {
		if (SrcCannyImg(x, y, 0) == 255) {
			//检测到是边缘点时，做hough transform并对矩阵累加
			accumulateTheHoughArray(x, y);
		}
	}

	findTheTopKParaPair();
	findTheNearest4Points();
}

//对HoughSpace矩阵做累加
void HoughEdgeDetect::accumulateTheHoughArray(int x, int y) {
	for (int i = 0; i < 360; i++) {
		double theta = (double)i * Pi / 180.0;
		int rho = (int)round(sin(theta) * (double)y + cos(theta) * (double)x);
		if (rho < imgW + imgH) {
			houghArray[(rho + imgW + imgH) / RhoScale][i]++;
			//cout << "houghArray[" << (rho + imgW + imgH) / RhoScale << "][" << i << "] = "
			//	<< houghArray[(rho + imgW + imgH) / RhoScale][i] << endl;
		}
	}
}

//找线交叉最多的TopK个θ、ρ参数对
void HoughEdgeDetect::findTheTopKParaPair() {
	for (int i = 0; i < (imgW + imgH) * 2 / RhoScale; i++) {
		for (int j = 0; j < 360; j++) {
			if (houghArray[i][j] != 0) {
				if (myHeap.size() < TopK) {  //堆里不够TopK个数，直接插入
					myHeap.insert(houghArray[i][j]);

					modifyTheParaArray(i, j);
				}
				else {  //堆里够了TopK个数，若检测到比堆顶大的数，先移除堆顶元素再插入
					IntHeap::iterator pIter = myHeap.begin();
					if (houghArray[i][j] > *pIter) {
						myHeap.erase(pIter);
						myHeap.insert(houghArray[i][j]);

						modifyTheParaArray(i, j);
					}
				}
			}
		}
	}
}

//根据θ、ρ参数获得b、m并插入数组
void HoughEdgeDetect::modifyTheParaArray(int i, int j) {
	int index = 0;
	IntHeap::iterator tIter = myHeap.begin();
	//先获取在堆里插入的位置index
	while (tIter != myHeap.end()) {
		if (*tIter == houghArray[i][j]) {
			break;
		}
		tIter++;
		index++;
	}

	for (int k = 0; k < index; k++) {
		k_TopList[k] = k_TopList[k + 1];
		b_TopList[k] = b_TopList[k + 1];
	}
	k_TopList[index] = -1.0 / tan((double)j * Pi / 180.0);
	b_TopList[index] = (double)(i * RhoScale - imgW - imgH) / sin((double)j * Pi / 180.0);
}

void HoughEdgeDetect::findTheNearest4Points() {
	//先对每个交点做相交次数累加
	for (int i = 0; i < TopK; i++) {
		double k1 = k_TopList[i];
		double b1 = b_TopList[i];

		for (int j = 0; j < TopK; j++) {
			if (i != j) {
				double k2 = k_TopList[j];
				double b2 = b_TopList[j];
				getValidCrossPointAndIncrement(k1, b1, k2, b2);
			}
		}
	}

	for (int i = 0; i < vertexSet.size(); i++) {
		cout << "vertexSet : (" << vertexSet[i].x << " , " << vertexSet[i].y << ") : " << vertexSet[i].crossTimes << endl;
	}

	//找相交次数最多的4个点
	int max = 0;
	int maxUnder = -1;
	while (top4vertexSet.size() < 4) {
		max = 0;
		for (int i = 0; i < vertexSet.size(); i++) {
			if (vertexSet[i].crossTimes > max) {
				max = vertexSet[i].crossTimes;
				maxUnder = i;
			}
		}
		top4vertexSet.push_back(vertexSet[maxUnder]);
		vertexSet[maxUnder].crossTimes = -1;
	}

	cout << endl;
	for (int i = 0; i < top4vertexSet.size(); i++) {
		cout << "top4vertexSet : (" << top4vertexSet[i].x << " , " << top4vertexSet[i].y << ") : " << top4vertexSet[i].crossTimes << endl;
	}
}

//根据两条直线的斜率、截距获取交点，检测是否在图像区域内
//对交点做交叉次数做累加
void HoughEdgeDetect::getValidCrossPointAndIncrement(double k1, double b1, double k2, double b2) {
	double xd = (b2 - b1) / (k1 - k2);
	int x = (int)round(xd);
	int y = (int)round(xd * k1 + b1);
	if (x >= 0 && x <= imgW && y >= 0 && y <= imgH) {  //在图像区域内
		int i = 0;
		for (i = 0; i < vertexSet.size(); i++) {
			int oldX = vertexSet[i].x;
			int oldY = vertexSet[i].y;

			//附近有特别靠近的点，可以合并
			if ((oldX - x) * (oldX - x) + (oldY - y) * (oldY - y) <= VertexGap) {
				vertexSet[i].addCrossTimes();
				break;
			}
		}
		if (i == vertexSet.size()) {  //如果该点附近没有距离特别近的点，自身作为一个新点
			Vertex newVertex(x, y);
			vertexSet.push_back(newVertex);
		}
	}
}

CImg<int> HoughEdgeDetect::getHoughSpaceImage() {
	CImg<int> answer = CImg<int>(360, (imgW + imgH) * 2 / RhoScale, 1, 1, 0);
	cimg_forXY(answer, x, y) {
		answer(x, y, 0) = houghArray[y][x];
	}
	return answer;
}

CImg<int> HoughEdgeDetect::getCannyGrayImageWithEdge() {
	CImg<int> answer = CImg<int>(SrcCannyImg._width, SrcCannyImg._height, 1, 3, 0);
	cimg_forXY(answer, x, y) {
		answer(x, y, 0) = SrcCannyImg(x, y, 0);
		answer(x, y, 1) = SrcCannyImg(x, y, 0);
		answer(x, y, 2) = SrcCannyImg(x, y, 0);
	}

	const double yellow[] = { 255, 255, 0 };
	const double green[] = { 0, 255, 255 };
	const double red[] = { 255, 0, 0 };
	const double purple[] = { 255, 0, 255 };
	for (int i = 0; i < TopK; i++) {
		double k = k_TopList[i];
		double b = b_TopList[i];
		//cout << "k = " << k << " , b = " << b << endl;

		//根据斜率、截距得到在图像区域内的两个点，然后画直线
		if (abs(k) < 1) {
			if (i >= TopK - 4)
				answer.draw_line(0, (int)round(b), imgW, (int)(round((double)imgW * k + b)), green);
			else
				answer.draw_line(0, (int)round(b), imgW, (int)(round((double)imgW * k + b)), purple);
		}
		else {   //abs(k) >= 1
			if (i >= TopK - 4)
				answer.draw_line((int)round(-b / k), 0, (int)round(((double)imgH - b) / k), imgH, green);
			else
				answer.draw_line((int)round(-b / k), 0, (int)round(((double)imgH - b) / k), imgH, purple);
		}
	}


	//相交次数最多的4个点，描出来
	for (int i = 0; i < top4vertexSet.size(); i++) {
		answer.draw_circle(top4vertexSet[i].x, top4vertexSet[i].y, 5, yellow, 1.0f);
	}

	return answer;
}

CImg<int> HoughEdgeDetect::getFinallyProcessedImage(const CImg<int>& SrcImg) {
	CImg<int> answer = CImg<int>(SrcImg._width, SrcImg._height, 1, 3, 0);
	cimg_forXY(answer, x, y) {
		answer(x, y, 0) = SrcImg(x, y, 0, 0);
		answer(x, y, 1) = SrcImg(x, y, 0, 1);
		answer(x, y, 2) = SrcImg(x, y, 0, 2);
	}

	DownSampledSize = (double)(SrcImg._width + SrcImg._height) / DownSampledSquareSize / 2.0;

	const double yellow[] = { 255, 255, 0 };
	for (int i = 0; i < top4vertexSet.size(); i++) {
		int x = (int)round(top4vertexSet[i].x * DownSampledSize);
		int y = (int)round(top4vertexSet[i].y * DownSampledSize);
		int radius = (int)round(DownSampledSize * 5);
		answer.draw_circle(x, y, radius, yellow, 1.0f);
	}

	drawLinesBetweenVertex(answer);

	return answer;
}

//给4个顶点之间画直线
void HoughEdgeDetect::drawLinesBetweenVertex(CImg<int>& img) {
	const double yellow[] = { 255, 255, 0 };
	int crossPoint = 0;

	for (int i = 1; i < 4; i++) {   //第0个点与第i个点连线
		double temp_k = (double)(top4vertexSet[i].y - top4vertexSet[0].y) / (double)(top4vertexSet[i].x - top4vertexSet[0].x);
		double temp_b = (double)top4vertexSet[0].y - temp_k * (double)top4vertexSet[0].x;

		int flag = 0;  //标志为正还是为负
		for (int j = 1; j < 4; j++) {
			if (j != i) {
				//第j个点的y坐标减线上坐标
				double diff = (double)top4vertexSet[j].y - (temp_k * (double)top4vertexSet[j].x + temp_b);
				if (flag == 0) {
					flag = diff > 0 ? 1 : -1;
				}
				else {
					if (flag == 1 && diff <= 0 || flag == -1 && diff > 0) {
						crossPoint = i;
						break;
					}
				}
			}
		}
		if (crossPoint != 0)
			break;
	}

	cout << "crossPoint = " << crossPoint << endl;
	for (int i = 1; i < 4; i++) {
		if (i != crossPoint) {
			double k = (double)(top4vertexSet[i].y - top4vertexSet[0].y) / (double)(top4vertexSet[i].x - top4vertexSet[0].x);
			double b = (double)top4vertexSet[0].y - k * (double)top4vertexSet[0].x;
			cout << "Line: y = " << k << "x + " << b << endl;

			img.draw_line(top4vertexSet[i].x * DownSampledSize, top4vertexSet[i].y * DownSampledSize,
				top4vertexSet[0].x * DownSampledSize, top4vertexSet[0].y * DownSampledSize, yellow);
			//加粗
			img.draw_line(top4vertexSet[i].x * DownSampledSize + 1, top4vertexSet[i].y * DownSampledSize,
				top4vertexSet[0].x * DownSampledSize + 1, top4vertexSet[0].y * DownSampledSize, yellow);

			k = (double)(top4vertexSet[i].y - top4vertexSet[crossPoint].y) / (double)(top4vertexSet[i].x - top4vertexSet[crossPoint].x);
			b = (double)top4vertexSet[crossPoint].y - k * (double)top4vertexSet[crossPoint].x;
			cout << "Line: y = " << k << "x + " << b << endl;
			img.draw_line(top4vertexSet[i].x * DownSampledSize, top4vertexSet[i].y * DownSampledSize,
				top4vertexSet[crossPoint].x * DownSampledSize, top4vertexSet[crossPoint].y * DownSampledSize, yellow);
			//加粗
			img.draw_line(top4vertexSet[i].x * DownSampledSize + 1, top4vertexSet[i].y * DownSampledSize,
				top4vertexSet[crossPoint].x * DownSampledSize + 1, top4vertexSet[crossPoint].y * DownSampledSize, yellow);
		}
	}
}

vector<Vertex> HoughEdgeDetect::getTopFourVertexSet() {
	return top4vertexSet;
}

double HoughEdgeDetect::getDownSampledSize() {
	return DownSampledSize;
}