#include <ppl.h>
#include "ssd_stereo.h"



#ifdef _WIN64 || _WIN32
	// use __popcnt (windows) built-in function to count the number of 1 in integer 
	static inline int HammingDistance(int a, int b) { return static_cast<int>(__popcnt(a ^ b)); }
#else
// to calculate metric for census transform
int HammingDistance(const int& a, const int& b) {
	int d = a ^ b;
	int res = 0;
	while (d > 0) {
		res += d & 1;
		d >>= 1;
	}
	return res;
}
#endif



Mat Stereo::rank_transform(Mat image, int windowsize) {
	int h = image.rows;
	int w = image.cols;
	Mat imgDisparity8U = Mat(image.rows, image.cols, CV_8U);
	int window_half = windowsize / 2;

	for (int y = window_half; y < h - window_half; ++y) {
		for (int x = window_half; x < w - window_half; ++x) {
			int ssd = 0;

			for (int v = -window_half; v < window_half + 1; ++v) {
				for (int u = -window_half; u < window_half + 1; ++u) {

					if (image.at<uchar>(y + v, x + u) > image.at<uchar>(y, x)) ++ssd;
				}

			}

			imgDisparity8U.at<uchar>(y, x) = ssd;

		}
	}
	return imgDisparity8U;
}
Mat Stereo::census_transform(Mat image, int windowsize) {
	int h = image.rows;
	int w = image.cols;
	Mat imgDisparity8U = Mat(image.rows, image.cols, CV_8U);
	int window_half = windowsize / 2;

	for (int y = window_half; y < h - window_half; ++y) {
		for (int x = window_half; x < w - window_half; ++x) {
			int ssd = 0;

			for (int v = -window_half; v < window_half + 1; ++v) {
				for (int u = -window_half; u < window_half + 1; ++u) {
					if (v != 0 && u != 0) { // skip the central pixel
						ssd <<= 1;
						if (image.at<uchar>(y + v, x + u) > image.at<uchar>(y, x))  ssd = ssd + 1; // assign last digit to 1 if pixel is larger than central pixel in the windows else assign 0
					}
				}

			}

			imgDisparity8U.at<uchar>(y, x) = ssd;

		}
	}
	return imgDisparity8U;
}
Mat Stereo::stereo_match(Mat left, Mat right) {
	int h = left.rows;
	int w = left.cols;
	Mat imgDisparity8U = Mat(left.rows, left.cols, CV_8U);
	int window_half = win_size_ / 2;
	int adjust = 255 / max_disparity_;
	//decide which matching cost function to use 
	if (cost_ == "rank") {
		left = Stereo::rank_transform(left, tran_win_size_);
		right = Stereo::rank_transform(right, tran_win_size_);
	}
	else if (cost_ == "census") {
		left = Stereo::census_transform(left, tran_win_size_);
		right = Stereo::census_transform(right, tran_win_size_);
	}
	if (parallel_) {
#pragma omp parallel for 
		for (int y = window_half; y < h - window_half; ++y) {
			uchar *imgDisparity_y = imgDisparity8U.ptr(y);
			for (int x = window_half; x < w - window_half; ++x) {
				int prev_ssd = INT_MAX;
				int best_dis = 0;
				for (int off = 0; off < max_disparity_; ++off) {
					int ssd = 0;
					int ssd_tmp = 0;
					for (int v = -window_half; v < window_half; ++v) {

						for (int u = -window_half; u < window_half; ++u) {
							if (cost_ == "census") {

								ssd_tmp = HammingDistance(left.at<uchar>(y + v, x + u), right.at<uchar>(y + v, x + u - off));
								//ssd_tmp = hamming_distance(left.at<uchar>(y + v, x + u), right.at<uchar>(y + v, x + u - off));
							}
							else {
								ssd_tmp = left.at<uchar>(y + v, x + u) - right.at<uchar>(y + v, x + u - off);
							}
							ssd += ssd_tmp * ssd_tmp;
						}

					}
					if (ssd < prev_ssd) {
						prev_ssd = ssd;
						best_dis = off;
					}


				}

				imgDisparity_y[x] = best_dis * adjust;
			}
		}
	}
	else {
		for (int y = window_half; y < h - window_half; ++y) {
			uchar *imgDisparity_y = imgDisparity8U.ptr(y);
			for (int x = window_half; x < w - window_half; ++x) {
				int prev_ssd = INT_MAX;
				int best_dis = 0;
				for (int off = 0; off < max_disparity_; ++off) {
					int ssd = 0;
					int ssd_tmp = 0;
					for (int v = -window_half; v < window_half; ++v) {

						for (int u = -window_half; u < window_half; ++u) {

							if (cost_ == "census") {
								ssd_tmp = HammingDistance(left.at<uchar>(y + v, x + u), right.at<uchar>(y + v, x + u - off));
							}
							else {
								ssd_tmp = left.at<uchar>(y + v, x + u) - right.at<uchar>(y + v, x + u - off);
							}
							ssd += ssd_tmp * ssd_tmp;
						}

					}
					if (ssd < prev_ssd) {
						prev_ssd = ssd;
						best_dis = off;
					}


				}

				imgDisparity_y[x] = best_dis * adjust;
			}
		}
	}

	//imwrite("dis.png", imgDisparity8U);
	return imgDisparity8U;
}


