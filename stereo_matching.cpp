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

	return imgDisparity8U;
}

Mat stereo_match_parallel(Mat left, Mat right, int window_size, int max_disparity) {
	int h = left.rows;
	int w = left.cols;
	Mat imgDisparity8U = Mat(left.rows, left.cols, CV_8U);
	float window_half = window_size / 2;
	int adjust = 255 / max_disparity;
	// parallel computation with lambda expression (c++11 or later)
	parallel_for_(Range(0, w*h), [&](const Range& range) {
		for (int r = range.start; r < range.end; r++) {
			float x = r / h + window_half;
			float y = r % h + window_half;
			if (x < w - window_half && y < h - window_half) {

				int prev_ssd = INT_MAX;
				int best_dis = 0;
				for (int off = 0; off < max_disparity; off++) {
					int ssd = 0;
					int ssd_tmp = 0;
					for (int v = -window_half; v < window_half; v++) {
						for (int u = -window_half; u < window_half; u++) {



							ssd_tmp = left.at<uchar>(static_cast<int>(y) + v, static_cast<int>(x) + u) - right.at<uchar>(static_cast<int>(y) + v, static_cast<int>(x) + u - off);
							ssd += ssd_tmp * ssd_tmp;
						}
					}
					if (ssd < prev_ssd) {
							prev_ssd = ssd;
							best_dis = off;
					}


				}
				
				imgDisparity8U.at<uchar>(static_cast<int>(y), static_cast<int>(x)) = best_dis * adjust;
				
			}
			else continue;
		}
	});
	
	return imgDisparity8U;
}

int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv,
		"{left||}{right||}{max-disparity|0|}{output||}{parallel||}");
	string leftname = parser.get<std::string>("left");
	string rightname = parser.get<std::string>("right");
	string outname = parser.get<std::string>("output");
	string parallel = parser.get<std::string>("parallel");
	int max_disparity = parser.get<int>("max-disparity");

	Mat dis
	Mat left = imread(leftname,0); //read images into grayscale
	Mat right = imread(rightname,0);
	if (parallel == "no"){
		dis = stereo_match(left, right, 6, max_disparity);
	}
	else dis = stereo_match_parallel(left, right, 6, max_disparity);
	imwrite(outname,dis);
	return 0;
}