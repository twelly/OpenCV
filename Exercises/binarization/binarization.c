#include <cv.h>
#include <highgui.h>
#include <stdio.h>

/* Inverse Image Binarization with noise filter */
int main()
{
    IplImage* imgGray;
    IplImage* imgThresh;
    IplImage* imgSmooth;
    IplImage* imgOriginal = cvLoadImage("superlotto.tif", 1);
    IplImage* imgResult = cvCreateImage(cvSize(imgOriginal->width, imgOriginal->height), IPL_DEPTH_8U, 1);
    cvZero(imgResult);
    cvCvtColor(imgOriginal,imgResult,CV_RGB2GRAY);
    imgGray = cvCloneImage(imgResult);
    cvSmooth(imgGray, imgResult, CV_GAUSSIAN, 1, 1);
    imgSmooth = cvCloneImage(imgResult);
    cvThreshold(imgSmooth, imgResult,32,255, CV_THRESH_BINARY_INV);
    imgThresh = cvCloneImage(imgResult);
    cvDilate(imgThresh,imgResult,NULL,2);
    cvNamedWindow("result");
    cvShowImage("result", imgResult);
    cvWaitKey(0);
    return 0;
}
