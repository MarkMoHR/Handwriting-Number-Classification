#include "PaperModification.h"

PaperModification::PaperModification(const CImg<int>& _SrcImg) {
	SrcImg = CImg<int>(_SrcImg._width, _SrcImg._height, 1, 3, 0);
	imgW = _SrcImg._width;
	imgH = _SrcImg._height;
	cimg_forXY(SrcImg, x, y) {
		SrcImg(x, y, 0, 0) = _SrcImg(x, y, 0, 0);
		SrcImg(x, y, 0, 1) = _SrcImg(x, y, 0, 1);
		SrcImg(x, y, 0, 2) = _SrcImg(x, y, 0, 2);
	}
}

PaperModification::~PaperModification() {
}

void PaperModification::setVertexSet(const vector<Vertex>& _vertexSet, const double DownSampledSize) {
	for (int i = 0; i < _vertexSet.size(); i++) {
		int x = (int)round(_vertexSet[i].x * DownSampledSize);
		int y = (int)round(_vertexSet[i].y * DownSampledSize);
		Vertex newVertex(x, y);
		vertexSet.push_back(newVertex);
	}

	for (int i = 0; i < vertexSet.size(); i++) {
		cout << vertexSet[i].x << " , " << vertexSet[i].y << endl;
	}
}

CImg<int> PaperModification::getModifiedPaper() {
	//time_t tempS, tempE, tempEE;
	//tempS = time(NULL);
	//先调整顶点集
	resetVertexSet();

	//tempE = time(NULL);
	//cout << endl << endl << "ResetVertexSet: " << tempE - tempS << " seconds is used." << endl << endl;

	CImg<int> answer;
	if (paperType == PaperVert)
		answer = CImg<int>(A4width * Scalar, A4height * Scalar, 1, 3, 0);
	else
		answer = CImg<int>(A4height * Scalar, A4width * Scalar, 1, 3, 0);

	double x0, y0, x1, y1, x2, y2, x3, y3;
	x0 = vertexSet[0].x;
	y0 = vertexSet[0].y;
	x1 = vertexSet[1].x;
	y1 = vertexSet[1].y;
	x2 = vertexSet[2].x;
	y2 = vertexSet[2].y;
	x3 = vertexSet[3].x;
	y3 = vertexSet[3].y;

	double dx32 = x3 - x2;
	double dx21 = x2 - x1;
	double dy32 = y3 - y2;
	double dy21 = y2 - y1;

	double a, b, c, d, e, f, g, h;

	c = x0;
	f = y0;
	if (paperType == PaperVert) {
		h = ((dy32 + y1 - f) * dx21 - (dx32 + x1 - c) * dy21) / ((A4height * 2.0) * (dx32 * dy21 - dy32 * dx21));
		g = ((A4height * 2.0) * dx32 * h + (dx32 + x1 - c)) / ((A4width * 2.0) * dx21);
		a = (((A4width * 2.0) * g + 1) * x1 - c) / (A4width * 2.0);
		d = (((A4width * 2.0) * g + 1) * y1 - f) / (A4width * 2.0);
		b = (((A4height * 2.0) * h + 1) * x3 - c) / (A4height * 2.0);
		e = (((A4height * 2.0) * h + 1) * y3 - f) / (A4height * 2.0);
	}
	else {
		h = ((dy32 + y1 - f) * dx21 - (dx32 + x1 - c) * dy21) / ((A4width * 2.0) * (dx32 * dy21 - dy32 * dx21));
		g = ((A4width * 2.0) * dx32 * h + (dx32 + x1 - c)) / ((A4height * 2.0) * dx21);
		a = (((A4height * 2.0) * g + 1) * x1 - c) / (A4height * 2.0);
		d = (((A4height * 2.0) * g + 1) * y1 - f) / (A4height * 2.0);
		b = (((A4width * 2.0) * h + 1) * x3 - c) / (A4width * 2.0);
		e = (((A4width * 2.0) * h + 1) * y3 - f) / (A4width * 2.0);
	}
	
	cout << "a, b, c, d, e, f, g, h" << endl;
	cout << a << " " << b << " " << c << " " << d << " " << e 
		 << " " << f << " " << g << " " << h << " " << endl;


	cimg_forXY(answer, x, y) {
		double scrX = (a * (double)x + b * (double)y + c) / (g * (double)x + h * (double)y + 1);
		double scrY = (d * (double)x + e * (double)y + f) / (g * (double)x + h * (double)y + 1);
		vector<int> RGBvec = getInterpolationRGB(scrX, scrY);    //插值

		answer(x, y, 0, 0) = RGBvec[0];
		answer(x, y, 0, 1) = RGBvec[1];
		answer(x, y, 0, 2) = RGBvec[2];
	}

	//tempEE = time(NULL);
	//cout << endl << endl << "Interpolation: " << tempEE - tempE << " seconds is used." << endl << endl;


	return answer;
}

