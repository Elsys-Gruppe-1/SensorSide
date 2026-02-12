from ultralytics import YOLO
# Load a pretrained YOLO26n model
model = YOLO("yolo26n.pt")
results = model.predict(source="0", show= True)

