#include "opencv2/core/utility.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <stdio.h>
#include <iostream>
#include <omp.h>
using namespace cv;
using namespace std;
class Stereo {
private:
	int win_size_, max_disparity_, tran_win_size_;
	string cost_;
	bool parallel_;
public:
	
	Stereo(int window_size, int max_dis, int tran_size,string cost, bool parallel) {
		win_size_ = window_size;
		max_disparity_ = max_dis;
		tran_win_size_ = tran_size;
		cost_ = cost;
		parallel_ = parallel;
	}
	//int get_tranwinsize() { return tran_win_size_; } // dubug
	Mat rank_transform(Mat image, int tran_size);
	Mat census_transform(Mat image, int tran_size);
	Mat stereo_match(Mat left, Mat right);
	Mat stereo_match_parallel(Mat left, Mat right);
	
};