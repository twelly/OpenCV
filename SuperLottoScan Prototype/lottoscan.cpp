#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <stdio.h>


struct MatchError {
    int    digit;
    double error;
};


int cmp_match_error(const void* a, const void* b) {
    struct MatchError* error_a = (struct MatchError*)a;
    struct MatchError* error_b = (struct MatchError*)b;

    int ret_val = 0;

    if(error_a->error < error_b->error) {
        ret_val = -1;
    }
    else if(error_a->error > error_b->error) {
        ret_val = 1;
    }

    return(ret_val);
}

double pghMatchShapes(CvSeq *shape1, CvSeq *shape2) {
    int dims[] = {8, 8};

    CvHistogram* hist1 = cvCreateHist(2, dims, CV_HIST_ARRAY, NULL, 1);
    CvHistogram* hist2 = cvCreateHist(2, dims, CV_HIST_ARRAY, NULL, 1);

    cvCalcPGH(shape1, hist1);
    cvCalcPGH(shape2, hist2);

    cvNormalizeHist(hist1, 100.0f);
    cvNormalizeHist(hist2, 100.0f);

    double corr = cvCompareHist(hist1, hist2, CV_COMP_BHATTACHARYYA);

    cvReleaseHist(&hist1);
    cvReleaseHist(&hist2);

    return(corr);
}

void match_template(CvSeq* contours, CvSeq* tmpl_contours) {

    int    digit_array[] = {9, 8, 6, 7, 5, 4, 3, 2, 0, 1};
    struct MatchError match_errors[10];

    for(int num_idx = 0; tmpl_contours != 0; tmpl_contours = tmpl_contours->h_next, num_idx++) {
        if (tmpl_contours->total < 50) {
            continue;
        }

        double error = pghMatchShapes(contours, tmpl_contours);

        match_errors[num_idx].digit = digit_array[num_idx];
        match_errors[num_idx].error = error;
    }

    qsort(match_errors, 10, sizeof(struct MatchError), cmp_match_error);
 
    printf("Closest match: %d or %d\n", match_errors[0].digit, match_errors[1].digit);

    return;
}

int main(int argc, char** argv)
{
    char *lotto_ticket_img_file = argv[1];
    char *tmpl_0_to_9_img_file  = argv[2];

    // Load template fle
    IplImage* tmpl_0_to_9_img = cvLoadImage(tmpl_0_t0_9_img_file, 0);

    if(tmpl_0_to_9_img == NULL) {
        printf("\nERROR: Failed to load 0 to 9 template image file: %s\n", tmpl_0_t0_9_img_file);
        return(1);
    }

    // Load ticket image
    IplImage* lotto_ticket_img = cvLoadImage(lotto_ticket_img_file, 0);

    if(lotto_ticket_img == NULL) {
        printf("\nERROR: Failed to load ticket image file: %s\n", lotto_ticket_img_file);
        return(1);
    }

    IplImage* tmp_ticket_img = cvCreateImage(cvGetSize(lotto_ticket_img), lotto_ticket_img->depth, 1);

    // Pre-process images
    cvThreshold(tmpl_0_to_9_img , tmpl_0_to_9_img , 100, 255, CV_THRESH_BINARY);
    cvThreshold(lotto_ticket_img, lotto_ticket_img, 100, 255, CV_THRESH_BINARY_INV);

    cvNamedWindow("Temporary");

    // Find contours
    CvSeq*        contours      = 0;
    CvSeq*        tmpl_contours = 0;
    CvMemStorage* storage       = cvCreateMemStorage(0);

    cvFindContours(tmpl_0_to_9_img , storage, &tmpl_contours, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
    cvFindContours(lotto_ticket_img, storage, &contours     , sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

    for(; contours != 0; contours = contours->h_next) {
        if (contours->total < 50) {
            continue;
        }

        cvDrawContours(tmp_ticket_img, contours, cvScalarAll(255), cvScalar(255), 0, 1);
        cvShowImage("Temporary", tmp_ticket_img);

        match_template(contours, tmpl_contours);

        cvWaitKey(0);
    }

    // Cleanup
    cvReleaseMemStorage(&storage);

    cvReleaseImage(&tmpl_0_to_9_img);
    cvReleaseImage(&lotto_ticket_img);
    cvReleaseImage(&tmp_ticket_img);

    cvDestroyWindow("Temporary");

    return(0);
}
