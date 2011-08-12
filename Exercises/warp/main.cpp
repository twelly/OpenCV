// Usage: warp <image>
//
#include <cv.h>
#include <highgui.h>
int main(int argc, char** argv)
{
   CvMat* warp_matrix = cvCreateMat(3,3,CV_32FC1);
   IplImage     *src, *dst;
   float mat_test[4][2] = {96.0f,236.0f,226.0f,305.0f,408.0f,94.0f,498.0f,106.0f};
   float mat_test2[4][2] = {98.0f,231.0f,225.0f,306.0f,410.0f,94.0f,498.0f,106.0f};

   if( argc == 2 && ((src=cvLoadImage(argv[1],1)) != 0 ))
   {
      dst = cvCloneImage(src);
      cvZero(dst);
      dst->origin = src->origin;
      cvInitMatHeader(&src_mat, 4,2, CV_32FC1,mat_test);
      cvInitMatHeader(&dst_mat, 4,2, CV_32FC1,mat_test2);
      cvFindHomography(&src_mat, &dst_mat, warp_matrix); 
      cvWarpPerspective(src, dst, transform);
      cvNamedWindow("Perspective_Warp", 1);
      cvShowImage("Perspective_Warp", dst);
      cvWaitKey();
   }
   cvReleaseImage(&dst);
   cvReleaseMat(&warp_matrix);
   return 0;
}
