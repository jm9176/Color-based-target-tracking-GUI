#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent), ui(new Ui::MainWindow){

    // Initialization parameters for the camera, camera output, and timer
    ui->setupUi(this);
    capWebcam.open(0);
    capWebcam.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    capWebcam.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    qtimer = new QTimer(this);
    connect(qtimer, SIGNAL(timeout()), this, SLOT(processFrameAndUpdateGUI()));
    qtimer->start(20);
    VideoWriter video;

}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::exitProgram() {

    if(qtimer->isActive()) qtimer->stop();
    QApplication::quit();

}

void MainWindow::morph_ft(Mat &thresh, int num_iter){

    // Creating the structuring elements for the erosion and dilation
    // process of the morphological operations.
    Mat erosion_st_elem = getStructuringElement(MORPH_RECT, Size(17, 17));
    Mat dilate_st_elem = getStructuringElement(MORPH_RECT, Size(27,27));

    for (int i = 0; i < num_iter; i++){
        erode(thresh, thresh, erosion_st_elem);
    }

    for (int i = 0; i < num_iter; i++){
        dilate(thresh, thresh, dilate_st_elem);
    }
}

void MainWindow::track_targets(float &x, float &y, Mat &thresh, bool &status){

    // Defining the min area value for the tracking. Calculating Image moments 
    // to get the centroid and the area of the white pixelated region in
    // the threshold image
    int min_objArea = 400;
    Moments moment1 = moments(thresh);
    double area1 = moment1.m00;
    status = false;

    if (area1>min_objArea){
        x = (moment1.m10 / area1);
        y = (moment1.m01 / area1);
        status = true;
    
    } else {
        x = 0;
        y = 0;
        status = false;
    }
}

