#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "eigenvalue.h"
#include <QMessageBox>
#include <QtSql>
#include <QSqlDatabase>
#include <QFileDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantList>
#include <QPainter>

#include "mainwindow.h"
#include "io.h"

#include <QApplication>
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>

#include <QAbstractTableModel>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;
using namespace std;

Mat base;
bool flag_color=false;
bool flag_edge=false;

QString Current_File_Name=NULL;
bool flag_color_only=false;
bool flag_edge_only=false;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_show_1->installEventFilter(this);
    ui->label_show_2->installEventFilter(this);
    ui->label_show_3->installEventFilter(this);
    ui->label_show_4->installEventFilter(this);
    ui->label_show_5->installEventFilter(this);
    ui->label_show_6->installEventFilter(this);
    ui->label_show_7->installEventFilter(this);
    ui->label_show_8->installEventFilter(this);






    QPixmap pix("C:/ImageTest/test.png");
//    int w = ui->label_choose->width();
//    int h = ui->label_choose->height();
//    ui->label_choose->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
    ui->label_choose->setPixmap((pix).scaled(ui->label_choose->size()));
}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_pushButton_choose_clicked()
{

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("image_list");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("root");
    bool ok = db.open();

    if(ok){
        QMessageBox::information(this,"Connection","Database connected successfully");
    }else{
        QMessageBox::information(this,"Not Connected","Database is not connected");
    }
    QString OpenFile, OpenFilePath;
        QImage image;

        OpenFile = QFileDialog::getOpenFileName(this,
            "please choose an image file",
            "",
            "Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)");
        if (OpenFile != "")
        {
            if (image.load(OpenFile))
            {
                //仅仅只是导入之后的图片仍然是原来的大小，这个时候需要缩放

                ui->label_choose->setPixmap(QPixmap::fromImage(image).scaled(ui->label_choose->size()));
            }

        }
        QFileInfo OpenFileInfo;
            OpenFileInfo = QFileInfo(OpenFile);
            OpenFilePath = OpenFileInfo.filePath();


            QSqlQuery query;
            QFile * file=new QFile(OpenFilePath); //fileName为二进制数据文件名
            file->open(QIODevice::ReadOnly);//设置为只读模式

            //QByteArray data = file->readAll();//将数据转换为QByteArray类型存储到数据库中
            file->close();
            Current_File_Name=OpenFilePath;



            query.exec("create table if not exists image_address(id INT,filename VARCHAR[45],similarity_color[DOUBLE],similarity_edge[DOUBLE],similarity_multi[DOUBLE])");
            QString sql = "replace into image_address(filename) values(:filename)";
//            QString sql = "insert ignore into image_address(filename) values(:filename)";
            query.prepare(sql);
//            query.bindValue(":id",NULL);
            query.bindValue(":filename",file->fileName());

            if(file->fileName().isEmpty()){
                QMessageBox::information(this,"Warning","No file specified");
            }else{



            base = imread(OpenFilePath.toStdString());

            if(!query.exec())
                {
                    QMessageBox::information(this,"Warning",query.lastError().text().toLocal8Bit().data());
                }
                else
                {
                    QMessageBox::information(this,"Message","Insert successfully");
                }
            }
}

QImage cvMat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
        delete pSrc;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
        delete pSrc;
    }
    else if(mat.type() == CV_8UC4)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
        delete pSrc;
    }
    else
    {
        return QImage();
    }
}

//std::vector <cv::Mat> img_matches_bf;

int lowerThreshold = 80;
int max_lowThreshold = 150;

