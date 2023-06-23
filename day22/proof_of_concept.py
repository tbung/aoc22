import numpy as np
from enum import Enum
from copy import deepcopy

# FULL = 4
# HALF = FULL // 2 - 0.5
# FILE = "test_in"

FULL = 50
HALF = FULL // 2 - 0.5
FILE = "input"


# relative to page
class Direction(Enum):
    RIGHT = 0
    DOWN = 1
    LEFT = 2
    UP = 3


dir_char = [
    "→",
    "↓",
    "←",
    "↑",
]

cube_normals = [
    [0, 0, 1],
    [0, 1, 0],
    [1, 0, 0],
    [-1, 0, 0],
    [0, -1, 0],
    [0, 0, -1],
]


def rotate(vector, axis):
    R = np.array(
        [
            [axis[0] ** 2, axis[0] * axis[1] - axis[2], axis[0] * axis[2] + axis[1]],
            [axis[1] * axis[0] + axis[2], axis[1] ** 2, axis[1] * axis[2] - axis[0]],
            [axis[2] * axis[0] - axis[1], axis[2] * axis[1] + axis[0], axis[2] ** 2],
        ]
    )

    return R @ vector


def hash(normal):
    temp = 3 * normal[0] + 2 * normal[1] + normal[2]
    return temp + 7 * ((1 - np.sign(temp)) // 2) - 1


def parse_cube():
    rot_axes = [
        np.array(
            [
                [0, 1, 0],  # right
                [1, 0, 0],  # down
                [0, -1, 0],  # left
                [-1, 0, 0],  # up
            ]
        )
    ] + 5 * [None]

    cube = 6 * [None]  # 6 faces, each face 4x4
    with open(FILE) as file:
        grid = list(map(list, file.read().splitlines()))

    instructions = grid[-1]

    del grid[-1]
    del grid[-1]

    max_x = max(map(len, grid))
    max_y = len(grid)
    grid = list(map(lambda x: x + [" "] * (max_x - len(x)), grid))
    x, y = 0, 0

    while grid[y][x] != ".":
        x += 1

    # print(f"starting point: ({x=}, {y=})")

    cube[0] = [x, y]

    # each edge defines an axis of rotation in 3d
    # going over one of these edges, we need to rotate
    # the normal vector and the rotation axes for the new face

    stack = [0]
    visited = 6 * [False]
    while len(stack) > 0:
        face = stack.pop(0)
        if visited[face]:
            continue
        visited[face] = True
        x, y = cube[face]

        # for debugging
        # for i in range(4):
        #     for j in range(4):
        #         grid[y + i][x + j] = str(face)

        if x + FULL < max_x and grid[y][x + FULL] != " ":
            new_face = hash(
                rotate(cube_normals[face], rot_axes[face][Direction.RIGHT.value])
            )

            if rot_axes[new_face] is None:
                rot_axes[new_face] = rotate(
                    rot_axes[face].T, rot_axes[face][Direction.RIGHT.value]
                ).T
            cube[new_face] = [x + FULL, y]
            stack.append(new_face)

        if y + FULL < max_y and grid[y + FULL][x] != " ":
            new_face = hash(
                rotate(cube_normals[face], rot_axes[face][Direction.DOWN.value])
            )
            if rot_axes[new_face] is None:
                rot_axes[new_face] = rotate(
                    rot_axes[face].T, rot_axes[face][Direction.DOWN.value]
                ).T
            cube[new_face] = [x, y + FULL]
            stack.append(new_face)

        if x - FULL >= 0 and grid[y][x - FULL] != " ":
            new_face = hash(
                rotate(cube_normals[face], rot_axes[face][Direction.LEFT.value])
            )
            if rot_axes[new_face] is None:
                rot_axes[new_face] = rotate(
                    rot_axes[face].T, rot_axes[face][Direction.LEFT.value]
                ).T
            cube[new_face] = [x - FULL, y]
            stack.append(new_face)

        if y - FULL >= max_y and grid[y - FULL][x] != " ":
            new_face = hash(
                rotate(cube_normals[face], rot_axes[face][Direction.UP.value])
            )
            if rot_axes[new_face] is None:
                rot_axes[new_face] = rotate(
                    rot_axes[face].T, rot_axes[face][Direction.UP.value]
                ).T
            cube[new_face] = [x, y - FULL]
            stack.append(new_face)

    return grid, cube, rot_axes, instructions


def rot2d(v, theta):
    theta = np.radians(theta)
    return np.round(
        np.array([[np.cos(theta), -np.sin(theta)], [np.sin(theta), np.cos(theta)]]) @ v,
        2,
    )


def rot_dir(d, theta):
    theta = theta // 90
    return Direction((d.value + theta) % 4)


def move(face, offset, dir):
    new_offset = ((offset + base[dir.value] + HALF) % FULL) - HALF
    new_face = face
    new_dir = dir
    if new_offset @ base[dir.value] == -HALF:
        a = rot_axes[face][dir.value]
        new_face = hash(rotate(cube_normals[face], rot_axes[face][dir.value]))
        n = np.array(cube_normals[new_face])
        b = rot_axes[new_face][dir.value]

        dot = np.dot(a, b)
        det = np.dot(n, np.cross(a, b))
        theta = np.degrees(np.arctan2(det, dot))

        new_offset = rot2d(new_offset, theta)
        new_dir = rot_dir(dir, theta)

    anchor = np.array(cube[new_face]) + np.array([HALF, HALF])
    pos = (anchor + new_offset).astype(int)
    if grid[pos[1]][pos[0]] != ".":
        return face, offset, dir

    return new_face, new_offset, new_dir


def print_grid(grid):
    for row in grid:
        print("".join(row))


if __name__ == "__main__":
    grid, cube, rot_axes, instructions = parse_cube()

    dgrid = deepcopy(grid)

    inst = []
    j = None
    for i in range(len(instructions)):
        if j is not None and i < j:
            continue
        ins = []
        if instructions[i].isdigit():
            j = i
            while j < len(instructions) and instructions[j].isdigit():
                ins.append(instructions[j])
                j += 1
            inst.append(int("".join(ins)))
        else:
            inst.append(instructions[i])

    face = 0
    offset = np.array([-HALF, -HALF])
    base = np.array(
        [
            [1, 0],
            [0, 1],
            [-1, 0],
            [0, -1],
        ]
    )
    dir = Direction.RIGHT

    anchor = np.array(cube[face]) + np.array([HALF, HALF])
    pos = (anchor + offset).astype(int)
    dgrid[pos[1]][pos[0]] = dir_char[dir.value]

    for i in inst:
        if isinstance(i, int):
            while i > 0:
                face, offset, dir = move(face, offset, dir)
                anchor = np.array(cube[face]) + np.array([HALF, HALF])
                pos = (anchor + offset).astype(int)
                dgrid[pos[1]][pos[0]] = dir_char[dir.value]
                i -= 1
        elif i == "R":
            dir = rot_dir(dir, 90)
        elif i == "L":
            dir = rot_dir(dir, -90)

        anchor = np.array(cube[face]) + np.array([HALF, HALF])
        pos = (anchor + offset).astype(int)
        dgrid[pos[1]][pos[0]] = dir_char[dir.value]

    anchor = np.array(cube[face]) + np.array([HALF, HALF])
    pos = (anchor + offset).astype(int)
    dgrid[pos[1]][pos[0]] = "X"

    print(f"{pos}, {dir.value} {dir_char[dir.value]}")
    print(1000 * (pos[1] + 1) + 4 * (pos[0] + 1) + dir.value)
