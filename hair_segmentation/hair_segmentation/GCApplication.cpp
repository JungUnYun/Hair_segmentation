#include <vector>
#include "GCApplication.h"

//Set value for the class
// 마스크를 백그라운드로 전부 해놓고 전경, 배경 픽셀 clear해놓음
void GCApplication::reset()
{
	if( !mask.empty() )
		mask.setTo(Scalar::all(GC_BGD));
	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear();  prFgdPxls.clear();

	isInitialized = false;
	rectState = NOT_SET;    
	lblsState = NOT_SET;
	prLblsState = NOT_SET;
	iterCount = 0;
}

//Set image and window name
// 이미지 사이즈의 마스크를 CV_8UC1로 만들어 놓는다
// CV_8UC1 -> 1채널 unsigned char
void GCApplication::setImageAndWinName( const Mat& _image, const string& _winName  )
{
	if( _image.empty() || _winName.empty() )
		return;
	image = &_image;
	winName = &_winName;
	mask.create( image->size(), CV_8UC1);
	reset();
}

//Show the result image
void GCApplication::showImage()
{
	if( image->empty() || winName->empty() )
		return;

	Mat res;
	Mat res1;	Size s;
	Mat binMask;
	if( !isInitialized )
		image->copyTo( res );		// 박스만 쳐진 영상
	else
	{
		getBinMask(mask, binMask);
		image->copyTo( res, binMask );  //show the GrabCuted image
	}
	imwrite("result.jpg", res);		// 결과 영상 저장
	vector<Point>::const_iterator it;		// 반복
	//Using four different colors show the point which have been selected
	for( it = bgdPxls.begin(); it != bgdPxls.end(); ++it )  
		circle( res, *it, radius, BLUE, thickness );
	for( it = fgdPxls.begin(); it != fgdPxls.end(); ++it )  
		circle( res, *it, radius, GREEN, thickness );
	for( it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it )
		circle( res, *it, radius, LIGHTBLUE, thickness );
	for( it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it )
		circle( res, *it, radius, PINK, thickness );
	s.height = res.rows * 2;
	s.width = res.cols * 2;
	resize(res, res1, s);
	imwrite("results.jpg", res1);
	//Draw the rectangle
	if( rectState == IN_PROCESS || rectState == SET )		// 네모 영역 표시해줄 때 빨간색 네모로
		rectangle( res, Point( rect.x, rect.y ), Point(rect.x + rect.width, rect.y + rect.height ), RED, 2);
	imshow( *winName, res );
}


//Using rect initialize the pixel 
void GCApplication::setRectInMask()
{
	assert( !mask.empty() );
	mask.setTo( GC_BGD );   //GC_BGD == 0  // 마스크 안을 백그라운드로 set
	rect.x = max(0, rect.x);
	rect.y = max(0, rect.y);
	rect.width = min(rect.width, image->cols-rect.x);
	rect.height = min(rect.height, image->rows-rect.y);
	(mask(rect)).setTo( Scalar(GC_PR_FGD) );    //GC_PR_FGD == 3	// rect안을 전경일수도 있는 것으로 set
}


void GCApplication::setLblsInMask( int flags, Point p, bool isPr )
{
	vector<Point> *bpxls, *fpxls;
	uchar bvalue, fvalue;
	if( !isPr ) //Points which are sure being FGD or BGD
	{
		bpxls = &bgdPxls;
		fpxls = &fgdPxls;
		bvalue = GC_BGD;    //0		// 배경이면 0
		fvalue = GC_FGD;    //1		// 전경이면 1
	}
	else    //Probably FGD or Probably BGD
	{
		bpxls = &prBgdPxls;
		fpxls = &prFgdPxls;
		bvalue = GC_PR_BGD; //2		// 배경일수도 있으면 2
		fvalue = GC_PR_FGD; //3		// 전경일수도 있으면 3
	}
	if( flags & BGD_KEY )	// 배경 키 눌리고 선택하면 그것들 배경일수도있는것으로
	{
		bpxls->push_back(p);
		circle( mask, p, radius, bvalue, thickness );   //Set point value = 2
	}
	if( flags & FGD_KEY )	// 전경 키 눌리고 선택하면 그것들 전경일수도있는것으로
	{
		fpxls->push_back(p);
		circle( mask, p, radius, fvalue, thickness );   //Set point value = 3
	}
}


//Mouse Click Function: flags work with CV_EVENT_FLAG 
void GCApplication::mouseClick( int event, int x, int y, int flags, void* )
{
	switch( event )
	{
	case CV_EVENT_LBUTTONDOWN: // Set rect or GC_BGD(GC_FGD) labels
		{
			bool isb = (flags & BGD_KEY) != 0,
				isf = (flags & FGD_KEY) != 0;
			if( rectState == NOT_SET && !isb && !isf )//Only LEFT_KEY pressed
			{
				rectState = IN_PROCESS; //Be drawing the rectangle
				rect = Rect( x, y, 1, 1 );
			}
			if ( (isb || isf) && rectState == SET ) //Set the BGD/FGD(labels).after press the "ALT" key or "SHIFT" key,and have finish drawing the rectangle
			lblsState = IN_PROCESS;
		}
		break;
	case CV_EVENT_RBUTTONDOWN: // Set GC_PR_BGD(GC_PR_FGD) labels
		{
			bool isb = (flags & BGD_KEY) != 0,
				isf = (flags & FGD_KEY) != 0;
			if ( (isb || isf) && rectState == SET ) //Set the probably FGD/BGD labels
				prLblsState = IN_PROCESS;
		}
		break;
	case CV_EVENT_LBUTTONUP:
		if( rectState == IN_PROCESS )
		{
			rect = Rect( Point(rect.x, rect.y), Point(x,y) );   //After draw the rectangle
			rectState = SET;
			setRectInMask();
			assert( bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty() );
			showImage();
		}
		if( lblsState == IN_PROCESS )   
		{
			setLblsInMask(flags, Point(x,y), false);    // Draw the FGD points
			lblsState = SET;
			showImage();
		}
		break;
	case CV_EVENT_RBUTTONUP:
		if( prLblsState == IN_PROCESS )
		{
			setLblsInMask(flags, Point(x,y), true); //Draw the BGD points
			prLblsState = SET;
			showImage();
		}
		break;
	case CV_EVENT_MOUSEMOVE:
		if( rectState == IN_PROCESS )
		{
			rect = Rect( Point(rect.x, rect.y), Point(x,y) );
			assert( bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty() );
			showImage();   //Continue showing image
		}
		else if( lblsState == IN_PROCESS )
		{
			setLblsInMask(flags, Point(x,y), false);
			showImage();
		}
		else if( prLblsState == IN_PROCESS )
		{
			setLblsInMask(flags, Point(x,y), true);
			showImage();
		}
		break;
	}
}

//Execute GrabCut algorithm，and return the iter count.
int GCApplication::nextIter()
{
	if( isInitialized )
		gc.GrabCut(*image, mask, rect, bgdModel, fgdModel,1,GC_CUT);
	else
	{
		if( rectState != SET )
			return iterCount;

		if( lblsState == SET || prLblsState == SET )
		 gc.GrabCut( *image, mask, rect, bgdModel, fgdModel, 1, GC_WITH_MASK );
		 else
		 gc.GrabCut(*image, mask, rect, bgdModel, fgdModel,1,GC_WITH_RECT);
		isInitialized = true;
	}
	iterCount++;

	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear(); prFgdPxls.clear();

	return iterCount;
}