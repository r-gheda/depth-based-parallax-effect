import cv2
import matplotlib as mpl
import matplotlib.cm as cm
import numpy as np
import os
import torch
from networks.depth_decoder import DepthDecoder
from networks.resnet_encoder import ResnetEncoder
from utils import output_to_depth
from PIL import Image
import sys

torch.backends.cudnn.benchmark = True
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
output_path = "../outputs/"

if __name__ == "__main__":

    dir_prefix = "./dist_depth/"

    with torch.no_grad():

        encoder = ResnetEncoder(152, False)
        loaded_dict_enc = torch.load(
            dir_prefix + "ckpts/encoder.pth",
            map_location=device,
        )

        filtered_dict_enc = {
            k: v for k, v in loaded_dict_enc.items() if k in encoder.state_dict()
        }
        encoder.load_state_dict(filtered_dict_enc)
        encoder.to(device)
        encoder.eval()

        depth_decoder = DepthDecoder(num_ch_enc=encoder.num_ch_enc, scales=range(4))

        loaded_dict = torch.load(
            dir_prefix + "ckpts/depth.pth",
            map_location=device,
        )
        depth_decoder.load_state_dict(loaded_dict)

        depth_decoder.to(device)
        depth_decoder.eval()

        if not os.path.exists(output_path):
            os.makedirs(output_path)

        file = sys.argv[1]

        raw_img = np.transpose(
            cv2.imread(file, -1)[:, :, :3], (2, 0, 1)
        )
        input_image = torch.from_numpy(raw_img).float().to(device)
        input_image = (input_image / 255.0).unsqueeze(0)
        original_size = [input_image.shape[-2], input_image.shape[-1]]

        # resize to input size
        input_image = torch.nn.functional.interpolate(
            input_image, (256, 256), mode="bilinear", align_corners=False
        )
        features = encoder(input_image)
        outputs = depth_decoder(features)

        out = outputs[("out", 0)]
        
        # resize to original size
        out_resized = torch.nn.functional.interpolate(
            out, (original_size[0], original_size[1]), mode="bilinear", align_corners=False
        )

        # convert disparity to depth
        depth = output_to_depth(out_resized, 0.1, 10)
        metric_depth = depth.cpu().numpy().squeeze()
        print(metric_depth.min())
        print(metric_depth.max())
        metric_depth_norm = 255.0 * (metric_depth - metric_depth.min()) / (
            metric_depth.max() - metric_depth.min()
        )
        
        im = Image.fromarray((metric_depth_norm).astype(np.uint8))
        im.save(output_path + 'predicted_depth.png')