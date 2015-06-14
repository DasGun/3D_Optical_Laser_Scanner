#include "image_handler.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

/* Private typedef -----------------------------------------------------------*/
/* Private namespaces --------------------------------------------------------*/
using namespace cv;
using namespace std;


/**
 * @brief  Image_Handler Constructor
 * @param  None
 * @retval None
 */
Image_Handler::Image_Handler()
{
    /* The following was used prior to the new planar method
     * and is not implemented in the final run */

    Pw = 719; //Pixal Wall Point APPROX
    Pc = 960; //640; //Pixal Centre Point

    //Angles - assumed Equalateral Triangle Formation for now
    theta = 22; //I think this is wrong!
    beta = 0; // laser angle

    //CTW - Centre to Wall distance in mm (assumed to be 150 for now)
    CTW = 99; //This is approximate
    XTW = 0; // X coord to wall in mm (to be determined)

    //Height of the platform to be subtracted from overall height scanned in mm
    platformHeight = 9.62;

    blue = CV_RGB(0,0,250);
}

/* The following section of code was used in the old algorithm, prior to the implementaiton of sub-pixel detection
 * and planar homography. Therefore, these functions were not used in the final system run */
#if 0
/**
 * @brief  Thresholds a given image into the HSV colour space based on the characteristics
 *         of the red laser line
 * @param  Mat original image to be thresholded
 * @retval Mat thresholded image
 */
Mat Image_Handler::laser_line_threshold (Mat imgOriginal){

    Mat imgThresholded,imgHSV;
    Mat imgGray,imgGBlur;

#if 1
    //Thresholding using grayscale image
    cvtColor(imgOriginal,imgGray,CV_RGB2GRAY);

    threshold(imgGray, imgThresholded, 200, 255, 0);

#endif

#if 0
    cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

    //HSV setpoints for Red - Basic Rig Thresh
    int iLowH = 0;
    int iHighH = 179;//179

    int iLowS = 0;
    int iHighS = 40;//200

    int iLowV = 255;
    int iHighV = 255;

    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image


    //imshow("thresh before", imgThresholded);

    //morphological opening (remove small objects from the foreground)
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)) );
    //dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (fill small holes in the foreground)
    //dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(1, 1)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(1, 1)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(1, 1)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(1, 1)) );
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(1, 1)) );
#if 0
    //morphological opening (remove small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (fill small holes in the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
#endif
    //erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );

#endif

#if 1
    //morphological opening (remove small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (fill small holes in the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
#endif

    return imgThresholded;
}


/**
 * @brief  Determines the average value of pixels per row inputted
 * @param  vector int row of pixels
 * @return average value in pixels
 */
double Image_Handler::average_pixels(vector<int> row){
    double value = 0.0;
    double average = 0.0;

    for(int i = 0; i < (int)row.size(); i++){
        //cout<<"row val: "<<row[i]<<" at: "<<i<<endl;
        value = value + (double)row[i];
    }

    average = value/((double)row.size());
    //cout<<"average: "<<average<<endl;
    return average;
}


/**
 * @brief  Iterates past the centre point and flood fills the background (this is at pw = 251)
 * @param  Mat img to be floodfilled, and its width and height as doubles
 * @retval None
 */
