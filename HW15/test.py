import time
from displayio import (
    Bitmap,
    Group,
    TileGrid,
    FourWire,
    release_displays,
    ColorConverter,
    Colorspace,
)
#from adafruit_st7789 import ST7789
from adafruit_st7735r import ST7735R
import board
import busio
import digitalio
from adafruit_ov7670 import (
    OV7670,
    OV7670_SIZE_DIV1,
    OV7670_SIZE_DIV16,
)

# Set up the display (You must customize this block for your display!)
release_displays()
spi = busio.SPI(clock=board.GP2, MOSI=board.GP3)
display_bus = FourWire(spi, command=board.GP0, chip_select=board.GP1, reset=None)
#display = ST7789(display_bus, width=320, height=240, rotation=270)
display = ST7735R(display_bus, width=128, height=128, colstart=2, rowstart=1, rotation=90)


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

# cam.test_pattern = OV7670_TEST_PATTERN_COLOR_BAR

bitmap = None
# Select the biggest size for which we can allocate a bitmap successfully, and
# which is not bigger than the display
for size in range(OV7670_SIZE_DIV1, OV7670_SIZE_DIV16 + 1):
    cam.size = size
    if cam.width > width:
        continue
    if cam.height > height:
        continue
    try:
        bitmap = Bitmap(cam.width, cam.height, 65536)
        break
    except MemoryError:
        continue

print(width, height, cam.width, cam.height)
if bitmap is None:
    raise SystemExit("Could not allocate a bitmap")
time.sleep(4)
g = Group(scale=1, x=(width - cam.width) // 2, y=(height - cam.height) // 2)
tg = TileGrid(
    bitmap, pixel_shader=ColorConverter(input_colorspace=Colorspace.RGB565_SWAPPED)
)
g.append(tg)
display.show(g)

def find_line(bitmap_in):
    line_top = 0
    line_bottom = 0

    #iterate through the top few lines of the image
    for y in range(bitmap_in.height)[:5]:
        line_start = None
        line_end = None

        for x in range(bitmap_in.width):
            # Get the color of the pixel at (x, y)
            pixel_color = bitmap_in[x, y]
            
            # Compare grayscale with white and black thresholds
            black_threshold = 150   # Adjust this threshold as needed
            if pixel_color < black_threshold:
                if line_start is None:
                    # Start of the line
                    line_start = x
                # Update the end position continuously
                line_end = x
            elif pixel_color >= black_threshold:
                if line_start is not None and line_end is not None:
                    # End of the line
                    break

        if line_start is not None and line_end is not None:
            # Calculate center point between the edges
            line_top = line_top + (line_start + line_end) // 2
    line_top = line_top // 5
    
    #Do the same for the bottom part of the line
    for y in range(bitmap_in.height)[-5:]:
        line_start = None
        line_end = None

        for x in range(bitmap_in.width):
            # Get the color of the pixel at (x, y)
            pixel_color = bitmap_in[x, y]
            
            # Compare grayscale with white and black thresholds
            black_threshold = 150   # Adjust this threshold as needed
            if pixel_color < black_threshold:
                if line_start is None:
                    # Start of the line
                    line_start = x
                # Update the end position continuously
                line_end = x
            elif pixel_color >= black_threshold:
                if line_start is not None and line_end is not None:
                    # End of the line
                    break

        if line_start is not None and line_end is not None:
            # Calculate center point between the edges
            line_bottom = line_bottom + (line_start + line_end) // 2
    line_bottom = line_bottom // 5
    
    # Return the list of line centers
    return (line_top, line_bottom)
            
t0 = time.monotonic_ns()
display.auto_refresh = False
while True:
    cam.capture(bitmap)
    bitmap.dirty()
    line_top, line_bottom = find_line(bitmap)
    display.refresh(minimum_frames_per_second=0)
    t1 = time.monotonic_ns()
    print("fps", 1e9 / (t1 - t0))
    t0 = t1