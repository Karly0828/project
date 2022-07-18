#include "mainwindow.h"
#include "eigenvalue.h"
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QTextStream>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();


}