void MainWindow::processFrameAndUpdateGUI() {

    // Setting the date and time for the GUI window
    ui->DateTime->setText(QTime::currentTime().toString("hh:mm:ss"));
    ui->lblDate->setText(QDate::currentDate().toString("MM/dd/yy"));

    // Read the camera frame to matOriginal, frame stores the original camera frame
    // without any processing to be used later for recording. This is done to record
    // for any external interference with the process to be used in case of a failure
    capWebcam.read(matOriginal);
    matOriginal.copyTo(frame);
    cvtColor(matOriginal, matProcessed, CV_BGR2HSV);

    // Creating the cross-marker for the calibration position 1 for color red and blue
    line(matOriginal, Point(calibP1X1, calibP1Y1-10), Point(calibP1X1, calibP1Y1+10), Scalar(0,0,255), 2, 8, 0);
    line(matOriginal, Point(calibP1X1-10, calibP1Y1), Point(calibP1X1+10, calibP1Y1), Scalar(0,0,255), 2, 8, 0);

    line(matOriginal, Point(calibP1X2, calibP1Y2-10), Point(calibP1X2, calibP1Y2+10), Scalar(255,0,0), 2, 8, 0);
    line(matOriginal, Point(calibP1X2-10, calibP1Y2), Point(calibP1X2+10, calibP1Y2), Scalar(255,0,0), 2, 8, 0);

    // Creating the circular marker for the calibration position 2 for color red and blue
    circle(matOriginal, Point(calibP2X1, calibP2Y1), 5, Scalar(0,0,255), 10, 8, 0);
    circle(matOriginal, Point(calibP2X2, calibP2Y2), 5, Scalar(255,0,0), 10, 8, 0);

    // Setting the parameter for the detection of Red color
    // Applying the mask for the red color range in the Hue channel)
    inRange(matProcessed, Scalar(0,70,70), Scalar(10,255,255), mask1);
    inRange(matProcessed, Scalar(170,70,70), Scalar(180,255,255), mask2);

    // Summation of both the masks gives the mask for two different color filter values
    mask = mask1 | mask2;

    // Applying the morphological filter to eliminate the pixelated noise
    morph_ft(mask, 2);

    // Initializing the status of the label in the gui window. If the detection
    // is true then changing the color and status of the label
    ui->RedStatus->setStyleSheet("color: white; background: red");
    ui->RedStatus->setText("No red");
    track_targets(X1, Y1, mask, statusRed);

    if (statusRed == true){

        // Changing the color status of the GUI window for the red color
        ui->RedStatus->setStyleSheet("color: white; background: green");
        ui->RedStatus->setText("Tracking red");

        // Creating the cross-marker to show the tracking on the final processed camera
        // RGB output window
        line(matOriginal, Point(X1,Y1-10), Point(X1,Y1+10), Scalar(0,255,0), 2, 8, 0);
        line(matOriginal, Point(X1-10,Y1), Point(X1+10,Y1), Scalar(0,255,0), 2, 8, 0);

    }

    // Updating the pixel coordinate for red (X1, Y1) on the GUI window
    ui->RedX->setText(QString::number(X1));
    ui->RedY->setText(QString::number(Y1));

    // Setting the parameter for the detection of blue color
    inRange(matProcessed, Scalar(90,70,70), Scalar(120,255,255), mask3);
    morph_ft(mask3, 2);
    ui->BlueStatus->setStyleSheet("color: white; background: red");
    ui->BlueStatus->setText("No blue");
    track_targets(X2, Y2, mask3, statusBlue);

    if(statusBlue == true){

        ui->BlueStatus->setStyleSheet("color: white; background: green");
        ui->BlueStatus->setText("Tracking blue");
        line(matOriginal, Point(X2,Y2-10), Point(X2,Y2+10), Scalar(0,255,0), 2, 8, 0);
        line(matOriginal, Point(X2-10,Y2), Point(X2+10,Y2), Scalar(0,255,0), 2, 8, 0);

    }

    // Updating the pixel coordinate for red (X1, Y1) on the GUI window
    ui->BlueX->setText(QString::number(X2));
    ui->BlueY->setText(QString::number(Y2));

    // Storing the data, date and time in the .csv file
    QString date = QDate::currentDate().toString("MM/dd/yy");
    string date1 = date.toStdString();
    QString time = QTime::currentTime().toString("hh:mm:ss");
    string time1 = time.toStdString();

    // writing the data to the file if the record data button is ON
    if(statusRec==false){
        fprintf(file,"%s, %s, %f, %f, %f, %f, %d\n",date1.c_str(),time1.c_str(), X1, Y1,X2, Y2);
    }

    // Storing the image frames to a .avi file if teh recod video button is ON
    if(statusRVideo == false){
        video.write(frame);
    }

    // Converting the mat format to QImage and displaying the processed frame
    // output on the image window of the GUI
    QImage qimgOriginal = matToQImage(matOriginal);
    ui->lblOriginal->setPixmap(QPixmap::fromImage(qimgOriginal));

}

