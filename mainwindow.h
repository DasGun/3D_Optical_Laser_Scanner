#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QByteArray>
#include <qextserialport.h>
#include <qextserialenumerator.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    //Push button from ui to turn on laser
    void on_Debug_Laser_On_clicked();

    //Push button from ui to turn off laser
    void on_Debug_Laser_Off_clicked();

    //Push button from ui to begin scanning process
    void on_Scan_Save_Button_clicked();

    void on_Cancel_Scan_button_clicked();

    int onReadyRead();

    void on_Debug_Motor_Left_clicked();

    void on_Debug_Motor_Right_clicked();

    void on_retry_Connection_button_clicked();

    void on_Debug_laser_Right_on_clicked();

    void on_Debug_Laser_Right_off_clicked();

private:
    Ui::MainWindow *ui;
    QextSerialPort *port;
    QString mesh_file_path;
    QByteArray received_byte;
    //QPixmap pix();
};

#endif // MAINWINDOW_H
