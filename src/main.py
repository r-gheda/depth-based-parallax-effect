import tkinter as tk
from tkinter import ttk
from tkinter import filedialog as fd
from PIL import ImageTk, Image, ImageDraw
import os
import subprocess
import cv2
import csv

SEL_IMAGE = None
SUPPORTED_FORMATS = ["JPG", "JPEG", "PNG", "jpg", "jpeg", "png"]
img = None
img_path = None
img_size = None
drawing = False
pt1_x , pt1_y = None , None
depth_annotation_window = None
canvas = None
out = None
scribbles = {}
anisotropic_depth_map = "anisotropic_depth_map.png"
poisson_depth_map = "poisson_depth_map.png"
focused_image = "focused_image.png"
predicted_depth_map = "predicted_depth.png"
merged_depth_map = "merged_depth_map.png"
sam_depth = "sam_depth.png"
edited_depth_map = "edited_depth_map.png"
anisotropic_depth_map_loaded = False
scribble_loaded = False
predicted_depth_map_loaded = False
focus_x = 0
focus_y = 0
iterations = '-1'
beta = '-1'
aperture_size = '-1'
last_selected = None

global scribbles_status_label

def select_image():
    '''
    load an image from the disk
    '''
    global SEL_IMAGE, img, img_path, img_name, img_size, scribbles
    res = False
    while not res:
        SEL_IMAGE = fd.askopenfilename()
        try:
            img = Image.open(SEL_IMAGE)
            img_path = SEL_IMAGE
        except IOError:
            print("Invalid file")
            continue
        if not img.format in SUPPORTED_FORMATS:
            print("Unsupported format: " + str(img.format))
        else:
            res = True    
            img_name = img_path.split('/')[-1].split('.')[0]
            img_size = img.size

        scribbles = {}

def _from_rgb(rgb):
    """
    translates an rgb tuple of int to a tkinter friendly color code
    """
    return "#%02x%02x%02x" % rgb   

def get_x_and_y(event):
    '''
    gets cursor position
    '''
    global lasx, lasy, color
    lasx, lasy = event.x, event.y
    color = _from_rgb((depth_slider.get(), depth_slider.get(), depth_slider.get()))

def draw_handler(event):
    '''
    draw handler for a movnig cursor
    '''
    global lasx, lasy
    canvas.create_line((lasx, lasy, event.x, event.y), 
                        fill=color, 
                        width=thickness_slider.get())
    for i in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
        for j in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
            scribbles[(lasy+j, lasx+i)] = depth_slider.get()
    lasx, lasy = event.x, event.y

def discard_scribbles_handler():
    '''
    removes saved scribbles
    '''
    global scribbles, scribble_loaded
    scribbles = {}
    scribble_loaded = False

def update_focus_point(event):
    '''
    gets the cursor position and updates the focus point variable
    '''
    global focus_x, focus_y
    focus_x, focus_y = event.x, event.y

def draw_annotations_callback():
    '''
    create a window for drawing scribbles on an image
    '''
    global depth_annotation_window, canvas, depth_slider, scribbles, thickness_slider
    depth_annotation_window = tk.Tk()
    depth_annotation_window.title("Draw")
    
    canvas = tk.Canvas(depth_annotation_window, width=img.width, height=img.height)
    canvas.grid(row=0, columnspan=2)
    tk_img = ImageTk.PhotoImage(image=img, master=depth_annotation_window)
    canvas.create_image(img.width/2, img.height/2, image=tk_img)

    canvas.bind("<Button-1>", get_x_and_y)
    canvas.bind('<B1-Motion>', draw_handler)

    depth_slider = tk.Scale(depth_annotation_window, from_=0, to=255, orient=tk.HORIZONTAL, label="Depth")
    depth_slider.set(0)
    depth_slider.grid(row=1, column=0)
    
    thickness_slider = tk.Scale(depth_annotation_window, from_=1, to=10, orient=tk.HORIZONTAL, label="Thickness")
    thickness_slider.set(2)
    thickness_slider.grid(row=1, column=1)

    scribble_loaded = True
    depth_annotation_window.mainloop()

def save_scribbles():
    '''
    save scribbles into a file
    '''
    if os.path.exists("outputs/scribbles"):
        os.remove("outputs/scribbles")
    
    with open("outputs/scribbles", "w") as f:
        for key, value in scribbles.items():
            f.write(str(key[0]) + " " + str(key[1]) + " " + str(value) + "\n")

