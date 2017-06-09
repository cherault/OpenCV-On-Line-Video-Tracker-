/*
 * trackerOnLine.cpp
 *
 *  Created on: 9 juin 2017
 *      Author: tux
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

using namespace std;
using namespace cv;

//--global variables--//
//--------------------//
static Mat image;
static Rect2d boundingBox;
static bool paused;
static bool selectObject = false;
static bool startSelection = false;

//--prototype functions--//
//-----------------------//
const string Date();
const string Heure();
void Square(Mat&, Point, const Scalar&, int, int, int);
void TriangleDown(Mat&, Point, const Scalar&, int, int, int);
void TriangleLeft(Mat&, Point, const Scalar&, int, int, int);
void onMouse(int, int, int, int, void*);

//--main program--//
//----------------//
int main()
{
    Mat frame;

    //--drawing constants--//
    //---------------------//
	int epais = 40;
	int delta = 40;
	int scale = 10;
	int offset = 20;

	//--draw reticule--//
	//-----------------//
	Point center;
	Point pt1, pt2;
	Point pt3, pt4;
	Point pt5, pt6;
	Point pt7, pt8;

	//--define colors--//
	//-----------------//
	Scalar blanc = Scalar(255,255,255);
	Scalar vert = Scalar(0,255,0);
	Scalar rouge = Scalar(0,0,255);
	Scalar bleu = Scalar(255,0,0);
	Scalar noir = Scalar(0,0,0);

	Point centroid;

	string video = "chase.mp4";
	VideoCapture cap(video);

	//--define tracker type--//
	//-----------------------//
	string tracker_algorithm = "KCF";
	Ptr<Tracker> tracker = Tracker::create(tracker_algorithm);

	//--activate mouse selector--//
	//---------------------------//
    paused = false;
    namedWindow("Frame", WINDOW_AUTOSIZE);
    setMouseCallback("Frame", onMouse, 0);

    //--read first frame--//
    //--------------------//
    cap >> frame;
    frame.copyTo(image);
    imshow("Frame", image);

    while(true)
    {
    	cap >> frame;
    	frame.copyTo(image);

    	//--define zone Lock and Track--//
    	//------------------------------//
		Rect2d zoneLOCK = Rect2d(image.cols/2-30, image.rows/2-30, 60,60);
		Rect2d zoneTRACK = Rect2d(image.cols/2-120, image.rows/2-120, 240, 240);

		//--draw reticule--//
		//-----------------//
		Square(image, Point(image.cols/2, image.rows/2), vert, 60, 1, 8);

		center = Point(image.cols/2, image.rows/2);
		circle(image, center, 1, vert, 2);

		pt1 = Point(0, image.rows/2);
		pt2 = Point(image.cols/2 - offset, image.rows/2);
		line(image, pt1, pt2, vert, 1);

		pt3 = Point(image.cols/2 + offset, image.rows/2);
		pt4 = Point(image.cols, image.rows/2);
		line(image, pt3, pt4, vert, 1);

		pt5 = Point(image.cols/2, 0);
		pt6 = Point(image.cols/2, image.rows/2 - offset);
		line(image, pt5, pt6, vert, 1);

		pt7 = Point(image.cols/2, image.rows/2 + offset);
		pt8 = Point(image.cols/2, image.rows);
		line(image, pt7, pt8, vert, 1);

		//--draw black mask around window--//
		//---------------------------------//
		rectangle(image, Point(0,0), Point(image.cols, epais), noir, -1);
		rectangle(image, Point(0, image.rows), Point(image.cols, image.rows-epais), noir, -1);
		rectangle(image, Point(0,0), Point(epais+20, image.rows), noir, -1);
		rectangle(image, Point(image.cols-epais, 0), Point(image.cols, image.rows), noir, -1);

		//--draw scale left and bottom--//
		//------------------------------//
		for(int i = 0;i<image.rows; i+=delta)
		{
			for(int j=0; j<image.cols; j+=delta)
			{
				line(image, Point(0, i), Point(scale, i), blanc,1);
				line(image, Point(j, image.rows), Point(j, image.rows-scale), blanc,1);
			}
		}

		//--calculate centroid target coordinates--//
		//-----------------------------------------//
		centroid = Point(boundingBox.x + (boundingBox.width/2), boundingBox.y + (boundingBox.height/2));

		//--init tracker and update it--//
		//------------------------------//
        if(selectObject)
        {
            tracker->init(frame, boundingBox);
            tracker->update(frame, boundingBox);
            rectangle(image, boundingBox, bleu,2,1);
            circle(image, centroid, 2, rouge, 2);
        }

        //--draw coordinate target with triangles--//
        //-----------------------------------------//
		TriangleDown(image, Point(centroid.x, centroid.y +(frame.rows-centroid.y)-offset), blanc, 20, 1, 8);
		TriangleLeft(image, Point(centroid.x-(centroid.x-offset), centroid.y), blanc, 20, 1, 8);

		//--entering Lock Mode--//
		//----------------------//
		if(centroid.inside(zoneLOCK))
		{
			Square(image, Point(image.cols/2, image.rows/2), rouge, 60, 3, 8);
			circle(image, center, 1, rouge, 2);
			line(image, center, centroid, bleu, 2);
		}

		//--entering Track Mode--//
		//-----------------------//
		if(centroid.inside(zoneTRACK))
		{
			Square(image, Point(image.cols/2, image.rows/2), rouge, 240, 1, 8);
			circle(image, center, 1, rouge, 2);
			line(image, center, centroid, blanc, 2);
		}

		//--int to string--//
		//-----------------//
		stringstream x, y;
		x << centroid.x;
		y << centroid.y;

		//--print coordinates values over triangles--//
		//-------------------------------------------//
		putText(image, x.str(), Point(centroid.x-10, centroid.y +(frame.rows-centroid.y)-offset-5),1,1, blanc, 1);
		putText(image, y.str(), Point(centroid.x-(centroid.x-offset-5), centroid.y+5),1,1, blanc, 1);

		//--print date and time on window--//
		//---------------------------------//
		putText(image, Date(),Point(60,25),1,1,blanc,1);
		putText(image, Heure(),Point(200,25),1,1,blanc,1);

		//--print infos LOCK on screen--//
		//------------------------------//
		putText(image, "M.LOCK: ", Point(500,25),1,1,blanc,1);

		if(centroid.inside(zoneLOCK))
			putText(image, "ON",Point(570,25),1,1,blanc,2);
		else
			putText(image, "OFF",Point(570,25),1,1,blanc,1);

		//--print infos TRACK on screen--//
		//-------------------------------//
		putText(image, "M.TRACK: ", Point(350,25),1,1,blanc,1);

		if(centroid.inside(zoneTRACK))
			putText(image, "ON",Point(430,25),1,1, blanc,2);
		else
			putText(image, "OFF",Point(430,25),1,1, blanc,1);

    	imshow("Frame", image);
    	waitKey(32);
    }

	return 0;
}

//--print date--//
//--------------//
const string Date()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%d/%m/%Y", &tstruct);

    return buf;
}

//--print hours--//
//---------------//
const string Heure()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

//--draw reticule square and target--//
//-----------------------------------//
void Square(Mat& img, Point position, const Scalar& color, int markerSize, int thickness, int line_type)
{
	line(img, Point(position.x-(markerSize/2), position.y-(markerSize/2)), Point(position.x+(markerSize/2), position.y-(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x+(markerSize/2), position.y-(markerSize/2)), Point(position.x+(markerSize/2), position.y+(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x+(markerSize/2), position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y+(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x-(markerSize/2), position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y-(markerSize/2)), color, thickness, line_type);
}

//--draw triangle oriented bottom--//
//---------------------------------//
void TriangleDown(Mat& img, Point position, const Scalar& color, int markerSize, int thickness, int line_type)
{
	line(img, Point(position.x+(markerSize/2), position.y), Point(position.x, position.y+(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x, position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y), color, thickness, line_type);
	line(img, Point(position.x-(markerSize/2), position.y), Point(position.x+(markerSize/2), position.y), color, thickness, line_type);
}

//--draw triangle oriented left--//
//-------------------------------//
void TriangleLeft(Mat& img, Point position, const Scalar& color, int markerSize, int thickness, int line_type)
{
	line(img, Point(position.x, position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y), color, thickness, line_type);
	line(img, Point(position.x-(markerSize/2), position.y), Point(position.x, position.y-(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x, position.y-(markerSize/2)), Point(position.x, position.y+(markerSize/2)), color, thickness, line_type);
}

//--select target with mouse--//
//----------------------------//
void onMouse(int event, int x, int y, int, void*)
{
	if(!selectObject)
	{
		switch(event)
		{
      		case EVENT_LBUTTONDOWN:

      			startSelection = true;
      			boundingBox.x = x;
      			boundingBox.y = y;
      			boundingBox.width = boundingBox.height = 0;
      			break;

      		case EVENT_LBUTTONUP:

      			boundingBox.width = std::abs(x - boundingBox.x);
      			boundingBox.height = std::abs(y - boundingBox.y);
      			paused = false;
      			selectObject = true;
      			break;

      		case EVENT_MOUSEMOVE:

      			if(startSelection && !selectObject)
      			{
      				Mat currentFrame;
      				image.copyTo(currentFrame);
      				rectangle(currentFrame, Point((int) boundingBox.x, (int)boundingBox.y), Point(x,y), Scalar(255,0,0),2,1);
      				imshow("Frame", currentFrame);
      			}

      			break;
		}
	}
}
