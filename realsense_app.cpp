#include <librealsense2/rs.hpp>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/point_cloud.h>
#include <pcl/visualization/cloud_viewer.h>

int main(int argc, char * argv[]) try
{
    // Create a simple OpenGL window for rendering:
    pcl::visualization::PCLVisualizer::Ptr viewer(new pcl::visualization::PCLVisualizer("RealSense PointCloud"));

    // Declare pointcloud object, for calculating pointclouds and texture mappings
    rs2::pointcloud pc;
    // We want the points object to be persistent so we can display the last cloud when a frame drops
    rs2::points points;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    // Start streaming with default recommended configuration
    pipe.start();

    while (!viewer->wasStopped())
    {
        // Wait for the next set of frames from the camera
        auto frames = pipe.wait_for_frames();

        auto depth = frames.get_depth_frame();

        // Generate the pointcloud and texture mappings
        points = pc.calculate(depth);

        // Create a PointCloud
        pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);

        // Get the color frame
        auto color = frames.get_color_frame();

        // Tell pointcloud object to map to this color frame
        pc.map_to(color);

        // Convert points to PCL format
        const rs2::vertex * vertices = points.get_vertices();
        const rs2::texture_coordinate * texture_coordinates = points.get_texture_coordinates();

        for (int i = 0; i < points.size(); i++)
        {
            // Ignore points that are far away
            if (vertices[i].z > 0)
            {
                pcl::PointXYZRGB p;
                p.x = vertices[i].x;
                p.y = vertices[i].y;
                p.z = vertices[i].z;

                // Color data from texture
                auto u = texture_coordinates[i].u;
                auto v = texture_coordinates[i].v;

                // Retrieve color from the color frame
                auto color_pixel = reinterpret_cast<const uint8_t*>(color.get_data()) + (int)v*color.get_stride_in_bytes() + (int)u*color.get_bits_per_pixel()/8;
                p.r = color_pixel[2];
                p.g = color_pixel[1];
                p.b = color_pixel[0];

                cloud->push_back(p);
            }
        }

        // Update the visualization
        viewer->removeAllPointClouds();
        viewer->addPointCloud<pcl::PointXYZRGB>(cloud);

        viewer->spinOnce();
    }

    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << '(' << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
