#!/usr/bin/env python3

import sys
import struct

if len(sys.argv) != 3:
    print("Binary to C converter\n")
    print("Usage: %s input.raw output" % sys.argv[0])
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file, "rb") as f:
    data = f.read()

samples = len(data) // 2  # 16 bit = 2 byte

with open(output_file + ".c", "w") as f:
    f.write("#include <stdint.h>\n")
    f.write(f"#include \"{output_file}.h\"\n\n")
    f.write(f"const int16_t i2s_data[{samples}] = {{\n")

    cntr = 1
    for i in range(samples):
        sample = struct.unpack_from("<h", data, i * 2)[0]  # signed 16-bit LE
        f.write(f"  {sample},")
        if cntr % 8 == 0:
            cntr = 0
            f.write("\n")
        cntr += 1

    f.write("};\n\n")
    f.write(f"const uint32_t i2s_samples = {samples};\n")

with open(output_file + ".h", "w") as f:
    f.write("#pragma once\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write(f"#define I2S_SAMPLES (uint32_t)({samples})\n")
    f.write(f"extern const int16_t i2s_data[{samples}];\n")
    f.write(f"extern const uint32_t i2s_samples;\n")

print(f"Converted {samples} samples.")

