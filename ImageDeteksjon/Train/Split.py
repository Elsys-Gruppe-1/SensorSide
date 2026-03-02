from yolosplitter import YoloSplitter
ys = YoloSplitter(imgFormat=['.jpg', '.jpeg', '.png'], labelFormat=['.txt'] )

# If you have yolo-format dataset already on the system
df = ys.from_yolo_dir(input_dir="yolo_dataset",ratio=(0.7,0.2,0.1),return_df=True) #input_dir, til der filene ligger

ys.info()