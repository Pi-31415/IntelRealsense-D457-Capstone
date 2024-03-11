#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
try
{
    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    // Start streaming with default recommended configuration
    pipe.start();

    // Capture 30 frames to give auto-exposure, etc. a chance to settle
    for (auto i = 0; i < 30; ++i)
        pipe.wait_for_frames();

    // Create a window
    cv::namedWindow("Depth and Color Frames", cv::WINDOW_AUTOSIZE);

    while (true)
    {
        // Block program until frames arrive
        rs2::frameset frames = pipe.wait_for_frames();

        // Get a frame of a depth image and apply color map for better visualization
        rs2::frame depth = frames.get_depth_frame().apply_filter(color_map);
        // Get a frame of a color image
        rs2::frame color = frames.get_color_frame();

        // Convert depth and color frames to OpenCV matrices
        cv::Mat depth_image(cv::Size(depth.as<rs2::video_frame>().get_width(), depth.as<rs2::video_frame>().get_height()), CV_8UC3, (void *)depth.get_data(), cv::Mat::AUTO_STEP);
        cv::Mat color_image(cv::Size(color.as<rs2::video_frame>().get_width(), color.as<rs2::video_frame>().get_height()), CV_8UC3, (void *)color.get_data(), cv::Mat::AUTO_STEP);

        // Convert color image from RGB to BGR
        cv::cvtColor(color_image, color_image, cv::COLOR_RGB2BGR);

        // Resize depth_image to match color_image size
        cv::resize(depth_image, depth_image, color_image.size());

        // Concatenate depth and color images horizontally
        cv::Mat concatenated_image;
        cv::hconcat(color_image, depth_image, concatenated_image);

        // Display the concatenated image
        cv::imshow("Depth and Color Frames", concatenated_image);

        // Check for 's' key press to save the image
        char key = (char)cv::waitKey(1);
        if (key == 's' || key == 'S')
        {
            // Save the concatenated image to the desktop
            std::string filename = "/home/pi/Desktop/captured_image.png";
            cv::imwrite(filename, concatenated_image);
            std::cout << "Saved " << filename << std::endl;
        }
        else if (key >= 0)
        {
            break;
        }
    }

    return EXIT_SUCCESS;
}
catch (const rs2::error &e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << '(' << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception &e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
