import cv2

# Hjelpefunksjon for å se kameraer: v4l2-ctl --list-devices

camera_ids = [0, 2, 6]
cameras = {}

# 1. Open all cameras and store them in a dictionary
for cid in camera_ids:
    cap = cv2.VideoCapture(cid, cv2.CAP_V4L2)
    if cap.isOpened():
        # Max bandwitdth is 3-4 Gbps
        # 1. FORCE the FourCC codec to MJPG (Motion JPEG)
        # This is the "magic" line that saves bandwidth
        cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))

        # 2. Lower the resolution (Optional but recommended for 4 cams)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

        # 3. Lower the Frame Rate (Saves CPU and Bandwidth)
        cap.set(cv2.CAP_PROP_FPS, 15)

        cameras[cid] = cap
        print(f"Success: Camera {cid} set to MJPG at 640x480.")

    else:
        print(f"Error: Could not open camera {cid}.")

if not cameras:
    print("No cameras found. Exiting.")
    exit()

print("Capturing... Press 'q' in a window or Ctrl+C in terminal to quit.")

try:
    while True:
        # 2. Loop through every active camera
        for cid, cap in cameras.items():
            ret, frame = cap.read()

            if ret:
                # Save each camera's frame with its ID in the filename
                filename = f'cam_{cid}_test.jpg'
                cv2.imwrite(filename, frame)

            else:
                print(f"Error: Could not read from camera {cid}.")

        # This waitKey is necessary for OpenCV to process background tasks
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("\nStopping capture...")

# 3. Release all cameras at the end
for cap in cameras.values():
    cap.release()

cv2.destroyAllWindows()
print("All cameras released and files saved.")