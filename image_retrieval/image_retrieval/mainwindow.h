#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QSqlDatabase>
#include <QMouseEvent>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>

using namespace cv;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
//    void displayImage(const Mat img1,const Mat img2,const int i);
bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButton_choose_clicked();

    void on_pushButton_clicked();


    void on_pushButton_upload_clicked();

    void on_pushButton_Histogram_clicked();


    void on_pushButton_Edge_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