// Function to convert the cv::Mat to QImage
QImage MainWindow::matToQImage(Mat mat) {

    // Returning the images in-case of a 1-channel or 3-channel RGB
    if(mat.channels() == 1) {
        return QImage((uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);

    } else if(mat.channels() == 3) {

        cvtColor(mat, mat, CV_BGR2RGB);
        return QImage((uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);

    } else {
        qDebug() << "Image Error";
    }

    return QImage();
}

// Function for the Pause or Resume button
// It actvates on a click
void MainWindow::on_btnPauseOrResume_clicked()
{
    // Applying the condition on the timer (to pause it) in
    // case pause or resume is pressed
    if(qtimer->isActive() == true) {

        qtimer->stop();
        ui->btnPauseOrResume->setStyleSheet("color: black");
        ui->btnPauseOrResume->setText("RESUME");
        ui->Record->setStyleSheet("color: black");
        ui->Record->setText("NA");

    } else {

        qtimer->start(10);
        ui->btnPauseOrResume->setStyleSheet("color: black");
        ui->btnPauseOrResume->setText("PAUSE");

        // Applying the conditions for the Record button. To pause the
        // recording if the pause is pressed and to resume in-case
        // resume button is pressed
        if(statusRec == false && statusPause == true){

            ui->Record->setStyleSheet("color: white; background: red");
            ui->Record->setText("RECORDING");

        } else if(statusRec == true && statusPause == false){

            ui->Record->setStyleSheet("color: black");
            ui->Record->setText("RECORD");

        } else {

            ui->Record->setStyleSheet("color: black");
            ui->Record->setText("RECORD");

        }
    }
}

// To save the calibrated coordinates of both the handles
// This calibration is the extreme end positioning of both the handles
void MainWindow::on_Calibration_clicked()
{
    if (calib == true){

        // Storing the coordinates for the top extreme position
        // as denoted by calibration position 1
        ui->pos1X1->setStyleSheet("color:black");
        ui->pos1X1->setText(QString::number(X1));

        ui->pos1Y1->setStyleSheet("color:black");
        ui->pos1Y1->setText(QString::number(Y1));

        ui->pos1X2->setStyleSheet("color:black");
        ui->pos1X2->setText(QString::number(X2));

        ui->pos1Y2->setStyleSheet("color:black");
        ui->pos1Y2->setText(QString::number(Y2));

        // Updating the calibration position coordinates
        calibP1X1 = X1;
        calibP1Y1 = Y1;
        calibP1X2 = X2;
        calibP1Y2 = Y2;

        // Updating the calibration status for position 1
        ui->Pos1Status->setStyleSheet("color: white; background: green");
        ui->Pos1Status->setText("Pos1 Calibrated");
        calib = false;

    } else {

        // Storing the coordinates for the bottom extreme position
        // as denoted by calibration position 2
        ui->pos2X1->setStyleSheet("color:black");
        ui->pos2X1->setText(QString::number(X1));

        ui->pos2Y1->setStyleSheet("color:black");
        ui->pos2Y1->setText(QString::number(Y1));

        ui->pos2X2->setStyleSheet("color:black");
        ui->pos2X2->setText(QString::number(X2));

        ui->pos2Y2->setStyleSheet("color:black");
        ui->pos2Y2->setText(QString::number(Y2));

        // Updating the calibration position coordinates
        calibP2X1 = X1;
        calibP2Y1 = Y1;
        calibP2X2 = X2;
        calibP2Y2 = Y2;

        // Updating the calibration status for the position 2
        ui->Pos2Status->setStyleSheet("color: white; background: green");
        ui->Pos2Status->setText("Pos2 Calibrated");
        calib = true;
    }
}

// Recoding the pixel coordinate data over time for both the markers (red and blue)
void MainWindow::on_Record_clicked()
{
    if(qtimer->isActive() == true && statusRec == true) {
        ui->Record->setStyleSheet("color: white; background: red");
        ui->Record->setText("RECORDING");
        statusRec = false;
        statusPause = true;
        string data1 = "C:/Users/shubh/Desktop/UI/Data/data";
        data1 += to_string(fileCount)+".csv";
        file = fopen(data1.c_str(),"w") ;
        fileCount++;
    } else {
        ui->Record->setStyleSheet("color: black");
        ui->Record->setText("REC DATA");
        statusRec = true;
        statusPause = false;
        fclose(file);
    }
}

// Recording the original (unprocessed) video ouput of the camera
void MainWindow::on_RVideo_clicked()
{
    if(statusRVideo == true)
    {
        statusRVideo = false;
        string data2 = "C:/Users/shubh/Desktop/UI/Videos/vid";
        data2 += to_string(vidCount) + ".avi";
        video.open(data2, CV_FOURCC('P','I','M','1'), 30, Size(640,480), true);
        ui->RVideo->setStyleSheet("color: white; background: red");
        ui->RVideo->setText("RECORDING");
        vidCount++;
    } else {
        statusRVideo = true;
        video.release();
        ui->RVideo->setStyleSheet("color: black");
        ui->RVideo->setText("REC VIDEO");
    }
}

// Generates an on-screen alert if there is an automatic shift for any of the marker
// from its calibrated position
// **The alert function has not been configured but the output can be seen in the video
void MainWindow::on_Alert_clicked()
{
    if(statusAlert == true)
    {
        statusAlert = false;

        // Changing the status of the Alert button on click
        ui->Alert->setStyleSheet("color: white; background: red");
        ui->Alert->setText("ALERT ON");

    } else {

        statusAlert = true;

        // Turning the Alert OFF if the button is pressed again
        ui->Alert->setStyleSheet("color: black");
        ui->Alert->setText("ALERT OFF");
        ui->H1Status->setStyleSheet("color: black");
        ui->H2Status->setStyleSheet("color: black");
    }
}