def run_poisson():
    '''
    runs the poisson c++ executable and shows the result
    '''
    global img, scribbles, beta, iterations
    if not os.path.exists("outputs"):
        os.makedirs("outputs")
    img.save("outputs/src_rgb.png")
    grey_img = img.convert('L')
    grey_img.save("outputs/greyscale-input.png")

    if int(beta) < 0:
        beta = '20'
    if int(iterations) < 0:
        iterations = '1000'

    save_scribbles()

    arglist = ["build/poisson", "outputs/greyscale-input.png", "outputs/src_rgb.png", "outputs/" + str(poisson_depth_map), "outputs/scribbles", "poisson", iterations, beta]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    out = Image.open("outputs/" + str(poisson_depth_map))
    out.show(title="Poisson Depth Map")
    out.save("outputs/" + str(poisson_depth_map))

def run_anisotropic():
    '''
    runs the anisotropic c++ executable and shows the result
    '''
    global img, scribbles, anisotropic_depth_map, beta, iterations
    if not os.path.exists("outputs"):
        os.makedirs("outputs")
    if os.path.exists("outputs/" + str(anisotropic_depth_map)):
        os.remove("outputs/" + str(anisotropic_depth_map))
    img.save("outputs/src_rgb.png")
    grey_img = img.convert('L')
    grey_img.save("outputs/greyscale-input.png")

    if int(beta) < 0:
        beta = '20'
    if int(iterations) < 0:
        iterations = '1000'

    save_scribbles()
    arglist = ["build/poisson", "outputs/greyscale-input.png", "outputs/greyscale-input.png", "outputs/" + str(anisotropic_depth_map), "outputs/scribbles", "anisotropic", iterations, beta]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    out = Image.open("outputs/" + str(anisotropic_depth_map))
    out.save("outputs/" + str(anisotropic_depth_map))
    out.show(title="Anisotropic Depth Map")
    anisotropic_depth_map_loaded = True

def update_iter(event):
    '''
    update the iteration value
    '''
    global iterations
    iterations = event.widget.get()

def update_beta(event):
    '''
    update the beta value
    '''
    global beta
    beta = event.widget.get()

def update_aperture_size(event):
    '''
    update the aperture size value
    '''
    global aperture_size
    aperture_size = event.widget.get()

def select_focus():
    '''
    opens the source image and lets the user select a focus point
    '''
    global img
    focus_selection_window = tk.Tk()
    focus_selection_window.title("Select Focus")

    canvas = tk.Canvas(focus_selection_window, width=img.width, height=img.height)
    canvas.pack()
    tk_img = ImageTk.PhotoImage(image=img, master=focus_selection_window)
    canvas.create_image(img.width/2, img.height/2, image=tk_img)

    canvas.bind("<Button-1>", update_focus_point)

    focus_selection_window.mainloop()

def run_bilateral_filter():
    '''
    runs the bilateral filter c++ executable and shows the result
    '''
    global img, anisotropic_depth_map, focus_x, focus_y, aperture_size, focused_image

    print(aperture_size)
    if int(aperture_size) < 0:
        aperture_size = '15'

    arglist = ["build/bilateral_filter", "outputs/src_rgb.png", "outputs/" +  str(depth_map_to_be_used.get()), "outputs/" + str(focused_image), str(focus_x), str(focus_y), aperture_size]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    out = Image.open("outputs/" + str(focused_image))
    out.show(title="Bilateral Filter")
    

def run_cnn(called = False):
    '''
    runs a cnn depth prediction via dist depth and shows the result
    '''
    global predicted_depth_map
    predicted_depth_map = "predicted_depth.png"
    arglist = ["python3", "src/dist_depth/run_rgb_cnn.py", img_path]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    out = Image.open("outputs/predicted_depth.png")
    if not called:
        out.show("CNN Predicted Depth Map")
    predicted_depth_map_loaded = True

