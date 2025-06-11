#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    // Default path to calibration file, assuming it's in the current working directory
    string calibFile = "calibration.yml";

    // Allow user to override via command-line argument
    if (argc > 1)
    {
        calibFile = argv[1];
    }

    // Optional: print current working directory (debugging)
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
        cout << "Current working directory: " << cwd << endl;

    cout << "Using calibration file: " << calibFile << endl;

    FileStorage fs(calibFile, FileStorage::READ);
    if (!fs.isOpened())
    {
        cerr << "Failed to open calibration file: " << calibFile << endl;
        return -1;
    }

    Mat cameraMatrix, distCoeffs;
    fs["CameraMatrix"] >> cameraMatrix;
    fs["DistCoeffs"] >> distCoeffs;
    fs.release();

    // GStreamer pipeline for Jetson CSI camera at 15 FPS
    string pipeline =
        "nvarguscamerasrc sensor-id=0 ! "
        "video/x-raw(memory:NVMM), width=640, height=480, format=NV12, "
        "framerate=15/1 ! "
        "nvvidconv ! video/x-raw, format=BGRx ! "
        "videoconvert ! video/x-raw, format=BGR ! "
        "appsink";

    VideoCapture cap(pipeline, CAP_GSTREAMER);
    if (!cap.isOpened())
    {
        cerr << "Error: Could not open the CSI camera." << endl;
        return -1;
    }

    Mat frame, undistorted;
    cap >> frame;
    if (frame.empty())
    {
        cerr << "Error: Could not capture initial frame." << endl;
        return -1;
    }

    int width = frame.cols;
    int height = frame.rows;

    // Setup video writer to save as MP4 using H264 codec
    VideoWriter writer("video.mp4", VideoWriter::fourcc('a','v','c','1'), 15, Size(width, height));
    if (!writer.isOpened())
    {
        cerr << "Error: Could not open video writer." << endl;
        return -1;
    }

    namedWindow("Undistorted Feed", WINDOW_AUTOSIZE);

    cout << "Recording... Press 'q' to stop and save video." << endl;

    while (true)
    {
        if (!cap.read(frame))
        {
            cerr << "Error: Unable to capture frame." << endl;
            break;
        }

        undistort(frame, undistorted, cameraMatrix, distCoeffs);
        writer.write(undistorted);
        imshow("Undistorted Feed", undistorted);

        char key = (char)waitKey(1);
        if (key == 'q' || key == 27) // 'q' or ESC to quit
        {
            break;
        }
    }

    cap.release();
    writer.release();
    destroyAllWindows();

    cout << "Video saved as 'video.mp4'" << endl;
    return 0;
}