//vc++ concurrency implementation of parallel execution, but slower than openMP, not good for census
//
Mat Stereo::stereo_match_parallel(Mat left, Mat right) {
	int h = left.rows;
	int w = left.cols;
	Mat imgDisparity8U = Mat(left.rows, left.cols, CV_8U);
	float window_half = win_size_ / 2;
	int adjust = 255 / max_disparity_;
	//for (int y = window_half; y < h - window_half; y++) {
	//using namespace concurrency;
	concurrency::parallel_for (size_t(window_half), size_t(h - window_half),[&](size_t y){
		uchar *imgDisparity_y = imgDisparity8U.ptr(y);
		for (int x = window_half; x < w - window_half; x++) {
			int prev_ssd = INT_MAX;
			int best_dis = 0;
			for (int off = 0; off < max_disparity_; off++) {
				int ssd = 0;
				int ssd_tmp = 0;
				for (int v = -window_half; v < window_half; v++) {

					for (int u = -window_half; u < window_half; u++) {

						if (cost_ == "census") {
							ssd_tmp = HammingDistance(left.at<uchar>(y + v, x + u), right.at<uchar>(y + v, x + u - off));
						}
						else {
							ssd_tmp = left.at<uchar>(y + v, x + u) - right.at<uchar>(y + v, x + u - off);
						}
						ssd += ssd_tmp * ssd_tmp;
					}

				}
				if (ssd < prev_ssd) {
					prev_ssd = ssd;
					best_dis = off;
				}


			}

			imgDisparity_y[x] = best_dis * adjust;
		}
	});
	//cout << "finished computing " << endl;
	//imwrite("dis.png", imgDisparity8U);
	return imgDisparity8U;
}



int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv,
		"{left||}{right||}{max-disparity|0|}{window_size|0|}{tranwin_size|0|}{output||}{parallel||}{cost||}{windows||}");
	string leftname = parser.get<std::string>("left");
	string rightname = parser.get<std::string>("right");
	string outname = parser.get<std::string>("output");
	string parallel = parser.get<std::string>("parallel");
	string windows = parser.get<std::string>("windows");
	string cost = parser.get<std::string>("cost");
	int max_disparity = parser.get<int>("max-disparity");
	int window_size = parser.get<int>("window_size");
	int tranwin_size = parser.get<int>("tranwin_size");

	Mat dis;
	Mat left = imread(leftname, 0); //read images into grayscale
	Mat right = imread(rightname, 0);
	int64 t2 = getTickCount();
	if (parallel == "no") {
		Stereo s(window_size, max_disparity, tranwin_size, cost, false);
		dis = s.stereo_match(left, right);
	}
	else {
		Stereo s(window_size, max_disparity, tranwin_size, cost, true);
		if (windows == "yes") {
			dis = s.stereo_match_parallel(left, right);
		}
		else {
			dis = s.stereo_match(left, right);
		}
	}



	t2 = getTickCount() - t2;
	printf("census Time elapsed: %fms\n", t2 * 1000 / getTickFrequency());
	cv::imwrite(outname, dis);

	//system("pause");
	return 0;
}

