#!/bin/bash

wget https://dl.fbaipublicfiles.com/segment_anything/sam_vit_h_4b8939.pth
mkdir -p models
mv sam_vit_h_4b8939 models

cd src/dist_depth && wget --load-cookies /tmp/cookies.txt "https://docs.google.com/uc?export=download&confirm=$(wget --quiet --save-cookies /tmp/cookies.txt --keep-session-cookies --no-check-certificate 'https://docs.google.com/uc?export=download&id=1N3UAeSR5sa7KcMJAeKU961KUNBZ6vIgi' -O- | sed -rn 's/.*confirm=([0-9A-Za-z_]+).*/\1\n/p')&id=1N3UAeSR5sa7KcMJAeKU961KUNBZ6vIgi" -O ckpts.zip && rm -rf /tmp/cookies.txt && unzip ckpts.zip && rm ckpts.zip
cd ../..

