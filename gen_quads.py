import numpy as np


front_quad =  [[0, 1, 1],
               [1, 1, 1],
               [1, 0, 1],
               [0, 0, 1]]

left_quad  =  [[1, 1, 0],
               [1, 1, 1],
               [1, 0, 1],
               [1, 0, 0]]

back_quad   = [[1, 1, 1],
               [0, 1, 1],
               [0, 0, 1],
               [1, 0, 1]]

right_quad  = [[1, 1, 1],
               [1, 1, 0],
               [1, 0, 0],
               [1, 0, 1]]

top_quad    = [[0, 1, 0],
               [1, 1, 0],
               [1, 1, 1],
               [0, 1, 1]]

bottom_quad = [[0, 1, 1],
               [1, 1, 1],
               [1, 1, 0],
               [0, 1, 0]]

tex_coords = [[0, 0],
              [1, 0],
              [1, 1],
              [0, 1]]

front_shade  = 166
left_shade   = 223
back_shade   = 166
right_shade  = 223
top_shade    = 255
bottom_shade = 143

scale = 64
tex_scale = 16
max_size = 8

code_template = f"""// Auto-generated
#include <nusys.h>

#define QUAD_ADDR(face, width, height) (quad_verts + ((face) * {max_size * max_size} + (width) * {max_size} + (height)) * 4)

static Vtx quad_verts[] = {{
%s
}};
"""


def write_quad(unit_quad, shade, sx, sy, sz, tx, ty):
    scaled_quad = np.array(unit_quad) * [sx, sy, sz] * scale
    scaled_tex = (np.array(tex_coords) * [tx, ty] * tex_scale) << 6
    for pos, tex in zip(scaled_quad, scaled_tex):
        line = "  {"
        line += ", ".join(f"{p:>3}" for p in pos)
        line += ",    0,    "
        line += f"{tex[0]:>4}, {tex[1]:>4}"
        line += ",    "
        line += ", ".join([f"{shade:>3}"] * 3)
        line += ", 255}"

        lines.append(line)


def generate_vertex_code(unit_quad, shade, max_sx, max_sy, max_sz, tex_x_axis, tex_y_axis, order='xyz'):
    if order == 'xyz':
        for sx in range(1, max_sx + 1):
            for sy in range(1, max_sy + 1):
                for sz in range(1, max_sz + 1):
                    tex_x = [sx, sy, sz][tex_x_axis]
                    tex_y = [sx, sy, sz][tex_y_axis]
                    write_quad(unit_quad, shade, sx, sy, sz, tex_x, tex_y)
    elif order == 'zyx':
        for sz in range(1, max_sz + 1):
            for sy in range(1, max_sy + 1):
                for sx in range(1, max_sx + 1):
                    tex_x = [sx, sy, sz][tex_x_axis]
                    tex_y = [sx, sy, sz][tex_y_axis]
                    write_quad(unit_quad, shade, sx, sy, sz, tex_x, tex_y)
    else:
        raise RuntimeError(f"Unsupported order: {order}")

lines = []

generate_vertex_code(front_quad,  front_shade,  max_size, max_size, 1, 0, 1, order='xyz')
generate_vertex_code(left_quad,   left_shade,   1, max_size, max_size, 2, 1, order='zyx')
generate_vertex_code(back_quad,   back_shade,   max_size, max_size, 1, 0, 1, order='xyz')
generate_vertex_code(right_quad,  right_shade,  1, max_size, max_size, 2, 1, order='zyx')
generate_vertex_code(top_quad,    top_shade,    max_size, 1, max_size, 0, 2)
generate_vertex_code(bottom_quad, bottom_shade, max_size, 1, max_size, 0, 2)

code = code_template % ",\n".join(lines)

with open('include/quads.h', 'w') as file:
    file.write(code)