def run_merged_depth_maps():
    '''
    runs the cnn prediction (if not done yet) and runs anisotropic c++ executable with it as a starting point
    '''
    global img, scribbles, anisotropic_depth_map, predicted_depth_map, beta, iterations
    if not os.path.exists("outputs"):
        os.makedirs("outputs")
    if not os.path.exists("outputs/" + str(predicted_depth_map)):
        run_cnn(True)

    img.save("outputs/src_rgb.png")
    grey_img = img.convert('L')
    grey_img.save("outputs/greyscale-input.png")

    if int(beta) < 0:
        beta = '20'
    if int(iterations) < 0:
        iterations = '1000'

    save_scribbles()
    arglist = ["build/poisson", "outputs/" + str(predicted_depth_map), "outputs/greyscale-input.png", "outputs/" + str(merged_depth_map), "outputs/scribbles", "anisotropic", iterations, beta]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()

    out = Image.open("outputs/" + str(merged_depth_map))
    p = Image.open("outputs/" + str(predicted_depth_map))
    tmp = out.convert('L')
    out.save("outputs/" + str(merged_depth_map))
    out.show(title="Merged Depth Map")


def apply_sam_mask(event):
    '''
    applies the sam mask onto the GUI
    '''
    global im, im_bak, tk_img, canvas, sam_root, curr_sam_mask, mask_dir
    im = im_bak.copy()
    curr_sam_mask = {}
    mask = Image.open(mask_dir + event.widget.get())
    for i in range(mask.width):
        for j in range(mask.height):
            if mask.getpixel((i,j)) != 0:
                curr_sam_mask[(i,j)] = im.getpixel((i,j))
                im.putpixel((i,j), (0,0,0))
    tk_img = ImageTk.PhotoImage(image=im, master=sam_root)
    canvas.create_image(im.width/2, im.height/2, image=tk_img)

def update_mask_depth_level(event):
    '''
    updates the mask depth level
    '''
    global depth_level_entry, curr_sam_mask, depth_map_im, toggle_state
    avg_depth = sum([depth_map_im.getpixel(k) for k in curr_sam_mask.keys()]) / len(curr_sam_mask)
    diff = int(depth_level_entry.get()) - avg_depth
    if not toggle_state:
        for key in curr_sam_mask:
            depth_map_im.putpixel(key, max(min(int(depth_map_im.getpixel(key) + (diff)), 255), 0))
    else:
        for key in curr_sam_mask:
            depth_map_im.putpixel(key, max(min(int(depth_level_entry.get()), 255),0))
    depth_map_im.save("outputs/" + str(sam_depth))

def load_masks():
    '''
    loads sam masks from its output files
    '''
    global img_name, masks
    masks = {}
    with open('outputs/sam-out/' + img_name + '/metadata.csv') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            if row[0] == 'id':
                continue

            image = Image.open('outputs/sam-out/' + img_name + '/' + row[0] + '.png')
            for x in range(int(row[2]), int(row[4]) + int(row[2])):
                for y in range(int(row[3]), int(row[5]) + int(row[3])):
                    if image.getpixel((x, y)) != 0:
                        if not (x,y) in masks:
                            masks[(x,y)] = []
                            masks[(x,y)].append(-1)
                        masks[(x,y)].append(int(row[0]))

def select_mask(event):
    '''
    select a mask on the cursor position and applies it on the GUI
    '''
    global last_selected, count, masks, curr_sam_mask, tk_img, im, canvas, sam_root, mask_dir, im_bak
    if not (event.x, event.y) in masks:
        return
    if last_selected == None:
        last_selected = (event.x, event.y)
        count = 0
    if len(masks[last_selected]) == len(masks[(event.x, event.y)]):
        count = (count + 1) % len(masks[(event.x, event.y)])
    else:
        count = min(1, len(masks[(event.x, event.y)]) - 1)
    last_selected = (event.x, event.y)
    im = im_bak.copy()

    curr_sam_mask = {}
    if (masks[(event.x, event.y)][count] != -1):
        mask = Image.open(mask_dir + str(masks[(event.x, event.y)][count]) + '.png')
        for i in range(mask.width):
            for j in range(mask.height):
                if mask.getpixel((i,j)) != 0:
                    curr_sam_mask[(i,j)] = im.getpixel((i,j))
                    im.putpixel((i,j), (0,0,0))
    tk_img = ImageTk.PhotoImage(image=im, master=sam_root)
    canvas.create_image(im.width/2, im.height/2, image=tk_img)

def toggle():
    '''
    toggles the `flatten gradient` option
    '''
    global toggle_btn, toggle_state
    if toggle_btn.config('relief')[-1] == 'sunken':
        toggle_btn.config(relief="raised")
        toggle_state = False
    else:
        toggle_btn.config(relief="sunken")
        toggle_state = True