void MainWindow::on_pushButton_clicked()
{
    if(ui->radioButton_color->isChecked()){




        std::vector <cv::String> filelist_color;
        std::vector <cv::String> finallist_color;
        QString qstr_1;
        QString qstr_2;
        QSqlQuery query_color_pre;
        QSqlQuery query_color;
        QSqlQuery query_color_final;
        std::vector <cv::Mat> convertlist;
        std::vector <cv::Mat> HSVlist;
//        std::vector <similarityInfo> color_list;


//        std::vector <cv::String> Simlist;



        query_color_pre.exec( "select * from image_address" );
        if(!query_color_pre.exec()){
            QMessageBox::information(this,"Warning",query_color_pre.lastError().text());
        }else{

        QMessageBox::information(this,"Method","Color-Based");
        while(query_color_pre.next())
                {
                     qstr_1 = query_color_pre.value(1).toString();
                     filelist_color.push_back(qstr_1.toStdString());
                }



        for (auto &i : filelist_color) {
            cout<<i<<endl;
            convertlist.push_back(imread(i,cv::IMREAD_REDUCED_COLOR_4));

        }



        if (!base.data)
        {
            QMessageBox::information(this,"Warning","Base image load faild.");
        }

        Mat HSVbase;
        //步骤一：从RGB空间转换到HSV空间
        cvtColor(base, HSVbase, CV_BGR2HSV);



        for (auto &i : convertlist) {

            Mat HSVtemp;

            cvtColor(i, HSVtemp, CV_BGR2HSV);

            HSVlist.push_back(HSVtemp);


        }

        int h_bins = 50;
        int s_bins = 60;
        int histsize[] = { h_bins,s_bins };
        //hue varies from 0 to 179,saturation from 0 to 255
        float h_ranges[] = { 0,180 };
        float s_ranges[] = { 0,256 };
        const float*histRanges[] = { h_ranges,s_ranges };
        //use the 0-th and 1-st channels
        int channels[] = { 0,1 };

        MatND hist_base;
        MatND hist_test;

        //计算直方图
        calcHist(&HSVbase, 1, channels, Mat(), hist_base, 2, histsize, histRanges, true, false);
        normalize(hist_base, hist_base, 0, 1, NORM_MINMAX, -1, Mat());//归一化

        int k=0;
        double Simlist[filelist_color.size()];
        for (auto &i : HSVlist)
        {
            calcHist(&i, 1, channels, Mat(), hist_test, 2, histsize, histRanges, true, false);


        //归一化

            normalize(hist_test, hist_test, 0, 1, NORM_MINMAX, -1, Mat());

        //步骤三：比较直方图，并返回值
            double similarity = compareHist(hist_base, hist_test, CV_COMP_BHATTACHARYYA);
            Simlist[k]=similarity;
            cout<<Simlist[k]<<endl;
            query_color.prepare("update image_address set similarity_color=? where filename=?");
            query_color.addBindValue(Simlist[k]);
//            QVariant var;
//            var.setValue(filelist_color[k]);
//            query_color.addBindValue(var);
            query_color.addBindValue(QString(QString::fromLocal8Bit(filelist_color[k].c_str())));
            query_color.exec();

//          QString sql = "insert ignore into image_address(similarity) values(:similarity)";

            k++;

    }

        query_color_final.exec( "select * from image_address order by similarity_color asc" );
//---------------------------------------------------------------------
        while(query_color_final.next())
                {
                     qstr_2 = query_color_final.value(1).toString();
                     finallist_color.push_back(qstr_2.toStdString());

                }

        for (auto &i : finallist_color) {
            cout<<i<<endl;}

        static QSqlQueryModel *mode= new QSqlQueryModel(ui->tableView);

        mode->setQuery("select * from image_address");

        mode->setHeaderData(1, Qt::Horizontal, tr("文件路径"));

        mode->setHeaderData(2, Qt::Horizontal, tr("颜色特征相似值"));

        mode->setHeaderData(3, Qt::Horizontal, tr("Canny算子边缘检测相似值"));

        mode->setHeaderData(4, Qt::Horizontal, tr("多特征融合相似值"));

        ui->tableView->setModel(mode);


            QImage img1= cvMat2QImage(imread(finallist_color[0]));

            ui->label_show_1->setPixmap(QPixmap::fromImage(img1));

            ui->label_show_1->setScaledContents(true);

            QImage img2= cvMat2QImage(imread(finallist_color[1]));

            ui->label_show_2->setPixmap(QPixmap::fromImage(img2));

            ui->label_show_2->setScaledContents(true);

            QImage img3= cvMat2QImage(imread(finallist_color[2]));

            ui->label_show_3->setPixmap(QPixmap::fromImage(img3));

            ui->label_show_3->setScaledContents(true);

            QImage img4= cvMat2QImage(imread(finallist_color[3]));

            ui->label_show_4->setPixmap(QPixmap::fromImage(img4));

            ui->label_show_4->setScaledContents(true);
            QImage img5= cvMat2QImage(imread(finallist_color[4]));

            ui->label_show_5->setPixmap(QPixmap::fromImage(img5));

            ui->label_show_5->setScaledContents(true);
            QImage img6= cvMat2QImage(imread(finallist_color[5]));

            ui->label_show_6->setPixmap(QPixmap::fromImage(img6));

            ui->label_show_6->setScaledContents(true);
            QImage img7= cvMat2QImage(imread(finallist_color[6]));

            ui->label_show_7->setPixmap(QPixmap::fromImage(img7));

            ui->label_show_7->setScaledContents(true);
            QImage img8= cvMat2QImage(imread(finallist_color[7]));

            ui->label_show_8->setPixmap(QPixmap::fromImage(img8));

            ui->label_show_8->setScaledContents(true);

            flag_color=true;
            flag_color_only=true;
            flag_edge_only=false;
        }

//            delete ui->label_show_1;
//            delete ui->label_show_2;
//            delete ui->label_show_3;
//            delete ui->label_show_4;
//            delete ui->label_show_5;
//            delete ui->label_show_6;




    }



    if(ui->radioButton_edge->isChecked()){

        std::vector <cv::String> filelist_edge;
        std::vector <cv::String> finallist_edge;
        QString qstr_1;
        QString qstr_2;
        QSqlQuery query_edge_pre;
        QSqlQuery query_edge;
        QSqlQuery query_edge_final;
        std::vector <cv::Mat> convertlist;
        std::vector <cv::Mat> Cannylist;
        std::vector <cv::Mat> Edgelist;

        double Simlist[filelist_edge.size()];

        int64 t1, t2;
        int k=0;
//        double tkpt, tdes;
        double tmatch_bf;

        query_edge_pre.exec( "select * from image_address" );
        if(!query_edge_pre.exec()){
            QMessageBox::information(this,"Warning",query_edge_pre.lastError().text());
        }else{

            lowerThreshold = 80;
            max_lowThreshold = 150;

            ui->lineEdit_min->setValidator(new QIntValidator(ui->lineEdit_min));
            ui->lineEdit_max->setValidator(new QIntValidator(ui->lineEdit_max));

            QString min = ui->lineEdit_min->text();



            QString max = ui->lineEdit_max->text();

            cout<<min.toInt()<<endl;
            cout<<max.toInt()<<endl;

            if (min.toInt() < 0 || min.toInt() > max.toInt() ||  max.toInt() <= 0 || max.toInt() > 250)
            {
                QMessageBox::information(this,"Message","Please input integers from 0 to 250, or it will be detect by default from 80 to 150.");
            }else{

            lowerThreshold = min.toInt();
            max_lowThreshold = max.toInt();
            }

            cout<<min.toInt()<<endl;
            cout<<max.toInt()<<endl;

        QMessageBox::information(this,"Method","Canny-Based Edge Detection");
        while(query_edge_pre.next())
                {
                     qstr_1 = query_edge_pre.value(1).toString();
                     filelist_edge.push_back(qstr_1.toStdString());
                }
        for (auto &i : filelist_edge) {
            cout<<i<<endl;
        }
        if (!base.data)
        {
            QMessageBox::information(this,"Warning","Base image load faild.");
        }

       Mat base_gray;
       cvtColor(base, base_gray, COLOR_BGR2GRAY);

       for (auto &i : filelist_edge) {
           cout<<i<<endl;
           convertlist.push_back(imread(i,cv::IMREAD_REDUCED_COLOR_4));

       }

       for (auto &i : convertlist) {

           Mat test_gray;

           cvtColor(i, test_gray, COLOR_BGR2GRAY);

           Cannylist.push_back(test_gray);

       }

       //int method;

       //Canny Edge Detection Variables




//       int lowerThreshold = 30;
//       int max_lowThreshold = 100;

       Mat base_blurred,base_edge;

           GaussianBlur(base_gray,
               base_blurred,
               cv::Size(3, 3),  //Smoothing window width and height in pixels
               3);  //How much the image will be blur

           Canny(base_blurred,
               base_edge,
               lowerThreshold, //lower threshold
               max_lowThreshold);    //higher threshold

            for (auto &i :Cannylist){
                Mat test_blurred,test_edge;
                GaussianBlur(i,
                    test_blurred,
                    cv::Size(3, 3),  //Smoothing window width and height in pixels
                    3);  //How much the image will be blur

                Canny(test_blurred,
                    test_edge,
                    lowerThreshold, //lower threshold
                    max_lowThreshold);    //higher threshold

              Edgelist.push_back(test_edge);

            }

//          if (!img_matches_bf.empty())img_matches_bf.clear();


            // 1. 读取图片
         for (auto &i :Edgelist){


            const cv::Mat image1 = base_edge; //Load as grayscale
            const cv::Mat image2 = i; //Load as grayscale
            std::vector<cv::KeyPoint> keypoints1;
            std::vector<cv::KeyPoint> keypoints2;



            cv::Ptr<cv::SiftFeatureDetector> sift = cv::SiftFeatureDetector::create();
            // 2. 计算特征点
//            t1 = cv::getTickCount();
            sift->detect(image1, keypoints1);
//            t2 = cv::getTickCount();
//            tkpt = 1000.0*(t2-t1) / cv::getTickFrequency();
            sift->detect(image2, keypoints2);


            // 3. 计算特征描述符
            cv::Mat descriptors1, descriptors2;
//            t1 = cv::getTickCount();
            sift->compute(image1, keypoints1, descriptors1);
//            t2 = cv::getTickCount();
//            tdes = 1000.0*(t2-t1) / cv::getTickFrequency();
            sift->compute(image2, keypoints2, descriptors2);


            // 4. 特征匹配
            cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE);
            // cv::BFMatcher matcher(cv::NORM_L2);

            // (1) 直接暴力匹配
            std::vector<cv::DMatch> matches;
            t1 = cv::getTickCount();
            matcher->match(descriptors1, descriptors2, matches);
            t2 = cv::getTickCount();
            tmatch_bf = 1000.0*(t2-t1) / cv::getTickFrequency();
            cout<<tmatch_bf<<endl;

             Simlist[k]=tmatch_bf;

//             drawMatches(image1, keypoints1, image2, keypoints2, matches, img_matches_bf[k]);

             query_edge.prepare("update image_address set similarity_edge=? where filename=?");
             query_edge.addBindValue(Simlist[k]);
             query_edge.addBindValue(QString(QString::fromLocal8Bit(filelist_edge[k].c_str())));
             query_edge.exec();




             k++;

     }


//         for (int i =0;i++;i<filelist_edge.size()){
//                 convertlist[i].release();
//                 Cannylist[i].release();
//                 Edgelist[i].release();
//         }
//---------------------------------------------------------------------
         query_edge_final.exec( "select * from image_address order by similarity_edge asc" );

         while(query_edge_final.next())
                 {
                      qstr_2 = query_edge_final.value(1).toString();
                      finallist_edge.push_back(qstr_2.toStdString());

                 }

        cout<<finallist_edge.size()<<endl;

         for (auto &i : finallist_edge) {

             cout<<i<<endl;

         }

         static QSqlQueryModel *mode= new QSqlQueryModel(ui->tableView);

         mode->setQuery("select * from image_address");

         mode->setHeaderData(1, Qt::Horizontal, tr("文件路径"));

         mode->setHeaderData(2, Qt::Horizontal, tr("颜色特征相似值"));

         mode->setHeaderData(3, Qt::Horizontal, tr("Canny算子边缘检测相似值"));

         mode->setHeaderData(4, Qt::Horizontal, tr("多特征融合相似值"));

         ui->tableView->setModel(mode);


         QImage img1= cvMat2QImage(imread(finallist_edge[0]));

         ui->label_show_1->setPixmap(QPixmap::fromImage(img1));

         ui->label_show_1->setScaledContents(true);

         QImage img2= cvMat2QImage(imread(finallist_edge[1]));

         ui->label_show_2->setPixmap(QPixmap::fromImage(img2));

         ui->label_show_2->setScaledContents(true);

         QImage img3= cvMat2QImage(imread(finallist_edge[2]));

         ui->label_show_3->setPixmap(QPixmap::fromImage(img3));

         ui->label_show_3->setScaledContents(true);

         QImage img4= cvMat2QImage(imread(finallist_edge[3]));

         ui->label_show_4->setPixmap(QPixmap::fromImage(img4));

         ui->label_show_4->setScaledContents(true);
         QImage img5= cvMat2QImage(imread(finallist_edge[4]));

         ui->label_show_5->setPixmap(QPixmap::fromImage(img5));

         ui->label_show_5->setScaledContents(true);
         QImage img6= cvMat2QImage(imread(finallist_edge[5]));

         ui->label_show_6->setPixmap(QPixmap::fromImage(img6));

         ui->label_show_6->setScaledContents(true);

         QImage img7= cvMat2QImage(imread(finallist_edge[6]));

         ui->label_show_7->setPixmap(QPixmap::fromImage(img7));

         ui->label_show_7->setScaledContents(true);
         QImage img8= cvMat2QImage(imread(finallist_edge[7]));

         ui->label_show_8->setPixmap(QPixmap::fromImage(img8));

         ui->label_show_8->setScaledContents(true);

         flag_edge=true;
         flag_color_only=false;
         flag_edge_only=true;

        }

//         delete ui->label_show_1;
//         delete ui->label_show_2;
//         delete ui->label_show_3;
//         delete ui->label_show_4;
//         delete ui->label_show_5;
//         delete ui->label_show_6;



    }




    if(ui->radioButton_multi->isChecked()){

        double color_percentage=50.0;
        double edge_percentage=50.0;

        if(!(flag_color&&flag_edge)){
            QMessageBox::information(this,"Method","Please make sure both methods above have been executed first.");
        }else{



            ui->lineEdit_color->setValidator(new QIntValidator(ui->lineEdit_color));
            ui->lineEdit_edge->setValidator(new QIntValidator(ui->lineEdit_edge));

            QString color = ui->lineEdit_color->text();



            QString edge = ui->lineEdit_edge->text();

            cout<<"begin"<<endl;


            if (color.toInt()<=0 || edge.toInt()<=0)
            {
                QMessageBox::information(this,"Message","Please input integers more than 0, or it will be calculate by default 50% and 50%.");
            }else{

                color_percentage = color.toDouble();
                edge_percentage = edge.toDouble();
            }

            cout<<color_percentage <<endl;
            cout<<edge_percentage <<endl;

            QMessageBox::information(this,"Method","Multi-Features Fusion");

        std::vector <cv::String> filelist_multi;
        std::vector <cv::String> finallist_multi;
        QString qstr_1;
        QString qstr_2;
        QSqlQuery query_multi_pre;
        QSqlQuery query_multi;
        QSqlQuery query_multi_final;

        double Simlist_color[filelist_multi.size()];
        double Simlist_edge[filelist_multi.size()];
        double Simlist_multi[filelist_multi.size()];

        int i=0;
        query_multi_pre.exec( "select * from image_address" );
        while(query_multi_pre.next())
                {
                     Simlist_multi[i]=0.0;
                     qstr_1 = query_multi_pre.value(1).toString();
                     filelist_multi.push_back(qstr_1.toStdString());
                     cout<<qstr_1.toStdString()<<endl;
                     Simlist_color[i] = query_multi_pre.value(2).toDouble();
                     cout<<Simlist_color[i]<<endl;
                     Simlist_edge[i] = query_multi_pre.value(3).toDouble();
                     cout<<Simlist_edge[i]<<endl;
                     Simlist_multi[i]=query_multi_pre.value(2).toDouble()*color_percentage*0.01+
                             query_multi_pre.value(3).toDouble()*edge_percentage*0.01;

                     cout<<Simlist_multi[i]<<endl;

                     i++;

                }

//        for (auto &i : filelist_multi) {
//            cout<<i<<endl;
//            cout<<query_multi_pre.value(1).toDouble()<<endl;
//            cout<<query_multi_pre.value(2).toDouble()<<endl;
//        }


//        while(query_multi_pre.next())
//                {
//                     int i=0;
//                     Simlist_color[i] = query_multi_pre.value(2).toDouble();
//                     cout<<Simlist_color[i]<<endl;
//                     Simlist_edge[i] = query_multi_pre.value(3).toDouble();
//                     cout<<Simlist_edge[i]<<endl;
//                     Simlist_multi[i]=Simlist_color[i]*color_percentage*0.01+Simlist_edge[i]*edge_percentage*0.01;
//                     cout<<Simlist_multi[i]<<endl;
//                }

        int k=0;
        for (auto &i:filelist_multi){
            query_multi.prepare("update image_address set similarity_multi=? where filename=?");
            query_multi.addBindValue(Simlist_multi[k]);
            query_multi.addBindValue(QString(QString::fromLocal8Bit(i.c_str())));
            query_multi.exec();
            k++;
        }

        query_multi_final.exec( "select * from image_address order by similarity_multi asc" );

        while(query_multi_final.next())
                {
                     qstr_2 = query_multi_final.value(1).toString();
                     finallist_multi.push_back(qstr_2.toStdString());

                }

       cout<<finallist_multi.size()<<endl;

        for (auto &i : finallist_multi) {

            cout<<i<<endl;

        }

        static QSqlQueryModel *mode= new QSqlQueryModel(ui->tableView);

        mode->setQuery("select * from image_address");

        mode->setHeaderData(1, Qt::Horizontal, tr("文件路径"));

        mode->setHeaderData(2, Qt::Horizontal, tr("颜色特征相似值"));

        mode->setHeaderData(3, Qt::Horizontal, tr("Canny算子边缘检测相似值"));

        mode->setHeaderData(4, Qt::Horizontal, tr("多特征融合相似值"));

        ui->tableView->setModel(mode);

        QImage img1= cvMat2QImage(imread(finallist_multi[0]));

        ui->label_show_1->setPixmap(QPixmap::fromImage(img1));

        ui->label_show_1->setScaledContents(true);

        QImage img2= cvMat2QImage(imread(finallist_multi[1]));

        ui->label_show_2->setPixmap(QPixmap::fromImage(img2));

        ui->label_show_2->setScaledContents(true);

        QImage img3= cvMat2QImage(imread(finallist_multi[2]));

        ui->label_show_3->setPixmap(QPixmap::fromImage(img3));

        ui->label_show_3->setScaledContents(true);

        QImage img4= cvMat2QImage(imread(finallist_multi[3]));

        ui->label_show_4->setPixmap(QPixmap::fromImage(img4));

        ui->label_show_4->setScaledContents(true);
        QImage img5= cvMat2QImage(imread(finallist_multi[4]));

        ui->label_show_5->setPixmap(QPixmap::fromImage(img5));

        ui->label_show_5->setScaledContents(true);
        QImage img6= cvMat2QImage(imread(finallist_multi[5]));

        ui->label_show_6->setPixmap(QPixmap::fromImage(img6));

        ui->label_show_6->setScaledContents(true);
        QImage img7= cvMat2QImage(imread(finallist_multi[6]));

        ui->label_show_7->setPixmap(QPixmap::fromImage(img7));

        ui->label_show_7->setScaledContents(true);
        QImage img8= cvMat2QImage(imread(finallist_multi[7]));

        ui->label_show_8->setPixmap(QPixmap::fromImage(img8));

        ui->label_show_8->setScaledContents(true);


        flag_color_only=true;
        flag_edge_only=true;

    }
}
}


