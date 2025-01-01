from PIL import Image
import numpy as np
from argparse import ArgumentParser
from sklearn.cluster import KMeans


code_header = """// Auto-generated
#include <nusys.h>

typedef struct {
  u32 color_indices[32];
  u16 pallet[16];
} __attribute__((aligned(8))) Texture;
"""


texture_template = """
Texture %s_texture = {
  {
    %s
  }, {
    %s
  }
};
"""


texture_specs = [
    ("dirt", 2, 0),
    ("stone",  1, 0),
    ("grass_top", 0, 0),
    ("grass_side", 3, 0),
    ("cobblestone", 0, 1),
    ("sand", 2, 1),
    ("wood_top", 5, 1),
    ("wood_side", 4, 1),
    ("leaves", 6, 1),
    ("planks", 4, 0),
    ("bricks", 7, 0)
]


def get_tile(sheet, x, y):
    return sheet[y*16 : (y+1)*16, x*16 : (x+1)*16, :]


def convert_rgba32_rgba16(rgba32):
    rgb = rgba32[:,:, :3].astype(np.float32)
    rgb *= 31 / 255
    rgb = rgb.astype(np.uint8)
    
    a = rgba32[:,:, 3].copy()
    a[a < 128] = 0
    a[a >= 128] = 1
    a = a.astype(np.uint8)
    
    return np.concatenate((rgb, a[:,:, None]), axis=2)


def convert_rgba16_rgba32(rgba16):
    rgb = rgba16[:,:, :3].astype(np.float32) * 255 / 31
    rgb = rgb.astype(np.uint8)

    a = rgba16[:,:, 3] * 255
    
    return np.concatenate((rgb, a[:,:, None]), axis=2)


def reduce_pallet(pallet):
    model = KMeans(16)
    index_map = model.fit_predict(pallet)
    reduced = np.array(model.cluster_centers_ + 0.5, dtype=np.uint16)
    return reduced, index_map


def convert_rgba16_ci(rgba16):
    pallet, ci, counts = np.unique(rgba16.reshape((-1,4)), return_inverse=True, return_counts=True, axis=0)

    if len(pallet) > 16:
        pallet, index_map = reduce_pallet(pallet)
        ci = index_map[ci]
        counts = np.array([np.sum(ci == i) for i in range(len(pallet))])

    old_inds = np.flip(counts.argsort())
    new_inds = np.empty_like(old_inds)
    new_inds[old_inds] = np.arange(len(old_inds))

    pallet = pallet[old_inds]
    ci = np.array(new_inds)[ci]
    ci = ci.reshape(rgba16.shape[:2])

    return ci, pallet


def convert_ci_rgba16(ci, pallet):
    return pallet[ci]


def pack_bytes(values, layout):
    size = sum(layout)
    if size == 16:
        dtype = np.uint16
    elif size == 32:
        dtype = np.uint32
    elif size == 64:
        dtype = np.uint64
    else:
        raise RuntimeError("Invalid layout size: %d" % size)

    offsets = np.array(size - np.cumsum(layout), dtype=np.uint64)
    shifted = np.left_shift(values.astype(dtype), offsets)
    return np.sum(shifted, axis=1)


def format_array(values, digits, row_len):
    rows = []
    for i in range(0, len(values), row_len):
        row_vals = values[i : i+row_len]
        row_str = ", ".join("0x%0*X" % (digits, v) for v in row_vals)
        rows.append(row_str)
    return ",\n".join(rows)


def convert_texture(image, name, tile_x, tile_y):
    tex32 = get_tile(image, int(tile_x), int(tile_y))
    tex16 = convert_rgba32_rgba16(tex32)
    tex_ci, pallet = convert_rgba16_ci(tex16)

    ci_packed = pack_bytes(tex_ci.reshape((-1, 8)), (4,) * 8)
    pallet_packed = pack_bytes(pallet, (5,5,5,1))

    ci_code = format_array(ci_packed, 8, 2).replace('\n', '\n    ')
    pallet_code = format_array(pallet_packed, 4, 4).replace('\n', '\n    ')

    return texture_template % (name, ci_code, pallet_code)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument('terrain')
    args = parser.parse_args()
    
    terrain = np.array(Image.open(args.terrain))

    code = code_header
    for name, tile_x, tile_y in texture_specs:
        code += convert_texture(terrain, name, tile_x, tile_y)
    
    with open("assets/texture_data.h", "w") as file:
        file.write(code)
