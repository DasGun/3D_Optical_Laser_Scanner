#ifndef IMAGE_HANDLER_H
#define IMAGE_HANDLER_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <QString>

/* Private define ------------------------------------------------------------*/
#define PI 3.14159265358979323846264338328

using namespace cv;
using namespace std;

struct laserLine{
    //Real coordinates transformed from pixel coordinates
    vector<double> x;
    vector<double> y;
    vector<double> z;
};

class Image_Handler
{
public:
    Image_Handler();

    Mat laser_line_threshold (Mat imgOriginal);

    double average_pixels (vector<int> row);

    int pixel_iterate_floodfiller_black (Mat img, double imgWidth, int laser_pos);

    void determine_real_coords (double alpha, int laser_pos);

    void write_coords_to_file (QString mesh_file_path);

    Mat set_grid_format (Mat img, int imgWidth, int imgHeight);

    void detect_laser_line (Mat img, int laser_pos);

private:
    int Pw;//Pixal Wall Point APPROX need to test between 748 and 760
    int Pc; //Pixal Centre Point

    //Angles - assumed Equalateral Triangle Formation for now
    int theta;
    int beta; // laser angle

    //CTW - Centre to Wall distance in mm (assumed to be 150 for now)
    int CTW;
    int XTW; // X coord to wall in mm (to be determined)

    //Height of the platform, to be subtracted from overall height calculation
    float platformHeight;

    vector<laserLine> baseRotation_PerDegree; //The following vector holds a struct of coordinate vectors per degree rotated
    vector<int> pixel_val_vector;

    /* left camera pixel coordinates */
    vector<double> xPixelCoords_left;
    vector<double> zPixelCoords_left;

    /* right camera pixel coordinates */
    vector<double> xPixelCoords_right;
    vector<double> zPixelCoords_right;


//    /* left camera pixel coordinates */
//    vector<int> xPixelCoords_left;
//    vector<int> zPixelCoords_left;

//    /* right camera pixel coordinates */
//    vector<int> xPixelCoords_right;
//    vector<int> zPixelCoords_right;

    CvScalar blue;
};

#endif // IMAGE_HANDLER_H
