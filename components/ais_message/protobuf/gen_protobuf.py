#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Gen Protobuf (Nanopb) for ESP-IDF (PlatformIO or standalone).
Layout:
components/ais_message/
  ├─ include/protobuf/     <- .pb.h (headers sinh ra)
  ├─ protobuf/             <- *.proto + this script
  └─ src/protobuf/         <- .pb.c (sources sinh ra)
"""

import os, sys, subprocess, shutil

# Cho phép chạy trong PlatformIO extra_scripts lẫn chạy tay
try:
    Import("env")  # type: ignore  # Provided by PlatformIO
except Exception:
    pass

# --- Paths ---
PROTO_DIR = os.path.dirname(__file__)                          # .../components/ais_message/protobuf
AIS_DIR   = os.path.dirname(PROTO_DIR)                         # .../components/ais_message
OUT_INC   = os.path.join(AIS_DIR, "include", "protobuf")
OUT_SRC   = os.path.join(AIS_DIR, "src", "protobuf")

os.makedirs(OUT_INC, exist_ok=True)
os.makedirs(OUT_SRC, exist_ok=True)

def find_nanopb_proto_include():
    """
    Tìm thư mục chứa nanopb.proto.
    Ưu tiên:
      1) env var NANOPB_DIR (nếu có generator/proto/nanopb.proto thì dùng; nếu không, thử chính thư mục đó)
      2) search trong sys.path cho '.../site-packages/nanopb/generator/proto'
    Trả về: đường dẫn chứa 'nanopb.proto' hoặc None.
    """
    # 1) Người dùng chỉ định thủ công
    env_dir = os.environ.get("NANOPB_DIR")
    if env_dir:
        gen_proto = os.path.join(env_dir, "generator", "proto")
        if os.path.exists(os.path.join(gen_proto, "nanopb.proto")):
            return gen_proto
        if os.path.exists(os.path.join(env_dir, "nanopb.proto")):
            return env_dir

    # 2) Tìm trong site-packages
    for p in sys.path:
        base = os.path.join(p, "nanopb")
        gen_proto = os.path.join(base, "generator", "proto")
        if os.path.exists(os.path.join(gen_proto, "nanopb.proto")):
            return gen_proto
        if os.path.exists(os.path.join(base, "nanopb.proto")):
            return base

    return None

PROTO_INC_DIR = find_nanopb_proto_include()
if not PROTO_INC_DIR:
    print("[Nanopb] Không tìm thấy đường dẫn chứa nanopb.proto.")
    print("  -> Đặt biến môi trường NANOPB_DIR trỏ tới thư mục 'nanopb' trong site-packages,")
    print("     vd: set NANOPB_DIR=C:\\Users\\Admin\\AppData\\...\\site-packages\\nanopb")
    print("  -> Hoặc kiểm tra vị trí bằng:")
    print('     python -c "import nanopb,inspect,os; print(os.path.dirname(inspect.getfile(nanopb)))"')
    sys.exit(1)

print(f"[Nanopb] proto_inc = {PROTO_INC_DIR}")

# Lấy danh sách .proto
protos = [f for f in os.listdir(PROTO_DIR) if f.endswith(".proto")]
if not protos:
    print(f"[Nanopb] Không tìm thấy file .proto trong {PROTO_DIR}")
    sys.exit(0)

# Generate cho từng .proto
for pr in protos:
    pr_path = os.path.join(PROTO_DIR, pr)
    cmd = [
        "nanopb_generator",
        f"-I{PROTO_DIR}",
        f"-I{PROTO_INC_DIR}",
        f"-D{OUT_SRC}",
        pr_path
    ]
    print("[Nanopb] Generating:", " ".join(cmd))
    try:
        subprocess.check_call(cmd)
    except FileNotFoundError:
        print("[Error] Không tìm thấy 'protoc'. Cài đặt protobuf compiler và thêm vào PATH.")
        print("  Tải binary: https://github.com/protocolbuffers/protobuf/releases")
        sys.exit(1)

# Move .pb.h sang include/protobuf
for f in list(os.listdir(OUT_SRC)):
    if f.endswith(".pb.h"):
        src = os.path.join(OUT_SRC, f)
        dst = os.path.join(OUT_INC, f)
        if os.path.exists(dst):
            os.remove(dst)
        shutil.move(src, dst)

print("[Nanopb] Done. Headers -> include/protobuf, Sources -> src/protobuf.")
