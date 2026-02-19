from ultralytics import YOLO
# Load a pretrained YOLO26n model
model = YOLO("yolo26n.pt")

results = model.train(data= '/Users/martinsteinsvollrikardsen/Documents/brackish-dataset/data.yaml', epochs=1, imgsz= 640)
