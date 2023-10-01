import tkinter as tk
from tkinter import filedialog as fd
from PIL import ImageTk, Image
import cv2

SEL_IMAGE = None
SUPPORTED_FORMATS = ["JPG", "JPEG", "PNG", "jpg", "jpeg", "png"]
img = None
drawing = False
pt1_x , pt1_y = None , None
depth_annotation_windowd = None
canvas = None

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
    #print('hello')
    global lasx, lasy
    canvas.create_line((lasx, lasy, event.x, event.y), 
                        fill=color, 
                        width=3)
    lasx, lasy = event.x, event.y

def run():
    global depth_annotation_windowd, canvas, depth_slider
    depth_annotation_windowd = tk.Tk()
    depth_annotation_windowd.title("Draw")
    
    canvas = tk.Canvas(depth_annotation_windowd, width=600, height=300)
    canvas.pack()
    tk_img = ImageTk.PhotoImage(image=img, master=depth_annotation_windowd)
    canvas.create_image(300, 150, image=tk_img)

    canvas.bind("<Button-1>", get_x_and_y)
    canvas.bind('<B1-Motion>', draw_handler)

    depth_slider = tk.Scale(depth_annotation_windowd, from_=0, to=255, orient=tk.HORIZONTAL, label="Depth")
    depth_slider.set(0)
    depth_slider.pack()

    depth_annotation_windowd.mainloop()

window = tk.Tk()

greeting = tk.Label(text="Hello, Tkinter")
greeting.pack()

#Create a button that lets to select an image file
image_button = tk.Button(
    text="Open A File",
    width=25,
    height=5,
    bg="blue",
    fg="yellow",
    command= select_image
)
image_button.pack()

start_button = tk.Button(
    text="Start",
    width=25,
    height=5,
    bg="green",
    fg="yellow",
    command= run
)
start_button.pack()

window.mainloop()
