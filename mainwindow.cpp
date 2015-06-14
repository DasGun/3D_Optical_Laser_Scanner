/* Include Headers */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "image_handler.h"

/* Include QT framework */
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QPixmap>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>

/* Colour States of UI */

//Normal - Green
#define NORMAL_STATE background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0.0903226 rgba(0, 114, 0, 255), stop:1 rgba(255, 255, 255, 255));
//Warning - Red
#define BAD_STATE qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0.154839 rgba(182, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));

/* Private namespaces --------------------------------------------------------*/
using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*****************Setting Time************************/
    int hour;
    int minute;
    int second;
    int millisecond;

    QTime *time = new QTime();

    /* Takes the current time on start up */
    hour = time->currentTime().hour();
    minute = time->currentTime().minute();
    second = time->currentTime().second();
    millisecond = time->currentTime().msec();

    /* Create display time string for LCD number display on start up */
    QString displayTime = QString::number(hour) + ":" + QString::number(minute);

    /**************Enable port for coms*********************/
    port = new QextSerialPort("/dev/tty.usbmodem1421");

    port->setBaudRate(BAUD9600);
    // check the baud rate on your Arduino since it has to be the same
    // as set with Serial.begin(...)
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);

    port->open(QIODevice::ReadWrite);

    if(port->isOpen()){
        /* Debug print outs */
        qDebug("listening for data on %s", qPrintable(port->portName()));
        connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

        /* Shows and hides certain UI labels and buttons when connection is established */
        ui->Connection_Colour_Label->setStyleSheet("background-color: rgb(14, 255, 0)");
        ui->No_Error_label->show();
        ui->Error_label->hide();
        ui->Connection_Colour_Label->hide();
        ui->retry_Connection_button->hide();

    }else{
        qDebug("device failed to open on %s", qPrintable(port->portName()));

        /* Shows and hides certain UI labels and buttons when connection is NOT established */
        ui->Connection_Colour_Label->setStyleSheet("background-color: rgb(255, 53, 36)");
        ui->No_Error_label->hide();
        ui->Error_label->show();
        ui->retry_Connection_button->show();
        //exit(-1);
    }

    /************* Setup UI ********************************/
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1608); //Steps for 360 revolution
    ui->progressBar->setValue(0);
    ui->progressBar->hide();
    ui->progressBar_label->hide();
    ui->save_label->show();
    ui->Cancel_Scan_button->hide();
}


MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief  When called - turns the Left laser on through arduino serial
 * @param  None
 * @retval None
 */
void MainWindow::on_Debug_Laser_On_clicked()
{
    /* local variables */
    QByteArray send_byte;

    /* Sets send byte to "on" position '2' */
    send_byte.append("1");
    /* Send "on" byte */
    port->write(send_byte);
}

/**
 * @brief  When called - turns off the Left laser through the arduino serial
 * @param  None
 * @retval None
 */
void MainWindow::on_Debug_Laser_Off_clicked()
{
    /* local variables */
    QByteArray send_byte;

    /* Sets send byte to "off" position '2' */
    send_byte.append("2");
    /* Send "off" byte */
    port->write(send_byte);
}


/**
 * @brief  Save and Scan Buttons - first implements saving the mesh file then enables for scanning
 *          This section of code runs the main functionality of the scanner.
 * @param  None
 * @retval None
 */
