
#include "ssd_stereo.h"

Mat Stereo::rank_transform(Mat image, int windowsize) {
	int h = image.rows;
	int w = image.cols;
	Mat imgDisparity8U = Mat(image.rows, image.cols, CV_8U);
	int window_half = windowsize / 2;

	for (int y = window_half; y < h - window_half; y++) {
		for (int x = window_half; x < w - window_half; x++) {
			int ssd = 0;

			for (int v = -window_half; v < window_half + 1; v++) {
				for (int u = -window_half; u < window_half + 1; u++) {

					if (image.at<uchar>(y + v, x + u) > image.at<uchar>(y, x)) ssd++;
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

	for (int y = window_half; y < h - window_half; y++) {
		for (int x = window_half; x < w - window_half; x++) {
			int ssd = 0;

			for (int v = -window_half; v < window_half + 1; v++) {
				for (int u = -window_half; u < window_half + 1; u++) {
					ssd <<= 1;
					if (image.at<uchar>(y + v, x + u) > image.at<uchar>(y, x)) ssd = ssd | 1;
					else  ssd = ssd | 0;
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
		for (int y = window_half; y < h - window_half; y++) {
			uchar *imgDisparity_y = imgDisparity8U.ptr(y);
			for (int x = window_half; x < w - window_half; x++) {
				int prev_ssd = INT_MAX;
				int best_dis = 0;
				for (int off = 0; off < max_disparity_; off++) {
					int ssd = 0;
					int ssd_tmp = 0;
					for (int v = -window_half; v < window_half; v++) {

						for (int u = -window_half; u < window_half; u++) {

							ssd_tmp = left.at<uchar>(y + v, x + u) - right.at<uchar>(y + v, x + u - off);
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
		for (int y = window_half; y < h - window_half; y++) {
			uchar *imgDisparity_y = imgDisparity8U.ptr(y);
			for (int x = window_half; x < w - window_half; x++) {
				int prev_ssd = INT_MAX;
				int best_dis = 0;
				for (int off = 0; off < max_disparity_; off++) {
					int ssd = 0;
					int ssd_tmp = 0;
					for (int v = -window_half; v < window_half; v++) {

						for (int u = -window_half; u < window_half; u++) {

							ssd_tmp = left.at<uchar>(y + v, x + u) - right.at<uchar>(y + v, x + u - off);
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


//opencv implementation of parallel execution, but didn't work well
//
//Mat stereo_match_parallel(Mat left, Mat right, int window_size, int max_disparity) {
//	int h = left.rows;
//	int w = left.cols;
//	Mat imgDisparity8U = Mat(left.rows, left.cols, CV_8U);
//	float window_half = window_size / 2;
//	int adjust = 255 / max_disparity;
//	parallel_for_(Range(0, w*h), [&](const Range& range) {
//		for (int r = range.start; r < range.end; r++) {
//			float x = r / h + window_half;
//			float y = r % h + window_half;
//			if (x < w - window_half && y < h - window_half) {
//				//cout << r << endl;
//				//system("pause");
//
//				int prev_ssd = INT_MAX;
//				int best_dis = 0;
//				for (int off = 0; off < max_disparity; off++) {
//					int ssd = 0;
//					int ssd_tmp = 0;
//					for (int v = -window_half; v < window_half; v++) {
//						//uchar *leftdata = left.ptr<uchar>(static_cast<int>(y) + v);
//						//uchar *rightdata = right.ptr<uchar>(static_cast<int>(y) + v);
//						for (int u = -window_half; u < window_half; u++) {
//
//
//
//							ssd_tmp = left.at<uchar>(static_cast<int>(y) + v, static_cast<int>(x) + u) - right.at<uchar>(static_cast<int>(y) + v, static_cast<int>(x) + u - off);
//							//ssd_tmp = leftdata[static_cast<int>(x) + u] - rightdata[static_cast<int>(x) + u - off]; 
//							ssd += ssd_tmp * ssd_tmp;
//						}
//						
//					}
//					if (ssd < prev_ssd) {
//						prev_ssd = ssd;
//						best_dis = off;
//					}
//
//
//				}
//
//				/*cout << x << endl;
//				cout << y << endl;
//				cout << best_dis * adjust << endl;*/
//				imgDisparity8U.at<uchar>(static_cast<int>(y), static_cast<int>(x)) = best_dis * adjust;
//				//system("pause");
//			}
//			else continue;
//		}
//	});
//	//cout << "finished computing " << endl;
//	imwrite("dis.png", imgDisparity8U);
//	return imgDisparity8U;
//}



int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv,
		"{left||}{right||}{max-disparity|0|}{window_size|0|}{tranwin_size|0|}{output||}{parallel||}{cost||}");
	string leftname = parser.get<std::string>("left");
	string rightname = parser.get<std::string>("right");
	string outname = parser.get<std::string>("output");
	string parallel = parser.get<std::string>("parallel");
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
		dis = s.stereo_match(left, right);
	}



	t2 = getTickCount() - t2;
	printf("census Time elapsed: %fms\n", t2 * 1000 / getTickFrequency());
	imwrite(outname, dis);

	//system("pause");
	return 0;
}

