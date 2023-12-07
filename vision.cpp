#include "vision.hpp"

Mat preprocess(Mat input){
    static string dst3 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! h264parse ! rtph264pay pt=96 ! udpsink host=203.234.58.169 port=8003 sync=false";
    static VideoWriter writer3(dst3, 0, (double)30, Size(640, 90), false);

    Mat cut_gray;
    cvtColor(input(Rect(0, 270, 640, 90)), cut_gray, COLOR_BGR2GRAY);
    cut_gray = cut_gray + (Scalar(160) - mean(cut_gray));
    GaussianBlur(cut_gray, cut_gray, Size(9, 9), 3, 3);
    writer3 << cut_gray;

    threshold(cut_gray, cut_gray, 190, 256, THRESH_BINARY);
    morphologyEx(cut_gray, cut_gray, MORPH_CLOSE, getStructuringElement(0, Size(7, 7)));
    
    return cut_gray;
}


int calc_err(Mat gray_img) {
    static string dst2 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! h264parse ! rtph264pay pt=96 ! udpsink host=203.234.58.169 port=8002 sync=false";
    static VideoWriter writer2(dst2, 0, (double)30, Size(640, 90), true);


    Mat labels, stats, centroids;
    int cnt = connectedComponentsWithStats(gray_img, labels, stats, centroids);

    Mat dst;
    cvtColor(gray_img, dst, COLOR_GRAY2BGR);
    static Point center_variable(320, 45);
    Rect fit;

    int min_norm = INT_MAX;

    Rect sel;
    for (int i = 1; i < cnt; i++) {
        int* p = stats.ptr<int>(i);
        Rect r(p[0], p[1], p[2], p[3]);
        rectangle(dst, r, Scalar(150, 150, 150), 1, 8);

        Point r_center = (r.br() + r.tl()) * 0.5;
        int diff_length = norm(center_variable - r_center);
        drawMarker(dst, r_center, Scalar(0, 0, 255), 0, 10, 2, 8);
        if (min_norm > diff_length && p[1] + p[3] > 60 && diff_length < 120) {
            sel = r;
            min_norm = diff_length;
            center_variable = r_center;
        }
    }
    if (!sel.empty()) rectangle(dst, sel, Scalar(0, 255, 255), 2);
    drawMarker(dst, center_variable, Scalar(255, 128, 255), 3, 10, 2, 8);
    writer2 << dst;
    return 320 - center_variable.x;
}