int Image_Handler::pixel_iterate_floodfiller_black (Mat img, double imgWidth, int laser_pos) {
    vector<int> row;
    double average = 0.0;
    int found = 0;

    for(int i =0; i<img.rows; i++){
        uchar* pointer = img.ptr(i);
        for(int j=0; j<(img.cols); j++){
            //Ignores past centre pixel point (runs floodfiller to remove these points
            if(laser_pos == 0){
                //Left laser is being used
                if(j < 965){
                    if(pointer[j] == 255){
#if 1
                        row.push_back(j);
                        average = average_pixels(row);
                        found = 1;
#endif
#if 0

                        xPixelCoords.push_back(j);
                        //zPixelCoords.push_back(imgHeight - i);
                        zPixelCoords.push_back(i);
                        //cout << "Found at point (z <height> , x) : (" << i << "," << j << ")" << endl;
                        //break;
                        //cout << "Found at point (z <height> , x) : (" << (imgHeight - i) << "," << j << ")" << endl;
#endif
                    }
                }
            }else if(laser_pos == 1){
                //Right laser is being used
                if(j > 965){
                    if(pointer[j] == 255){
#if 1
                        row.push_back(j);
                        average = average_pixels(row);
                        found = 1;
#endif
#if 0

                        xPixelCoords.push_back(j);
                        //zPixelCoords.push_back(imgHeight - i);
                        zPixelCoords.push_back(i);
                        //cout << "Found at point (z <height> , x) : (" << i << "," << j << ")" << endl;
                        //break;
                        //cout << "Found at point (z <height> , x) : (" << (imgHeight - i) << "," << j << ")" << endl;
#endif
                    }
                }
            }
        }
#if 1
//        cout<<"ave: "<<average<<endl;
        if(found == 1){
            cout<<"ave: "<<average<<endl;
            if(laser_pos == 0){
                xPixelCoords_left.push_back(average);
                zPixelCoords_left.push_back((double)i);
//                  xPixelCoords_left.push_back(average);
//                  zPixelCoords_left.push_back(i);
            }else if(laser_pos == 1){
                xPixelCoords_right.push_back(average);
                zPixelCoords_right.push_back((double)i);
//                 xPixelCoords_right.push_back(average);
//                 zPixelCoords_right.push_back(i);
            }
            row.clear();
            found = 0;
        }
        //row.clear();
#endif
    }

    /* return size of xPixelCoords */
    if(laser_pos == 0){
        pixel_val_vector.push_back(xPixelCoords_left.size());
        return xPixelCoords_left.size();
    }else if(laser_pos == 1){
        pixel_val_vector.push_back(xPixelCoords_right.size());
        return xPixelCoords_right.size();
    }else{
        return -1;
    }
}

#endif
/**
 * @brief  Takes in the raw image - runs a gray scale and determines the intensity values
 *         given these intensity values, for each row of the image a gaussian kernal convolution is done to determine a max
 *         2 peak intensity fluctuation and determine subpixel accuracy coordinates to be used in 3d real world data acquisition.
 * @param  Mat raw image
 * @retval None
 */
