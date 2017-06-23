#define _CRT_SECURE_NO_WARNINGS

#include "ImageSegmentation.h"


ImageSegmentation::ImageSegmentation(const CImg<int>& _SrcImg) {
	SrcGrayImg = CImg<int>(_SrcImg._width, _SrcImg._height, 1, 1, 0);
	imgW = _SrcImg._width;
	imgH = _SrcImg._height;
	cimg_forXY(SrcGrayImg, x, y) {
		SrcGrayImg(x, y, 0) = _SrcImg(x, y, 0);
	}
}

ImageSegmentation::~ImageSegmentation() {
}

//XY方向的正扩张
int getDilationIntensityXY(const CImg<int>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (0 <= x + i && x + i < Img._width
					&& 0 <= y + j && y + j < Img._height) {
					if (i != -1 && j != -1 || i != 1 && j != 1 || i != 1 && j != -1 || i != -1 && j != 1)
						if (Img(x + i, y + j, 0) == 0) {
							intensity = 0;
							break;
						}
				}
			}
			if (intensity != 255)
				break;
		}
	}
	return intensity;
}
//X方向的正扩张
int getDilationIntensityX(const CImg<int>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {
		for (int i = -1; i <= 1; i++) {
			if (0 <= x + i && x + i < Img._width) {
				if (Img(x + i, y, 0) == 0) {
					intensity = 0;
					break;
				}
			}
		}
	}
	return intensity;
}
//Y方向的正扩张
int getDilationIntensityY(const CImg<int>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {
		for (int j = -1; j <= 1; j++) {
			if (0 <= y + j && y + j < Img._height) {
				if (Img(x, y + j, 0) == 0) {
					intensity = 0;
					break;
				}
			}
		}
	}
	return intensity;
}

//X方向2个单位的负扩张，Y方向2个单位的正扩张
int getDilationIntensityXXYY(const CImg<int>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {    //若中间点为白色
		int blackAccu = 0;

		for (int i = -2; i <= 2; i++) {
			if (0 <= y + i && y + i < Img._height) {    //垂直方向累加
				if (Img(x, y + i, 0) == 0)
					blackAccu++;
			}
			if (0 <= x + i && x + i < Img._width) {     //水平方向累减
				if (Img(x + i, y, 0) == 0)
					blackAccu--;
			}
		}

		if (blackAccu > 0)
			intensity = 0;
	}
	return intensity;
}

//X方向2个单位的负扩张，Y方向1个单位的正扩张
int getDilationIntensityXXY(const CImg<int>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {    //若中间点为白色
		int blackAccu = 0;

		for (int i = -1; i <= 1; i++) {
			if (0 <= y + i && y + i < Img._height) {    //垂直方向累加
				if (Img(x, y + i, 0) == 0)
					blackAccu++;
			}
		}
		for (int i = -2; i <= 2; i++) {
			if (0 <= x + i && x + i < Img._width) {     //水平方向累减
				if (Img(x + i, y, 0) == 0)
					blackAccu--;
			}
		}

		if (blackAccu > 0)
			intensity = 0;
	}
	return intensity;
}

//X方向1个单位的负扩张，Y方向1个单位的正扩张
int getDilationIntensityXrY(const CImg<int>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {    //若中间点为白色
		int blackAccu = 0;

		for (int i = -1; i <= 1; i++) {
			if (0 <= y + i && y + i < Img._height) {    //垂直方向累加
				if (Img(x, y + i, 0) == 0)
					blackAccu++;
			}
			if (0 <= x + i && x + i < Img._width) {     //水平方向累减
				if (Img(x + i, y, 0) == 0)
					blackAccu--;
			}
		}

		if (blackAccu > 0)
			intensity = 0;
	}
	return intensity;
}

