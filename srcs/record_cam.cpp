#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main()
{
    FileStorage fs("calibration.yml", FileStorage::READ);
    if (!fs.isOpened())
    {
        cerr << "Failed to open calibration.yml" << endl;
        return -1;
    }
    Mat cameraMatrix, distCoeffs;
    fs["CameraMatrix"] >> cameraMatrix;
    fs["DistCoeffs"] >> distCoeffs;
    fs.release();

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

    // Setup video writer
    VideoWriter writer("video.avi", VideoWriter::fourcc('X','V','I','D'), 15, Size(width, height));
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
        if (key == 'q' || key == 27) // 'q' or ESC
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    cap.release();
    writer.release();
    destroyAllWindows();

    cout << "Video saved as 'video.avi'" << endl;
    return 0;
}
