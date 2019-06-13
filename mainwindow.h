#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QtCore>
#include <QTimer>
#include <QCoreApplication>
#include <QMediaRecorder>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void processFrameAndUpdateGUI();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnPauseOrResume_clicked();                                             // pause button
    void on_Calibration_clicked();                                                  // calibration button
    void on_Record_clicked();                                                       // data recording button
    void on_RVideo_clicked();                                                       // video recording button
    void on_Alert_clicked();                                                        // Alert button
    void track_targets(float &x, float &y, cv::Mat &thresh, bool &status);
    void morph_ft(cv::Mat &thresh, int num_iter);

private:
    Ui::MainWindow *ui;                                                             // pointer to all the widgets

    // These are the basic varible requirement for the initialization
    // to read the camera output
    cv::VideoCapture capWebcam;                                                     // Capture object to use with webcam
    cv::VideoWriter video;
    cv::Mat matOriginal, frame;                                                     // Original Image
    cv::Mat matProcessed;                                                           // HSV image

    // The mask variable stores the threshold images, where the
    // combination of mask1 and mask2 (mask1 + maks2) stores the
    // hue color processing for red and the combination of the both
    // is stored in mask
    // whereas maske 3 is used for the blue color
    cv::Mat1b mask1, mask2, mask, mask3;

    // timer for processFrameAndUpdateGUI()
    QTimer* qtimer;

    // Counters used for updating the file name stored with each press
    // of a record button
    int fileCount = 0, vidCount = 0;
    //QFile file;

    // These variables stores the current X and Y pixel position
    // of the two handles (X1, Y1, X2, Y2), and the calibration
    // pixel coordinates used in the initialization
    float X1, Y1, X2, Y2,
        calibP1X1, calibP1Y1, calibP1X2, calibP1Y2,
        calibP2X1, calibP2Y1, calibP2X2, calibP2Y2;

    // These status variables stores the current status of each button
    // to behave like a on/off switch
    bool calib= true,
        statusRec=true,
        statusPause=false,
        statusRVideo=true,
        statusAlert=true,
        statusRed = false,
        statusBlue = false;

    // file variable for storing the data in the csv format
    FILE *file;

    // Function for the QImage, used for the conversion of cv::Mat
    // format to QImage (used in mainwindow.cpp file)
    QImage matToQImage(cv::Mat mat);
    void exitProgram();
};

#endif