void Image_Handler::detect_laser_line(Mat raw_img, int laser_pos){

    /* Local Variables */
    Mat imgGray;
    Mat GaussKernel;
    vector<int> row_intensity_values;
    vector<int> gaussKernal;
    double y = 0.0;
    double temp = 0.0;
    double temp1 = 0.0;
    int key = 0;
    int key1 = 0;
    double a, b, c;
    double y1 = 0.0;
    double y2 = 0.0;
    double y3 = 0.0;
    double x1 = 0.0;
    double x2 = 0.0;
    double x3 = 0.0;
    double xmax = 0.0;
    double ymax = 0.0;
    int trigger = 0;
    vector<double> y_out;

    /* Turns the image to grayscale for intensity detection */
    cvtColor(raw_img,imgGray,CV_RGB2GRAY);
    /* Addition of blur for smoothing of centre line detected */
    GaussianBlur(imgGray, imgGray, Size(15,15), 0);
    /* Creates a gaussian kernel - using the OpenCV function */
    GaussKernel = getGaussianKernel(3, 3);

    /* Get the gaussian kernel
     * given that the number of pixels to process is 3
     * and that the standard deviation is 3 - encaplusate approximately 90%
     */
    for(int i =0; i<GaussKernel.rows; i++){
        for(int j=0; j<(GaussKernel.cols); j++){
            uchar KernelVal = GaussKernel.at<uchar>(i,j);
            gaussKernal.push_back((int)KernelVal);
//            cout << "kernel value: "<< (int)KernelVal << " at ("<<i<<","<<j<<")"<<endl;
        }
    }

    /* Iterates through each row at a time */
    for(int i =0; i<imgGray.rows; i++){
        /* Clears the row intensity value vector before every row
         * Clears the y_out vector before every row
         * sets key and temp values to 0 before every row
         */
        row_intensity_values.clear();
        y_out.clear();
        key = 0;
        temp = 0;
        temp1 = 0;
        trigger = 0;

        /* Grabs the intensity value from every pixel on a set row
         * gets it from the gray scale image
         * puts this into the row intensity vector for processing
         */
        for(int j=0; j<(imgGray.cols); j++){
            /*left laser is used */
            if(laser_pos == 0){
                /* To the left of the platform middle position in column pixel */
                if(j < 966){
                    uchar intensity = imgGray.at<uchar>(i,j);
                    row_intensity_values.push_back((int)intensity);
    //                cout << "intensity value: "<< (int)intensity << " at ("<<i<<","<<j<<")"<<endl;
                }
            /* Right laser is used */
            }else if(laser_pos == 1){
                /* To the right of the platform middle position in column pixel */
                if(j > 966 && j < 1600 ){
                    uchar intensity = imgGray.at<uchar>(i,j);
                    row_intensity_values.push_back((int)intensity);
//                    cout << "intensity value: "<< (int)intensity << " at ("<<i<<","<<j<<")"<<endl;
                }
            }
        }

        /* Goes through each identified intensity for the current row
         * convolutes with the created gaussian kernel - output is stored in y_out vector
         * as this is happening - the max intensity value is recorded
         */
        for ( int k = 0; k < (int)row_intensity_values.size(); k++ )
        {
            y = 0;                       // set to zero before sum
            for ( int l = 0; l < (int)gaussKernal.size(); l++ )
            {
                y += row_intensity_values[k - l] * gaussKernal[l];    // convolve: multiply and accumulate
            }

            /* Store the convoluted value for the current interation */
            y_out.push_back(y);
//            cout<<"value of y: "<<y<<" at (943,"<<i<<")"<<endl;

            /* Triggers for the detection of peaks in covolution values */
            if(y > temp && k != 0 && k != 1 && trigger == 1){
                temp = y;
                key = k;
            }else if(y > temp1 && k != 0 && k != 1 && trigger == 3){
                temp1 = y;
                key1 = k;
            }

            /* First time trigger - beginning of first peak */
            if(y > 15000 && trigger == 0 && k != 0 && k != 1){
                //set trigger to 1;
                trigger = 1;
//                cout<<"peak 1"<<endl;
            /* Second time trigger - end of first peak */
            }else if (y < 15000 && trigger == 1 && k != 0 && k != 1){
                //set trigger to 2;
                trigger = 2;
//                cout<<"end of peak 1"<<endl;
            }

            /* First time trigger - beginning of second peak */
            if(y > 15000 && trigger == 2 && k != 0 && k != 1){
                //set trigger to 3;
                trigger = 3;
//                cout<<"peak 2"<<endl;
            /* Second time trigger - end of second peak */
            }else if (y < 15000 && trigger == 3 && k != 0 && k != 1){
                //set trigger to 4;
                trigger = 4;
//                cout<<"end of peak 2"<<endl;
            }
        }
//        cout<<"temp value: "<<temp<<endl;
//        cout<<"temp1 value: "<<temp1<<endl;

        /* Applies a restriction on the intensity values to consider - testing reports approximately 25000
         * but can be set to what ever intensity is required to get the points (darker objects)
         *
         * The following code uses the determined max peak in the intensity values (after convolution)
         * and determines the values to its immediate left and right
         * Given this, a parabola is approximated over these points giving an
         * accurate approximation to the real peak value
         *
         * After this is determined, the real max pixel and the corresponding row pixel is stored as the
         * centre line true subpixel detection
         */
        /* First Peak */
        if(temp > 15000){
//            cout<<"left of max value from key: "<<y_out[key-1]<<endl;
//            cout<<"max value from key: "<<y_out[key]<<endl;
//            cout<<"right of max value from key: "<<y_out[key+1]<<endl;
//            cout<<"key to left: "<<key-1<<endl;
//            cout<<"key max: "<<key<<endl;
//            cout<<"key to right: "<<key+1<<endl;

            /*Grabs the top three points from convolution */
            y1 = y_out[key-1];
            y2 = y_out[key];
            y3 = y_out[key+1];

            /* Left laser - do normal */
            if(laser_pos == 0){
                x1 = (key-1);
                x2 = key;
                x3 = (key+1);
            /* Right laser - add on mid shift */
            }else if(laser_pos == 1){
                x1 = (key-1) + (raw_img.size().width/2);
                x2 = key + (raw_img.size().width/2);
                x3 = (key+1) + (raw_img.size().width/2);
            }

            /* calculate the constants a, b, and c for a parabola equation using
             * the known three points */
            a = (((y2 - y1)*(x1 - x3)) + ((y3 - y1)*(x2 - x1))) / ((x1 - x3)*(pow(x2, 2) - pow(x1,2)) + (x2 - x1)*(pow(x3,2) - pow(x1,2)));
            b = ((y2 - y1) - a*(pow(x2,2) - pow(x1,2))) / (x2 - x1);
            c = (y1 - a*(pow(x1,2)) - b*(x1));

            xmax = ((-1)*(b/(2*a)));
            ymax = a*(pow(xmax,2)) + b*(xmax) + c;

            cout<<"max intensity at col pixel: "<<xmax<<" for pixel row: "<<i<<endl;

            if(laser_pos == 0){
                xPixelCoords_left.push_back(xmax);
                zPixelCoords_left.push_back((double)i);
            }else if(laser_pos == 1){
                xPixelCoords_right.push_back(xmax);
                zPixelCoords_right.push_back((double)i);
            }
        }

        /* Peak Two */
        if(temp1 > 15000){
//            cout<<"skipped"<<endl;
//            cout<<"left of max value from key: "<<y_out[key-1]<<endl;
//            cout<<"max value from key: "<<y_out[key]<<endl;
//            cout<<"right of max value from key: "<<y_out[key+1]<<endl;
//            cout<<"key to left: "<<key-1<<endl;
//            cout<<"key max: "<<key<<endl;
//            cout<<"key to right: "<<key+1<<endl;

            /*Grabs the top three points from convolution */
            y1 = y_out[key1-1];
            y2 = y_out[key1];
            y3 = y_out[key1+1];

            /* Left laser - do normal */
            if(laser_pos == 0){
                x1 = (key-1);
                x2 = key;
                x3 = (key+1);

            /* Right laser - add on mid shift */
            }else if(laser_pos == 1){
                x1 = (key-1) + (raw_img.size().width/2);
                x2 = key + (raw_img.size().width/2);
                x3 = (key+1) + (raw_img.size().width/2);
            }

            /* calculate the constants a, b, and c for a parabola equation using
             * the known three points */
            a = (((y2 - y1)*(x1 - x3)) + ((y3 - y1)*(x2 - x1))) / ((x1 - x3)*(pow(x2, 2) - pow(x1,2)) + (x2 - x1)*(pow(x3,2) - pow(x1,2)));
            b = ((y2 - y1) - a*(pow(x2,2) - pow(x1,2))) / (x2 - x1);
            c = (y1 - a*(pow(x1,2)) - b*(x1));

            xmax = ((-1)*(b/(2*a)));
            ymax = a*(pow(xmax,2)) + b*(xmax) + c;

            cout<<"max intensity at col pixel SECOND PEAK: "<<xmax<<" for pixel row: "<<i<<endl;

            if(laser_pos == 0){
                xPixelCoords_left.push_back(xmax);
                zPixelCoords_left.push_back((double)i);
            }else if(laser_pos == 1){
                xPixelCoords_right.push_back(xmax);
                zPixelCoords_right.push_back((double)i);
            }
        }
    }

    /* return size of xPixelCoords */
    if(laser_pos == 0){
        pixel_val_vector.push_back(xPixelCoords_left.size());
    }else if(laser_pos == 1){
        pixel_val_vector.push_back(xPixelCoords_right.size());
    }
}

