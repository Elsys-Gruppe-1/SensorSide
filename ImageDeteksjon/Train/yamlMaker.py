from pathlib import Path
import yaml

def create_yolo_yaml(dataset_root, class_names, output_file="data.yaml"):
    dataset_root = Path(dataset_root)

    data = {
        "train": "train/images",
        "val": "val/images",
        "nc": len(class_names),
        "names": class_names
    }

    output_path = dataset_root / output_file

    with open(output_path, "w") as f:
        yaml.dump(data, f, sort_keys=False)

    print(f"Created YOLO YAML → {output_path}")

dataset = "yolo_dataset"   # folder containing train/ and val/
classes = ["person", "car", "dog"]
create_yolo_yaml(dataset, classes) 