void MainWindow::on_pushButton_upload_clicked()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setDatabaseName("image_list");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("root");
    bool ok = db.open();

    if(ok){
        QMessageBox::information(this,"Connection","Database connected successfully");
    }else{
        QMessageBox::information(this,"Not Connected","Database is not connected");
    }

    std::vector <cv::String> m_qlistLoadImgs;
    QSqlQuery query;
    QFileDialog fileDlg(this);
    fileDlg.setWindowTitle("Choose Pictures");
    QStringList qstrFilters;//设置文件过滤器的list
    QStringList strPathList;
    qstrFilters<<"Image files(*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm)";
    qstrFilters<<"Any files (*)";
    fileDlg.setNameFilters(qstrFilters);//设置文件过滤器
    fileDlg.setFileMode(QFileDialog::ExistingFiles);//设置能选择多个文件，如果是单个文件就写成QFileDialog::ExistingFile
    if(!strPathList.isEmpty())
    {
        strPathList.clear();
    }
    if(fileDlg.exec() == QDialog::Accepted)
    {
        strPathList = fileDlg.selectedFiles();
    }

        if(strPathList.isEmpty())
        {
           QMessageBox::information(this,"Warning","No file selected");
        }else

       {

       for(int i = 0;i<strPathList.count();i++)
       {
            cout<<strPathList[i].toStdString()<<endl;
       }

//       for (auto &i:strPathList){
//           cout<<i.toStdString()<<endl;
//           QString sql = "replace into image_address(filename) values(:filename)";
////            QString sql = "insert ignore into image_address(filename) values(:filename)";
//           query.prepare(sql);
////            query.bindValue(":id",NULL);
//           query.addBindValue(":filename",i);
//       }

       int k=0;
       for (auto &i:strPathList){
           query.prepare("replace into image_address(filename) values(?)");
           query.addBindValue(QString(i));
           query.exec();
           k++;
       }




       if(!query.exec())
           {
               QMessageBox::information(this,"Warning",query.lastError().text().toLocal8Bit().data());
           }
           else
           {
               QMessageBox::information(this,"Message","Upload successfully");
           }
    }
}



