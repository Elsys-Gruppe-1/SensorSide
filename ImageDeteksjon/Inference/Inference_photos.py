from ultralytics import YOLO 
model = YOLO("SensorSide/ImageDeteksjon/Inference/Embret_pc.pt") #pretrained modell fra internett, trent på community fish dataset.
results = model(["SensorSide/ImageDeteksjon/Inference/Test_photos/Easy_test.jpg", "SensorSide/ImageDeteksjon/Inference/Test_photos/Hard_test.jpg"]) # burde være 1024, men den er drit... treig. 

for result in results: 
    boxes = result.boxes
    keypoints = result.keypoints
    probs = result.probs
    result.show()
