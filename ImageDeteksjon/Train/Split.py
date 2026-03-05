from yolosplitter import YoloSplitter
ys = YoloSplitter(imgFormat=['.jpg', '.jpeg', '.png'], labelFormat=['.txt'] )

# If you have yolo-format dataset already on the system
df = ys.from_mixed_dir(input_dir="/Users/martinsteinsvollrikardsen/Documents/Elsys_prosjekt/drevja_data/new_dataset/full_frames/mixed",ratio=(0.7,0.2,0.1),return_df=True) #input_dir, til der filene ligger

ys.info()

ys.save_split(output_dir="/Users/martinsteinsvollrikardsen/Documents/Elsys_prosjekt/drevja_data/new_dataset/full_frames/dataset")