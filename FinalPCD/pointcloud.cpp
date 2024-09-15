// g++ -std=c++11 -o pointcloud pointcloud.cpp -lrealsense2 `pkg-config --cflags --libs opencv4` -lGL -lGLU -lglfw
// View :
// pcl_viewer -multiview 1 captured_point_cloud.pcd
#include <fstream>              // Add this to handle file writing
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering
#include <algorithm>            // For std::min, std::max
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string.h>

// Struct for managing rotation of pointcloud view
// Modify the `state` struct
struct state
{
    double yaw, pitch, last_x, last_y;
    bool ml;
    float offset_x, offset_y;
    texture tex;
    bool save_points; // New member to trigger saving
};

// Helper functions
void save_to_pcd(const std::string &filename, rs2::points &points, const rs2::video_frame &color);
void register_glfw_callbacks(window &app, state &app_state);
void draw_pointcloud(window &app, state &app_state, rs2::points &points);
int main(int argc, char *argv[])
try
{
    // Create a simple OpenGL window for rendering:
    window app(1280, 720, "Capstone RoV Point Cloud Capture");
    // Construct an object to manage view state
    // Initialize state with appropriate constructors
    state app_state;
    app_state.yaw = 0;
    app_state.pitch = 0;
    app_state.last_x = 0;
    app_state.last_y = 0;
    app_state.ml = false;
    app_state.offset_x = 0;
    app_state.offset_y = 0;
    // app_state.tex is initialized with its default constructor
    app_state.save_points = false;
    // register callbacks to allow manipulation of the pointcloud
    register_glfw_callbacks(app, app_state);
    glfwFocusWindow(app); // Focus on the current window
    using namespace rs2;
    // Declare pointcloud object, for calculating pointclouds and texture mappings
    pointcloud pc;
    // We want the points object to be persistent so we can display the last cloud when a frame drops
    points points;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    pipeline pipe;
    // Start streaming with default recommended configuration
    pipe.start();

    while (app)
    {
        glfwPollEvents(); // This should be present in your main loop
        // Wait for the next set of frames from the camera
        auto frames = pipe.wait_for_frames();

        auto depth = frames.get_depth_frame();
        // Generate the pointcloud and texture mappings
        points = pc.calculate(depth);

        auto color = frames.get_color_frame();
        // Tell pointcloud object to map to this color frame
        pc.map_to(color);

        // Upload the color frame to OpenGL
        app_state.tex.upload(color);

        draw_pointcloud(app, app_state, points);
        if (app_state.save_points)
        {
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d_%H-%M-%S");
            std::string timestamp = ss.str();

            std::string desktop_path = getenv("HOME");
            desktop_path += "/Desktop/captured_point_cloud_" + timestamp + ".pcd";
            save_to_pcd(desktop_path, points, color);
            app_state.save_points = false;
        }
    }

    return EXIT_SUCCESS;
}
catch (const rs2::error &e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception &e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

void save_to_pcd(const std::string &filename, rs2::points &points, const rs2::video_frame &color)
{
    std::ofstream out_file(filename);
    if (out_file)
    {
        // Count valid points
        size_t valid_points = 0;
        auto vertices = points.get_vertices();
        for (size_t i = 0; i < points.size(); ++i)
        {
            if (vertices[i].z)
            {
                valid_points++;
            }
        }

        // PCD Header
        out_file << "# .PCD v.7 - Point Cloud Data file format\n";
        out_file << "VERSION .7\n";
        out_file << "FIELDS x y z rgb\n";
        out_file << "SIZE 4 4 4 4\n";
        out_file << "TYPE F F F F\n";
        out_file << "COUNT 1 1 1 1\n";
        out_file << "WIDTH " << valid_points << "\n";
        out_file << "HEIGHT 1\n";
        out_file << "VIEWPOINT 0 0 0 1 0 0 0\n";
        out_file << "POINTS " << valid_points << "\n";
        out_file << "DATA ascii\n";

        // Get texture coordinates and write valid points
        auto tex_coords = points.get_texture_coordinates();
        const auto color_data = reinterpret_cast<const uint8_t *>(color.get_data());

        for (size_t i = 0; i < points.size(); ++i)
        {
            if (vertices[i].z)
            {
                int u = std::min(std::max(int(tex_coords[i].u * color.get_width() + .5f), 0), color.get_width() - 1);
                int v = std::min(std::max(int(tex_coords[i].v * color.get_height() + .5f), 0), color.get_height() - 1);
                int color_idx = (v * color.get_width() + u) * 3;

       // Assuming the color data is already in BGR format
        uint32_t bgr = ((uint32_t)color_data[color_idx + 2] << 16) |
                       ((uint32_t)color_data[color_idx + 1] << 8) |
                       (uint32_t)color_data[color_idx];

        // Write to PCD
        float rgb_float;
        memcpy(&rgb_float, &bgr, sizeof(uint32_t));
        out_file << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << " " << rgb_float << "\n";
            }
        }
    }
}

void register_glfw_callbacks(window &app, state &app_state)
{
    // ...
    // Print out callback is called with cout
    std::cout << "Press 's' key to save the point cloud" << std::endl;
    app.on_key_release = [&](int key)
    {
        if (key == GLFW_KEY_S) // Handle 's' key for saving point cloud
        {
            // Print out the message
            std::cout << "Saving point cloud..." << std::endl;
            app_state.save_points = true;
        }
        // Existing code...
    };
}

void draw_pointcloud(window &app, state &app_state, rs2::points &points)
{
    // Setup OpenGL viewpoint
    glPopMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Set perspective projection
    float width = app.width(), height = app.height();
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, width / height, 0.01f, 10.0f);

    // Set camera view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 0, 0, 0, 1, 0, -1, 0);

    // Rotate view
    glTranslatef(app_state.offset_x, app_state.offset_y, 0);
    glRotatef(app_state.pitch, 1, 0, 0);
    glRotatef(app_state.yaw, 0, 1, 0);
    glTranslatef(0, 0, -0.5f);

    // Draw pointcloud
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, app_state.tex.get_gl_handle());
    float tex_border_color[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, tex_border_color);
    glBegin(GL_POINTS);

    // Iterate over each point in the pointcloud
    auto vertices = points.get_vertices();              // get vertices
    auto tex_coords = points.get_texture_coordinates(); // and texture coordinates
    for (int i = 0; i < points.size(); ++i)
    {
        if (vertices[i].z)
        {
            // Upload the point and texture coordinates only for points we have depth data for
            glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
            glTexCoord2f(tex_coords[i].u, tex_coords[i].v);
        }
    }

    glEnd();
    glPopMatrix();
    glPopAttrib();
}