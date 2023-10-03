import tkinter as tk
from tkinter import filedialog as fd
from PIL import ImageTk, Image
from poisson import solve_poisson, anisotropic_solver
import os
import subprocess

SEL_IMAGE = None
SUPPORTED_FORMATS = ["JPG", "JPEG", "PNG", "jpg", "jpeg", "png"]
img = None
drawing = False
pt1_x , pt1_y = None , None
depth_annotation_windowd = None
canvas = None
out = None
scribbles = {}
THICKNESS = 2

def select_image():
    global SEL_IMAGE, img
    res = False
    while not res:
        SEL_IMAGE = fd.askopenfilename()
        try:
            img = Image.open(SEL_IMAGE)
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
                        width=3)
    for i in range(int(-THICKNESS / 2), int(THICKNESS /2)):
        for j in range(int(-THICKNESS / 2), int(THICKNESS /2)):
            scribbles[(lasy+j, lasx+i)] = depth_slider.get()
    lasx, lasy = event.x, event.y

def run():
    global depth_annotation_windowd, canvas, depth_slider, scribbles
    depth_annotation_windowd = tk.Tk()
    depth_annotation_windowd.title("Draw")
    
    canvas = tk.Canvas(depth_annotation_windowd, width=img.width, height=img.height)
    canvas.pack()
    tk_img = ImageTk.PhotoImage(image=img, master=depth_annotation_windowd)
    canvas.create_image(img.width/2, img.height/2, image=tk_img)

    canvas.bind("<Button-1>", get_x_and_y)
    canvas.bind('<B1-Motion>', draw_handler)

    depth_slider = tk.Scale(depth_annotation_windowd, from_=0, to=255, orient=tk.HORIZONTAL, label="Depth")
    depth_slider.set(0)
    depth_slider.pack()

    scribbles = {}

    depth_annotation_windowd.mainloop()

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

    arglist = ["../build/a1_hdr", "../outputs/greyscale-input.png", "../outputs/src_rgb.png", "../outputs/poisson_out.png", "../outputs/scribbles", "poisson", iterations, beta]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    print('Done')
    out = Image.open("../outputs/poisson_out.png")
    out.show()
    # solve_poisson(img, scribbles)

def run_anisotropic():
    global img, scribbles
    if not os.path.exists("../outputs"):
        os.makedirs("../outputs")
    img.save("../outputs/src_rgb.png")
    grey_img = img.convert('L')
    grey_img.save("../outputs/greyscale-input.png")

    save_scribbles()

    arglist = ["../build/a1_hdr", "../outputs/greyscale-input.png", "../outputs/src_rgb.png", "../outputs/anisotropic_out.png", "../outputs/scribbles", "anisotropic", iterations, beta]
    proc = subprocess.Popen(arglist, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print(stdout)
    print(stderr)
    proc.wait()
    print('Done')
    out = Image.open("../outputs/anisotropic_out.png")
    out.show()

def update_iter(event):
    global iterations
    iterations = event.widget.get()

def update_beta(event):
    global beta
    beta = event.widget.get()

window = tk.Tk()

#Create a button that lets to select an image file
image_button = tk.Button(
    text="Open A File",
    width=25,
    height=5,
    bg="blue",
    fg="yellow",
    command= select_image
).grid(row=0, column=0, columnspan=2)

draw_button = tk.Button(
    text="Draw",
    width=25,
    height=5,
    bg="green",
    fg="yellow",
    command= run
).grid(row=1, column=0, columnspan=2)

poisson_button = tk.Button(
    text="Poisson",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_poisson
).grid(row=2, column=0, columnspan=2)

anisotropic_button = tk.Button(
    text="Anisotropic",
    width=25,
    height=5,
    bg="red",
    fg="yellow",
    command= run_anisotropic
).grid(row=3, column=0, columnspan=2)

iter_label = tk.Label(text="Number of iterations").grid(row=4, column=0)
n_of_iter = tk.Entry()
n_of_iter.bind("<Return>", update_iter)
n_of_iter.grid(row=4, column=1)

beta_label = tk.Label(text="Beta").grid(row=5, column=0)
beta_entry = tk.Entry()
beta_entry.bind("<Return>", update_beta)
beta_entry.grid(row=5, column=1)

window.mainloop()
