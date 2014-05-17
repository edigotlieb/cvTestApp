
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <stdio.h>

#include <time.h>
#include <opencv2/core/types_c.h>

using namespace std;
using namespace cv;


#define EPS (5)
#define MAX_THETAS 5
#define THETA_RANGE (30)

void imfill(Mat& src,Mat& dst,double size);
void imRemoveBlobs(Mat& src,Mat& dst,double size);
void applyMorphologies(Mat& src,Mat& dst, int* peaks,int numPeaks);

bool containsTheta(int *usedThetas,int numUsedThetas, float eps, float theta) {
    float upperBound,lowerBound;
    for(uint8_t i=0; i < numUsedThetas; i++) {
        bool upper = false,lower = false;
        upperBound= usedThetas[i] + eps;
        if(upperBound > 90) {
            upper = (theta >= usedThetas[i] && theta <= 90) || (theta <= upperBound - 180);
        } else {
            upper = theta <= upperBound;
        }
        lowerBound = usedThetas[i] - eps;
        if(lowerBound < -90) {
            lower = (theta <= usedThetas[i] && theta >= -90) || (theta >= upperBound + 180);
        } else {
            lower = theta >= lowerBound;
        }
        if(upper && lower) {
            return true;
        }
    }
    return false;
}

bool thetaInRange(int theta, int theta_center,int theta_range) {
    return theta > theta_center - theta_range && theta < theta_center + theta_range;
}


int main(int argc, char** argv)
{   
     clock_t start,finish;     
     start = clock();
     
     
        float tempTheta = (70.0/180*CV_PI);
        float tanTheta = tan(tempTheta);        
        int rows = 250 * sin(tempTheta);
        
        int cols = 250 * cos(tempTheta);        
        if(rows % 2 == 0) 
            rows++;
        if(cols % 2 == 0)
            cols++;        
        
        cout << "rows: " << rows << " cols: " << cols << endl;
        Mat line(rows,cols, CV_8UC1,Scalar(0));
        int x = 0,y = rows - 1;
        float firstOptionM,secondOptionM;
        while(x < cols && y >= 0) {
            line.at<unsigned char>(y,x) = 255; 
                
            if(x != 0)
                firstOptionM = (rows-1 - (y-1)) / x;
                
            secondOptionM =  (rows-1 - (y-1)) / (x + 1);
            
            if(x != 0) {
                firstOptionM = abs(firstOptionM - tanTheta);
                secondOptionM = abs(secondOptionM - tanTheta);
            } else {
                firstOptionM = abs(CV_PI/2 - tempTheta);
                secondOptionM = abs(atan2((rows-1 - (y-1)),(x+1)) - tempTheta);
            }
            
            cout << firstOptionM << " ," << secondOptionM << endl;
            if(firstOptionM >= secondOptionM)
                x++;
            y--;
        } 
        
        
        
    finish = clock();
    float diff = (((float)finish - (float)start) / 1000000.0F ) * 1000; 
    cout << diff << endl;    
    imshow("line",line);
            
    waitKey();           
    return 0;
    
    
    
 const char* filename = "/home/edi/NetBeansProjects/cvTestApp/testPic2.jpg";
 

 
 Mat dst;
 Mat src = imread(filename, CV_LOAD_IMAGE_COLOR);

 
 start = clock();

 
 
 medianBlur(src,dst,9);
 cvtColor(dst, dst, CV_BGR2GRAY);
 
 Mat binary;
 
 threshold( dst, binary, 90, 255,THRESH_BINARY_INV );
 
  
Mat clean = binary.clone();
imfill(binary,clean,150);

Mat temp = clean.clone();
Mat cleanClone = clean.clone();
imRemoveBlobs(cleanClone,temp,750);
 
 

 vector<Vec2f> lines; 
 HoughLines(temp,lines, 1, CV_PI/180, 300);
 int thetas[MAX_THETAS];
 int numUsedThetas = 0;
 
 float theta;
 float theta_center = 90;
 for(int i = 0; i < lines.size() && numUsedThetas < MAX_THETAS; i++) {
     theta = 90 - lines[i][1]*180/CV_PI;
     
     if((!containsTheta(thetas,numUsedThetas,EPS,theta))  && (thetaInRange(theta,theta_center,THETA_RANGE) || thetaInRange(theta,-90,THETA_RANGE)) && (numUsedThetas < MAX_THETAS)) {
         thetas[numUsedThetas++] = theta;
     }
 } 



 
  
 
 //Mat morphed;
 //applyMorphologies(clean,morphed,thetas,numUsedThetas);
 
 
finish = clock();
//float diff = (((float)finish - (float)start) / 1000000.0F ) * 1000; 
cout << diff << endl;


for(int i = 0; i<numUsedThetas; i++) {
    cout << "theta: " << thetas[i] << endl;
}
/*
for(int i = 0; i<lines.size(); i++) {
    cout << "theta_pot: " << 90 - lines[i][1]*180/CV_PI << " rho: " << lines[i][0]<< endl;   
}
*/
// cout << lines.size() << endl;
// cout << numUsedThetas << endl;
 
// imshow("binary",binary);
imshow("temp",temp);
// imshow("clean",clean);
// imshow("filled", clean);
// imshow("morphed", morphed);

 waitKey();

 return 0;
}


