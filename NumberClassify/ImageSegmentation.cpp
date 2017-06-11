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

//XY方向的扩张
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
//Y方向的扩张
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

void ImageSegmentation::processBinaryImage(bool doDilationY) {
	CImg<int> tmpAnswer = CImg<int>(SrcGrayImg._width, SrcGrayImg._height, 1, 1, 0);

	cimg_forXY(tmpAnswer, x, y) {
		int intensity = SrcGrayImg(x, y, 0);

		//先去掉黑色边缘
		if (x <= BoundaryRemoveGap || y <= BoundaryRemoveGap
			|| x >= imgW - BoundaryRemoveGap || y >= imgH - BoundaryRemoveGap) {
			tmpAnswer(x, y, 0) = 255;
		}
		else {
			if (intensity < BinaryGap)
				tmpAnswer(x, y, 0) = 0;
			else
				tmpAnswer(x, y, 0) = 255;
		}
	}

	//扩张Dilation XY方向
	CImg<int> answerXY = CImg<int>(tmpAnswer._width, tmpAnswer._height, 1, 1, 0);
	cimg_forXY(tmpAnswer, x, y) {
		int intensity = getDilationIntensityXY(tmpAnswer, x, y);
		answerXY(x, y, 0) = intensity;
	}

	if (doDilationY) {
		//扩张Dilation Y方向
		CImg<int> answerY = CImg<int>(answerXY._width, answerXY._height, 1, 1, 0);
		cimg_forXY(answerXY, x, y) {
			int intensity = getDilationIntensityY(answerXY, x, y);
			answerY(x, y, 0) = intensity;
		}
		BinaryImg = answerY;
	}
	else
		BinaryImg = answerXY;
}

CImg<int> ImageSegmentation::getBinaryImage() {
	return BinaryImg;
}

void ImageSegmentation::numberSegmentationMainProcess() {
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
			if (blackPixel == 0 && HistogramImage(0, y - 1, 0) == 0) {    //下白上黑：取下
				inflectionPointSet.push_back(y);
				//HistogramImage.draw_line(0, y, HistogramImage._width - 1, y, lineColor);
			}
			else if (blackPixel != 0 && HistogramImage(0, y - 1, 0) != 0) {    //下黑上白：取上
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
			HistogramImage.draw_line(0, divideLinePoint, HistogramImage._width - 1, divideLinePoint, lineColor);
			DividingImg.draw_line(0, divideLinePoint, HistogramImage._width - 1, divideLinePoint, lineColor);
		}
	}
	divideLinePointSet.push_back(BinaryImg._height - 1);
}

void ImageSegmentation::divideIntoBarItemImg() {
	for (int i = 1; i < divideLinePointSet.size(); i++) {
		int barHright = divideLinePointSet[i] - divideLinePointSet[i - 1];
		CImg<int> barItemImg = CImg<int>(BinaryImg._width, barHright, 1, 1, 0);
		cimg_forXY(barItemImg, x, y) {
			barItemImg(x, y, 0) = BinaryImg(x, divideLinePointSet[i - 1] + 1 + y, 0);
		}
		subImageSet.push_back(barItemImg);
		barItemImg.display("barItemImg");
	}
}

void ImageSegmentation::connectedRegionsTagging() {
	for (int i = 0; i < subImageSet.size(); i++) {
		connectedRegionsTaggingOfBarItemImg(i);
	}
}

void ImageSegmentation::connectedRegionsTaggingOfBarItemImg(int barItemIndex) {
	TagImage = CImg<int>(subImageSet[barItemIndex]._width, subImageSet[barItemIndex]._height, 1, 1, 0);

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
						if (y + i < imgH)
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
		if (y + i < imgH) {
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
	for (int i = 0; i < pointPosListSet.size(); i++) {
		if (pointPosListSet[i].size() != 0) {
			classTagNum++;

			int colorIndex = i % ColorSet.size();
			list<PointPos>::iterator it = pointPosListSet[i].begin();
			for (; it != pointPosListSet[i].end(); it++) {
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

CImg<int> ImageSegmentation::getNumberDividedCircledImg() {
	int lineColor[3]{ 255, 0, 0 };

	CImg<int> answer = CImg<int>(BinaryImg._width, BinaryImg._height, 1, 3, 0);
	cimg_forXY(answer, x, y) {
		answer(x, y, 0) = BinaryImg(x, y, 0);
		answer(x, y, 1) = BinaryImg(x, y, 0);
		answer(x, y, 2) = BinaryImg(x, y, 0);
	}

	for (int i = 0; i < pointPosListSet.size(); i++) {
		if (pointPosListSet[i].size() != 0) {
			//先找到数字的包围盒
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

void ImageSegmentation::saveSingleNumberImage(const string baseAddress) {
	for (int i = 0; i < pointPosListSet.size(); i++) {
		if (pointPosListSet[i].size() != 0) {
			//先找到数字的包围盒
			int xMin, xMax, yMin, yMax;
			getBoundingOfSingleNum(i, xMin, xMax, yMin, yMax);

			int width = xMax - xMin;
			int height = yMax - yMin;

			//将单个数字填充到新图像
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
			singleNum.display("single Number");
			char addr[200];
			string postfix = ".bmp";
			sprintf(addr, "%s%d%s", baseAddress.c_str(), i, postfix.c_str());
			singleNum.save(addr);
		}
	}
}