def run_sam():
    '''
    runs sam prediction (if not cached) and lets user edit the selected depth map
    '''
    global img_path, im, im_bak, tk_img, canvas, sam_root, depth_level_entry, depth_map_im, mask_dir, img_name, toggle_btn, toggle_state

    depth_map_im = Image.open("outputs/" + str(depth_map_to_be_used.get())).convert('L')
    if not os.path.exists("outputs/sam-out/" + img_name):      
        arglist = ["python3", "src/segment-anything/scripts/amg.py",'--checkpoint', 'models/sam_vit_h_4b8939.pth','--model', 'vit_h', '--input', img_path, '--output', 'outputs/sam-out', '--device', 'cpu']
        proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate()
        print(stdout)
        print(stderr)
        proc.wait()

    load_masks()

    im = Image.open(img_path)
    im_bak = im.copy()
    sam_root = tk.Tk()
    canvas = tk.Canvas(sam_root, width=im.width, height=im.height)
    canvas.grid(row=0, columnspan=3)

    tk_img = ImageTk.PhotoImage(image=im, master=sam_root)
    canvas.create_image(im.width/2, im.height/2, image=tk_img)
    os.chdir("outputs/sam-out/" + img_name)
    mask_dir = os.getcwd() + '/'
    files = [f for f in os.listdir() if os.path.isfile(f)]
    os.chdir("../../..")
    depth_map_im.save("outputs/" + str(sam_depth))
    stream = os.popen('xdg-open outputs/' + str(sam_depth))

    canvas.bind("<Button-1>", select_mask)

    Combo = ttk.Combobox(sam_root, values = files)
    Combo.set("Pick a SAM mask")
    Combo.grid(row=1, column=0)

    depth_level_label = tk.Label(sam_root, text="Depth Level").grid(row=1, column=2)
    depth_level_entry = tk.Entry(sam_root)
    depth_level_entry.bind("<Return>", update_mask_depth_level)
    depth_level_entry.grid(row=1, column=3)

    Combo.bind("<<ComboboxSelected>>", apply_sam_mask)
    
    toggle_btn = tk.Button(sam_root, text="Flatten gradient", width=12, relief="raised", command=toggle)
    toggle_btn.grid(row=1, column=1)
    toggle_state = False
    sam_root.mainloop()
    
def edit_image_pos(event):
    '''
    increase depth level at event position
    '''
    global lasx, lasy, color, depth_img, intensity_slider, val
    lasx, lasy = event.x, event.y
    val = min(depth_img.getpixel((lasx, lasy)) + intensity_slider.get(), 255)
    color = _from_rgb((val, val, val))
    return

def edit_image_neg(event):
    '''
    decrease depth level at event position
    '''
    global lasx, lasy, color, depth_img, intensity_slider
    lasx, lasy = event.x, event.y
    val = max(depth_img.getpixel((lasx, lasy)) - intensity_slider.get(), 0)
    color = _from_rgb((val, val, val))
    return

def draw_handler_pos_edit(event):
    '''
    increase draw handler (analogous to scribble draw handler)
    '''
    global lasx, lasy, color, depth_img, intensity_slider, thickness_slider, canvas, val, draw_depth_img
    val = min(depth_img.getpixel((event.x, event.y)) + intensity_slider.get(), 255)
    color = _from_rgb((val, val, val))
    canvas.create_line((lasx, lasy, event.x, event.y), 
                        fill=color, 
                        width=thickness_slider.get())
    
    for i in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
        for j in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
            draw_depth_img.line((event.x, event.y, lasx, lasy), fill=color, width=thickness_slider.get())
    depth_img.save('outputs/' + str(edited_depth_map))
    lasx, lasy = event.x, event.y

def draw_handler_neg_edit(event):
    '''
    decrease draw handler (analogous to scribble draw handler)
    '''
    global lasx, lasy, color, depth_img, intensity_slider, thickness_slider, canvas, val, draw_depth_img
    val = min(depth_img.getpixel((event.x, event.y)) - intensity_slider.get(), 255)
    color = _from_rgb((val, val, val))
    canvas.create_line((lasx, lasy, event.x, event.y), 
                        fill=color, 
                        width=thickness_slider.get())
    for i in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
        for j in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
            draw_depth_img.line((event.x, event.y, lasx, lasy), fill=color, width=thickness_slider.get())
    depth_img.save('outputs/' + str(edited_depth_map))
    lasx, lasy = event.x, event.y

