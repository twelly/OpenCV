#include <cv.h>
#include <highgui.h>
#include <stdio.h>


#define TEMPLATE_IMG_FILE "/home/jong/Desktop/templates/template_0_to_9.png"
#define DIFF_THRESHOLD    4


int findTemplateMatch(IplImage* source_img, IplImage* template_img) {
    int img_width  = source_img->width;
    int img_height = source_img->height;
    int tpl_width  = template_img->width;
    int tpl_height = template_img->height;
    int res_width  = img_width  - tpl_width  + 1;
    int res_height = img_height - tpl_height + 1;

    // Resize the template image to match source image
    IplImage* resized_template_img;

    if (tpl_height != img_height) {
        tpl_width  = (tpl_width * img_height) / tpl_height;
        tpl_height = img_height;
        res_width  = img_width  - tpl_width  + 1;
        res_height = img_height - tpl_height + 1;

        resized_template_img = cvCreateImage(cvSize(tpl_width, tpl_height),
                                             template_img->depth, 
                                             template_img->nChannels);

        cvResize(template_img, resized_template_img);
    }
    else {
        resized_template_img = template_img;
    }

    // Create new image for template matching computation
    IplImage* result_img = cvCreateImage(cvSize(res_width, res_height), 32, 1);

    cvMatchTemplate(source_img, resized_template_img, result_img, CV_TM_CCOEFF_NORMED);

    CvPoint minloc;
    CvPoint maxloc;
    double  minval;
    double  maxval;

    cvMinMaxLoc(result_img, &minval, &maxval, &minloc, &maxloc, 0);

    int template_match = -1;
    int template_x_pos[] = {0, 44, 77, 120, 163, 206, 250, 299, 344, 389};

    for(int pos_idx = 0; pos_idx <= 9; pos_idx++) {
        int diff = abs(maxloc.x - template_x_pos[pos_idx]);

        if (diff <= DIFF_THRESHOLD) {
            template_match = pos_idx;
            break;
        }
    }

    return(template_match);
}

int main(int argc, char** argv) {
    char *lotto_ticket_img_file = argv[1];

    // Load ticket and lotto image files
    IplImage* lotto_ticket_img = cvLoadImage(lotto_ticket_img_file, 0);
    IplImage* template_img     = cvLoadImage(TEMPLATE_IMG_FILE, 0);

    // Pre-process lotto ticket image
    cvThreshold(lotto_ticket_img, lotto_ticket_img, 100, 255, CV_THRESH_BINARY_INV);
    cvThreshold(template_img    , template_img    , 100, 255, CV_THRESH_BINARY_INV);

    // Duplicate lotto ticket image for processing
    IplImage* clone_ticket_img = cvCloneImage(lotto_ticket_img);
 
    // Show ticket image to visualize ROI being processed
    cvNamedWindow("Ticket");

    // Create individual character image from lotto ticket and find a match in template
    CvSeq*        contours = 0;
    CvMemStorage* storage  = cvCreateMemStorage(0);

    cvFindContours(clone_ticket_img, storage, &contours, sizeof(CvContour), CV_RETR_CCOMP);

    for(; contours != 0; contours = contours->h_next) {
        CvRect rect = cvBoundingRect(contours);

        if ((rect.width * rect.height) < 500) {
            continue;
        }

        // Copy image region for template matching
        cvSetImageROI(lotto_ticket_img, rect);

        IplImage *roi_img = cvCreateImage(cvSize(rect.width, rect.height),
                                          lotto_ticket_img->depth, 
                                          lotto_ticket_img->nChannels);
 
        cvCopy(lotto_ticket_img, roi_img, NULL);

        cvShowImage("Ticket", roi_img);

        int template_match = findTemplateMatch(template_img, roi_img);
    
        printf("Matching Template is %d\n", template_match);

        cvResetImageROI(lotto_ticket_img);

        cvWaitKey(0);
    }

    // Cleanup
    cvReleaseImage(&lotto_ticket_img);
    cvReleaseImage(&clone_ticket_img);
    cvReleaseImage(&template_img);

    cvDestroyWindow("Ticket");

    return(0);
}