void MainWindow::on_Scan_Save_Button_clicked()
{
    /* Local Variables */
    double alpha;
    double left_offset = 14.47463699;
    double right_offset = 14.74356284;
    int steps = 0;
    int xPixelCoords_val = 0;
    QString Button_Text = ui->Scan_Save_Button->text();

    /* Checks to see if save button was clicked, if so the selected path is stored for
     * the saving of the output point cloud */
    if(Button_Text == "Save"){
        qDebug() << "Save found!" << endl;

        /* Set Mesh File Name and Path Before Scanning */
        mesh_file_path = QFileDialog::getSaveFileName(this,
            tr("Set Mesh File Destination"), "",
            tr("ActionScript Communication File (*.asc);;All Files (*)"));

        if(mesh_file_path.isEmpty()){
            return;
        }else{
            QFile file(mesh_file_path);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                                         file.errorString());
                return;
            }

            QMessageBox::information(this, tr("File Name"), mesh_file_path);
        }


        ui->Scan_Save_Button->setText("Begin Scan");
        ui->save_label->hide();
        ui->Cancel_Scan_button->show();

        /* Checks to see if begin scan was clicked,
         * if so the system runs the main scanning section
         */
    }else if(Button_Text == "Begin Scan"){
        qDebug() << "Begin Scan Found!" << endl;

        /* Show Progress Bar and Label */
        ui->progressBar->show();
        ui->progressBar_label->show();
        ui->Cancel_Scan_button->hide();

        //Start Scanning Process
        /* Create Image_Handler Object */
        Image_Handler Img_Handle;

        /* Opens up video capture on connected camera */
        VideoCapture cap(0);
        if(!cap.isOpened()){
            qDebug()<<"Failed to open camera"<<endl;
            exit(-1);
        }

        /* Runs two revolutions - one with the left laser and then the right laser
         * when i = 0, then left laser is being used
         * when i = 1, then right laser is being used
         */
        for(int i = 0; i < 2; i++){

            if(i == 0){
                //Left laser is on
                on_Debug_Laser_On_clicked();    //Turns on left laser
                //Make sure right laser is off
                on_Debug_Laser_Right_off_clicked(); //Makes sure right laser is off
            }else if(i == 1){
                //Left laser is done, so turn off
                on_Debug_Laser_Off_clicked();   //Turns off left laser
                //Turn on the right laser
                on_Debug_laser_Right_on_clicked(); //Turns right laser on

                waitKey(1000);
            }

            /* Simulates the rotation platform */
            /* approximately 402 steps for 90 degree rotation, therefore 1608 steps for 360 degrees */
            for(steps = 0; steps < 1608; steps++){

                /* Gets a frame from camera - using set image for now */
                //Mat imgOriginal_right = imread("/Users/Dasun/Documents/Uni/2014 Sem 2/Thesis/Test Images/TestImage_cap.jpg");
                //Mat imgOriginal = imread("/Users/Dasun/Documents/Uni/2014 Sem 2/Thesis/Test Pictures (Hackaday)/center10point8.jpg");

                /* Get an image of the scanning object */
                Mat imgOriginal;

                /* Delays for 60 frames to ensure exposure of camera to laser light has reduced */
                int counter = 0;
                while(counter < 1){
                    cap >> imgOriginal;
                    counter ++;
                    waitKey(30);
                }

                /* New Subpixel code to test */
                Img_Handle.detect_laser_line(imgOriginal, i);

                /* alpha represents a degree of rotation from the rotation platform */
                alpha = (360.0/1608.0)*(double)steps;

                /* To account for the placement of the lasers */
                if(i == 0){
                    //left laser
//                    alpha = alpha - left_offset;
                }else if(i == 1){
                    //Right laser
                    alpha = alpha + 32.0;
                }

                /* Determine the real coordinates using the homography matrix
                 * returns nothing */
                Img_Handle.determine_real_coords(alpha, i);

                /* The following section resizes the captured image and sets it to the Qlabel for viewing */
                Mat size_ex = cvCreateMat(240,320, CV_8UC1);
                cv::resize(imgOriginal, imgOriginal, size_ex.size());

                QImage imageView_orig = QImage((const unsigned char*)(imgOriginal.data), imgOriginal.size().width,imgOriginal.size().height,QImage::Format_RGB888).rgbSwapped();
                ui->Original_img->setPixmap(QPixmap::fromImage(imageView_orig));

                /* Debugging views of images */
//                imshow("cam thresh", imgThresholded);

                /* Writes out the captured image to a designated folder */
#if 0
                /* Write out imgOriginal */
                vector<int> compression_params; //vector that stores the compression parameters of the image
                compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //specify the compression technique
                compression_params.push_back(98); //specify the compression quality

                bool bSuccess = imwrite("/Users/Dasun/Documents/Uni/2014 Sem 2/Thesis/Test Images/TestImage_thincil2.jpg", imgOriginal, compression_params); //write the image to file
                if ( !bSuccess ){
                    cout << "ERROR : Failed to save the image" << endl;
                }
#endif
                /* Rotate Motor by one step - to be determined optimum step */
                on_Debug_Motor_Right_clicked();

                /* Updates the progress bar event */
                ui->progressBar->setValue(steps);
                ui->progressBar->update();

                /* Updates all events in loop */
                qApp->processEvents();
            }
        }

        /* Final write out of determined coordinated to a mesh file for viewing and 3D printing */
        Img_Handle.write_coords_to_file(mesh_file_path);

        //Turns Both Lasers Off When Completed
        on_Debug_Laser_Off_clicked();
        on_Debug_Laser_Right_off_clicked();

        qDebug()<<"Laser Scan Completed!"<<endl;

        /* Change button name back to save and re-hide the progress bar */
        ui->Scan_Save_Button->setText("Save");
        ui->progressBar->hide();
        ui->progressBar_label->hide();
        ui->save_label->show();
    }

}