def edit_merged_depth_map():
    '''
    opens the manual depth map editing GUI
    '''
    global depth_map_to_be_used, intensity_slider, thickness_slider, depth_img, canvas, draw_depth_img
    im = Image.open('outputs/' + str(depth_map_to_be_used.get()))
    im_bak = im.copy()
    edit_root = tk.Tk()
    canvas = tk.Canvas(edit_root, width=im.width, height=im.height)
    canvas.grid(row=0, columnspan=3)

    tk_img = ImageTk.PhotoImage(image=im, master=edit_root)
    canvas.create_image(im.width/2, im.height/2, image=tk_img)
    if (os.path.exists('outputs/' + str(edited_depth_map))):
        stream = os.popen('rm outputs/' + str(edited_depth_map))
        stream.read()
    stream = os.popen('cp outputs/' + str(depth_map_to_be_used.get()) + ' outputs/' + str(edited_depth_map))
    # wait for stream to finish
    stream.read()

    depth_img = Image.open('outputs/' + str(edited_depth_map)).convert('L')
    draw_depth_img = ImageDraw.Draw(depth_img)

    canvas.bind("<Button-1>", edit_image_pos)
    canvas.bind('<B1-Motion>', draw_handler_pos_edit)
    canvas.bind("<Button-3>", edit_image_neg)
    canvas.bind('<B3-Motion>', draw_handler_neg_edit)

    intensity_slider = tk.Scale(edit_root, from_=0, to=50, orient=tk.HORIZONTAL, label="Intensity")
    intensity_slider.set(0)
    intensity_slider.grid(row=1, column=0)
    
    thickness_slider = tk.Scale(edit_root, from_=1, to=10, orient=tk.HORIZONTAL, label="Thickness")
    thickness_slider.set(2)
    thickness_slider.grid(row=1, column=1)

    edit_root.mainloop()

def load_scribble_from_file():
    '''
    loads a set of scribbles from a file
    '''
    global scribbles, scribble_loaded
    res = False
    while not res:
        scribble_file = fd.askopenfilename()
        scribbles = {}
        try:
            with open(scribble_file, "r") as f:
                for line in f:
                    line = line.strip().split()
                    scribbles[(int(line[0]), int(line[1]))] = int(line[2]) 
                res = True
                scribble_loaded = True
        except IOError:
            print("Invalid file")
            continue   

def save_scribbles_from_file():
    '''
    save scribbles into file
    '''
    global scribbles
    res = False
    while not res:
        scribble_file = fd.asksaveasfilename()
        try:
            with open(scribble_file, "w") as f:
                for key, value in scribbles.items():
                    f.write(str(key[0]) + " " + str(key[1]) + " " + str(value) + "\n")
                res = True
        except IOError:
            print("Invalid file")
            continue

def run_parallax():
    '''
    runs the parallax c++ executable
    '''
    global depth_map_to_be_used, img_size
    arglist = ["build/parallax", 'outputs/src_rgb.png', 'outputs/' + str(depth_map_to_be_used.get()), str(180), str(0.01)]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()

    out = cv2.VideoWriter('outputs/output_video.avi',cv2.VideoWriter_fourcc(*'DIVX'), 30, img_size)

    for i in range(60):
        img = cv2.imread('outputs/frames/' + str(i) + '.png')
        out.write(img)

    for i in reversed(range(60)):
        img = cv2.imread('outputs/frames/' + str(i) + '.png')
        out.write(img)

    out.release()

'''
GUI buttons
'''

window = tk.Tk()

#Create a button that lets to select an image file
image_button = tk.Button(
    text="Open A File",
    width=25,
    height=5,
    bg="blue",
    fg="yellow",
    command= select_image
).grid(row=0, column=1, columnspan=4)

draw_button = tk.Button(
    text="Draw Scribbles",
    width=25,
    height=5,
    bg="green",
    fg="yellow",
    command= draw_annotations_callback
).grid(row=1, column=1, columnspan=2)

