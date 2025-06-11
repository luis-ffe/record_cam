import cv2
import numpy as np

# Load calibration parameters from YAML
fs = cv2.FileStorage("calibration.yml", cv2.FILE_STORAGE_READ)
camera_matrix = fs.getNode("CameraMatrix").mat()
dist_coeffs = fs.getNode("DistCoeffs").mat()
fs.release()

# GStreamer pipeline for Jetson Nano CSI camera (640x480, 30fps)
gst_pipeline = (
    "nvarguscamerasrc sensor-id=0 ! "
    "video/x-raw(memory:NVMM), width=640, height=480, format=NV12, framerate=30/1 ! "
    "nvvidconv ! video/x-raw, format=BGRx ! "
    "videoconvert ! video/x-raw, format=BGR ! "
    "appsink"
)

cap = cv2.VideoCapture(gst_pipeline, cv2.CAP_GSTREAMER)
if not cap.isOpened():
    print("Error: Unable to open camera with GStreamer pipeline.")
    exit()

# Read first frame to get frame size
ret, frame = cap.read()
if not ret:
    print("Error: Unable to read frame from camera.")
    cap.release()
    exit()

height, width = frame.shape[:2]

# Compute undistortion map for remapping
new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(
    camera_matrix, dist_coeffs, (width, height), 1, (width, height)
)
map1, map2 = cv2.initUndistortRectifyMap(
    camera_matrix, dist_coeffs, None, new_camera_matrix, (width, height), cv2.CV_16SC2
)

# Setup VideoWriter to save calibrated video
fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter('calibrated_output.avi', fourcc, 30.0, (width, height))

print("Recording started. Press 'q' in the window to stop.")

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Frame capture failed, exiting.")
            break

        # Undistort the frame
        undistorted = cv2.remap(frame, map1, map2, interpolation=cv2.INTER_LINEAR)

        # Write the undistorted frame to file
        out.write(undistorted)

        # Show live preview window
        cv2.imshow("Calibrated Recording", undistorted)

        # Quit on 'q' key press
        if cv2.waitKey(1) & 0xFF == ord('q'):
            print("Stopping recording by user request.")
            break

except KeyboardInterrupt:
    print("Stopped by KeyboardInterrupt.")

finally:
    # Release resources
    cap.release()
    out.release()
    cv2.destroyAllWindows()
    print("Recording saved as 'calibrated_output.avi'")
