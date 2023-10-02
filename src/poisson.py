from PIL import ImageTk, Image
import numpy as np
import multiprocessing

def copy_pixels(pxl, width, height):
    res = np.zeros((width, height))
    for i in range(width):
        for j in range(height):
            res[i][j] = pxl[(i, j)]
    return res

def pixel_coeff(x_1,y_1, x_2, y_2, ill, beta=1):
    ill_1 = ill[x_1][y_1] if x_1 >= 0 and y_1 >= 0 and x_1 < len(ill) and y_1 < len(ill[x_1]) else 0
    ill_2 = ill[x_2][y_2] if x_2 >= 0 and y_2 >= 0 and x_2 < len(ill) and y_2 < len(ill[x_2]) else 0
    return np.exp(-beta * abs(ill_1 - ill_2)/255)

def poisson_update(x, y, scribbles, res):
    if (x,y) in scribbles:
        return x,y, scribbles[(x,y)]
    prv_x = res[x-1][y] if x > 0 else 0
    nxt_x = res[x+1][y] if x < len(res)-1 else 0
    prv_y = res[x][y-1] if y > 0 else 0
    nxt_y = res[x][y+1] if y < len(res[x])-1 else 0
    return x, y, (prv_x + nxt_x + prv_y + nxt_y) / 4

def solve_poisson(img, scribbles, iterations=500, tolerance=1e-3):
    res = img.load()

    for key in scribbles:
        res[key[0], key[1]] = scribbles[key]
    res = copy_pixels(res, img.width, img.height)
    for _ in range(iterations):
        res_next = res.copy()
        with multiprocessing.Pool() as pool:
            items = [(x, y, scribbles, res) for x in range(img.width) for y in range(img.height)]
            for results in pool.starmap(poisson_update,items):
                res_next[results[0]][results[1]] = results[2]


        # for x in range(img.width):
        #     for y in range(img.height):
        #         if (x,y) in scribbles:
        #             continue
        #         prv_x = res[x-1][y] if x > 0 else 0
        #         nxt_x = res[x+1][y] if x < len(res)-1 else 0
        #         prv_y = res[x][y-1] if y > 0 else 0
        #         nxt_y = res[x][y+1] if y < len(res[x])-1 else 0
        #         res_next[x][y] = (prv_x + nxt_x + prv_y + nxt_y) / 4
        res = res_next.copy()
        if _ % 10 == 0:
            print(_)
    
    im = Image.fromarray(res)
    im.show()
    return res

def anisotropic_list():
    pass

def anisotropic_update(x, y, scribbles, res, original):
        if (x,y) in scribbles:
            return x, y, scribbles[(x,y)]
        prv_x = res[x-1][y] if x > 0 else 0
        nxt_x = res[x+1][y] if x < len(res)-1 else 0
        prv_y = res[x][y-1] if y > 0 else 0
        nxt_y = res[x][y+1] if y < len(res[x])-1 else 0
        w_prv_x = pixel_coeff(x, y, x-1, y, original)
        w_prv_y = pixel_coeff(x, y, x, y-1, original)
        w_nxt_x = pixel_coeff(x, y, x+1, y, original)
        w_nxt_y = pixel_coeff(x, y, x, y+1, original)
        return x, y, (w_prv_x*prv_x + w_nxt_x*nxt_x + w_prv_y*prv_y + w_nxt_y*nxt_y) / (w_prv_x + w_nxt_x + w_prv_y + w_nxt_y)
        
def anisotropic_solver(img, scribbles, iterations=2000, tolerance=1e-3):
    res = img.load()

    for key in scribbles:
        res[key[0], key[1]] = scribbles[key]
    res = copy_pixels(res, img.width, img.height)
    original = res.copy()
    for _ in range(iterations):
        res_next = res.copy()
        with multiprocessing.Pool() as pool:
            items = [(x, y, scribbles, res, original) for x in range(img.width) for y in range(img.height)]
            for results in pool.starmap(anisotropic_update,items):
                res_next[results[0]][results[1]] = results[2]

        # for x in range(img.width):
        #     for y in range(img.height):
        #         if (x,y) in scribbles:
        #             continue
        #         prv_x = res[x-1][y] if x > 0 else 0
        #         nxt_x = res[x+1][y] if x < len(res)-1 else 0
        #         prv_y = res[x][y-1] if y > 0 else 0
        #         nxt_y = res[x][y+1] if y < len(res[x])-1 else 0
        #         w_prv_x = pixel_coeff(x, y, x-1, y, original)
        #         w_prv_y = pixel_coeff(x, y, x, y-1, original)
        #         w_nxt_x = pixel_coeff(x, y, x+1, y, original)
        #         w_nxt_y = pixel_coeff(x, y, x, y+1, original)
        #         res_next[x][y] = (w_prv_x*prv_x + w_nxt_x*nxt_x + w_prv_y*prv_y + w_nxt_y*nxt_y) / (w_prv_x + w_nxt_x + w_prv_y + w_nxt_y)
        res = res_next.copy()
        if _ % 10 == 0:
            print(_)
    
    im = Image.fromarray(res)
    im.show()
    return res