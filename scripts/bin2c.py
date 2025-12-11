#!/usr/bin/env python3
"""
Binary to C Header Converter for ClankerOS
Converts binary files to C byte arrays for embedding in code.

Compatible with MicroPython.
"""

import sys
import os


def bin2c(input_file, output_file, var_name=None):
    """Convert binary file to C header with byte array."""

    # Generate variable name from filename if not provided
    if var_name is None:
        var_name = os.path.basename(input_file)
        # Replace non-alphanumeric chars with underscores
        var_name = ''.join(c if c.isalnum() else '_' for c in var_name)

    # Read binary data
    try:
        with open(input_file, 'rb') as f:
            data = f.read()
    except Exception as e:
        print("Error reading file {}: {}".format(input_file, e))
        return 1

    # Generate C header
    header_guard = "{}_H".format(var_name.upper())

    lines = []
    lines.append("/* Auto-generated from {} */".format(os.path.basename(input_file)))
    lines.append("#ifndef {}".format(header_guard))
    lines.append("#define {}".format(header_guard))
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <stddef.h>")
    lines.append("")
    lines.append("static const size_t {}_size = {};".format(var_name, len(data)))
    lines.append("static const uint8_t {}[] = {{".format(var_name))

    # Write bytes in rows of 12
    for i in range(0, len(data), 12):
        chunk = data[i:i+12]
        hex_bytes = ', '.join('0x{:02x}'.format(b) for b in chunk)
        if i + 12 < len(data):
            hex_bytes += ','
        lines.append("    {}".format(hex_bytes))

    lines.append("};")
    lines.append("")
    lines.append("#endif /* {} */".format(header_guard))

    # Write output
    try:
        with open(output_file, 'w') as f:
            f.write('\n'.join(lines) + '\n')
    except Exception as e:
        print("Error writing file {}: {}".format(output_file, e))
        return 1

    print("Generated {} ({} bytes)".format(output_file, len(data)))
    return 0


def main():
    if len(sys.argv) < 3:
        print("Usage: {} <input.bin> <output.h> [var_name]".format(sys.argv[0]))
        print("")
        print("Converts binary file to C header with byte array.")
        print("")
        print("Arguments:")
        print("  input.bin   Input binary file")
        print("  output.h    Output C header file")
        print("  var_name    Optional variable name (default: derived from filename)")
        return 1

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    var_name = sys.argv[3] if len(sys.argv) > 3 else None

    return bin2c(input_file, output_file, var_name)


if __name__ == '__main__':
    sys.exit(main())