void ImageSegmentation::processBinaryImage() {
	CImg<int> answer = CImg<int>(SrcGrayImg._width, SrcGrayImg._height, 1, 1, 0);

	cimg_forXY(answer, x, y) {
		int intensity = SrcGrayImg(x, y, 0);

		//先去掉黑色边缘
		if (x <= BoundaryRemoveGap || y <= BoundaryRemoveGap
			|| x >= imgW - BoundaryRemoveGap || y >= imgH - BoundaryRemoveGap) {
			answer(x, y, 0) = 255;
		}
		else {
			if (intensity < BinaryGap)
				answer(x, y, 0) = 0;
			else
				answer(x, y, 0) = 255;
		}
	}

	BinaryImg = answer;
}

CImg<int> ImageSegmentation::getBinaryImage() {
	return BinaryImg;
}

void ImageSegmentation::numberSegmentationMainProcess(const string baseAddress) {
	basePath = baseAddress;

	findDividingLine();
	divideIntoBarItemImg();

	connectedRegionsTagging();
}

void ImageSegmentation::findDividingLine() {
	HistogramImage = CImg<int>(BinaryImg._width, BinaryImg._height, 1, 3, 0);
	DividingImg = CImg<int>(BinaryImg._width, BinaryImg._height, 1, 3, 0);
	int lineColor[3]{ 255, 0, 0 };

	cimg_forY(HistogramImage, y) {
		int blackPixel = 0;
		cimg_forX(BinaryImg, x) {
			HistogramImage(x, y, 0) = 255;
			HistogramImage(x, y, 1) = 255;
			HistogramImage(x, y, 2) = 255;
			DividingImg(x, y, 0) = BinaryImg(x, y, 0);
			DividingImg(x, y, 1) = BinaryImg(x, y, 0);
			DividingImg(x, y, 2) = BinaryImg(x, y, 0);
			if (BinaryImg(x, y, 0) == 0)
				blackPixel++;
		}

		cimg_forX(HistogramImage, x) {
			if (x < blackPixel) {
				HistogramImage(x, y, 0) = 0;
				HistogramImage(x, y, 1) = 0;
				HistogramImage(x, y, 2) = 0;
			}
		}

		//判断是否为拐点
		if (y > 0) {
			if (blackPixel <= HistogramValleyMaxPixelNumber 
				&& HistogramImage(HistogramValleyMaxPixelNumber, y - 1, 0) == 0) {    //下白上黑：取下
				inflectionPointSet.push_back(y);
				//HistogramImage.draw_line(0, y, HistogramImage._width - 1, y, lineColor);
			}
			else if (blackPixel > HistogramValleyMaxPixelNumber
				&& HistogramImage(HistogramValleyMaxPixelNumber, y - 1, 0) != 0) {    //下黑上白：取上
				inflectionPointSet.push_back(y - 1);
				//HistogramImage.draw_line(0, y - 1, HistogramImage._width - 1, y - 1, lineColor);
			}
		}
	}

	divideLinePointSet.push_back(-1);
	//两拐点中间做分割
	if (inflectionPointSet.size() > 2) {
		for (int i = 1; i < inflectionPointSet.size() - 1; i = i + 2) {
			int divideLinePoint = (inflectionPointSet[i] + inflectionPointSet[i + 1]) / 2;
			divideLinePointSet.push_back(divideLinePoint);
		}
	}
	divideLinePointSet.push_back(BinaryImg._height - 1);
}

