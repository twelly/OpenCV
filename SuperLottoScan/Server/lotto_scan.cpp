#include <cv.h>
#include <highgui.h>
#include <stdio.h>


#define WINDOW_NAME "test_window"


int stoi(char *string) {
    int integer;

    sscanf(string, "%d", &integer);

    return(integer);
}

int main(int argc, char** argv)
{
    char *lotto_ticket_img_file = argv[1];

    IplImage* lotto_ticket_img = cvLoadImage(lotto_ticket_img_file, 0);

    // Process image
    IplImage* processed_img = cvCreateImage(cvGetSize(lotto_ticket_img), lotto_ticket_img->depth, 1);

    cvThreshold(lotto_ticket_img, processed_img, 100, 100, CV_THRESH_BINARY_INV);

    // Find contours
    CvSeq*        contours = 0;
    CvMemStorage* storage  = cvCreateMemStorage(0);
 
    cvFindContours(processed_img, storage, &contours, sizeof(CvContour), CV_RETR_CCOMP);

    for(; contours != 0; contours = contours->h_next) {
        CvRect rect = cvBoundingRect(contours);

        CvPoint point1;
        CvPoint point2;

        point1.x = rect.x;
        point2.x = (rect.x + rect.width);
        point1.y = rect.y;
        point2.y = (rect.y + rect.height);

        cvRectangle(lotto_ticket_img, point1, point2, cvScalar(100), 1, 8, 0);
    }

    // Display processd image in a window
    cvNamedWindow(WINDOW_NAME);
    cvShowImage(WINDOW_NAME, lotto_ticket_img);

    cvWaitKey(0);

    cvReleaseImage(&lotto_ticket_img);
    cvReleaseImage(&processed_img);

    cvDestroyWindow(WINDOW_NAME);

    return 0;
}
