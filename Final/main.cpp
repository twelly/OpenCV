#include <stdio.h>
#include <TemplateMatch.h>


int main(int argc, char** argv) {
    char *lotto_ticket_img_file = argv[1];

    // Load ticket and lotto image files
    IplImage* lotto_ticket_img = cvLoadImage(lotto_ticket_img_file, 0);

    if (lotto_ticket_img == NULL) {
        return(1);
    }

    // Pre-process lotto ticket image
    cvThreshold(lotto_ticket_img, lotto_ticket_img, 90, 255, CV_THRESH_BINARY_INV);
    //cvSmooth(lotto_ticket_img, lotto_ticket_img, CV_GAUSSIAN, 3, 0, 0, 0);
    //cvErode(lotto_ticket_img, lotto_ticket_img, NULL, 1);
    //cvDilate(lotto_ticket_img, lotto_ticket_img, NULL, 1);

    // Find template match
    tplm_initialize();
    tplm_findMatches(lotto_ticket_img);
    tplm_destroy();

    // Cleanup
    cvReleaseImage(&lotto_ticket_img);

    return(0);
}
