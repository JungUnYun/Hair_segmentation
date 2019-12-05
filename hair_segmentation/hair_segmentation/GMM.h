#ifndef GMM_H_
#define GMM_H_
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
//
class Gauss {
public:
	//
	Gauss();
	//
	static double gauss(const double, const double, const double);
	//
	static double possibility(const cv::Vec3f&, const cv::Mat&, cv::Vec3f);
	//
	static void discret(std::vector<double>&, std::vector<double>&);
	//delta range from [0,6], sigma range from [0,delta/3]
	//
	void addsample(cv::Vec3f);
	//
	void learn();
	cv::Vec3f getmean()const { return mean; }
	cv::Mat getcovmat()const { return covmat; }
private:
	//
	cv::Vec3f mean;
	//
	cv::Mat covmat;
	//
	std::vector<cv::Vec3f> samples;
	
};


class GMM {
public:
	//
	static const int K = 5;
	//
	GMM(cv::Mat& _model);
	//
	double possibility(int, const cv::Vec3d) const;
	//
	double tWeight(const cv::Vec3d) const;
	//
	int choice(const cv::Vec3d) const;
	//
	void learningBegin();
	//
	void addSample(int, const cv::Vec3d);
	//
	void learningEnd();
private:
	//
	void calcuInvAndDet(int);
	//
	cv::Mat model;
	//
	double *coefs, *mean, *cov;
	//
	double covInv[K][3][3];
	//
	double covDet[K];
	//
	double sums[K][3];
	double prods[K][3][3];
	int sampleCounts[K];
	int totalSampleCount;
};
#endif