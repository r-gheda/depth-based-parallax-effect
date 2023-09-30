import tkinter as tk
from tkinter import filedialog as fd
from PIL import ImageTk, Image
import cv2

SEL_IMAGE = None
SUPPORTED_FORMATS = ["JPG", "JPEG", "PNG", "jpg", "jpeg", "png"]
img = None
drawing = False
pt1_x , pt1_y = None , None

def select_image():
    res = False
    while not res:
        global SEL_IMAGE
        SEL_IMAGE = fd.askopenfilename()
        try:
            img = Image.open(SEL_IMAGE)
        except IOError:
            print("Invalid file")
            continue
        if not img.format in SUPPORTED_FORMATS:
            print("Unsupported format")
            print(SEL_IMAGE.format)
        else:
            res = True
    
def run():
    img = cv2.imread("image.jpg")
    cv2.namedWindow('test draw')
    cv2.setMouseCallback('test draw',line_drawing)

    while(1):
        cv2.imshow('test draw',img)
        if cv2.waitKey(1) & 0xFF == 27:
            break
    cv2.destroyAllWindows()

def line_drawing(event,x,y,flags,param):
    global pt1_x,pt1_y,drawing
    if event==cv2.EVENT_LBUTTONDOWN:
        drawing=True
        pt1_x,pt1_y=x,y

    elif event==cv2.EVENT_MOUSEMOVE:
        if drawing==True:
            cv2.line(img,(pt1_x,pt1_y),(x,y),color=(255,0,0),thickness=3)
            pt1_x,pt1_y=x,y
    elif event==cv2.EVENT_LBUTTONUP:
        drawing=False
        cv2.line(img,(pt1_x,pt1_y),(x,y),color=(255,0,0),thickness=3)        

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
