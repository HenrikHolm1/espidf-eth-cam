#!/usr/bin/python3
import argparse
from argparse import RawTextHelpFormatter
import struct
import os
from PIL import Image
from enum import Enum

class Mode(Enum):
    BIN = ".bin"
    XXX = ".xxx"

example_text = """Examples:
* Convert png file to rgb565 in little endian format  *
* Convert bin in little endian format to png with the *
* image size given in the args                        *
python conv.py -i f1.png -o f1.bin
python conv.py -s 60 62 -i f1.bin -o f1x.png

* Convert png file to rgb565 in big endian format     *
* Convert bin in big endian format to png             *
python conv.py -b -i f1.png -o f1.bin
python conv.py -b -s 60 62 -i f1.bin -o f1x.png

* Convert png file and add the png file size as       *
* (width, height) as uint16 values in big endian      *
* format                                              *
* Convert bin to png use the size given in the bin    *
* file                                                *
python conv.py -b -u -i f1.png -o f1.bin
python conv.py -u -b -i f1.bin -o f1x.png

* Convert jpg to rs565 bin file                       *
* Convert the bin to either jpg or png                *
python conv.py -u -i f2.jpg -o f2.bin
python conv.py -u -i f2.bin -o f2x.jpg
python conv.py -u -i f2.bin -o f2x.png
"""

def main():
    parser = argparse.ArgumentParser(
        description="Image format converter.",
        formatter_class=RawTextHelpFormatter,
        epilog=example_text
    )
    parser.add_argument(
        "-i",
        "--input",
        required=True,
        dest="InputFile",
        help="Input file to be converted."
    )
    parser.add_argument(
        "-o",
        "--output",
        required=True,
        dest="OutputFile",
        help="Output file."
    )
    parser.add_argument(
        "-b",
        "--bigendian",
        action='store_true',
        dest="b",
        default=argparse.SUPPRESS,
        help="Send output as big endian if binary file is selected, ignored otherwise."
    )
    parser.add_argument(
        "-u",
        "--usesize",
        action='store_true',
        dest="u",
        default=argparse.SUPPRESS,
        help="Has two functionalities:\n"
             " * Input file is binary: Use the width and height given in the file.\n"
             " * Output file is binary: Embed input file size (width,height)\n"
             "   at the start of the bin file as uint16 values in big endian format."
    )
    parser.add_argument(
        "-s",
        "--size",
        nargs=2,
        type=int,
        metavar=('width', 'height'),
        dest="s",
        help="If input file is binary then use this size on the new Image.\n"
             "(Is overruled if the -u arg is present)."
    )



    args = parser.parse_args()

    in_path = os.path.basename(args.InputFile).rsplit('.', 1)

    mode = Mode.BIN if (in_path[1] == 'bin') else Mode.XXX

    out_path = os.path.basename(args.OutputFile).rsplit('.', 1)

    if len(out_path) != 2:
        print("Error: Invalid arguments.")
        exit(1)

    if (in_path[1] not in ['png', 'jpg', 'jpeg', 'bin']):
        print("Error: Input file must be a .png, .jpg or .bin file.")
        exit(1)

    if (out_path[1] not in ['png', 'jpg', 'jpeg', 'bin']):
        print("Error: Output file must be a .png, .jpg or .bin file.")
        exit(1)

    if (in_path[1] == out_path[1]):
        print("Error: Input and output file must be different.")
        exit(1)

    if args.s:
        if len(args.s) != 2:
            print("Error: Size must be given with both image Width and Height values.")
            exit(1)

    if mode == Mode.BIN:
        convert_rgb565_to_png(args)
    else:
        convert_png_to_rgb565(args)

def convert_png_to_rgb565(args):
    im = Image.open(args.InputFile)
    width, height = im.size
    bo = 'little'
    if hasattr(args, 'b'):
        bo = 'big'
    print("Input file: Width: " + str(width) + ", Height: " + str(height))

    # iterate over the pixels
    with open(args.OutputFile, 'wb') as f:
        if hasattr(args, 'u'):
            f.write((width).to_bytes(2, byteorder='big', signed=False))
            f.write((height).to_bytes(2, byteorder='big', signed=False))
        for i, pixel in enumerate(im.getdata()):
            r = (pixel[0] >> 3) & 0x1F
            g = (pixel[1] >> 2) & 0x3F
            b = (pixel[2] >> 3) & 0x1F
            rgb = r << 11 | g << 5 | b
            #rgb = 0xf800
            #gggbbbbb rrrrrggg
            #rgb = (rgb >> 8) | ((rgb & 0xFF) << 8)
            #f.write((rgb).to_bytes(2, byteorder="little"))
            f.write((rgb).to_bytes(2, byteorder=bo, signed=False))

def convert_rgb565_to_png(args):
    with open(args.InputFile, 'rb') as f:
        if args.s:
            width, height = args.s
        if hasattr(args, 'u'):
            width  = int.from_bytes(f.read(2), byteorder='big')
            height = int.from_bytes(f.read(2), byteorder='big')
        data = f.read()
        a = struct.unpack("H" * ((len(data)) // 2), data)
        im = Image.new('RGB', (width, height))
        for i, word in enumerate(a):
            if hasattr(args, 'b'):
                word = word >> 8 | word << 8
            r = (word >> 11) & 0x1F
            g = (word >> 5) & 0x3F
            b = (word) & 0x1F
            im.putpixel((i % width, i // width), (r << 3, g << 2, b << 3))
        im.save(args.OutputFile)

if __name__ == '__main__':
    main()