/**
 * @brief  Transforms the pixel coordinates to the real dimensions of the object being scanned (mm).
 * @param  imgheight and the degree of rotation of the base plate - alpha 0 to 360
 * @retval None
 */
void Image_Handler::determine_real_coords (double alpha, int laser_pos){
    //Holds the temporary calculated plane coordinates
    double temp_x;
    double temp_y;
    double temp_w;
    double height;
    double m_x, m_y;
    double radial_unrotated;

    //Create new line
    laserLine Line;

    if (laser_pos == 0){
        //Left laser image used
        for(int i = 0; i < (int)xPixelCoords_left.size(); i++){
            /* Left Laser is being used */
            //Calculate the temp x and y values on the plane (including the temp_w projection coordinate)
            temp_x = (-0.000346764)*xPixelCoords_left[i] + (-0.000483817)*zPixelCoords_left[i] + (0.792025620);
            temp_y = (-0.001896695)*xPixelCoords_left[i] + (0.000006896)*zPixelCoords_left[i] + (1.826770698);
            temp_w = (-0.000005024)*xPixelCoords_left[i] + (0.000000029)*zPixelCoords_left[i] + (0.007575171);

            //Inverse Homography Matrix (working)
//            -0.000346764 -0.000483817  0.792025620
//            -0.001896695  0.000006896  1.826770698
//            -0.000005024  0.000000029  0.007575171

            //Calculate the (mm) value of displacement from the specified origin (centre of table)
            height = temp_x/temp_w;
            radial_unrotated = temp_y/temp_w;

            //Rotate y_unrotated given the alpha (table rotation)
            m_x = radial_unrotated*cos(alpha*PI/180);
            m_y = radial_unrotated*sin(alpha*PI/180);

            /* Store values */
            Line.x.push_back(m_x);
            Line.y.push_back(m_y);
            Line.z.push_back(height);

        }
    }else if(laser_pos == 1){
        //Right Laser image used
        for(int i = 0; i < (int)xPixelCoords_right.size(); i++){
            /* Old camera (720p) to the right of laser */
            //Calculate the temp x and y values on the plane (including the temp_w projection coordinate)
            temp_x = (0.000337951)*xPixelCoords_right[i] + (-0.000483816)*zPixelCoords_right[i] + (0.132590418);
            temp_y = (0.001816343)*xPixelCoords_right[i] + (0.000008445)*zPixelCoords_right[i] + (-1.750755837);
            temp_w = (0.000004804)*xPixelCoords_right[i] + (0.000000029)*zPixelCoords_right[i] + (-0.001889596);

            //Inverse Homography Matrix (working)
//            0.000337951 -0.000483816  0.132590418
//            0.001816343  0.000008445 -1.750755837
//            0.000004804  0.000000029 -0.001889596

            //Calculate the (mm) value of displacement from the specified origin (centre of table)
            height = temp_x/temp_w;
            radial_unrotated = temp_y/temp_w;

            //Rotate y_unrotated given the alpha (table rotation)
            m_x = radial_unrotated*cos(alpha*PI/180);
            m_y = radial_unrotated*sin(alpha*PI/180);

            /* Store values */
            Line.x.push_back(m_x);
            Line.y.push_back(m_y);
            Line.z.push_back(height);

        }
    }
    // Iterates through all the points found in a laser line and
    // adds them to the line struct

    /* Update the current base rotation frame */
    baseRotation_PerDegree.push_back(Line);

    /* Clear current Line and x and z pixel coord vectors */
    if(laser_pos == 0){
        xPixelCoords_left.clear();
        zPixelCoords_left.clear();
    }else if(laser_pos == 1){
        xPixelCoords_right.clear();
        zPixelCoords_right.clear();
    }

    /* Clear */
    Line.x.clear();
    Line.y.clear();
    Line.z.clear();
}

