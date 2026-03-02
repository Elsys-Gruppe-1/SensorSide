from ultralytics import YOLO 
model = YOLO("ImageDeteksjon/Inference/cfd-yolov12x-1.00.pt") #pretrained modell fra internett, trent på community fish dataset.
results = model(source=1, imgsz=640, show=True) # burde være 1024, men den er drit... treig. 