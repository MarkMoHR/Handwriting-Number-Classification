#include <iostream>
#include "canny.h"
#include "SimpleImgProcess.h"
#include "HoughEdgeDetect.h"
#include "PaperModification.h"
#include "ImageSegmentation.h"
#include "SVMPredictor.h"
using namespace std;

int main() {
	time_t start, midterm, end;
	start = time(NULL);

	CImg<int> SrcImg, GrayImg, DownSampledImg, CannyImg, HoughSpaceImg, 
		Edge_Point_Img, SrcImgWithVertexAndEdge, PaperModifiedImg;
	/*
	SrcImg.load_bmp("Input/5.bmp");

	//------------------------- 提取顶点部分 -------------------------//

	GrayImg = grayTheImg(SrcImg);
	//GrayImg.display("GrayImg");
	DownSampledImg = downSampleTheImg(GrayImg, DownSampledSquareSize);
	//DownSampledImg.display("DownSampledImg");
	CannyImg = canny(DownSampledImg, DownSampledImg._width, DownSampledImg._height);
	//CannyImg.display("EdgeImg");
	
	HoughEdgeDetect myHoughEdgeDetect(CannyImg);
	myHoughEdgeDetect.HoughTransAndDetectEdge();
	HoughSpaceImg = myHoughEdgeDetect.getHoughSpaceImage();
	//HoughSpaceImg.display("houghArray");
	
	Edge_Point_Img = myHoughEdgeDetect.getCannyGrayImageWithEdge();
	//Edge_Point_Img.display("Edge_Point_Img");

	SrcImgWithVertexAndEdge = myHoughEdgeDetect.getFinallyProcessedImage(SrcImg);
	SrcImgWithVertexAndEdge.display("Finally Processed Image");


	//------------------------- A四纸矫正部分 -------------------------//

	PaperModification myPaperModification(SrcImg);
	myPaperModification.setVertexSet(myHoughEdgeDetect.getTopFourVertexSet(), myHoughEdgeDetect.getDownSampledSize());

	PaperModifiedImg = myPaperModification.getModifiedPaper();

	end = time(NULL);
	cout << endl << endl << "==== PaperModification End: " << end - start << " seconds is used." << endl << endl;

	PaperModifiedImg.display("Paper Modified Image");
	//PaperModifiedImg.save("Output/Modification/5-Modification.bmp");
	*/

	/*
	//测试模式读取文件
	PaperModifiedImg.load_bmp("Output/Modification/5-Modification.bmp");

	//------------------------- 数字提取分割部分 -------------------------//

	CImg<int> NewGraySrcImg, BinaryImg, HistogramImage, DividingImg, 
		ColoredNumberDividedImg, NumberDividedCircledImg;
	NewGraySrcImg = grayTheImg(PaperModifiedImg);
	//NewGraySrcImg.display("New Gray SrcImg");

	ImageSegmentation myImageSegmentation(NewGraySrcImg);
	myImageSegmentation.processBinaryImage();
	BinaryImg = myImageSegmentation.getBinaryImage();
	BinaryImg.display("Binary Img");
	//BinaryImg.save("Output/BinaryImage/DilationFe/5-BinaryImage-FeFeFa.bmp");
	
	myImageSegmentation.numberSegmentationMainProcess("Output/SingleNumberImage/5/");
	HistogramImage = myImageSegmentation.getHistogramImage();
	HistogramImage.display("Histogram Image");
	//HistogramImage.save("Output/HistogramImage/5-HistogramImage.bmp");
	DividingImg = myImageSegmentation.getImageWithDividingLine();
	DividingImg.display("Image With Dividing Line");
	//DividingImg.save("Output/HistogramImage/5-DividingImg.bmp");

	ColoredNumberDividedImg = myImageSegmentation.getColoredNumberDividedImg();
	ColoredNumberDividedImg.display("Colored Number Divided Img");
	//ColoredNumberDividedImg.save("Output/ColoredNumberDividedImg/5-ColoredNumberDividedImg.bmp");
	NumberDividedCircledImg = myImageSegmentation.getNumberDividedCircledImg();
	NumberDividedCircledImg.display("Number Divided Circled Img");
	//NumberDividedCircledImg.save("Output/NumberDividedCircledImg/5-NumberDividedCircledImg.bmp");
	*/
	//------------------------- 数字识别预测部分 -------------------------//

	
	SVMPredictor svmPredictor("Output/SingleNumberImage/5/");
	svmPredictor.readPredictImagelist("predict_imagelist.txt");
	svmPredictor.predict_mainProcess("TrainedModels/trainData_scaled_c=4_g=0.015625.model");
	

	int pause;
	cin >> pause;
	return 0;
}