void ImageSegmentation::divideIntoBarItemImg() {
	vector<int> newDivideLinePointSet;
	int lineColor[3]{ 255, 0, 0 };

	for (int i = 1; i < divideLinePointSet.size(); i++) {
		int barHright = divideLinePointSet[i] - divideLinePointSet[i - 1];
		int blackPixel = 0;
		CImg<int> barItemImg = CImg<int>(BinaryImg._width, barHright, 1, 1, 0);
		cimg_forXY(barItemImg, x, y) {
			barItemImg(x, y, 0) = BinaryImg(x, divideLinePointSet[i - 1] + 1 + y, 0);
			if (barItemImg(x, y, 0) == 0)
				blackPixel++;
		}

		double blackPercent = (double)blackPixel / (double)(BinaryImg._width * barHright);
		cout << "blackPercent " << blackPercent << endl;
		if (blackPercent > SubImgBlackPixelPercentage) {
			subImageSet.push_back(barItemImg);
			newDivideLinePointSet.push_back(divideLinePointSet[i - 1]);
			//barItemImg.display("barItemImg");

			if (i > 1) {
				HistogramImage.draw_line(0, divideLinePointSet[i - 1], 
					HistogramImage._width - 1, divideLinePointSet[i - 1], lineColor);
				DividingImg.draw_line(0, divideLinePointSet[i - 1], 
					HistogramImage._width - 1, divideLinePointSet[i - 1], lineColor);
			}
		}
	}
	divideLinePointSet.clear();
	for (int i = 0; i < newDivideLinePointSet.size(); i++)
		divideLinePointSet.push_back(newDivideLinePointSet[i]);
}

void ImageSegmentation::connectedRegionsTagging() {
	//对每张子图操作
	for (int i = 0; i < subImageSet.size(); i++) {
		doDilationForEachBarItemImg(i);
		connectedRegionsTaggingOfBarItemImg(i);

		saveSingleNumberImageAndImglist(i);
		cout << imglisttxt.c_str() << endl;
	}

	ofstream predictImageListOutput(basePath + "predict_imagelist.txt");
	predictImageListOutput << imglisttxt.c_str();
	predictImageListOutput.close();
}

void ImageSegmentation::doDilationForEachBarItemImg(int barItemIndex) {
	//扩张Dilation -X-X-X-XYY方向
	CImg<int> answerXXY = CImg<int>(subImageSet[barItemIndex]._width, subImageSet[barItemIndex]._height, 1, 1, 0);
	cimg_forXY(subImageSet[barItemIndex], x, y) {
		int intensity = getDilationIntensityXXY(subImageSet[barItemIndex], x, y);
		answerXXY(x, y, 0) = intensity;
	}

	//扩张Dilation -X-X-X-XYY方向
	CImg<int> answerXXY2 = CImg<int>(answerXXY._width, answerXXY._height, 1, 1, 0);
	cimg_forXY(answerXXY, x, y) {
		int intensity = getDilationIntensityXXY(answerXXY, x, y);
		answerXXY2(x, y, 0) = intensity;
	}

	//扩张Dilation XY方向
	CImg<int> answerXY = CImg<int>(answerXXY2._width, answerXXY2._height, 1, 1, 0);
	cimg_forXY(answerXXY2, x, y) {
		int intensity = getDilationIntensityXY(answerXXY2, x, y);
		answerXY(x, y, 0) = intensity;
	}

	cimg_forXY(subImageSet[barItemIndex], x, y) {
		subImageSet[barItemIndex](x, y, 0) = answerXY(x, y, 0);
	}
}

void ImageSegmentation::connectedRegionsTaggingOfBarItemImg(int barItemIndex) {
	TagImage = CImg<int>(subImageSet[barItemIndex]._width, subImageSet[barItemIndex]._height, 1, 1, 0);
	tagAccumulate = -1;

	cimg_forX(subImageSet[barItemIndex], x)
		cimg_forY(subImageSet[barItemIndex], y) {
		//第一行和第一列
		if (x == 0 || y == 0) {
			int intensity = subImageSet[barItemIndex](x, y, 0);
			if (intensity == 0) {
				addNewClass(x, y, barItemIndex);
			}
		}
		//其余的行和列
		else {
			int intensity = subImageSet[barItemIndex](x, y, 0);
			if (intensity == 0) {
				//检查正上、左上、左中、左下这四个邻点

				int minTag = Infinite;        //最小的tag
				PointPos minTagPointPos(-1, -1);

				//先找最小的标记
				findMinTag(x, y, minTag, minTagPointPos, barItemIndex);

				//当正上、左上、左中、左下这四个邻点有黑色点时，合并；
				if (minTagPointPos.x != -1 && minTagPointPos.y != -1) {
					mergeTagImageAndList(x, y - 1, minTag, minTagPointPos, barItemIndex);
					for (int i = -1; i <= 1; i++) {
						if (y + i < subImageSet[barItemIndex]._height)
							mergeTagImageAndList(x - 1, y + i, minTag, minTagPointPos, barItemIndex);
					}

					//当前位置
					TagImage(x, y, 0) = minTag;
					PointPos cPoint(x, y + divideLinePointSet[barItemIndex] + 1);
					pointPosListSet[minTag].push_back(cPoint);

				}
				//否则，作为新类
				else {
					addNewClass(x, y, barItemIndex);
				}
			}
		}
	}
}