/**
 * @brief  Writes the calculated coordinates to a asc file for meshing
 * @param  None
 * @retval None
 */
void Image_Handler::write_coords_to_file (QString mesh_file_path){
#if 1

    cout << "Size of base rotation vector: "<< baseRotation_PerDegree.size() << endl;
    cout << "size of pixel val vector: " << pixel_val_vector.size() <<endl;

    int val;

    ofstream myfile;
    myfile.open(mesh_file_path.toStdString());

    for(int i=0; i< (int)baseRotation_PerDegree.size(); i++){
        val = pixel_val_vector[i];
        cout<< "size of val: " << val << endl;
        for(int j=0; j<val; j++){
            //cout<<"["<<baseRotation_PerDegree[i].x[j]<<","<<baseRotation_PerDegree[i].y[j]<<","<<baseRotation_PerDegree[i].z[j]<<"]"<<" "<<"<mm>"<<endl;
//            if(baseRotation_PerDegree[i].z[j] > 1){
                myfile <<baseRotation_PerDegree[i].x[j]<<","<<baseRotation_PerDegree[i].y[j]<<","<<baseRotation_PerDegree[i].z[j]<<endl;
//            }
        }
    }
    myfile.close();
#endif
}

/**
 * @brief  Sets a grid format on the image captured
 * @param  Mat image captured, and the image width and height
 * @retval Mat image after grid setup
 */
Mat Image_Handler::set_grid_format (Mat img, int imgWidth, int imgHeight){

    //Set points for center line
    CvPoint mid_bottom, mid_top;
    mid_bottom.x = imgWidth/2;
    mid_bottom.y = 0;
    mid_top.x = imgWidth/2;
    mid_top.y = imgHeight;
    //drawing the line
    line(img, mid_bottom, mid_top, blue, 2);

    //Set points for center line
    CvPoint mid_bottom2, mid_top2;
    mid_bottom2.x = 0;
    mid_bottom2.y = 692;
    mid_top2.x = imgWidth;
    mid_top2.y = 692;
    //drawing the line
    line(img, mid_bottom2, mid_top2, blue, 2);

    return img;
}
