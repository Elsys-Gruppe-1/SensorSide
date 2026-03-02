import ultralytics
model = YOLO("yolo26n.pt") ## skriv inn modell navn her
model.export(format="ncnn") # ncnn format, spesielt bra for rasberry pi.

