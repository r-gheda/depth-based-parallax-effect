#!/bin/bash

wget https://dl.fbaipublicfiles.com/segment_anything/sam_vit_h_4b8939.pth
mkdir -p models
mv sam_vit_h_4b8939 models
