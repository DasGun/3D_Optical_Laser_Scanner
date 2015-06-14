//
//  main.cpp
//  3D Optical Laser Scanner
//
//  Created by Dasun Gunasinghe on 27/07/2014.
//  Copyright (c) 2014 Dasun Gunasinghe. All rights reserved.
//

#include "mainwindow.h"
#include <qtabwidget.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("3D Scanning System - Prototype 1");
    w.show();

    return a.exec();
}
