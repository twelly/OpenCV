#include <stdio.h>
#include <cv.h>
#include <highgui.h>

int main (void)
{
	int i, corner_count = 150;
	IplImage *dst_img1, *dst_img2, *src_img_gray;
	IplImage *eig_img, *temp_img;
	CvPoint2D32f *corners;

	//image file
	char imagePath[256] = "ticket.jpg";
	printf("%s\n", imagePath);

	dst_img1 = cvLoadImage (imagePath, CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
	dst_img2 = cvCloneImage (dst_img1);
	src_img_gray = cvLoadImage (imagePath, CV_LOAD_IMAGE_GRAYSCALE);
	eig_img = cvCreateImage (cvGetSize (src_img_gray), IPL_DEPTH_32F, 1);
	temp_img = cvCreateImage (cvGetSize (src_img_gray), IPL_DEPTH_32F, 1);
	corners = (CvPoint2D32f *) cvAlloc (corner_count * sizeof (CvPoint2D32f));

	// (1)Corner detection using cvCornerMinEigenVal
	cvGoodFeaturesToTrack (src_img_gray, eig_img, temp_img, corners, &corner_count, 0.1, 15);
	cvFindCornerSubPix (src_img_gray, corners, corner_count,
					  cvSize (3, 3), cvSize (-1, -1), cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
	// (2)Draw the detected corner
	for (i = 0; i < corner_count; i++)
	cvCircle (dst_img1, cvPointFrom32f (corners[i]), 3, CV_RGB (255, 0, 0), 2);

	//Message for debugging
	printf("MinEigenVal corner count = %d\n", corner_count);

	// (3)Corner detection using cvCornerHarris
	corner_count = 150;
	cvGoodFeaturesToTrack (src_img_gray, eig_img, temp_img, corners, &corner_count, 0.1, 15, NULL, 3, 1, 0.01);
	cvFindCornerSubPix (src_img_gray, corners, corner_count,
					  cvSize (3, 3), cvSize (-1, -1), cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
	// (4)Draw the detected corner
	for (i = 0; i < corner_count; i++)
	cvCircle (dst_img2, cvPointFrom32f (corners[i]), 3, CV_RGB (0, 0, 255), 2);

	//Message for debugging
	printf("Harris corner count = %d\n", corner_count);

	// (5)Display the result
	cvNamedWindow ("EigenVal", CV_WINDOW_AUTOSIZE);
	cvShowImage ("EigenVal", dst_img1);
	cvNamedWindow ("Harris", CV_WINDOW_AUTOSIZE);
	cvShowImage ("Harris", dst_img2);
	cvWaitKey (0);

	cvDestroyWindow ("EigenVal");
	cvDestroyWindow ("Harris");
	cvReleaseImage (&dst_img1);
	cvReleaseImage (&dst_img2);
	cvReleaseImage (&eig_img);
	cvReleaseImage (&temp_img);
	cvReleaseImage (&src_img_gray);

	return 0;
}

