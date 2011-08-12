#include <cv.h>
#include <highgui.h>
#include <stdio.h>

/* Inverse Image Binarization with noise filter */
int main()
{
    IplImage* imgGray;
    IplImage* imgThresh;
    IplImage* imgSmooth;
    IplImage* imgOriginal = cvLoadImage("photo.jpg", 1);
    IplImage* imgResult = cvCreateImage(cvSize(imgOriginal->width, imgOriginal->height), IPL_DEPTH_8U, 1);
    cvZero(imgResult);
    /* convert to grayscale */
    cvCvtColor(imgOriginal,imgResult,CV_RGB2GRAY);
    imgGray = cvCloneImage(imgResult);
    cvZero(imgResult);
    /* apply gaussian blur */
    cvSmooth(imgGray, imgResult, CV_GAUSSIAN, 1, 1);
    imgSmooth = cvCloneImage(imgResult);
    cvZero(imgResult);
    /* apply inverse binarization */
    cvThreshold(imgSmooth, imgResult,60,255, CV_THRESH_BINARY_INV);
    //cvAdaptiveThreshold(imgSmooth, imgResult, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 5, 2);
    imgThresh = cvCloneImage(imgResult);
    cvZero(imgResult);
    cvDilate(imgThresh,imgResult,NULL,1);
    cvNamedWindow("result");
    cvShowImage("result", imgResult);
    cvWaitKey(0);
    return 0;
}