void MainWindow::on_pushButton_Histogram_clicked()
{
    Mat src, dst, dst1;
        src = imread(Current_File_Name.toStdString());
        if (!src.data)
        {
            QMessageBox::information(this,"Warning","Image Load Faild");
        }else{

        //步骤一：分通道显示
        vector<Mat>bgr_planes;
        split(src, bgr_planes);
        //split(// 把多通道图像分为多个单通道图像 const Mat &src, //输入图像 Mat* mvbegin）// 输出的通道图像数组

        //步骤二：计算直方图
        int histsize = 256;
        float range[] = { 0,256 };
        const float*histRanges = { range };
        Mat b_hist, g_hist, r_hist;
        calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histsize, &histRanges, true, false);
        calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histsize, &histRanges, true, false);
        calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histsize, &histRanges, true, false);


        //归一化
        int hist_h = 400;//直方图的图像的高
        int hist_w = 512;//直方图的图像的宽
        int bin_w = hist_w / histsize;//直方图的等级
        Mat histImage(hist_w, hist_h, CV_8UC3, Scalar(0, 0, 0));//绘制直方图显示的图像
        normalize(b_hist, b_hist, 0, hist_h, NORM_MINMAX, -1, Mat());//归一化
        normalize(g_hist, g_hist, 0, hist_h, NORM_MINMAX, -1, Mat());
        normalize(r_hist, r_hist, 0, hist_h, NORM_MINMAX, -1, Mat());

        //步骤三：绘制直方图（render histogram chart）
        for (int i = 1; i < histsize; i++)
        {
            //绘制蓝色分量直方图
            line(histImage, Point((i - 1)*bin_w, hist_h - cvRound(b_hist.at<float>(i - 1))),
                Point((i)*bin_w, hist_h - cvRound(b_hist.at<float>(i))), Scalar(255, 0, 0), 2, CV_AA);
            //绘制绿色分量直方图
            line(histImage, Point((i - 1)*bin_w, hist_h - cvRound(g_hist.at<float>(i - 1))),
                Point((i)*bin_w, hist_h - cvRound(g_hist.at<float>(i))), Scalar(0, 255, 0), 2, CV_AA);
            //绘制红色分量直方图
            line(histImage, Point((i - 1)*bin_w, hist_h - cvRound(r_hist.at<float>(i - 1))),
                Point((i)*bin_w, hist_h - cvRound(r_hist.at<float>(i))), Scalar(0, 0, 255), 2, CV_AA);
        }
        imshow("Histogram Image", histImage);
        waitKey(0);
        }
}

