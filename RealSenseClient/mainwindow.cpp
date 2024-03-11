#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    imageLabel = new QLabel(this);
    saveButton = new QPushButton("Save Screenshot", this);

    // Set up RealSense pipeline
    pipe.start();

    // Set up the timer for updating the frame
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(30); // Update every 30 ms

    // Connect the save button to the screenshot function
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveScreenshot);

    // Layout
    ui->verticalLayout->addWidget(imageLabel);
    ui->verticalLayout->addWidget(saveButton);
}

MainWindow::~MainWindow()
{
    pipe.stop();
    delete ui;
}

void MainWindow::updateFrame()
{
    rs2::frameset frames = pipe.wait_for_frames();
    rs2::frame depth = frames.get_depth_frame().apply_filter(rs2::colorizer());
    rs2::frame color = frames.get_color_frame();

    cv::Mat depthImage(cv::Size(depth.as<rs2::video_frame>().get_width(), depth.as<rs2::video_frame>().get_height()), CV_8UC3, (void*)depth.get_data(), cv::Mat::AUTO_STEP);
    cv::Mat colorImage(cv::Size(color.as<rs2::video_frame>().get_width(), color.as<rs2::video_frame>().get_height()), CV_8UC3, (void*)color.get_data(), cv::Mat::AUTO_STEP);

    cv::cvtColor(colorImage, colorImage, cv::COLOR_RGB2BGR);
    cv::resize(depthImage, depthImage, colorImage.size());
    cv::hconcat(colorImage, depthImage, concatenatedImage);

    QImage qimg(concatenatedImage.data, concatenatedImage.cols, concatenatedImage.rows, concatenatedImage.step, QImage::Format_RGB888);
    imageLabel->setPixmap(QPixmap::fromImage(qimg));
}

void MainWindow::saveScreenshot()
{
    QString filename = QDateTime::currentDateTime().toString("yyyyMMddHHmmss") + ".png";
    QString filepath = QDir::homePath() + "/Desktop/" + filename;
    cv::imwrite(filepath.toStdString(), concatenatedImage);
}
