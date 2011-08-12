#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "../Blob.h"
#include "../BlobResult.h"

int filter(int r, int g, int b, int threshold) {
    int orange[3] = {200, 250, 10}; // Orange in HSV
    int diff =  (orange[0]-r)*(orange[0]-r) +
                (orange[1]-g)*(orange[1]-g);

    if(diff < threshold) return abs(diff-threshold);
    return 0;
}

int main() 
{
    IplImage* input;
    IplImage* img;
    IplImage *hsv_img;
    IplImage *bw_img;
    IplImage* i1;
    CBlobResult blobs;
    CBlob blobArea;

    // Initialize image, allocate memory
    input = cvLoadImage("blob.jpg", 1);
    img = cvCloneImage(input);
    hsv_img = cvCloneImage(img);
    bw_img = cvCreateImage(cvSize(img->width, img->height), 8, 1);
    i1 = cvCreateImage(cvSize(img->width, img->height), 8, 1);

    // Smooth input image using a Gaussian filter, assign HSV, BW image
    cvSmooth(input, img, CV_GAUSSIAN, 7, 9, 0 ,0);
    cvCvtColor(img, bw_img, CV_RGB2GRAY);
    cvCvtColor(img, hsv_img, CV_RGB2HSV);

     //// Simple filter that creates a mask whose color is near orange in i1
    for(int i = 0; i < hsv_img->height; i++) {
        for(int j = 0; j < hsv_img->width; j++) {
            int r = ((uchar *)(hsv_img->imageData + i*hsv_img->widthStep))[j*hsv_img->nChannels + 2];
            int g = ((uchar *)(hsv_img->imageData + i*hsv_img->widthStep))[j*hsv_img->nChannels + 1];
            int b = ((uchar *)(hsv_img->imageData + i*hsv_img->widthStep))[j*hsv_img->nChannels + 0];
            int f = filter(r, g, b, 5000);
            if(f) {
                ((uchar *)(i1->imageData + i*i1->widthStep))[j] = 255;
            } else
                ((uchar *)(i1->imageData + i*i1->widthStep))[j] = 0;
        }
    }

    // Detect Blobs using the mask and a threshold for area. Sort blobs according to area

    blobs = CBlobResult(bw_img, i1, 0);
    blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_GREATER, 500);
    blobs.GetNthBlob(CBlobGetArea(), blobs.GetNumBlobs() - 1, blobArea); // example

    printf("Total number of blobs = %d\n", blobs.GetNumBlobs());

    for (int i = 0; i < blobs.GetNumBlobs(); i++ )
    {
        blobArea = blobs.GetBlob(i);
        blobArea.FillBlob(img, cvScalar(255, 0, 0));
    }  

    //Debug images
    cvNamedWindow("bw_img", 1);
    cvShowImage("bw_img", bw_img);

    cvNamedWindow("i1", 1);
    cvShowImage("i1", i1);

    // Display blobs
    cvNamedWindow("Input", 1);
    cvShowImage("Input", input);

    cvNamedWindow("Output Image", 1);
    cvShowImage("Output Image", img);

    cvWaitKey(0); // Wait till windows are closed
    // Cleanup
    cvReleaseImage(&i1);
    cvReleaseImage(&bw_img);
    cvReleaseImage(&hsv_img);
    cvReleaseImage(&img);
    cvReleaseImage(&input);
    return 0;

}