discard_button = tk.Button(
    text="Discard Scribbles",
    width="25",
    height="5",
    bg="green",
    fg="yellow",
    command=discard_scribbles_handler
).grid(row=1, column=3, columnspan=2)

save_scibbles_button = tk.Button(
    text="Save Scribbles",
    width=25,
    height=5,
    bg="green",
    fg="yellow",
    command= save_scribbles_from_file
).grid(row=2, column=1, columnspan=2)

load_scribbles_button = tk.Button(
    text="Load Scribbles",
    width=25,
    height=5,
    bg="green",
    fg="yellow",
    command= load_scribble_from_file
).grid(row=2, column=3, columnspan=2)

iter_label = tk.Label(text="Number of iterations").grid(row=3, column=1, columnspan=2)
n_of_iter = tk.Entry()
n_of_iter.bind("<Return>", update_iter)
n_of_iter.grid(row=3, column=3, columnspan=2)

beta_label = tk.Label(text="Beta").grid(row=4, column=1, columnspan=2)
beta_entry = tk.Entry()
beta_entry.bind("<Return>", update_beta)
beta_entry.grid(row=4, column=3, columnspan=2)

poisson_button = tk.Button(
    text="Poisson",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_poisson
).grid(row=5, column=1, columnspan=2)

anisotropic_button = tk.Button(
    text="Anisotropic",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_anisotropic
).grid(row=5, column=3, columnspan=2)

run_cnn_button = tk.Button(
    text="Run CNN",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_cnn
).grid(row=6, column=1, columnspan=2)

merge_depth_maps_button = tk.Button(
    text="Merge Depth Maps",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_merged_depth_maps
).grid(row=6, column=3, columnspan=2)

edit_merged_depth_map_button = tk.Button(
    text="Manual Edit Depth Map",
    width=25,
    height=5,
    bg="orange",
    fg="black",
    command= edit_merged_depth_map
).grid(row=7, column=1, columnspan=2)

run_sam_button = tk.Button(
    text="Run SAM",
    width=25,
    height=5,
    bg="orange",
    fg="black",
    command= run_sam
).grid(row=7, column=3, columnspan=2)

aperture_size_label = tk.Label(text="Aperture Size").grid(row=10, column=1, columnspan=2)
aperture_size_entry = tk.Entry()
aperture_size_entry.bind("<Return>", update_aperture_size)
aperture_size_entry.grid(row=10, column=3, columnspan=2)

select_focus = tk.Button(
    text="Select Focus",
    width=25,
    height=5,
    bg="blue",
    fg="yellow",
    command= select_focus
).grid(row=11, column=1, columnspan=2)

run_bilateral_filter_button = tk.Button(
    text="Bilateral Filter",
    width=25,
    height=5,
    bg="light blue",
    fg="yellow",
    command= run_bilateral_filter
).grid(row=11, column=3, columnspan=2)

run_parallax_button = tk.Button(
    text="Parallax",
    width=25,
    height=5,
    bg="purple",
    fg="yellow",
    command= run_parallax
).grid(row=12, column=1, columnspan=4)

depth_map_to_be_used = tk.StringVar(window, value=anisotropic_depth_map)

R0 = tk.Radiobutton(window, text="Poisson Depth Map", variable=depth_map_to_be_used, value=poisson_depth_map
                    )
R0.grid(row=8, column=0, columnspan=2)

R1 = tk.Radiobutton(window, text="Anisotropic Depth Map", variable=depth_map_to_be_used, value=anisotropic_depth_map
                  )
R1.grid(row=8, column=2, columnspan=2)

R2 = tk.Radiobutton(window, text="CNN Predicted Depth Map", variable=depth_map_to_be_used, value=predicted_depth_map,
                  )
R2.grid(row=8, column=4, columnspan=2)

R3 = tk.Radiobutton(window, text="Merged Depth Map", variable=depth_map_to_be_used, value=merged_depth_map,
                  )
R3.grid(row=9, column=0, columnspan=2)

R4 = tk.Radiobutton(window, text="SAM Depth Map", variable=depth_map_to_be_used, value=sam_depth,
                    )
R4.grid(row=9, column=2, columnspan=2)

R5 = tk.Radiobutton(window, text="Edited Depth Map", variable=depth_map_to_be_used, value=edited_depth_map,
                    )
R5.grid(row=9, column=4, columnspan=2)
window.resizable(height = None, width = None)

window.mainloop()
