#include "opencv2/core/utility.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

Mat stereo_match(Mat left,Mat right,int window_size,int max_disparity) {
	int h = left.rows;
	int w = left.cols;
	Mat imgDisparity8U = Mat(left.rows, left.cols, CV_8U);
	int window_half = window_size / 2;
	int adjust = 255 / max_disparity; //adjust disparity value to visualize in 8 bit
	for (int y = window_half; y < h - window_half; y++) {
		for (int x = window_half; x < w - window_half; x++) {
			int prev_ssd = INT_MAX;
			int best_dis = 0;
			for (int off = 0; off < max_disparity; off++) {
				int ssd = 0;
				int ssd_tmp = 0;
				// calculate sum of square difference in windows given the offset (disparity)
				for (int v = -window_half; v < window_half; v++) {
					for (int u = -window_half; u < window_half; u++) {
						ssd_tmp = left.at<uchar>(y + v, x + u) - right.at<uchar>(y + v, x + u - off);
						ssd += ssd_tmp * ssd_tmp;
					}
					// store the best disparity (with smallest ssd)
					if (ssd < prev_ssd) {
						prev_ssd = ssd;
						best_dis = off;
					}
				}
				imgDisparity8U.at<uchar>(y, x) = best_dis * adjust;
			}
		
		}

	}

	return imgDisparity8U;
}

int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv,
		"{left||}{right||}{max-disparity|0|}{output||}");
	string leftname = parser.get<std::string>("left");
	string rightname = parser.get<std::string>("right");
	string outname = parser.get<std::string>("output");
	int max_disparity = parser.get<int>("max-disparity");

	
	Mat left = imread(leftname,0); //read images into grayscale
	Mat right = imread(rightname,0);
	
	Mat dis = stereo_match(left, right, 6, max_disparity);
	imwrite(outname,dis);
	return 0;
}