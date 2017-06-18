# Handwriting Number Classification
An computer vision project, based on cimg library and svm training, to classify handwriting number.


## 1. Project Requirement
### 1.1 Input
- A photo in which an A4 paper is placed on the desk, with some handwriting numbers on
### 1.2 Output
- A string of the numbers written on the paper, like "70026548853"
### 1.3 Intermediate Processes
- Modify the paper in the photo into a standard A4 paper
- Implement the segmentation of the numbers, dividing them into single number
- Use _Adaboost_ or _SVM_ to train a classifier of the handwriting numbers
- Classify the handwriting numbers with the trained classifier

---

## 2. Coding Environment & 3rd Party Library
- Windows10 + VS2015
- C++
- cimg library : http://www.cimg.eu/
- opencv (For extracting features of images)
- libsvm (for training, testing and predicting) : http://www.csie.ntu.edu.tw/~cjlin/libsvm/
- ......

---

## 3. Implementation Procedure
1. Find the 4 vertices of the paper
1. Modify the paper into a standard A4 paper
1. Segmentation of the numbers in order:
    - Convert into binary image
    - Divide the source image into sub-image (each sub-image contains a line of numbers)
    - Foreach sub-image, implement dilation to thicken the number (and join the broken ones)
    - Foreach sub-image, use connected-component_labeling algorithm to divide the single number: 
    https://en.wikipedia.org/wiki/Connected-component_labeling
    - Foreach sub-image, save all single number images and a list of their name in .txt

1. Use libsvm to train model and test, and predict the number finally
    - Data prepairing: 
        - convert the mnist(http://yann.lecun.com/exdb/mnist/) binary data into .jpg as well as their labels
    - Model training: 
        - extract the HOG features of each image and construct them into svm format (in **.txt)
        - Scale the features(in **.txt) with `svm-scale.exe` (search the _windows/_ folder)
        - Train the model with `svm-train.exe`, and get **.model
        - (Optional) Test the data with `svm-predict.exe` and see the accuracy (modify the training parameter to get the highest accuracy)
    - Number predicting: 
        - read the number images you segmented just now and do prediction with the trained model
        
1. ......

---

## 4. Result Screenshots
- 4 vertices of the paper & A4 paper modification

![Image text](https://github.com/MarkMoHR/HandwritingNumberClassification/raw/master/ResultScreenshots/1.png)
- Segmentation of the numbers:

    - Binary image with dilation & Divided Image & Circled single number
    
    ![Image text](https://github.com/MarkMoHR/HandwritingNumberClassification/raw/master/ResultScreenshots/2.png)
    - Divided into single numbers in order
    
    ![Image text](https://github.com/MarkMoHR/HandwritingNumberClassification/raw/master/ResultScreenshots/singleNumbers.png)

