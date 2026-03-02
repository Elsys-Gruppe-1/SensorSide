from ultralytics import YOLO
# Load a pretrained YOLO26n model
model = YOLO("yolo26n.pt")
results = model.train(data= '/Users/martinsteinsvollrikardsen/Documents/brackish-dataset/data.yaml', epochs=6, imgsz= 640, device='mps')

