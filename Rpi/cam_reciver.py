import cv2
import imagezmq
import numpy as np
from threading import Thread
import time

class VideoStreamSubscriber:
    def __init__(self, port="5555"):
        self.image_hub = imagezmq.ImageHub(open_port=f"tcp://Gruppe1Pi.local:{port}", REQ_REP=False)
        self.image_hub.zmq_socket.setsockopt(imagezmq.zmq.RCVHWM, 1)

        self.latest_frame = None 
        self.keep_running = True
        # Start the background thread
        self.thread = Thread(target=self._update, args=())
        self.thread.daemon = True
        self.thread.start()

    def _update(self):
        while self.keep_running:
            # Constantly drain the socket
            try:
                name, jpg_buffer = self.image_hub.recv_jpg()
                self.latest_frame = cv2.imdecode(
                    np.frombuffer(jpg_buffer, dtype=np.uint8), cv2.IMREAD_COLOR
                )
            except Exception as e:
                print(f"Error: {e}")

    def get_frame(self):
        return self.latest_frame

# --- Main Logic ---
receiver = VideoStreamSubscriber()
print("Receiver started...")

while True:
    frame = receiver.get_frame()
    size = (720,720)
   
    if frame is not None:
        frame = cv2.resize(frame, size, fx=0, fy=0, interpolation=cv2.INTER_LINEAR)
        cv2.imshow("Low Latency Stream", frame)
        
    
    # waitKey(1) is critical—it processes the GUI events
    if cv2.waitKey(1) & 0xFF == ord('q'):
        receiver.keep_running = False
        break

cv2.destroyAllWindows()