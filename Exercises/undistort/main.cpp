#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <X11/Xlibint.h>
#define CLIP2(minv, maxv, value) (min(maxv, max(minv, value)))
CvMat* cvmGetCol(CvMat * src, int iCol){
    int i;
    int nRow = src->rows;
    CvMat *curCol = cvCreateMat(nRow,1,src->type);
    for (i=0; i<nRow; i++)
        cvmSet(curCol, i, 0, cvmGet(src, i, iCol));
    return curCol;
}
int main(int argc, char *argv[])
{
    IplImage *img_in = 0, *img_affine = 0, *img_scene = 0, *img_interp = 0;;
    int height, width, step, channels;
    uchar *data_in, *data_affine, *data_scene, *data_interp;
    int i,j,k;
    double distp_x, distp_y;
    double tmpxp[50], tmpyp[50];
    int curpi, curpj;
    if (argc<2)
    {
        printf("Usage: main <image-file-name>\n\7");
        exit(0);
    }
// X_c is the camera image, X_a is the affinely rectified image, X is the true
// X_a = H_1*X_c X_a = H_2*X
// load the distorted image X_c
    img_in = cvLoadImage(argv[1]);
    if (!img_in)
    {
        printf("Could not load image file: %s\n",argv[1]);
        exit(0);
    }
    height = img_in->height;
    width = img_in->width;
    step = img_in->widthStep;
    channels = img_in->nChannels;
    data_in = (uchar *)img_in->imageData;
    printf("Processing a %d x %d image with %d channels\n",height,width,channels);
    CvMat *check = cvCreateMat(height, width, CV_64FC1);
    cvZero(check);
// allocate the output image and initialize
    img_affine = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, channels);
    img_scene = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, channels);
    cvZero(img_affine);
    cvZero(img_scene);
    data_affine = (uchar *)img_affine->imageData;
    data_scene = (uchar *)img_scene->imageData;
// remove projective distortion
    CvMat * vetex = cvCreateMat(3,4,CV_64FC1);
// get the coordinate values of four vetexes of a square
// up-left, up-right, down-left, down-right
    for (i=0; i<4; i++)
    {
        printf("input the %d th point's coordinates (x',y') ", i);
        scanf("%f %f", &distp_x, &distp_y);
        distp_x = tmpxp[i];
        distp_y = tmpyp[i];
        cvmSet(vetex,0,i,distp_x);
        cvmSet(vetex,1,i,distp_y);
        cvmSet(vetex,2,i,1);
        printf("\n");
    }
//************** Affine Rectification *****************
    CvMat *l1 = cvCreateMat(3,1,CV_64FC1);
    CvMat *l2 = cvCreateMat(3,1,CV_64FC1);
    CvMat *m1 = cvCreateMat(3,1,CV_64FC1);
    CvMat *m2 = cvCreateMat(3,1,CV_64FC1);
    CvMat *v1 = cvCreateMat(3,1,CV_64FC1);
    CvMat *v2 = cvCreateMat(3,1,CV_64FC1);
    CvMat *vanishL = cvCreateMat(3,1,CV_64FC1);
    cvCrossProduct(cvmGetCol(vetex,0), cvmGetCol(vetex,1), l1);
    cvCrossProduct(cvmGetCol(vetex,2), cvmGetCol(vetex,3), l2);
    cvCrossProduct(cvmGetCol(vetex,0), cvmGetCol(vetex,2), m1);
    cvCrossProduct(cvmGetCol(vetex,1), cvmGetCol(vetex,3), m2);
    cvCrossProduct(l1, l2, v1);
    cvCrossProduct(m1, m2, v2);
    cvCrossProduct(v1, v2, vanishL);
// normalize vanishing line
// in order to map the distorted image back to the image window
    double scale = 1.0;
    cvmSet(vanishL,0,0,cvmGet(vanishL,0,0)/cvmGet(vanishL,2,0)*scale);
    cvmSet(vanishL,1,0,cvmGet(vanishL,1,0)/cvmGet(vanishL,2,0)*scale);
    cvmSet(vanishL,2,0,1.0*scale);
    double H1data[] =
    {1,0,0,0,1,0,cvmGet(vanishL,0,0),cvmGet(vanishL,1,0),cvmGet(vanishL,2,0)};
    CvMat H1 = cvMat(3,3,CV_64FC1, H1data);
