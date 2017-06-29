#ifndef SVMPREDICTOR_H
#define SVMPREDICTOR_H

#include "fstream"
#include "svm.h"
#include <iostream>
#include <string>
#include <vector>
#include <list>
using namespace std;

#define NormalizeLow 0.0    //对维度特征归一化的最小值
#define NormalizeUp 1.0     //对维度特征归一化的最大值

class SVMPredictor
{
public:
	SVMPredictor();
	~SVMPredictor();

	SVMPredictor(string _basePath);

	void readPredictImagelist(string predictImagelistFile);    //读取图片名字txt列表

	void predict_mainProcess(string LibSVMModelFile);          //做预测主入口

private:
	//对每张子图做预测
	void predict_foreach_subImage(string LibSVMModelFile, list<string> predictImgList);

private:
	string basePath = "";                      //单个数字图像与图像名字列表文本的存储路径

	string predictResult = "\n";               //预测数字串
	string predictResultInDetail = "";         //预测详细结果：每个数字对应的预测

	vector<list<string>> predictImgListSet;    //预测图像名称列表集合（每张子图对应一个list）
};

#endif
