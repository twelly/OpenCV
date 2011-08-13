#include <stdio.h>
#include <TemplateMatch.h>


// Globals
IplImage*   G_template_images[2]           = {NULL, NULL};
const char* G_template_files[2]            = {TEMPLATE_0_TO_9_FILE, TEMPLATE_A_TO_Z_FILE};
int         G_template_valid_coords[2][10] = {{0, 44, 77, 120, 163, 206, 250, 299, 344, 389},
                                              {0, 44, 77, 120, 163, 206, 250, 299, 344, 389}};

int tplm_cmpMatchInfo(const void* p_a, const void* p_b) {
    TplmMatchInfo* info_a = (TplmMatchInfo*)p_a;
    TplmMatchInfo* info_b = (TplmMatchInfo*)p_b;

    int ret_val    = 0;
    int y_diff     = info_a->y_coord - info_b->y_coord;
    int abs_y_diff = abs(y_diff);

    if(abs_y_diff <= MATCH_DIFF_THRESHOLD) {
        if (info_a->x_coord == info_b->x_coord) {
            ret_val = 0;
        }
        else if (info_a->x_coord < info_b->x_coord) {
            ret_val = -1;
        }
        else {
            ret_val = 1;
        }
    }
    else {
        if(y_diff < 0) {
            ret_val = -1;
        }
        else {
            ret_val = 1;
        }
    }

    return(ret_val);
}

void tplm_sortMatches(TplmMatchInfo* p_match_info_arr, int p_num_items) {
    qsort(p_match_info_arr, p_num_items, sizeof(TplmMatchInfo), tplm_cmpMatchInfo);

    int row = 0;
    
    for(int idx = 0; idx < p_num_items; idx++) {
        int diff = abs(row - p_match_info_arr[idx].y_coord);

        if (diff > MATCH_DISP_THRESHOLD) {
            row = p_match_info_arr[idx].y_coord;
            printf("\n");
        }

        if (p_match_info_arr[idx].match == -1) {
            printf("? ");
        }
        else {
            printf("%d ", p_match_info_arr[idx].match);
        }
    }

    printf("\n");
}

void tplm_initialize(void) {
    // Load the template images and binarize
    int num_templates = sizeof(G_template_files) / sizeof(char*);

    for(int template_idx = 0; template_idx < num_templates; template_idx++) {
        G_template_images[template_idx] = cvLoadImage(G_template_files[template_idx], 0);
        cvThreshold(G_template_images[template_idx], G_template_images[template_idx], 90, 255, CV_THRESH_BINARY_INV);
    }
}

void tplm_destroy(void) {
    // Destory the template images
    int num_templates = sizeof(G_template_images) / sizeof(char*);

    for(int template_idx = 0; template_idx < num_templates; template_idx++) {
        cvReleaseImage(&G_template_images[template_idx]);
    }
}

bool tplm_is_valid_contour(CvSeq* p_contour) {
    bool   valid = true;
    CvRect rect  = cvBoundingRect(p_contour);
    int    area  = rect.width * rect.height;
    
    //printf("%d %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height, area);

    if ((rect.width  <= CONTOUR_WIDTH_THRESHOLD)  or
        (rect.height <= CONTOUR_HEIGHT_THRESHOLD) or 
        (area        < CONTOUR_AREA_THRESHOLD)) {
        valid = false;
    }

    return(valid);
}