Mat img, gray, blurred, edge;
int lowerThreshold_1 = 0;
int max_lowThreshold_1 = 250;

void CannyThreshold(int, void*) {
    GaussianBlur(gray,
        blurred,
        cv::Size(3, 3),  //Smoothing window width and height in pixels
        3);  //How much the image will be blur

    Canny(blurred,
        edge,
        lowerThreshold_1, //lower threshold
        50);    //higher threshold
    imshow("Edge Detection", edge);
}

Mat Canny_for_edgematches(const cv::String s){
    Mat img, gray, blurred, edge;

    img=imread(s);
    cvtColor(img, gray, COLOR_BGR2GRAY);

    GaussianBlur(gray,
        blurred,
        cv::Size(3, 3),  //Smoothing window width and height in pixels
        3);  //How much the image will be blur

    Canny(blurred,
        edge,
        lowerThreshold, //lower threshold
        max_lowThreshold);
    return edge;
}

void MainWindow::on_pushButton_Edge_clicked()
{
    img = imread(Current_File_Name.toStdString());
    if (!img.data)
    {
        QMessageBox::information(this,"Warning","Image Load Faild");
     }else{

    cvtColor(img, gray, COLOR_BGR2GRAY);
    cv::namedWindow("Edge Detection", 0);
    cv::resizeWindow("Edge Detection",900,800);
    createTrackbar("Min Threshold:", "Edge Detection", &lowerThreshold_1, max_lowThreshold_1, CannyThreshold);
    CannyThreshold(lowerThreshold_1, 0);
    waitKey(0);
    }
}

