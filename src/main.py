import tkinter as tk
from tkinter import ttk
from tkinter import filedialog as fd
from PIL import ImageTk, Image
import os
import subprocess
import dist_depth

SEL_IMAGE = None
SUPPORTED_FORMATS = ["JPG", "JPEG", "PNG", "jpg", "jpeg", "png"]
img = None
img_path = None
drawing = False
pt1_x , pt1_y = None , None
depth_annotation_window = None
canvas = None
out = None
scribbles = {}
depth_map = "depth_map.png"
focused_image = "focused_image.png"
preficted_depth_map = "predicted_depth.png"
computed_depth_map_loaded = False
scribble_loaded = False
predicted_depth_map_loaded = False
focus_x = None
focus_y = None
iterations = 1000
beta = 20
aperture_size = 15

def select_image():
    global SEL_IMAGE, img, img_path
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
            print("Unsupported format")
            print(img.format)
        else:
            res = True    

def _from_rgb(rgb):
    """translates an rgb tuple of int to a tkinter friendly color code
    """
    return "#%02x%02x%02x" % rgb   

def get_x_and_y(event):
    global lasx, lasy, color
    lasx, lasy = event.x, event.y
    color = _from_rgb((depth_slider.get(), depth_slider.get(), depth_slider.get()))

def draw_handler(event):
    global lasx, lasy
    canvas.create_line((lasx, lasy, event.x, event.y), 
                        fill=color, 
                        width=thickness_slider.get())
    for i in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
        for j in range(int(-thickness_slider.get() / 2), int(thickness_slider.get() /2)):
            scribbles[(lasy+j, lasx+i)] = depth_slider.get()
    lasx, lasy = event.x, event.y

def update_focus_point(event):
    global focus_x, focus_y
    focus_x, focus_y = event.x, event.y

def draw_annotations_callback():
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

    scribbles = {}

    depth_annotation_window.mainloop()

def save_scribbles():
    if os.path.exists("../outputs/scribbles"):
        os.remove("../outputs/scribbles")
    
    with open("../outputs/scribbles", "w") as f:
        for key, value in scribbles.items():
            f.write(str(key[0]) + " " + str(key[1]) + " " + str(value) + "\n")

def run_poisson():
    global img, scribbles
    if not os.path.exists("../outputs"):
        os.makedirs("../outputs")
    img.save("../outputs/src_rgb.png")
    grey_img = img.convert('L')
    grey_img.save("../outputs/greyscale-input.png")

    save_scribbles()
    print(iterations)

    arglist = ["../build/poisson", "../outputs/greyscale-input.png", "../outputs/src_rgb.png", "../outputs/" + str(depth_map), "../outputs/scribbles", "poisson", iterations, beta]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    print('Done')
    out = Image.open("../outputs/poisson_out.png")
    out.show()

def run_anisotropic():
    global img, scribbles, depth_map
    if not os.path.exists("../outputs"):
        os.makedirs("../outputs")
    if os.path.exists("../outputs/" + str(depth_map)):
        os.remove("../outputs/" + str(depth_map))
    img.save("../outputs/src_rgb.png")
    grey_img = img.convert('L')
    grey_img.save("../outputs/greyscale-input.png")

    save_scribbles()
    arglist = ["../build/poisson", "../outputs/greyscale-input.png", "../outputs/src_rgb.png", "../outputs/" + str(depth_map), "../outputs/scribbles", "anisotropic", iterations, beta]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    print('Done')
    out = Image.open("../outputs/" + str(depth_map))
    out.show()

def update_iter(event):
    global iterations
    iterations = event.widget.get()

def update_beta(event):
    global beta
    beta = event.widget.get()

def update_aperture_size(event):
    global aperture_size
    aperture_size = event.widget.get()

def select_focus():
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
    global img, depth_map, focus_x, focus_y, aperture_size, focused_image

    arglist = ["../build/bilateral_filter", "../outputs/src_rgb.png", "../outputs/" +  str(predicted_depth_map), "../outputs/" + str(focused_image), str(focus_x), str(focus_y), aperture_size]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    out = Image.open("../outputs/" + str(focused_image))
    out.show()
    pass

def run_cnn():
    global predicted_depth_map
    predicted_depth_map = "predicted_depth.png"
    arglist = ["python3", "../src/dist_depth/run_rgb_cnn.py", img_path]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    out = Image.open("../outputs/predicted_depth.png")
    out.show()

def apply_sam_mask(event):
    global im, im_bak, tk_img, canvas, sam_root
    im = im_bak.copy()
    print(event.widget.get())
    mask = Image.open(event.widget.get())
    for i in range(mask.width):
        for j in range(mask.height):
            if mask.getpixel((i,j)) == 0:
                im.putpixel((i,j), (0,0,0))
    tk_img = ImageTk.PhotoImage(image=im, master=sam_root)
    canvas.create_image(im.width/2, im.height/2, image=tk_img)



