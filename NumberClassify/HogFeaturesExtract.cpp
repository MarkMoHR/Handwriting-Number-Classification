//*********************************************************************
// 使用时期: 此独立代码用于libsvm训练步骤
// 代码功能: 得到mnist的jpg图片格式后，提取图像的hog特征，并存储到一个txt
//*********************************************************************


#include "opencv2/opencv.hpp"
#include "fstream"
#include <list>
using namespace std;
using namespace cv;

vector<string> trainImageList;    //训练图像列表，此处路径
vector<int> trainLabelList;       //标签
string trainImageFile = "ImageData/train_imagelist.txt";
string trainBasePath = "ImageData/TrainData/";

CvMat * dataMat;
CvMat * labelMat;

// 读取训练的图像列表和图像的位置
void readTrainFileList(string trainImageFile, string basePath,
	vector<string> &trainImageList, vector<int> &trainLabelList) {
	ifstream readData(trainImageFile);
	string buffer;
	while (readData) {
		if (getline(readData, buffer)) {
			int label = int((buffer[0]) - '0');    //在我这里路径中第一个文件夹就是类别
			trainLabelList.push_back(label);
			trainImageList.push_back(buffer);      //图像路径       
		}
	}
	readData.close();
	cout << "Read Train Data Complete" << endl;
}


// 计算Hog特征，输出到一个txt
void processHogFeature(vector<string> trainImageList,
	vector<int> trainLabelList, CvMat * &dataMat, CvMat * &labelMat) {
	int trainSampleNum = trainImageList.size();
	dataMat = cvCreateMat(trainSampleNum, 324, CV_32FC1);  //324为Hog feature Size
	cvSetZero(dataMat);
	labelMat = cvCreateMat(trainSampleNum, 1, CV_32FC1);
	cvSetZero(labelMat);
	IplImage* src;
	IplImage* trainImg = cvCreateImage(cvSize(20, 20), 8, 3);    //20 20

	char buffer[512];
	ofstream sourceDataOutput(trainBasePath + "train_svm_data.txt");

	for (int i = 0; i != trainImageList.size(); i++) {

		sprintf(buffer, "%d ", trainLabelList[i]);
		sourceDataOutput << buffer;

		src = cvLoadImage((trainBasePath + trainImageList[i]).c_str(), 1);
		if (src == NULL) {
			cout << " can not load the image: " << (trainBasePath + trainImageList[i]).c_str() << endl;
			continue;
		}
		//cout<<"Calculate Hog Feature "<<(trainBasePath  + trainImageList[i]).c_str()<<endl;    

		cvResize(src, trainImg);
		HOGDescriptor *hog = new HOGDescriptor(cvSize(20, 20), cvSize(10, 10), cvSize(5, 5), cvSize(5, 5), 9);
		vector<float> descriptors;
		cv::Mat trainImgMat = cv::cvarrToMat(trainImg, true);
		hog->compute(trainImgMat, descriptors, Size(1, 1), Size(0, 0));

		int j = 0;
		for (vector<float>::iterator iter = descriptors.begin(); iter != descriptors.end(); iter++) {
			cvmSet(dataMat, i, j, *iter);    //存储HOG特征
			j++;

			sprintf(buffer, "%d:%f ", j, *iter);
			sourceDataOutput << buffer;
		}
		cvmSet(labelMat, i, 0, trainLabelList[i]);
		//cout<<"Image and label "<<trainImageList[i].c_str()<<" "<<trainLabelList[i]<<endl; 

		sprintf(buffer, "\r\n");
		sourceDataOutput << buffer;
	}
	cout << "Calculate Hog Feature Complete" << endl;
	cout << dataMat << endl;

	sourceDataOutput.close();
}

// 释放相应的资源
void releaseAll() {
	cvReleaseMat(&dataMat);
	cvReleaseMat(&labelMat);
	cout << "Release All Complete" << endl;
}

/*
int main() {

	readTrainFileList(trainImageFile, trainBasePath, trainImageList, trainLabelList);
	processHogFeature(trainImageList, trainLabelList, dataMat, labelMat);

	releaseAll();

	return 0;
}
*/



