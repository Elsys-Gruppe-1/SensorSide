import cv2
import imagezmq
import socket
from threading import Thread, Lock  # Added Lock here

# Initialize sender
sender = imagezmq.ImageSender(connect_to='tcp://192.168.210.176:5555')
host_name = socket.gethostname()

# 1. Create the Lock object
net_lock = Lock()

def stream_camera(camera_id):
    cap = cv2.VideoCapture(camera_id, cv2.CAP_V4L2)
    cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    
    if not cap.isOpened():
        return

    while True:
        ret, frame = cap.read()
        if ret:
            # 2. Use the lock before sending
            with net_lock:
                sender.send_image(f"cam_{camera_id}", frame)
        else:
            # If a camera fails, wait a bit before retrying
            cv2.waitKey(1000)

# Based on your v4l2 output, try 0 first. 
# If you have 4 cams, they are likely 0, 2, 4, 6
camera_ids = [0,4,8,25] 

for cid in camera_ids:
    t = Thread(target=stream_camera, args=(cid,))
    t.daemon = True
    t.start()

print("Streaming started. Ensure PC receiver is running!")
try:
    while True:
        # Just keep the main thread alive
        cv2.waitKey(1)
except KeyboardInterrupt:
    print("Stopping...")