# Computational Depth Of Field
This repository contains a program to create a Parallax effect video from a given image. It comes with several tools to generate and edit depth maps from such given image. \
This projects exploits standard monoculuar depth estimation techniques such as Poisson Image editing as well as novel AI-based methodologies including a user-friendly depth map editing achieved leveraging Segment Anything Model[[4]](#4) (SAM) by Meta.
## Prerequisites
This app has been developed and tested on **Ubuntu 22.04** using **Python 3.10.13**, **cmake 3.22.1** and **C++**. \
Moreover, the `xdg-open` command is used for image displaying during SAM-based image editing (see below for details). For a proper user experience, please install `xdg-utils`. \
More libraries to be installed for downloading NN models include `wget` and `curl`.
```
sudo apt-get install xdg-utils curl wget
```
## External code
Code in directories [src/dist_depth](src/dist_depth/) and [src/segment-anything](src/segment-anything/) come from the respective GitHub repositories and contain the core code for running the respective deep learning models. \
The [framework](framework) directory contains the image processing framework used in the assignments. \
**Everything else is original code made by me.**

## Build and run
Build the C++ executables:
```
mkdir -p build && cd build/ && cmake .. && make && cd ..
```
Then create and activate a Python virtual environment:
```
python3 -m venv env && source env/bin/activate
```
Install Python requirements:
```
python3 -m pip install -r requirements.txt
```
Download DistDepth CNN  [[3]](#3) and SAM models [[4]](#4):
```
source download_models.sh
```
In order to run the program, please run: 
```
python3 src/main.py
```

## Implemented Functionalities
An image of the program GUI is provided as a reference for instructions on how to run the features.

![](readme-imgs/gui.png)
### Basic Features
### 1. Load an RGB image from disk
An image can be opened using the "Open A File" blue button. A set of sample images are provided in the [data directory](data/).\
**Implemented in** [src/main.py at line 40](src/main.py#L40).
### 2. Allow users to scribble depth annotations in UI
It is possible to draw scribbles using the "Draw Scribbles" button. In order to save them it is sufficient to close the drawing window. Please note that drawing scribbles is not mandatory for using the subsuquent functionalities. Scribbles can also be saved into files using the "Save Scribbles" button or loaded from files using the "Load Scribbles" button.\
**Implemented in** [src/main.py at line 105](src/main.py#L105).
### 3. Diffuse annotations across the image using Poisson image editing
First it is possible to choose a number of iterations by writing it in its bar and **pressing enter**. Then it is possible ot run either the standard Poisson image editing [[1]](#1) or an enhanced anisotropic version inspired by the relative paper [[2]](#2).<br>
Please note that in order to run the anisotropic version choosing a proper value of beta (strength of anisotropic effect) is needed. A suggested beta value is 20. A sufficient number of iterations depends on variables factor including image size and amount of scribbles.\
**Implemented in** [src/poisson.h at line 47](src/poisson.h#L47) and [line 130](src/poisson.h#L130).
### 4. Allow users to select focus depth and aperture size
Aperture size can be set using its bar and **pressing enter**. A focus point can be chosen the "Select Focus" button and closing the window once the focus point has been chosen.
### 5. Simulate depth-of-field using a spatially varying cross-bilateral filter
A Cross-Bilateral Filter can be run using the light blue "Bilateral Filter". In order to do so it is needed to choose which depth map to use via the above radio button. \
**Implemented in** [src/bilateral_filter.h at line 51](src/bilateral_filter.h#L51).
### 6. Save and display the result
All results are saved in the [outputs directory](outputs/) and automatically shown when produced.
### Extended Features
### 1. Use a pretrained RGB->Depth CNN to supplement the depth
Depth can be predicted using the DistDepth model by Meta AI [[3]](#3). Just press the "Run CNN" button in order to run it.\
**Implemented in** [src/main.py at line 259](src/main.py#L259).
### 2. Find a user-friendly way to combine predicted depth map and user scribbles
It is possible to "merge" depth-maps obtained from scribbles and CNN using the "Merge Depth Maps" button. What it does is running an anisotropic Poisson image editing initializing the depth map as the CNN predicted depth-map. This allows to fix some errors that brought up from lack of enough scribbles from the users and brings to more realistic results with a lower amount of iterations and user scribbles.\
**Implemented in** [src/main.py at line 276](src/main.py#276).
### 3. Implement Ken-Burns effect with depth-based parallax
In order to run the depth-based Parallax effect select a depth map using the dedicated radio button and press the Parallax button. The output will be saved as `outputs/output_video.avi`\
**Implemented in** [src/parallax_main.cpp at line 7](src/parallax_main.cpp#L7).
### Extra Custom Features
### 1. Allow user to manually edit the final depth-map via GUI
Image depth map can edited via GUI to correct users common mistakes. In order to do so just use the dedicated radio button to choose a depth map to be edited and press the "Edit Depth Map" button. Select intensity and thickness of the brush. Then use the left mouse button to increase depth or the right one to decrease it.\
**Implemented in** [src/main.py at line 505](src/main.py#L505)
### 2. Allow user to run segmentation via SAM and edit the depth-map using it
It is possible to run the Segment Anything Model [[4]](#4) and edit a depth map by selecting an object an setting a custom depth level. In order to do so, select a depth map using the dedicated radio button and press the "Run SAM" button. By default the object gradient will be kept. It can be removed by pressing the "Flatten Gradient" button and seleting a new depth level. This feature can help in achieving better results in the Parallax effect.\
**Implemented in** [src/main.py at line 402](src/main.py#402)

## Repository content
- `data`: contains some input examples.
- `examples`: contains some output examples.
- `framework`: contains external code imported from the assignments.
- `src`: contains all of the source code.
### Generated directories
- `build`: contains C++ executables
- `env`: contains Python3 virtual environment.
- `models`: contains the Segment Anything model.
- `outputs`: contains outputs from the program.
    - `outputs/sam-out`: contains SAM cached masks. If masks for a specific input are stored there, the program will avoid computing everything from scratch.

## References
<a id="1">[1]</a> 
Pérez, Patrick, Michel Gangnet, and Andrew Blake. "Poisson image editing." ACM Transactions on Graphics (TOG) 22.3 (2003): 313-318.

<a id="2">[2]</a>
Liao, Jingtang, Shuheng Shen, and Elmar Eisemann: "Depth annotations: Designing depth of a single image for depth-based effects." Computers & Graphics (2018).

<a id="3">[3]</a>
Wu CY, Wang J, Hall M, Neumann U, Su S. Toward practical monocular indoor depth estimation. InProceedings of the IEEE/CVF Conference on Computer Vision and Pattern Recognition 2022 (pp. 3814-3824).

<a id="4">[4]</a> 
Kirillov A, Mintun E, Ravi N, Mao H, Rolland C, Gustafson L, Xiao T, Whitehead S, Berg AC, Lo WY, Dollár P. Segment anything. arXiv preprint arXiv:2304.02643. 2023 Apr 5.