from segment_anything import SamPredictor, sam_model_registry
import cv2

model_type = "vit_h"
prompt = "a photo of a cat"
img = cv2.imread("../data/cat.jpg")

sam = sam_model_registry[model_type](checkpoint="../models/sam_vit_h_4b8939.pth")
predictor = SamPredictor(sam)
predictor.set_image(img)
masks, _, _ = predictor.predict(prompt)