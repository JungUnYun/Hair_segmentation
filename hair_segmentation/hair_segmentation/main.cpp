#include <iostream>
#include <opencv2/opencv.hpp>
#include "GCApplication.h"
static void help()
{
	std::cout << 
		"\nSelect a rectangular area around the object you want to segment\n" <<
		"\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tr - restore the original image\n"
		"\tn - next iteration\n"
		"\n"
		"\tleft mouse button - set rectangle\n"
		"\n"
		"\tCTRL+left mouse button - set GC_BGD pixels\n"
		"\tSHIFT+left mouse button - set CG_FGD pixels\n"
		"\n"
		//"\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
		//"\tSHIFT+right mouse button - set CG_PR_FGD pixels\n"
		<< endl;
}


GCApplication gcapp;

static void on_mouse( int event, int x, int y, int flags, void* param )
{
	gcapp.mouseClick( event, x, y, flags, param );
}


int main()
{
	//string filename = "D:\\JUYUN\\4.jpg";
		//"F:\\Data\\Ls_Data\\00\\0050.bmp";
	// 이미지 읽기
	string filename = "3_s.jpg";
	Mat image = imread( filename, 1 );
	Size s;		// 영상 사이즈
	s.height = image.rows / 2;
	s.width = image.cols / 2;
	///////////////////////////////////////////////////////
	// 영상의 가로가 150보다 크면 가로를 150으로 만들고
	// 영상의 세로는 비율에 맞게 나눠준다
/*	if ( image.cols > 300 )
	{
		//s.height = image.rows / 2;
		//s.width = image.cols / 2;
		int k = image.cols / 300;
		s.height = image.rows / k;
		s.width = 300;
		//s.height = image.rows;
		//s.width = image.cols;
	}
	else
	{
		s.height = image.rows;
		s.width = image.cols;
	}
	
*/	///////////////////////////////////////////////////////

	resize(image, image, s);	// 스트록 주기 위한 resize
	
	if( image.empty() )
	{
		cout << "\n , couldn't read image filename " << filename << endl;
		return 1;
	}

	help();		// 위의 도움말 출력

	const string winName = "image";		// 스트록 주기 위한 윈도우 이름
	cvNamedWindow( winName.c_str(), CV_WINDOW_AUTOSIZE );	// window autosize
	cvSetMouseCallback( winName.c_str(), on_mouse, 0 );		// 마우스 입력 받기 위해

	gcapp.setImageAndWinName( image, winName );		// winName의 윈도우에 image set
	gcapp.showImage();								// 이미지 보여주기

	for(;;)
	{
		int c = cvWaitKey(0);
		switch( (char) c )
		{
		case '\x1b':	// esc 눌렀을 때 종료
			cout << "Exiting ..." << endl;
			goto exit_main;
		case 'r':		// r 눌렀을 때 reset 후 이미지 다시 보여주기
			cout << endl;
			gcapp.reset();
			gcapp.showImage();
			break;
		case 'n':		// n 눌렀을 때 grabcut 진행
			int iterCount = gcapp.getIterCount();
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if( newIterCount > iterCount )
			{
				gcapp.showImage();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}

exit_main:
	cvDestroyWindow( winName.c_str() );
	return 0;

	//return 0;
}