Mat HSV_show(const cv::String str,const int i)
{
    Mat srcimage = imread(str);

        Mat srcimageHSV;
        //图像转化HSV颜色空间图像
        cvtColor(srcimage, srcimageHSV, COLOR_BGR2HSV);
//        imshow("HSV空间图像", srcimageHSV);
        int channels = 0;
        int histsize[] = { 518 };
        float midranges[] = { 0,255 };
        const float *ranges[] = { midranges };
        MatND  dsthist;
        calcHist(&srcimageHSV, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
        Mat b_drawImage = Mat::zeros(Size(518, 518), CV_8UC3);

        double g_dhistmaxvalue;
        minMaxLoc(dsthist, 0, &g_dhistmaxvalue, 0, 0);
        for (int i = 0;i < 518;i++) {
            int value = cvRound(518 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
            line(b_drawImage, Point(i, b_drawImage.rows - 1), Point(i, b_drawImage.rows - 1 - value), Scalar(255, 0, 0));
        }
//        imshow("H通道直方图", b_drawImage);

        channels = 1;
        calcHist(&srcimageHSV, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
        Mat g_drawImage = Mat::zeros(Size(518, 518), CV_8UC3);
        for (int i = 0;i < 518;i++) {
            int value = cvRound(518 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
            line(g_drawImage, Point(i, g_drawImage.rows - 1), Point(i, g_drawImage.rows - 1 - value), Scalar(0, 255, 0));
        }
//        imshow("S通道直方图", g_drawImage);

        channels = 2;
        calcHist(&srcimageHSV, 1, &channels, Mat(), dsthist, 1, histsize, ranges, true, false);
        Mat r_drawImage = Mat::zeros(Size(518, 518), CV_8UC3);
        for (int i = 0;i < 518;i++) {
            int value = cvRound(518 * 0.9 *(dsthist.at<float>(i) / g_dhistmaxvalue));
            line(r_drawImage, Point(i, r_drawImage.rows - 1), Point(i, r_drawImage.rows - 1 - value), Scalar(0, 0, 255));
        }
//        imshow("V通道直方图", r_drawImage);
        cv::Mat add1, add2;
        add(b_drawImage, g_drawImage, add1);
        add(add1, r_drawImage, add2);

        QSqlQuery query_color_final;
        QString qstr_2;
        std::vector <cv::String> finallist_color;

        query_color_final.exec( "select * from image_address order by similarity_color asc" );
//---------------------------------------------------------------------
        while(query_color_final.next())
                {
                     qstr_2 = query_color_final.value(2).toString();
                     finallist_color.push_back(qstr_2.toStdString());
                }
        cv::String text="HSV-Similarity:"+finallist_color[i];

        putText(add2,text,Point(30,30),CV_FONT_HERSHEY_COMPLEX,0.7,Scalar(255,255,255),1,1);

        return add2;
//        在img图片上，显示Hello，位置在（50,50），字体类型为FONT_HERSHEY_SIMPLEX，字体大小为2，颜色为绿色，字体厚度为4，线型默认为8.
//         imshow("HSV",add2);
//         waitKey(0);


}



Mat Matches_edge(const cv::String im1,const cv::String im2,const int i){
//        int64 t1, t2;
//        double tkpt, tdes;
//        double tmatch_bf;

        // 1. 读取图片
        const cv::Mat image1 = Canny_for_edgematches(im1);//Load as grayscale
        const cv::Mat image2 = Canny_for_edgematches(im2); //Load as grayscale
        std::vector<cv::KeyPoint> keypoints1;
        std::vector<cv::KeyPoint> keypoints2;

        cv::Ptr<cv::SiftFeatureDetector> sift = cv::SiftFeatureDetector::create();
        // 2. 计算特征点
//        t1 = cv::getTickCount();
        sift->detect(image1, keypoints1);
//        t2 = cv::getTickCount();
//        tkpt = 1000.0*(t2-t1) / cv::getTickFrequency();
        sift->detect(image2, keypoints2);


        // 3. 计算特征描述符
        cv::Mat descriptors1, descriptors2;
//        t1 = cv::getTickCount();
        sift->compute(image1, keypoints1, descriptors1);
//        t2 = cv::getTickCount();
//        tdes = 1000.0*(t2-t1) / cv::getTickFrequency();
        sift->compute(image2, keypoints2, descriptors2);


        // 4. 特征匹配
        cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE);
        // cv::BFMatcher matcher(cv::NORM_L2);

        // (1) 直接暴力匹配
        std::vector<cv::DMatch> matches;
//        t1 = cv::getTickCount();
        matcher->match(descriptors1, descriptors2, matches);
//        t2 = cv::getTickCount();
//        tmatch_bf = 1000.0*(t2-t1) / cv::getTickFrequency();
        // 画匹配图
        cv::Mat img_matches_bf;
        drawMatches(image1, keypoints1, image2, keypoints2, matches, img_matches_bf);

        QSqlQuery query_edge_final;
        QString qstr_2;
        std::vector <cv::String> finallist_edge;

        query_edge_final.exec( "select * from image_address order by similarity_edge asc" );
//---------------------------------------------------------------------
        while(query_edge_final.next())
                {
                     qstr_2 = query_edge_final.value(3).toString();
                     finallist_edge.push_back(qstr_2.toStdString());
                }
        cv::String text="Edge-Similarity:"+finallist_edge[i];

        putText(img_matches_bf,text,Point(20,30),CV_FONT_HERSHEY_COMPLEX,0.7,Scalar(255,255,255),1,1);

        return img_matches_bf;
//        在img图片上，显示Hello，位置在（50,50），字体类型为FONT_HERSHEY_SIMPLEX，字体大小为2，颜色为白色，字体厚度为4，线型默认为8.
//        imshow("Edge Matches", img_matches_bf);

}

Mat edge_list(int i){
    QSqlQuery query_edge_final;
    QString qstr_2;
    std::vector <cv::String> finallist_edge;


    query_edge_final.exec( "select * from image_address order by similarity_edge asc" );
    while(query_edge_final.next())
            {
                 qstr_2 = query_edge_final.value(1).toString();
                 finallist_edge.push_back(qstr_2.toStdString());
            }
//    int n = atoi(finallist_edge[i].c_str());
//    imshow("bf_matches", img_matches_bf[n]);
      return Matches_edge(Current_File_Name.toStdString(),finallist_edge[i],i);

}


Mat color_list(int i){
    QSqlQuery query_color_final;
    QString qstr_2;
    std::vector <cv::String> finallist_color;


    query_color_final.exec( "select * from image_address order by similarity_color asc" );
    while(query_color_final.next())
            {
                 qstr_2 = query_color_final.value(1).toString();
                 finallist_color.push_back(qstr_2.toStdString());
            }
//    int n = atoi(finallist_edge[i].c_str());
//    imshow("bf_matches", img_matches_bf[n]);
      return HSV_show(finallist_color[i],i);

}

//Mat QImage2cvMat(QImage image)
//{
//    cv::Mat mat;
//    switch(image.format())
//    {
//    case QImage::Format_ARGB32:
//    case QImage::Format_RGB32:
//    case QImage::Format_ARGB32_Premultiplied:
//        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.bits(), image.bytesPerLine());
//        break;
//    case QImage::Format_RGB888:
//        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.bits(), image.bytesPerLine());
//        cv::cvtColor(mat, mat, CV_BGR2RGB);
//        break;
//    case QImage::Format_Indexed8:
//        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.bits(), image.bytesPerLine());
//        break;
//    }
//    return mat;
//}

bool displayImage(const Mat img1,const Mat img2,const int i)
{
    if (img1.empty() || img2.empty())
        {
            return false;
        }
//        double height = img1.rows;

//        double width1 = img1.cols;
//        double width2 = img2.cols;
        double height2 = img2.rows;

        cout<<img1.cols<<endl;
        cout<<img2.cols<<endl;
        cout<<img1.rows<<endl;
        cout<<img2.rows<<endl;



        double re_width=(518.0/height2)*img2.cols;

        cout<<re_width<<endl;

        Mat re_img;
        cv::resize(img2,re_img,Size(re_width,518.0));




//        // 将高图像等比缩放与低图像高度一致
//        if (img1.rows > img2.rows)
//        {
//            height = img2.rows;
//            width1 = img1.cols * ((float)img2.rows / (float)img1.rows);
//            resize(img1, img1, Size(width1, height));
//        }
//        else if(img1.rows <= img2.rows)
//        {
//            width2 = img2.cols * ((float)img1.rows / (float)img2.rows);
//            resize(img2, img2, Size(width2, height));
//        }

        cout<<img1.rows<<","<<img1.cols<<endl;
        cout<<re_img.rows<<","<<re_img.cols<<endl;

        //创建目标Mat
            vector<Mat> vImgs;
            Mat result;
            vImgs.push_back(img1);
            vImgs.push_back(re_img);
//            vconcat(vImgs, result); //垂直方向拼接
            hconcat(vImgs, result); //水平方向拼接





    QSqlQuery query_multi_final;
    QString qstr_2;
    std::vector <cv::String> finallist_multi;

    query_multi_final.exec( "select * from image_address order by similarity_color asc" );
//---------------------------------------------------------------------
    while(query_multi_final.next())
            {
                 qstr_2 = query_multi_final.value(4).toString();
                 finallist_multi.push_back(qstr_2.toStdString());
            }
    cv::String text="Multi-Similarity:"+finallist_multi[i];

    putText(result,text,Point(30,60),CV_FONT_HERSHEY_COMPLEX,0.7,Scalar(255,255,255),1,1);


    imshow("Multi-Features Fusion", result);

    waitKey(0);
    return 0;

//    ui->label->clear();     //清空缓存区，否则会导致第二次加载同一张图片不会显示，详细百度有说
//    ui->label->setPixmap(image);   //3.方便起见，直接采用label显示
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event){

    if (obj == ui->label_show_1)//指定某个QLabel
         {
             if (event->type() == QEvent::MouseButtonPress) //鼠标点击
             {
                 if(flag_edge_only && flag_color_only){

                  displayImage(color_list(0),edge_list(0),0);

                 }else{
                 if(flag_edge_only){
                 QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                 if(mouseEvent->button() == Qt::LeftButton)
                 {

                    imshow("Edge Matches", edge_list(0));

   // 添加代码
                            // 添加代码
                     return true;
                 }

             }
                  if(flag_color_only){
                     QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                     if(mouseEvent->button() == Qt::LeftButton)
                     {

                        imshow("HSV",color_list(0));

       // 添加代码
                                // 添加代码
                         return true;
                     }

                 }
                  else
                  {
                      return false;
                  }

             }
             }
             else
             {
                 return false;
             }
         }

         else if (obj == ui->label_show_2)//指定某个QLabel
    {
        if (event->type() == QEvent::MouseButtonPress) //鼠标点击
        {
            if(flag_edge_only && flag_color_only){

             displayImage(color_list(1),edge_list(1),1);

            }else{
            if(flag_edge_only){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

            if(mouseEvent->button() == Qt::LeftButton)
            {

               imshow("Edge Matches", edge_list(1));

// 添加代码
                       // 添加代码
                return true;
            }

        }
             if(flag_color_only){
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                if(mouseEvent->button() == Qt::LeftButton)
                {

                   imshow("HSV",color_list(1));

  // 添加代码
                           // 添加代码
                    return true;
                }

            }
             else
             {
                 return false;
             }

        }
        }
        else
        {
            return false;
        }
    }
    else if (obj == ui->label_show_3)//指定某个QLabel
    {
        if (event->type() == QEvent::MouseButtonPress) //鼠标点击
        {
            if(flag_edge_only && flag_color_only){

             displayImage(color_list(2),edge_list(2),2);

            }else{
            if(flag_edge_only){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

            if(mouseEvent->button() == Qt::LeftButton)
            {

               imshow("Edge Matches", edge_list(2));

// 添加代码
                       // 添加代码
                return true;
            }

        }
             if(flag_color_only){
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                if(mouseEvent->button() == Qt::LeftButton)
                {

                   imshow("HSV",color_list(2));

  // 添加代码
                           // 添加代码
                    return true;
                }

            }
             else
             {
                 return false;
             }

        }
        }
        else
        {
            return false;
        }
    }
    else if (obj == ui->label_show_4)//指定某个QLabel
    {
        if (event->type() == QEvent::MouseButtonPress) //鼠标点击
        {
            if(flag_edge_only && flag_color_only){

             displayImage(color_list(3),edge_list(3),3);

            }else{
            if(flag_edge_only){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

            if(mouseEvent->button() == Qt::LeftButton)
            {

               imshow("Edge Matches", edge_list(3));

// 添加代码
                       // 添加代码
                return true;
            }

        }
             if(flag_color_only){
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                if(mouseEvent->button() == Qt::LeftButton)
                {

                   imshow("HSV",color_list(3));

  // 添加代码
                           // 添加代码
                    return true;
                }

            }
             else
             {
                 return false;
             }

        }
        }
        else
        {
            return false;
        }
    }
    else if (obj == ui->label_show_5)//指定某个QLabel
    {
        if (event->type() == QEvent::MouseButtonPress) //鼠标点击
        {
            if(flag_edge_only && flag_color_only){

             displayImage(color_list(4),edge_list(4),4);

            }else{
            if(flag_edge_only){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

            if(mouseEvent->button() == Qt::LeftButton)
            {

               imshow("Edge Matches", edge_list(4));

// 添加代码
                       // 添加代码
                return true;
            }

        }
             if(flag_color_only){
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                if(mouseEvent->button() == Qt::LeftButton)
                {

                   imshow("HSV",color_list(4));

  // 添加代码
                           // 添加代码
                    return true;
                }

            }
             else
             {
                 return false;
             }

        }
        }
        else
        {
            return false;
        }
    }
    else if (obj == ui->label_show_6)//指定某个QLabel
    {
        if (event->type() == QEvent::MouseButtonPress) //鼠标点击
        {
            if(flag_edge_only && flag_color_only){

             displayImage(color_list(5),edge_list(5),5);

            }else{
            if(flag_edge_only){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

            if(mouseEvent->button() == Qt::LeftButton)
            {

               imshow("Edge Matches", edge_list(5));

// 添加代码
                       // 添加代码
                return true;
            }

        }
             if(flag_color_only){
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                if(mouseEvent->button() == Qt::LeftButton)
                {

                   imshow("HSV",color_list(5));

  // 添加代码
                           // 添加代码
                    return true;
                }

            }
             else
             {
                 return false;
             }

        }
        }
        else
        {
            return false;
        }
    }
    else if (obj == ui->label_show_7)//指定某个QLabel
    {
        if (event->type() == QEvent::MouseButtonPress) //鼠标点击
        {
            if(flag_edge_only && flag_color_only){

             displayImage(color_list(6),edge_list(6),6);

            }else{
            if(flag_edge_only){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

            if(mouseEvent->button() == Qt::LeftButton)
            {

               imshow("Edge Matches", edge_list(6));

// 添加代码
                       // 添加代码
                return true;
            }

        }
             if(flag_color_only){
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                if(mouseEvent->button() == Qt::LeftButton)
                {

                   imshow("HSV",color_list(6));

  // 添加代码
                           // 添加代码
                    return true;
                }

            }
             else
             {
                 return false;
             }

        }
        }
        else
        {
            return false;
        }
    }

    else if (obj == ui->label_show_8)//指定某个QLabel
    {
        if (event->type() == QEvent::MouseButtonPress) //鼠标点击
        {
            if(flag_edge_only && flag_color_only){

             displayImage(color_list(7),edge_list(7),7);

            }else{
            if(flag_edge_only){
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

            if(mouseEvent->button() == Qt::LeftButton)
            {

               imshow("Edge Matches", edge_list(7));

// 添加代码
                       // 添加代码
                return true;
            }

        }
             if(flag_color_only){
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event); // 事件转换

                if(mouseEvent->button() == Qt::LeftButton)
                {

                   imshow("HSV",color_list(7));

  // 添加代码
                           // 添加代码
                    return true;
                }

            }
             else
             {
                 return false;
             }

        }
        }
        else
        {
            return false;
        }
    }


         else{
             // pass the event on to the parent class
             return QWidget::eventFilter(obj, event);
         }
}

//truncate table image_address;
