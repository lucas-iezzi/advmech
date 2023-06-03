# requires adafruit_ov7670.mpy and adafruit_st7735r.mpy in the lib folder
import time
from displayio import (
    Bitmap,
    Group,
    TileGrid,
    FourWire,
    release_displays,
    ColorConverter,
    Colorspace,
    Palette,
)
from adafruit_st7735r import ST7735R
import board
import busio
import digitalio
from adafruit_ov7670 import (
    OV7670,
    OV7670_SIZE_DIV1,
    OV7670_SIZE_DIV8,
    OV7670_SIZE_DIV16,
)
from ulab import numpy as np

release_displays()
spi = busio.SPI(clock=board.GP2, MOSI=board.GP3)
display_bus = FourWire(spi, command=board.GP0, chip_select=board.GP1, reset=None)
display = ST7735R(display_bus, width=160, height=128, rotation=90)

# Ensure the camera is shut down, so that it releases the SDA/SCL lines,
# then create the configuration I2C bus

with digitalio.DigitalInOut(board.GP10) as reset:
    reset.switch_to_output(False)
    time.sleep(0.001)
    bus = busio.I2C(board.GP9, board.GP8) #GP9 is SCL, GP8 is SDA

# Set up the camera (you must customize this for your board!)
cam = OV7670(
    bus,
    data_pins=[
        board.GP12,
        board.GP13,
        board.GP14,
        board.GP15,
        board.GP16,
        board.GP17,
        board.GP18,
        board.GP19,
    ],  # [16]     [org] etc
    clock=board.GP11,  # [15]     [blk]
    vsync=board.GP7,  # [10]     [brn]
    href=board.GP21,  # [27/o14] [red]
    mclk=board.GP20,  # [16/o15]
    shutdown=None,
    reset=board.GP10,
)  # [14]

width = display.width
height = display.height

bitmap = None
# Select the biggest size for which we can allocate a bitmap successfully, and
# which is not bigger than the display
for size in range(OV7670_SIZE_DIV1, OV7670_SIZE_DIV16 + 1):
    #cam.size = size # for 4Hz
    #cam.size = OV7670_SIZE_DIV16 # for 30x40, 9Hz
    cam.size = OV7670_SIZE_DIV8 # for 60x80, 9Hz
    if cam.width > width:
        continue
    if cam.height > height:
        continue
    try:
        bitmap = Bitmap(cam.width, cam.height, 65535)
        break
    except MemoryError:
        continue

print(width, height, cam.width, cam.height)
if bitmap is None:
    raise SystemExit("Could not allocate a bitmap")