def run_sam():
    global img_path, im, im_bak, tk_img, canvas, sam_root
    # arglist = ["python3", "segment-anything/scripts/amg.py",'--checkpoint', '../models/sam_vit_h_4b8939.pth','--model', 'vit_h', '--input', img_path, '--output', '../outputs/sam-out', '--device', 'cpu']
    # proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # stdout, stderr = proc.communicate()
    # print(stdout)
    # print(stderr)
    # proc.wait()

    im = Image.open(img_path)
    im_bak = im.copy()
    print(img_path)
    sam_root = tk.Tk()
    canvas = tk.Canvas(sam_root, width=im.width, height=im.height)
    canvas.grid(row=0, columnspan=2)

    tk_img = ImageTk.PhotoImage(image=im, master=sam_root)
    canvas.create_image(im.width/2, im.height/2, image=tk_img)
    os.chdir("../outputs/sam-out/montenegro")
    files = [f for f in os.listdir() if os.path.isfile(f)]
    
    Combo = ttk.Combobox(sam_root, values = files)
    Combo.set("Pick a SAM mask")
    Combo.grid(row=1, columnspan=2)

    Combo.bind("<<ComboboxSelected>>", apply_sam_mask)
    
    sam_root.mainloop()

window = tk.Tk()

#Create a button that lets to select an image file
image_button = tk.Button(
    text="Open A File",
    width=25,
    height=5,
    bg="blue",
    fg="yellow",
    command= select_image
).grid(row=0, column=0, columnspan=4)

draw_button = tk.Button(
    text="Draw Scribbles",
    width=25,
    height=5,
    bg="green",
    fg="yellow",
    command= draw_annotations_callback
).grid(row=1, column=0, columnspan=2)

load_scribbles_button = tk.Button(
    text="Load Scribbles",
    width=25,
    height=5,
    bg="green",
    fg="yellow",
    command= draw_annotations_callback
).grid(row=1, column=2, columnspan=2)

iter_label = tk.Label(text="Number of iterations").grid(row=2, column=0)
n_of_iter = tk.Entry()
n_of_iter.bind("<Return>", update_iter)
n_of_iter.grid(row=2, column=1)

beta_label = tk.Label(text="Beta").grid(row=3, column=0)
beta_entry = tk.Entry()
beta_entry.bind("<Return>", update_beta)
beta_entry.grid(row=3, column=1)

poisson_button = tk.Button(
    text="Poisson",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_poisson
).grid(row=4, column=0, columnspan=2)

anisotropic_button = tk.Button(
    text="Anisotropic",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_anisotropic
).grid(row=4, column=2, columnspan=2)


run_cnn_button = tk.Button(
    text="Run CNN",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_cnn
).grid(row=5, column=0, columnspan=2)

merge_depth_maps_button = tk.Button(
    text="Merge Depth Maps",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_cnn
).grid(row=5, column=2, columnspan=2)

run_sam_button = tk.Button(
    text="Run SAM",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_sam
).grid(row=6, column=0, columnspan=4)

aperture_size_label = tk.Label(text="Aperture Size").grid(row=8, column=0)
aperture_size = tk.Entry()
aperture_size.bind("<Return>", update_aperture_size)
aperture_size.grid(row=8, column=1)

select_focus = tk.Button(
    text="Select Focus",
    width=25,
    height=5,
    bg="blue",
    fg="yellow",
    command= select_focus
).grid(row=9, column=0, columnspan=2)

run_bilateral_filter_button = tk.Button(
    text="Bilateral Filter",
    width=25,
    height=5,
    bg="light blue",
    fg="yellow",
    command= run_bilateral_filter
).grid(row=9, column=2, columnspan=2)

run_parallax_button = tk.Button(
    text="Parallax",
    width=25,
    height=5,
    bg="purple",
    fg="yellow",
    command= run_bilateral_filter
).grid(row=10, column=1, columnspan=4)

file_open_label = tk.Label(text="File Opened: " + str(SEL_IMAGE)).grid(row=0, column=5, columnspan=2)
scribbles_status_label = tk.Label(text="Scibbles: " + str(scribble_loaded)).grid(row=1, column=5, columnspan=2)
computed_depth_map_status_label = tk.Label(text="Computed Depth Map: " + str(computed_depth_map_loaded)).grid(row=4, column=5, columnspan=2)
predicted_depth_map_status_label = tk.Label(text="Predicted Depth Map: " + str(predicted_depth_map_loaded)).grid(row=5, column=5, columnspan=2)
number_of_iterations_label = tk.Label(text="Number of iterations: " + str(iterations)).grid(row=2, column=5, columnspan=2)
beta_label = tk.Label(text="Beta: " + str(beta)).grid(row=3, column=5, columnspan=2)
selected_focus_label = tk.Label(text="Selected Focus: (" + str(focus_x) + ", " + str(focus_y) + ")").grid(row=9, column=5, columnspan=2)
selected_aperture_size_label = tk.Label(text="Aperture Size: " + str(aperture_size)).grid(row=8, column=5, columnspan=2)

depth_map_to_be_used = None

R1 = tk.Radiobutton(window, text="Computed Depth Map", variable=depth_map_to_be_used, value=1,
                  )
R1.grid(row=7, column=0, columnspan=2)

R2 = tk.Radiobutton(window, text="CNN Predicted Depth Map", variable=depth_map_to_be_used, value=2,
                  )
R2.grid(row=7, column=1, columnspan=2)

R3 = tk.Radiobutton(window, text="Merged Depth Map", variable=depth_map_to_be_used, value=3,
                  )
R3.grid(row=7, column=2, columnspan=2)


window.mainloop()