/**
 * @brief  Cancels the scan function - resets back to save mesh state
 * @param  None
 * @retval None
 */
void MainWindow::on_Cancel_Scan_button_clicked()
{
    /* Resets Scan_Save button back to "Save" state - Cancelling Scanning Process */
    ui->Cancel_Scan_button->hide();
    ui->Scan_Save_Button->setText("Save");
    ui->save_label->show();

}

/**
 * @brief  When called - Triggers a read from the buffer - confirmation from arduino
 * @param  None
 * @retval None
 */
int MainWindow::onReadyRead()
{
    /* Grab bytes from serial */
    int num_bytes = port->bytesAvailable();
    received_byte.resize(num_bytes);
    port->read(received_byte.data(), received_byte.size());

#if 1
    qDebug() << "bytes read:" << received_byte << endl;
#endif

    switch(received_byte[0]){
    /* Laser On Confirmed */
    case 'A':
        return 1;
    /* Laser Off Confirmed */
    case 'B':
        return 2;
    /* Motor Right Confirmed */
    case 'C':
        return 3;
    /* Motor Left Confirmed */
    case 'D':
        return 4;
    default:
        return -1;
    }
}

/**
 * @brief  When called - turns motor to the right through the arduino serial
 * @param  None
 * @retval None
 */
void MainWindow::on_Debug_Motor_Right_clicked()
{
    /* local variables */
    QByteArray send_byte;

    /* Sets send byte to "right" position '3' */
    send_byte.append("3");
    /* Send "off" byte */
    port->write(send_byte);
}

/**
 * @brief  When called - turns motor to the left through the arduino serial
 * @param  None
 * @retval None
 */
void MainWindow::on_Debug_Motor_Left_clicked()
{
    /* local variables */
    QByteArray send_byte;

    /* Sets send byte to "left" position '4' */
    send_byte.append("4");
    /* Send "off" byte */
    port->write(send_byte);
}


/**
 * @brief  When called - retrys for a connection to the Arduino on a set serial port (change this
 *                      if it is different on your computer)
 * @param  None
 * @retval None
 */
void MainWindow::on_retry_Connection_button_clicked()
{
    /**************Enable port for coms*********************/
    port = new QextSerialPort("/dev/tty.usbmodem1421");

    port->setBaudRate(BAUD9600);
    // check the baud rate on your Arduino since it has to be the same
    // as set with Serial.begin(...)
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);

    port->open(QIODevice::ReadWrite);

    if(port->isOpen()){
        qDebug("listening for data on %s", qPrintable(port->portName()));
        connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

        ui->Connection_Colour_Label->setStyleSheet("background-color: rgb(14, 255, 0)");
        ui->No_Error_label->show();
        ui->Error_label->hide();
        ui->Connection_Colour_Label->hide();
        ui->retry_Connection_button->hide();

    }else{
        qDebug("device failed to open on %s", qPrintable(port->portName()));

        ui->Connection_Colour_Label->setStyleSheet("background-color: rgb(255, 53, 36)");
        ui->No_Error_label->hide();
        ui->Error_label->show();
        ui->retry_Connection_button->show();
    }
}

/**
 * @brief  When called - turns on the Right laser through the arduino serial
 * @param  None
 * @retval None
 */
void MainWindow::on_Debug_laser_Right_on_clicked()
{
    /* local variables */
    QByteArray send_byte;

    /* Sets send byte to "on" position '5' */
    send_byte.append("5");
    /* Send "on" byte */
    port->write(send_byte);
}

/**
 * @brief  When called - turns off the Right laser through the arduino serial
 * @param  None
 * @retval None
 */
void MainWindow::on_Debug_Laser_Right_off_clicked()
{
    /* local variables */
    QByteArray send_byte;

    /* Sets send byte to "off" position '6' */
    send_byte.append("6");
    /* Send "off" byte */
    port->write(send_byte);
}