time.sleep(4)
g = Group(scale=1, x=(width - cam.width) // 2, y=(height - cam.height) // 2)
tg = TileGrid(
    bitmap, pixel_shader=ColorConverter(input_colorspace=Colorspace.BGR565_SWAPPED)
)
g.append(tg)
display.show(g)

t0 = time.monotonic_ns()
display.auto_refresh = False

red_cal = [0,0]
green_cal = [0,0]
blue_cal = [0,0]

#calibrate background
cam.capture(bitmap)
bitmap.dirty() # updae the image on the screen
display.refresh(minimum_frames_per_second=0)

# calculate the colors
red_cals = []
green_cals = []
blue_cals = []
row = 10
for i in range(0,60):
    red_cals.append(((bitmap[row,i]>>5)&0x3F)/0x3F*100)
    green_cals.append(((bitmap[row,i])&0x1F)/0x1F*100)
    blue_cals.append((bitmap[row,i]>>11)/0x1F*100)
red_cal[0] = sum(red_cals) / len(red_cals)
green_cal[0] = sum(green_cals) / len(green_cals)
blue_cal[0] = sum(blue_cals) / len(blue_cals)
#calibrate bottom
red_cals = []
green_cals = []
blue_cals = []
row = 70
for i in range(0,60):
    red_cals.append(((bitmap[row,i]>>5)&0x3F)/0x3F*100)
    green_cals.append(((bitmap[row,i])&0x1F)/0x1F*100)
    blue_cals.append((bitmap[row,i]>>11)/0x1F*100)
red_cal[1] = sum(red_cals) / len(red_cals)
green_cal[1] = sum(green_cals) / len(green_cals)
blue_cal[1] = sum(blue_cals) / len(blue_cals)

print("red:" + str(red_cal) + "green:" + str(green_cal) + "blue:" + str(blue_cal))

# arrays to store the color data
#reds = np.zeros(60,dtype=np.uint16)
#greens = np.zeros(60,dtype=np.uint16)
#blues = np.zeros(60,dtype=np.uint16)
#bright = np.zeros(60)
#top = np.zeros(60,dtype=np.uint16)
#bottom = np.zeros(60,dtype=np.uint16)

uart = busio.UART(board.GP4, board.GP5, baudrate=230400, timeout=0)

while True:
    # wait for a newline from the computer
    #input()
    byte_read = uart.read(100)  # Read one byte over UART lines

    cam.capture(bitmap)
    top = []
    bottom = []
    # colors:
    #0x1F ->   0b0000000000011111 # all green
    #0x7E0 ->  0b0000011111100000 # all red
    #0xF800 -> 0b1111100000000000 # all blue

    row = 10 # which row to send to the computer
    # calculate the colors
    for i in range(0,60):
        red = ((bitmap[row,i]>>5)&0x3F)/0x3F*100
        green = ((bitmap[row,i])&0x1F)/0x1F*100
        blue = (bitmap[row,i]>>11)/0x1F*100

        if (red<(red_cal[0]*1.6) and red>(red_cal[0]*0.4)) and (green<(green_cal[0]*1.6) and green>(green_cal[0]*0.4)) and (blue<(blue_cal[0]*1.6) and blue>(blue_cal[0]*0.4)):
            bitmap[row,i] = 0b0000000000000000
            bottom.append(0)
        else:
            bitmap[row,i] = 0b1111111111111111
            bottom.append(1)
        #print("red:" + str(red) + "green:" + str(green) + "blue:" + str(blue))

    row = 70 # which row to send to the computer
    # calculate the colors
    for i in range(0,60):
        red = ((bitmap[row,i]>>5)&0x3F)/0x3F*100
        green = ((bitmap[row,i])&0x1F)/0x1F*100
        blue = (bitmap[row,i]>>11)/0x1F*100

        if (red<(red_cal[1]*1.6) and red>(red_cal[1]*0.4)) and (green<(green_cal[1]*1.6) and green>(green_cal[1]*0.4)) and (blue<(blue_cal[1]*1.6) and blue>(blue_cal[1]*0.4)):
            bitmap[row,i] = 0b0000000000000000
            top.append(0)
        else:
            bitmap[row,i] = 0b1111111111111111
            top.append(1)
    #print(top)
    #print(bottom)

    top_center = 0
    for i in range(len(top)):
        top_center = top_center + top[i]*i
    try:
        top_center = top_center / sum(top)
    except:
        top_center = 0

    bottom_center = 0
    for i in range(len(bottom)):
        bottom_center = bottom_center + bottom[i]*i
    try:
        bottom_center = bottom_center / sum(bottom)
    except:
        bottom_center = 0

    #print(top_center)
    #print(bottom_center)
    # print the raw pixel value to the computer
    #for i in range(0,60):
    #    print(str(i)+" "+str(bitmap[row,i]))
    
    if not byte_read==None:
        print(byte_read)
        heading = str((30 - bottom_center) / 30) + "\n\r"
        print(heading)
        uart.write(bytearray(heading))

    bitmap.dirty() # updae the image on the screen
    display.refresh(minimum_frames_per_second=0)
    t1 = time.monotonic_ns()
    #print("fps", 1e9 / (t1 - t0))
    t0 = t1
