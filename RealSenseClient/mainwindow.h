#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void saveScreenshot();

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QPushButton *saveButton;
    QTimer *timer;
    cv::Mat concatenatedImage;
    rs2::pipeline pipe;
};
#endif // MAINWINDOW_H