/**
 * changes src
 * @param src
 * @param dst
 * @param size
 */
void imRemoveBlobs(Mat& src,Mat& dst,double size)
{      
    // Only accept CV_8UC1
    if(src.channels() != 1 || src.type() != CV_8UC1)
	return;

    // Find all contours
    vector<Vec4i> hierarchy;    
    vector<vector < Point> > contours;    

    
    findContours(src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    
    for(int i = 0; i < contours.size(); i++)
    {
	// Calculate contour area
	double area = contourArea(contours[i]);

	// Remove small objects by drawing the contour with black color
	if(area >= 0 && area <= size)
	{
	    drawContours(dst, contours, i, 0, CV_FILLED,8,hierarchy);
	}
    }
    
}

void imfill(Mat& src,Mat& dst,double size)
{
      
    // Only accept CV_8UC1
    if(src.channels() != 1 || src.type() != CV_8UC1)
	return;

    // Find all contours
    vector<Vec4i> hierarchy;    
    vector<vector < Point> > contours;
    
    Mat invertedSrc;
    
    threshold(src,invertedSrc,100,255,THRESH_BINARY_INV);
    
    findContours(invertedSrc, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    
    for(int i = 0; i < contours.size(); i++)
    {
	// Calculate contour area
	double area = contourArea(contours[i]);

	// fill holes - paint white
	if(area >= 0 && area <= size)
	{
	    drawContours(dst, contours, i, 255, CV_FILLED,8,hierarchy);
	}
    }
    
}



void applyMorphologies(Mat& src,Mat& dst, int* peaks,int numPeaks)
{
    // Open morphology
    Mat opened;
    (dst) = Mat::zeros(src.rows, src.cols,src.type());
        
    for(int i = 0; i < numPeaks; i++)
    {
        ~opened;
	int size = 250;
        float theta = (peaks[i]*CV_PI/180);
        float tanTheta = tan(theta);
        int rows = size * sin(theta);
        int cols = size * cos(theta);        
        if(rows % 2 == 0) 
            rows++;
        if(cols % 2 == 0)
            cols++;        
        Mat line(cols,rows, CV_8UC1);
        
        if(theta > 0) {
            int x = 0,y = rows - 1;
            float firstOptionM,secondOptionM;
            while(x < cols && y >= 0) {
                line.at<unsigned char>(x,y) = 255; 
                
                if(x != 0)
                    firstOptionM = (rows-1 - (y-1)) / x;
                else 
                    firstOptionM = tan(CV_PI/2);
                
                secondOptionM =  (rows-1 - (y)) / (x + 1);
                firstOptionM = abs(firstOptionM - tanTheta);
                secondOptionM = abs(secondOptionM - tanTheta);
                if(firstOptionM < secondOptionM)
                    y--;
                else 
                    x++;
            }
        }
        
       /* 
        int y;
        for(int x = 0; x < line.cols; x++)
            {
                y = (int) (size / 2 * sin(theta) - tan(theta) * (x - size / 2 * cos(theta)));
                line.at<unsigned char>(x, y) = 0xFF;
            }
        */
        
         
        
	morphologyEx(src, opened, MORPH_OPEN, line);  
         
        
	// imfill(opened, opened,50);
        // imshow("",opened);
	bitwise_or((dst), opened,(dst));
    }
}