void ImageSegmentation::addNewClass(int x, int y, int barItemIndex) {
	tagAccumulate++;
	//cout << "tagAccumulate " << tagAccumulate << endl;
	TagImage(x, y, 0) = tagAccumulate;
	classTagSet.push_back(tagAccumulate);
	list<PointPos> pList;
	PointPos cPoint(x, y + divideLinePointSet[barItemIndex] + 1);
	pList.push_back(cPoint);
	pointPosListSet.push_back(pList);
}

void ImageSegmentation::findMinTag(int x, int y, int &minTag, PointPos &minTagPointPos, int barItemIndex) {
	if (subImageSet[barItemIndex](x, y - 1, 0) == 0) {     //正上
		if (TagImage(x, y - 1, 0) < minTag) {
			minTag = TagImage(x, y - 1, 0);
			minTagPointPos.x = x;
			minTagPointPos.y = y - 1;
		}
	}
	for (int i = -1; i <= 1; i++) {        //左上、左中、左下
		if (y + i < subImageSet[barItemIndex]._height) {
			if (subImageSet[barItemIndex](x - 1, y + i, 0) == 0 && TagImage(x - 1, y + i, 0) < minTag) {
				minTag = TagImage(x - 1, y + i, 0);
				minTagPointPos.x = x - 1;
				minTagPointPos.y = y + i;
			}
		}
	}
}

void ImageSegmentation::mergeTagImageAndList(int x, int y, const int minTag, const PointPos minTagPointPos, int barItemIndex) {
	//赋予最小标记，归并列表
	if (subImageSet[barItemIndex](x, y, 0) == 0) {
		int tagBefore = TagImage(x, y, 0);
		if (tagBefore != minTag) {    //不是最小的tag
			//把所有同一类的tag替换为最小tag、把list接到最小tag的list
			list<PointPos>::iterator it = pointPosListSet[tagBefore].begin();
			for (; it != pointPosListSet[tagBefore].end(); it++) {
				TagImage((*it).x, (*it).y - divideLinePointSet[barItemIndex] - 1, 0) = minTag;
			}
			pointPosListSet[minTag].splice(pointPosListSet[minTag].end(), pointPosListSet[tagBefore]);
		}
	}
}


