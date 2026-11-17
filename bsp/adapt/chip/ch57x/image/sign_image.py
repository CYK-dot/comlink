#!/usr/bin/env python3
"""Sign a CH57x image: fill .imginfo.{magic,length,crc32}.

The payload covered by the CRC is everything after the metadata section
(--meta-section, typically .metainfo holding the version). The info sections
must lie before it, so writing them back cannot invalidate the CRC.

The script only knows section *names* and nothing about the layout: the caller
passes them in, so a different image format reuses it unchanged.
"""

import argparse
import os
import struct
import subprocess
import sys
import tempfile
import zlib

INFO_SECTIONS = (".imginfo.magic", ".imginfo.length", ".imginfo.crc32")


def run(cmd):
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        sys.exit(f"{os.path.basename(cmd[0])} failed: {proc.stderr.strip()}")
    return proc.stdout


def read_sections(objdump, elf):
    """Parse `objdump -h` into {name: (size, vma, lma)}."""
    sections = {}
    for line in run([objdump, "-h", elf]).splitlines():
        parts = line.split()
        if len(parts) >= 6 and parts[0].isdigit() and parts[1].startswith("."):
            sections[parts[1]] = (int(parts[2], 16), int(parts[3], 16), int(parts[4], 16))
    return sections


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, help="unsigned ELF")
    parser.add_argument("--output", required=True, help="signed ELF")
    parser.add_argument("--objcopy", required=True)
    parser.add_argument("--objdump", required=True)
    parser.add_argument("--magic", required=True, help="magic word, e.g. 0x37354843")
    parser.add_argument("--meta-section", default=".metainfo")
    parser.add_argument("--info-sections", default=",".join(INFO_SECTIONS))
    args = parser.parse_args()

    info_names = [s.strip() for s in args.info_sections.split(",") if s.strip()]
    sections = read_sections(args.objdump, args.input)

    for name in info_names + [args.meta_section]:
        if name not in sections:
            sys.exit(f"sign: section {name} not found in {args.input}")
        if name in info_names and sections[name][0] != 4:
            sys.exit(f"sign: section {name} must be 4 bytes, got {sections[name][0]}")

    # Image base = lowest load address of any non-empty, allocatable section.
    # Debug/comment sections carry LMA 0 and must not drag the base down.
    lmas = [lma for (name, (size, _vma, lma)) in sections.items()
            if size > 0 and not name.startswith((".debug", ".comment", ".riscv"))]
    if not lmas:
        sys.exit("sign: no sections with contents")
    image_base = min(lmas)

    meta_size, _meta_vma, meta_lma = sections[args.meta_section]
    payload_abs = meta_lma + meta_size
    payload_off = payload_abs - image_base
    if payload_off < 0:
        sys.exit("sign: metadata section starts before the image base")

    with tempfile.TemporaryDirectory() as tmp:
        raw = os.path.join(tmp, "image.bin")
        run([args.objcopy, "-O", "binary", "--gap-fill", "0xFF", args.input, raw])
        with open(raw, "rb") as fh:
            image = fh.read()

        if payload_off > len(image):
            sys.exit("sign: metadata section lies beyond the image contents")

        payload = image[payload_off:]
        length = len(payload)
        crc = zlib.crc32(payload) & 0xFFFFFFFF
        magic = int(args.magic, 0) & 0xFFFFFFFF

        values = {info_names[0]: magic, info_names[1]: length, info_names[2]: crc}

        current = args.input
        for name, value in values.items():
            blob = os.path.join(tmp, name.strip(".").replace(".", "_") + ".bin")
            with open(blob, "wb") as fh:
                fh.write(struct.pack("<I", value))
            target = args.output if name == info_names[-1] else os.path.join(tmp, "step.elf")
            run([args.objcopy, f"--update-section={name}={blob}", current, target])
            current = target

    print(f"sign: {os.path.basename(args.output)} magic=0x{magic:08X} "
          f"length={length} crc32=0x{crc:08X}")


if __name__ == "__main__":
    main()