TplmFindMatchInfo tplm_findMatch(int p_template, IplImage* p_target_img) {
    // Get the template image
    IplImage* template_img = G_template_images[p_template];

    // Get image dimensions
    int template_img_width  = template_img->width;
    int template_img_height = template_img->height;
    int target_img_width    = p_target_img->width;
    int target_img_height   = p_target_img->height;
    int result_img_width    = template_img_width  - target_img_width  + 1;
    int result_img_height   = template_img_height - target_img_height + 1;

    // Resize the target image to match template image
    IplImage* resized_target_img;

    if (template_img_height != target_img_height) {
        // Just do a simple ratio and proportion calculation
        target_img_width  = (target_img_width * template_img_height) / target_img_height;
        target_img_height = template_img_height;
        result_img_width  = template_img_width  - target_img_width  + 1;
        result_img_height = template_img_height - target_img_height + 1;

        CvSize resized_target_img_size = cvSize(target_img_width, target_img_height);

        resized_target_img = cvCreateImage(resized_target_img_size, p_target_img->depth, p_target_img->nChannels);

        cvResize(p_target_img, resized_target_img);
    }
    else {
        resized_target_img = cvCloneImage(p_target_img);
    }

    int     template_match = -1;
    CvPoint minloc;
    CvPoint maxloc;
    double  minval;
    double  maxval;

    if ((result_img_width > 0) and (result_img_height > 0)) {
        // Find a match of the target image from the template image
        CvSize result_img_size = cvSize(result_img_width, result_img_height);
        IplImage* result_img   = cvCreateImage(result_img_size, 32, 1);
    
        cvMatchTemplate(template_img, resized_target_img, result_img, CV_TM_CCOEFF_NORMED);
    
        cvMinMaxLoc(result_img, &minval, &maxval, &minloc, &maxloc, 0);
    
        int* template_x_pos     = G_template_valid_coords[p_template];
        int  percent_confidence = (int)(maxval * 100);
    
        if (percent_confidence > MATCH_CONFIDENCE_THRESHOLD) {
            // Find the max x coordinates that matches the template
            for(int pos_idx = 0; pos_idx <= 9; pos_idx++) {
                int diff = abs(maxloc.x - template_x_pos[pos_idx]);
        
                if (diff <= TEMPLATE_X_COORD_DIFF_THRESHOLD) {
                    template_match = pos_idx;
                    break;
                }
            }
        }

        //printf("Matching Template is %d, minval=%f, maxval=%f\n", template_match, minval, maxval);
    }

    // Clean up
    cvReleaseImage(&resized_target_img);

    TplmFindMatchInfo match_info;
    match_info.match      = template_match;
    match_info.confidence = maxval;

    return(match_info);
}

void tplm_findMatches(IplImage* p_source_img) {
    IplImage*     clone_source_img = cvCloneImage(p_source_img);
    CvSeq*        contours         = NULL;
    CvMemStorage* storage          = cvCreateMemStorage(0);

    int match_info_idx = 0;
    TplmMatchInfo match_info_array[250];

    //cvNamedWindow("debug");

    // Find and process contours
    cvFindContours(clone_source_img, storage, &contours, sizeof(CvContour), CV_RETR_CCOMP);

    for(; contours != 0; contours = contours->h_next) {
        // Validate contour
        bool valid_contour = tplm_is_valid_contour(contours);

        if (valid_contour == false) {
            continue;
        }

        // Get image region for template matching
        CvRect rect         = cvBoundingRect(contours);
        CvSize roi_img_size = cvSize(rect.width, rect.height);

        cvSetImageROI(p_source_img, rect);

        IplImage *roi_img = cvCreateImage(roi_img_size, p_source_img->depth, p_source_img->nChannels);
 
        cvCopy(p_source_img, roi_img, NULL);

        TplmFindMatchInfo find_info = tplm_findMatch(TEMPLATE_0_TO_9, roi_img);

        match_info_array[match_info_idx].match      = find_info.match;
        match_info_array[match_info_idx].x_coord    = rect.x;
        match_info_array[match_info_idx].y_coord    = rect.y;
        match_info_array[match_info_idx].width      = rect.width;
        match_info_array[match_info_idx].height     = rect.height;
        match_info_array[match_info_idx].confidence = find_info.confidence;
        match_info_idx++;

        //printf("m=%d, x=%d, y=%d, w=%d, h=%d, c=%f\n", find_info.match, rect.x, rect.y, rect.width, rect.height, find_info.confidence);

        //cvShowImage("debug", roi_img);
        //cvWaitKey(0);

        cvReleaseImage(&roi_img);

        cvResetImageROI(p_source_img);
    }

    // Sort matches
    tplm_sortMatches(match_info_array, match_info_idx);

    // Clean up
    cvReleaseMemStorage(&storage);

    cvReleaseImage(&clone_source_img);

    //cvDestroyWindow("debug");
}
