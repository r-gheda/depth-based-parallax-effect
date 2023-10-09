# Student Workspace for CS4365 Applied Image Processing
### Roberto Gheda - 5863503
## Build and run
Build the C++ executables
```
mkdir build && cd build/ && cmake .. && make && cd ..
```
Then install and activate the conda environment 
```
conda env create -f environment.yml
```
Install DistDepth CNN and SAM models
```
chmod +x download_models.sh && source download_models.sh
```
In order to run the program, just 
```
python3 src/main.py
```

## References
<a id="1">[1]</a> 
Pérez, Patrick, Michel Gangnet, and Andrew Blake. "Poisson image editing." ACM Transactions on Graphics (TOG) 22.3 (2003): 313-318.

<a id="2">[2]</a>
Liao, Jingtang, Shuheng Shen, and Elmar Eisemann: "Depth annotations: Designing depth of a single image for depth-based effects." Computers & Graphics (2018).

<a id="3">[3]</a>
Wu CY, Wang J, Hall M, Neumann U, Su S. Toward practical monocular indoor depth estimation. InProceedings of the IEEE/CVF Conference on Computer Vision and Pattern Recognition 2022 (pp. 3814-3824).

<a id="4">[4]</a> 
Kirillov A, Mintun E, Ravi N, Mao H, Rolland C, Gustafson L, Xiao T, Whitehead S, Berg AC, Lo WY, Dollár P. Segment anything. arXiv preprint arXiv:2304.02643. 2023 Apr 5.