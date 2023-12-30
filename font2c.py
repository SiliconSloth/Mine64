from PIL import Image
import numpy as np
from argparse import ArgumentParser

from tex2c import pack_bytes, format_array


code_template = """// Auto-generated
#include <nusys.h>

u32 font_texture[] __attribute__((aligned(8))) = {
  %s
};
"""

parser = ArgumentParser()
parser.add_argument('bitmap')
args = parser.parse_args()

img = np.array(Image.open(args.bitmap))[:64, :, 0]
img = np.minimum(img, 15)

packed = pack_bytes(img.reshape(-1, 8), (4,) * 8)
code = format_array(packed, 8, 16)

with open('assets/font.h', 'w') as file:
    file.write(code_template % code.replace('\n', '\n  '))