CImg<int> do_Y_Erosion(CImg<int>& Img) {
	CImg<int> result = CImg<int>(Img._width, Img._height, 1, 1, 0);
	cimg_forXY(Img, x, y) {
		result(x, y, 0) = Img(x, y, 0);
		if (Img(x, y, 0) == 255) {
			if (y - 1 >= 0) {
				if (Img(x, y - 1, 0) == 0)
					result(x, y, 0) = 0;
			}

			if (y + 1 < Img._height) {
				if (Img(x, y + 1, 0) == 0)
					result(x, y, 0) = 0;
			}
		}
	}
	return result;
}
void ImageSegmentation::saveSingleNumberImageAndImglist(int barItemIndex) {
	//先统计每张数字图像黑色像素个数平均值
	int totalBlacks = 0, numberCount = 0;
	for (int i = 0; i < pointPosListSet.size(); i++) {
		if (pointPosListSet[i].size() != 0) {
			//cout << "pointPosListSet[i].size() " << pointPosListSet[i].size() << endl;
			totalBlacks += pointPosListSet[i].size();
			numberCount++;
		}
	}
	int avgBlacks = totalBlacks / numberCount;
	//cout << endl << "avgBlacks " << avgBlacks << endl << endl;

	for (int i = 0; i < pointPosListSet.size(); i++) {
		//只有黑色像素个数大于 平均值的一定比例，才可视为数字图像（去除断点）
		if (pointPosListSet[i].size() != 0 && pointPosListSet[i].size() > avgBlacks * NumberImageBlackPixelPercentage) {
			//先找到数字的包围盒
			int xMin, xMax, yMin, yMax;
			getBoundingOfSingleNum(i, xMin, xMax, yMin, yMax);

			int width = xMax - xMin;
			int height = yMax - yMin;

			//将单个数字填充到新图像：扩充到正方形
			int imgSize = (width > height ? width : height) + SingleNumberImgBoundary * 2;
			CImg<int> singleNum = CImg<int>(imgSize, imgSize, 1, 1, 0);

			list<PointPos>::iterator it = pointPosListSet[i].begin();
			for (; it != pointPosListSet[i].end(); it++) {
				int x = (*it).x;
				int y = (*it).y;
				int singleNumImgPosX, singleNumImgPosY;
				if (height > width) {
					singleNumImgPosX = (x - xMin) + (imgSize - width) / 2;
					singleNumImgPosY = (y - yMin) + SingleNumberImgBoundary;
				}
				else {
					singleNumImgPosX = (x - xMin) + SingleNumberImgBoundary;
					singleNumImgPosY = (y - yMin) + (imgSize - height) / 2;
				}
				singleNum(singleNumImgPosX, singleNumImgPosY, 0) = 255;
			}

			//将单个数字填充到新图像：原长宽比
			//int imgSizeH = height + SingleNumberImgBoundary * 2;
			//int imgSizeW = width + SingleNumberImgBoundary * 2;
			//CImg<int> singleNum = CImg<int>(imgSizeW, imgSizeH, 1, 1, 0);
			//list<PointPos>::iterator it = pointPosListSet[i].begin();
			//for (; it != pointPosListSet[i].end(); it++) {
			//	int x = (*it).x;
			//	int y = (*it).y;
			//	int singleNumImgPosX, singleNumImgPosY;
			//	singleNumImgPosX = (x - xMin) + SingleNumberImgBoundary;
			//	singleNumImgPosY = (y - yMin) + SingleNumberImgBoundary;
			//	singleNum(singleNumImgPosX, singleNumImgPosY, 0) = 255;
			//}


			//对单个数字图像做Y方向腐蚀操作
			singleNum = do_Y_Erosion(singleNum);

			//singleNum.display("single Number");
			string postfix = ".bmp";
			char shortImgName[200];
			sprintf(shortImgName, "%d_%d%s\n", barItemIndex, classTagSet[i], postfix.c_str());
			imglisttxt += string(shortImgName);

			char addr[200];
			sprintf(addr, "%s%d_%d%s", basePath.c_str(), barItemIndex, classTagSet[i], postfix.c_str());
			singleNum.save(addr);

			pointPosListSetForDisplay.push_back(pointPosListSet[i]);
		}
	}
	imglisttxt += "*\n";

	//把tag集、每一类链表数据集清空
	classTagSet.clear();
	for (int i = 0; i < pointPosListSet.size(); i++) {
		pointPosListSet[i].clear();
	}
	pointPosListSet.clear();
}

CImg<int> ImageSegmentation::getHistogramImage() {
	return HistogramImage;
}

CImg<int> ImageSegmentation::getImageWithDividingLine() {
	return DividingImg;
}