// transform X_a = H_1*X_c
    CvMat * ptx = cvCreateMat(3,1,CV_64FC1);
    CvMat * ptxp = cvCreateMat(3,1,CV_64FC1);
    cvmSet(&H1,2,2,1.0);
    for (i=0; i<height; i++) //y - ver
    {
        for (j=0; j<width; j++) //x - hor
        {
// set X_c
            cvmSet(ptxp,0,0,(double)j);
            cvmSet(ptxp,1,0,(double)i);
            cvmSet(ptxp,2,0,1.0);
// compute X_a
            cvMatMul(&H1, ptxp, ptx);
            curpi = CLIP2(0, height-1, (int)(cvmGet(ptx,1,0)/cvmGet(ptx,2,0)));
            curpj = CLIP2(0, width-1, (int)(cvmGet(ptx,0,0)/cvmGet(ptx,2,0)));
            cvSet2D(img_affine,curpi,curpj,cvGet2D(img_in,i,j));
            cvmSet(check,curpi,curpj,1);
        }
    }
// output reconstructed affine image
    img_interp = cvCloneImage(img_affine);
    data_interp = (uchar *)img_interp->imageData;
//interpolation
    double count;
    for (i=1; i<height-1; i++) //y - ver
    {
        for (j=1; j<width-1; j++) //x - hor
        {
            if (cvmGet(check,i,j) == 0)
            {
                count = (cvmGet(check,i-1,j)==1)+(cvmGet(check,i+1,j)==1)+
                        (cvmGet(check,i,j-1)==1)+(cvmGet(check,i,j+1)==1)+
                        (cvmGet(check,i-1,j-1)==1)+(cvmGet(check,i-
                                                           1,j+1)==1)+
                        (cvmGet(check,i+1,j-
                                1)==1)+(cvmGet(check,i+1,j+1)==1);
                if (count != 0)
                {
                    for (k=0; k<channels; k++)
                        data_interp[i*step+j*channels+k] = (int)
                                                           ((data_affine[(i-
                                                                          1)*step+j*channels+k]+data_affine[(i+1)*step+j*channels+k]
                                                             +data_affine[i*step+(j-
                                                                                  1)*channels+k]+data_affine[i*step+(j+1)*channels+k]
                                                             +data_affine[(i-1)*step+(j-
                                                                                      1)*channels+k]+data_affine[(i-1)*step+(j+1)*channels+k]
                                                             +data_affine[(i+1)*step+(j-
                                                                                      1)*channels+k]+data_affine[(i+1)*step+(j+1)*channels+k])/count);
                }
            }
        }
    }
    img_affine = cvCloneImage(img_interp);
    if (!cvSaveImage("affine.jpg",img_affine))
        printf("Could not save file\n");
//************** Metric Rectification *****************
// transform points by H1
    CvMat *pt = cvCreateMat(3,1,CV_64FC1);
    for (i=0; i<4; i++)
    {
        cvMatMul(&H1, cvmGetCol(vetex,i), pt);
        cvmSet(vetex,0,i,(int)(cvmGet(pt,0,0)/cvmGet(pt,2,0)));
        cvmSet(vetex,1,i,(int)(cvmGet(pt,1,0)/cvmGet(pt,2,0)));
        cvmSet(vetex,2,i,1.0);
    }
    cvCrossProduct(cvmGetCol(vetex,0), cvmGetCol(vetex,1), l1);
    cvCrossProduct(cvmGetCol(vetex,0), cvmGetCol(vetex,2), m1);
    cvCrossProduct(cvmGetCol(vetex,0), cvmGetCol(vetex,3), l2);
    cvCrossProduct(cvmGetCol(vetex,2), cvmGetCol(vetex,1), m2);
    double l11, l12, m11, m12, l21, l22, m21, m22;
    l11 = cvmGet(l1,0,0); l12 = cvmGet(l1,1,0);
    l21 = cvmGet(l2,0,0); l22 = cvmGet(l2,1,0);
    m11 = cvmGet(m1,0,0); m12 = cvmGet(m1,1,0);
    m21 = cvmGet(m2,0,0); m22 = cvmGet(m2,1,0);
