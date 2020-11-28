// ColorCastDetection.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"


double CastDetection(IplImage* src)
{
	IplImage* lab = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvCvtColor(src, lab, CV_BGR2Lab);
	IplImage* l = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage* a = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage* b = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage* fa = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* fb = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvSplit(lab, l, a, b, NULL);
	cvConvertScale(a, fa, 1.0, -128);
	cvConvertScale(b, fb, 1.0, -128);
	CvScalar aMean = cvAvg(fa);
	CvScalar bMean = cvAvg(fb);
	double D = sqrt(aMean.val[0] * aMean.val[0] + bMean.val[0] * bMean.val[0]);
	cvConvertScale(fa, fa, 1.0, -aMean.val[0]);
	cvConvertScale(fb, fb, 1.0, -bMean.val[0]);
	cvPow(fa, fa, 2.0);
	cvPow(fb, fb, 2.0);
	CvScalar aSquareMean = cvAvg(fa);
	CvScalar bSquareMean = cvAvg(fb);
	double M = sqrt(aSquareMean.val[0] + bSquareMean.val[0]);
	double K = D / M;
	cvReleaseImage(&lab);
	cvReleaseImage(&l);
	cvReleaseImage(&a);
	cvReleaseImage(&b);
	cvReleaseImage(&fa);
	cvReleaseImage(&fb);
	return K;
}

void colorCorrect(IplImage* src,IplImage* dst)
{
	float u,v;
	double srcMax,srcMin,dstMax,dstMin,squareMax,squareMin;
	CvScalar cvSrcSum,cvDstSum,cvSquareSum;
	IplImage* square = cvCreateImage(cvGetSize(dst),IPL_DEPTH_32F,1);

	cvPow(dst,square,2.0);

	cvSrcSum = cvSum(src);
	cvDstSum = cvSum(dst);
	cvSquareSum = cvSum(square);

	cvMinMaxLoc(src,&srcMin,&srcMax);

	cvMinMaxLoc(dst,&dstMin,&dstMax);
	cvMinMaxLoc(square,&squareMin,&squareMax);

	u = (srcMax*cvDstSum.val[0] - dstMax*cvSrcSum.val[0])/(squareMax*cvDstSum.val[0] - dstMax*cvSquareSum.val[0]);
	v = (squareMax*cvSrcSum.val[0] - srcMax*cvSquareSum.val[0])/(squareMax*cvDstSum.val[0] - dstMax*cvSquareSum.val[0]);

	cvConvertScale(square,square,u);
	cvConvertScale(dst,dst,v);
	cvAdd(dst,square,dst);
	cvReleaseImage(&square);
}

void AdjustCast(IplImage* img, IplImage* dst)
{

	float ur,vr,ub,vb;
	CvScalar gSum,rSum,bSum,rSqrSum,bSqrSum;
	double rMax,rMin,rSqrMax,rSqrMin,gMax,
		gMin,bMax,bMin,bSqrMax,bSqrMin;
	IplImage* fImg = cvCreateImage(cvGetSize(img),IPL_DEPTH_32F,3);
	IplImage* fRed = cvCreateImage(cvGetSize(img),IPL_DEPTH_32F,1);
	IplImage* fGreen = cvCreateImage(cvGetSize(img),IPL_DEPTH_32F,1);
	IplImage* fBlue = cvCreateImage(cvGetSize(img),IPL_DEPTH_32F,1);	

	cvConvertScale(img,fImg,1.0/255,0);
	cvSplit(fImg,fBlue,fGreen,fRed,NULL);

	colorCorrect(fRed,fGreen);
	colorCorrect(fRed,fBlue);
	cvThreshold(fGreen, fGreen, 1.0, 1.0, CV_THRESH_TRUNC);
	cvThreshold(fBlue, fBlue, 1.0, 1.0, CV_THRESH_TRUNC);
	cvMerge(fBlue,fGreen,fRed,NULL,fImg);

	cvConvertScale(fImg,dst,255);
	
	cvReleaseImage(&fImg);
	cvReleaseImage(&fRed);
	cvReleaseImage(&fGreen);
	cvReleaseImage(&fBlue);
	
}
int _tmain(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("please input image path\n");
		return -1;
	}
	IplImage* image = cvLoadImage(argv[1],1);	
	IplImage* dst = cvCloneImage(image);
	double K = CastDetection(image);
	if (K > 1.5)
	{		
		AdjustCast(image, dst);
	}	
	cvNamedWindow("src",1);
	cvNamedWindow("dst", 1);
	cvShowImage("src",image);
	cvShowImage("dst", dst);
	cvWaitKey(0);
	cvReleaseImage(&dst);
	return 0;
}

