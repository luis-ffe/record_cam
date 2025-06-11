import cv2
import numpy as np

# Load calibration parameters
fs = cv2.FileStorage("calibration.yml", cv2.FILE_STORAGE_READ)
camera_matrix = fs.getNode("CameraMatrix").mat()
dist_coeffs = fs.getNode("DistCoeffs").mat()
fs.release()

# Open the camera
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

if not cap.isOpened():
    print("Error: Could not open camera.")
    exit()

# Grab a sample frame to get dimensions
ret, frame = cap.read()
if not ret:
    print("Error: Could not read frame.")
    cap.release()
    exit()

h, w = frame.shape[:2]

# Compute undistortion map
new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(
    camera_matrix, dist_coeffs, (w, h), 1, (w, h)
)
map1, map2 = cv2.initUndistortRectifyMap(
    camera_matrix, dist_coeffs, None, new_camera_matrix, (w, h), cv2.CV_16SC2
)

# Setup video writer
fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter('calibrated_output.avi', fourcc, 20.0, (w, h))

print("Recording calibrated video... Press Ctrl+C to stop.")

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame.")
            break

        # Apply undistortion
        undistorted = cv2.remap(frame, map1, map2, interpolation=cv2.INTER_LINEAR)

        # Write to file
        out.write(undistorted)

        # Optional: show preview
        # cv2.imshow('Calibrated Video', undistorted)
        # if cv2.waitKey(1) & 0xFF == ord('q'):
        #     break

except KeyboardInterrupt:
    print("\nStopped by user.")

# Cleanup
cap.release()
out.release()
cv2.destroyAllWindows()
print("Video saved as 'video1.avi'")
