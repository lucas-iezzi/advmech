import busio
import digitalio
import time
from displayio import (
    Bitmap,
    Palette,
    Group,
    TileGrid,
    FourWire,
    release_displays,
    ColorConverter,
    Colorspace,
)
import board
from adafruit_imageload import png, load

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

image, palette = load("testline.bmp", bitmap=Bitmap)

line_list = find_line(image)
print(line_list)