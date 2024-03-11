# IntelRealsense-D457-Capstone
 Custom C++ app to perform point cloud operations using D457 camera for my capstone project



 ### Project Setup

 ```
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys F6B0FC61
sudo add-apt-repository "deb http://realsense-hw-public.s3.amazonaws.com/Debian/apt-repo xenial main" -u
sudo apt-get update
sudo apt-get install librealsense2-dkms
sudo apt-get install librealsense2-utils
sudo apt-get install librealsense2-dev
sudo apt-get install librealsense2-dbg


sudo apt-get install libopencv-dev

sudo apt install -y qtcreator qtbase5-dev qt5-qmake cmake

 ```

 For installing point cloud library

 sudo apt-get install libusb-1.0-0-dev
 https://alwynm.github.io/blog/general/pcl



 sudo apt-get install libglfw3-dev libglew-dev

 sudo apt-get install libglfw3-dev
sudo apt-get install libglu1-mesa-dev


 ### Build and Run

 ```
 g++ -std=c++11 -o colourdepth colourdepth.cpp -lrealsense2 `pkg-config --cflags --libs opencv4`
```
