#!/usr/bin/env python3
"""Explicit Linux bench transmitter; never run automatically by the test suite."""
import argparse
from pathlib import Path
import socket
import struct


def mac_bytes(value):
    try:
        result = bytes.fromhex(value.replace(":", ""))
    except ValueError as exc:
        raise argparse.ArgumentTypeError("invalid MAC") from exc
    if len(result) != 6 or not any(result) or result[0] & 1:
        raise argparse.ArgumentTypeError("a nonzero unicast MAC is required")
    return result


def frame(local, peer, sequence, payload, rate, noack):
    # RATE at offset 8; TX_FLAGS aligned to offset 10.
    radiotap = struct.pack("<BBHIBBH", 0, 0, 12, (1 << 2) | (1 << 15),
                           rate * 2, 0, 8 if noack else 0)
    wlan = struct.pack("<HH", 0x0008, 0) + peer + local + local
    wlan += struct.pack("<H", (sequence & 0xfff) << 4)
    return radiotap + wlan + b"\xaa\xaa\x03\x00\x00\x00\x88\xb5" + payload


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("interface")
    p.add_argument("peer", type=mac_bytes)
    p.add_argument("--count", type=int, default=100)
    p.add_argument("--payload", type=int, default=256)
    p.add_argument("--rate", type=int, choices=(6, 12, 24), default=6)
    p.add_argument("--noack", action="store_true")
    args = p.parse_args()
    if not 1 <= args.count <= 100000 or not 8 <= args.payload <= 3000:
        p.error("count must be 1..100000 and payload 8..3000 bytes")
    if "/" in args.interface or "\\" in args.interface or args.interface in (".", ".."):
        p.error("invalid interface name")
    local = mac_bytes(Path("/sys/class/net", args.interface, "address").read_text().strip())
    with socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(3)) as tx:
        tx.bind((args.interface, 0))
        for sequence in range(args.count):
            payload = struct.pack("<Q", sequence) + bytes(args.payload - 8)
            packet = frame(local, args.peer, sequence, payload, args.rate, args.noack)
            if tx.send(packet) != len(packet):
                raise RuntimeError("short packet submission")
    print(f"Submitted {args.count} frames; this does not establish on-air delivery or ACKs.")


if __name__ == "__main__":
    main()