// M*x = b
    double Mdata[] = {l11*m11, l11*m12+l12*m11, l21*m21, l21*m22+l22*m21};
    double bdata[] = {-l12*m12, -l22*m22};
    CvMat M = cvMat(2,2,CV_64FC1,Mdata);
    CvMat b = cvMat(2,1,CV_64FC1,bdata);
    CvMat *x = cvCreateMat(2,1,CV_64FC1);
    cvSolve(&M,&b,x);
// Set matrix S
    double Sdata[] = {cvmGet(x,0,0), cvmGet(x,1,0), cvmGet(x,1,0), 1.0};
    CvMat S = cvMat(2,2,CV_64FC1, Sdata);
// SVD S=UDV_T
    CvMat* U = cvCreateMat(2,2,CV_64FC1);
    CvMat* D = cvCreateMat(2,2,CV_64FC1);
    CvMat* V = cvCreateMat(2,2,CV_64FC1);
    cvSVD(&S, D, U, V, CV_SVD_U_T|CV_SVD_V_T);
//The flags cause U and V to be returned transposed (does not work well
//Therefore, in OpenCV, S = U^T D V
    CvMat* U_T = cvCreateMat(2,2,CV_64FC1);
    CvMat* sqrtD = cvCreateMat(2,2,CV_64FC1);
    CvMat* A = cvCreateMat(2,2,CV_64FC1);
    cvPow(D, sqrtD, 0.5);
    cvTranspose(U, U_T);
    cvMatMul(U_T, sqrtD, A);
    cvMatMul(A, V, A);
// Set H2
    double t[] = {0, 0};
    double H2data[] = {cvmGet(A,0,0),cvmGet(A,0,1),t[0],
        cvmGet(A,1,0),cvmGet(A,1,1),t[1], 0,0,1};
    CvMat H2 = cvMat(3,3,CV_64FC1, H2data);
    CvMat *invH2 = cvCreateMat(3,3,CV_64FC1);
    cvInvert(&H2, invH2);
    cvZero(check);
    for (i=0; i<height; i++) //y - ver
    {
        for (j=0; j<width; j++) //x - hor
        {
// set X_a
            cvmSet(ptxp,0,0,(double)j);
            cvmSet(ptxp,1,0,(double)i);
            cvmSet(ptxp,2,0,1.0);
// compute X
            cvMatMul(invH2, ptxp, ptx);
            curpi = CLIP2(0, height-1, (int)(cvmGet(ptx,1,0)/cvmGet(ptx,2,0)));
            curpj = CLIP2(0, width-1, (int)(cvmGet(ptx,0,0)/cvmGet(ptx,2,0)));
            cvSet2D(img_scene,curpi,curpj,cvGet2D(img_affine,i,j));
            cvmSet(check,curpi,curpj,1);
        }
    }
// output reconstructed scene image
    img_interp = cvCloneImage(img_scene);
    data_interp = (uchar *)img_interp->imageData;
//interpolation
    for (i=1; i<height-1; i++) //y - ver
    {
        for (j=1; j<width-1; j++) //x - hor
        {
            if (cvmGet(check,i,j) == 0)
            {
                count = (cvmGet(check,i-
                                1,j)==1)+(cvmGet(check,i+1,j)==1)+(cvmGet(check,i,j-
                                                                          1)==1)+(cvmGet(check,i,j+1)==1);
                if (count != 0 )
                {
                    for (k=0; k<channels; k++)
                    {
                        data_interp[i*step+j*channels+k] =
                        (int)((data_scene[(i-
                                           1)*step+j*channels+k]+data_scene[(i+1)*step+j*channels+k]+data_scene[i*step+(j-
                                                                                                                        1)*channels+k]+data_scene[i*step+(j+1)*channels+k])/count);
                    }
                }
            }
        }
    }
    if (!cvSaveImage("scene.jpg",img_interp))
        printf("Could not save file\n");
// release the image
    cvReleaseImage(&img_in);
    cvReleaseImage(&img_affine);
    cvReleaseImage(&img_scene);
    cvReleaseImage(&img_interp);
    return 0;
}
