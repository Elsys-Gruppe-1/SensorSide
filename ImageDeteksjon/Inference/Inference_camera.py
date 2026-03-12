from ultralytics import YOLO 
model = YOLO("SensorSide/ImageDeteksjon/Inference/Embret_pc.pt") #pretrained modell fra internett, trent på community fish dataset.
results = model(source=1, imgsz=640, show=True) # burde være 1024, men den er drit... treig. 