void PaperModification::resetVertexSet() {
	vector<Vertex> newVertexSet;

	//先找距离原点最近的点A
	int minPointA = getPointNearestToOrigin();

	//再找点A的对角点C
	int crossPointC = getPointAtCrossline(minPointA);

	//然后根据A、C点计算纸的横/竖/斜
	computeThePaperType(minPointA, crossPointC);

	//再根据纸的横/竖/斜找到B、D两点
	int pointB = findPointB(minPointA, crossPointC);
	int pointD;
	for (int i = 0; i < 4; i++) {
		if (i != minPointA && i != crossPointC && i != pointB) {
			pointD = i;
			break;
		}
	}

	//按顺序摆放
	newVertexSet.push_back(vertexSet[minPointA]);
	newVertexSet.push_back(vertexSet[pointB]);
	newVertexSet.push_back(vertexSet[crossPointC]);
	newVertexSet.push_back(vertexSet[pointD]);
	vertexSet.clear();
	for (int i = 0; i < 4; i++) {
		vertexSet.push_back(newVertexSet[i]);
		cout << "Point" << i << ": " << vertexSet[i].x << " , " << vertexSet[i].y << endl;
	}
}
int PaperModification::getPointNearestToOrigin() {
	int minDist, minPoint;
	for (int i = 0; i < 4; i++) {
		int dist = vertexSet[i].x * vertexSet[i].x + vertexSet[i].y * vertexSet[i].y;
		if (i == 0) {
			minDist = dist;
			minPoint = i;
		}
		else {
			if (dist < minDist) {
				minDist = dist;
				minPoint = i;
			}
		}
	}
	return minPoint;
}
int PaperModification::getPointAtCrossline(int point) {
	int crossPoint = point;
	for (int i = 0; i < 4; i++) {   //第point个点与第i个点连线
		if (i == point)
			continue;

		double temp_k = (double)(vertexSet[i].y - vertexSet[point].y) / (double)(vertexSet[i].x - vertexSet[point].x);
		double temp_b = (double)vertexSet[point].y - temp_k * (double)vertexSet[point].x;

		int flag = 0;  //标志为正还是为负
		for (int j = 0; j < 4; j++) {
			if (j != i && j != point) {
				//第j个点的y坐标减线上坐标
				double diff = (double)vertexSet[j].y - (temp_k * (double)vertexSet[j].x + temp_b);
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
		if (crossPoint != point)
			break;
	}
	return crossPoint;
}
void PaperModification::computeThePaperType(int pointA, int pointC) {
	//通过AC斜率判断
	double k_ac = (double)(vertexSet[pointA].y - vertexSet[pointC].y) / (double)(vertexSet[pointA].x - vertexSet[pointC].x);
	if (abs(k_ac) < 1) {    //横
		paperType = PaperHori;
	}
	else {
		//再找到另外两个点计算斜率判断
		int y3, y4, x3, x4;
		y3 = y4 = x3 = x4 = -1;
		for (int i = 0; i < 4; i++) {
			if (i != pointA && i != pointC) {
				if (y3 == -1) {
					y3 = vertexSet[i].y;
					x3 = vertexSet[i].x;
				}
				else {
					y4 = vertexSet[i].y;
					x4 = vertexSet[i].x;
				}
			}
		}
		double k_bd = (double)(y3 - y4) / (double)(x3 - x4);
		if (abs(k_bd) < 1)    //斜
			paperType = PaperDiag;
		else
			paperType = PaperVert;    //竖
	}
}
int PaperModification::findPointB(int pointA, int pointC) {
	int pointB;
	int point3, point4;
	point3 = point4 = -1;
	for (int i = 0; i < 4; i++) {
		if (i != pointA && i != pointC) {
			if (point3 == -1)
				point3 = i;
			else
				point4 = i;
		}
	}

	double dist3 = pow((vertexSet[point3].x - vertexSet[pointA].x), 2.0) + 
		pow((vertexSet[point3].y - vertexSet[pointA].y), 2.0);
	double dist4 = pow((vertexSet[point4].x - vertexSet[pointA].x), 2.0) +
		pow((vertexSet[point4].y - vertexSet[pointA].y), 2.0);

	if (paperType == PaperVert) {    //竖，为离A近点
		pointB = dist3 < dist4 ? point3 : point4;
	}
	else {    //横/斜，为离A远点
		pointB = dist3 > dist4 ? point3 : point4;
	}

	return pointB;
}

vector<int> PaperModification::getInterpolationRGB(double scrX, double scrY) {
	vector<int> RGBvec;
	double scrX_head = floor(scrX);
	double scrY_head = floor(scrY);
	double scrX_tail = 0, scrY_tail = 0;

	int r00 = SrcImg((int)scrX_head, (int)scrY_head, 0, 0);
	int g00 = SrcImg((int)scrX_head, (int)scrY_head, 0, 1);
	int b00 = SrcImg((int)scrX_head, (int)scrY_head, 0, 2);
	int r01, r10, r11, g01, g10, g11, b01, b10, b11;
	r01 = r10 = r11 = g01 = g10 = g11 = b01 = b10 = b11 = 0;

	if ((int)scrY_head < SrcImg._height - 1) {
		scrY_tail = scrY - scrY_head;
		r01 = SrcImg((int)scrX_head, (int)scrY_head + 1, 0, 0);
		g01 = SrcImg((int)scrX_head, (int)scrY_head + 1, 0, 1);
		b01 = SrcImg((int)scrX_head, (int)scrY_head + 1, 0, 2);
	}
	if ((int)scrX_head < SrcImg._width - 1) {
		scrX_tail = scrX - scrX_head;
		r10 = SrcImg((int)scrX_head + 1, (int)scrY_head, 0, 0);
		g10 = SrcImg((int)scrX_head + 1, (int)scrY_head, 0, 1);
		b10 = SrcImg((int)scrX_head + 1, (int)scrY_head, 0, 2);
	}
	if (((int)scrY_head < SrcImg._height - 1) && ((int)scrX_head < SrcImg._width - 1)) {
		r11 = SrcImg((int)scrX_head + 1, (int)scrY_head + 1, 0, 0);
		g11 = SrcImg((int)scrX_head + 1, (int)scrY_head + 1, 0, 1);
		b11 = SrcImg((int)scrX_head + 1, (int)scrY_head + 1, 0, 2);
	}

	double scrX_tail_anti = (double)1 - scrX_tail;
	double scrY_tail_anti = (double)1 - scrY_tail;

	double temp_r = (double)r00 * scrX_tail_anti * scrY_tail_anti
		+ (double)r01 * scrX_tail_anti * scrY_tail
		+ (double)r10 * scrX_tail * scrY_tail_anti
		+ (double)r11 * scrX_tail * scrY_tail;
	double temp_g = (double)g00 * scrX_tail_anti * scrY_tail_anti
		+ (double)g01 * scrX_tail_anti * scrY_tail
		+ (double)g10 * scrX_tail * scrY_tail_anti
		+ (double)g11 * scrX_tail * scrY_tail;
	double temp_b = (double)b00 * scrX_tail_anti * scrY_tail_anti
		+ (double)b01 * scrX_tail_anti * scrY_tail
		+ (double)b10 * scrX_tail * scrY_tail_anti
		+ (double)b11 * scrX_tail * scrY_tail;
	int new_r = (int)floor(temp_r);
	int new_g = (int)floor(temp_g);
	int new_b = (int)floor(temp_b);

	RGBvec.push_back(new_r);
	RGBvec.push_back(new_g);
	RGBvec.push_back(new_b);

	return RGBvec;
}