CImg<int> ImageSegmentation::getColoredNumberDividedImg() {
	vector<int*> ColorSet;
	int red[3]{ 255, 0, 0 };
	int green[3]{ 0, 255, 0 };
	int blue[3]{ 0, 0, 255 };
	int yellow[3]{ 255, 255, 0 };
	int purple[3]{ 255, 0, 255 };
	int qing[3]{ 0, 255, 255 };
	ColorSet.push_back(red);
	ColorSet.push_back(green);
	ColorSet.push_back(blue);
	ColorSet.push_back(yellow);
	ColorSet.push_back(purple);
	ColorSet.push_back(qing);

	CImg<int> answer = CImg<int>(BinaryImg._width, BinaryImg._height, 1, 3, 0);
	cimg_forXY(answer, x, y) {
		answer(x, y, 0) = 255;
		answer(x, y, 1) = 255;
		answer(x, y, 2) = 255;
	}

	int classTagNum = 0;
	for (int i = 0; i < pointPosListSetForDisplay.size(); i++) {
		if (pointPosListSetForDisplay[i].size() != 0) {
			classTagNum++;

			int colorIndex = i % ColorSet.size();
			list<PointPos>::iterator it = pointPosListSetForDisplay[i].begin();
			for (; it != pointPosListSetForDisplay[i].end(); it++) {
				for (int k = 0; k < 3; k++) {
					answer((*it).x, (*it).y, k) = ColorSet[colorIndex][k];
				}
			}
		}
	}
	cout << "classTagNum " << classTagNum << endl;

	return answer;
}

void ImageSegmentation::getBoundingOfSingleNum(int listIndex, int& xMin, int& xMax, int& yMin, int& yMax) {
	xMin = yMin = Infinite;
	xMax = yMax = -1;

	if (!pointPosListSet.empty()) {
		list<PointPos>::iterator it = pointPosListSet[listIndex].begin();
		for (; it != pointPosListSet[listIndex].end(); it++) {
			int x = (*it).x;
			int y = (*it).y;
			xMin = x < xMin ? x : xMin;
			yMin = y < yMin ? y : yMin;
			xMax = x > xMax ? x : xMax;
			yMax = y > yMax ? y : yMax;
		}
	}
	else {
		list<PointPos>::iterator it = pointPosListSetForDisplay[listIndex].begin();
		for (; it != pointPosListSetForDisplay[listIndex].end(); it++) {
			int x = (*it).x;
			int y = (*it).y;
			xMin = x < xMin ? x : xMin;
			yMin = y < yMin ? y : yMin;
			xMax = x > xMax ? x : xMax;
			yMax = y > yMax ? y : yMax;
		}
	}
}

CImg<int> ImageSegmentation::getNumberDividedCircledImg() {
	int lineColor[3]{ 255, 0, 0 };

	CImg<int> answer = CImg<int>(BinaryImg._width, BinaryImg._height, 1, 3, 0);
	cimg_forXY(answer, x, y) {
		answer(x, y, 0) = 255;
		answer(x, y, 1) = 255;
		answer(x, y, 2) = 255;
	}
	
	for (int i = 0; i < pointPosListSetForDisplay.size(); i++) {
		if (pointPosListSetForDisplay[i].size() != 0) {
			//先绘制数字
			list<PointPos>::iterator it = pointPosListSetForDisplay[i].begin();
			for (; it != pointPosListSetForDisplay[i].end(); it++) {
				for (int k = 0; k < 3; k++) {
					answer((*it).x, (*it).y, k) = 0;
				}
			}

			//找到数字的包围盒
			int xMin, xMax, yMin, yMax;
			getBoundingOfSingleNum(i, xMin, xMax, yMin, yMax);

			//然后绘制包围盒
			answer.draw_line(xMin, yMin, xMax, yMin, lineColor);
			answer.draw_line(xMin, yMin, xMin, yMax, lineColor);
			answer.draw_line(xMin, yMax, xMax, yMax, lineColor);
			answer.draw_line(xMax, yMin, xMax, yMax, lineColor);
		}
	}

	return answer;
}