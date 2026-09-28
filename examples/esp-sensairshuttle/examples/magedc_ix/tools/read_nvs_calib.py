#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
"""Dump and parse MagEDC IX demo NVS calibration (close serial monitor first)."""
from __future__ import annotations

import argparse
import struct
import subprocess
import sys
from pathlib import Path

NVS_OFFSET = 0x9000
NVS_SIZE = 0x6000
CALIB_SCHEMA_VER = 4
ZONE_MAGIC = 0x4D41475A  # MAGZ


def run_esptool(port: str, out: Path) -> None:
    esptool = Path(r"C:\Espressif\tools\python\v5.5.4\venv\Scripts\esptool.exe")
    if not esptool.is_file():
        raise SystemExit("esptool not found; set path or pass --bin")
    cmd = [
        str(esptool),
        "--port",
        port,
        "read_flash",
        hex(NVS_OFFSET),
        hex(NVS_SIZE),
        str(out),
    ]
    print("Running:", " ".join(cmd))
    subprocess.check_call(cmd)


def dump_nvs_text(bin_path: Path) -> str:
    idf = Path(r"C:\esp\v5.5.4\esp-idf")
    tool = idf / "components/nvs_flash/nvs_partition_tool/nvs_tool.py"
    if not tool.is_file():
        return ""
    proc = subprocess.run(
        [sys.executable, str(tool), str(bin_path), "-d", "minimal"],
        capture_output=True,
        text=True,
        check=False,
    )
    return proc.stdout + proc.stderr


def parse_zone_blob(data: bytes) -> dict:
    if len(data) < 8:
        return {"error": f"blob too short ({len(data)} B)"}
    magic, count = struct.unpack_from("<II", data, 0)
    if magic != ZONE_MAGIC:
        return {"error": f"bad magic 0x{magic:08X}"}
    zones = []
    off = 8
    zone_fmt = "<i17f"  # polar_zone_t: id + 17 floats
    zone_size = struct.calcsize(zone_fmt)
    for i in range(count):
        if off + zone_size > len(data):
            break
        vals = struct.unpack_from(zone_fmt, data, off)
        zid = vals[0]
        r_min, r_max, th_min, th_max = vals[1:5]
        z_min, z_max = vals[9], vals[10]
        zones.append(
            {
                "id": zid,
                "R": (r_min, r_max),
                "TH": (th_min, th_max),
                "Z": (z_min, z_max),
                "Q": vals[17],
            }
        )
        off += zone_size
    return {"magic": hex(magic), "count": count, "zones": zones}


def analyze(zones: list[dict], center: tuple[int, int, int] | None, meta: dict) -> list[str]:
    lines: list[str] = []
    wizard = meta.get("wizard_done")
    schema = meta.get("calib_schema")
    fw_ts = meta.get("calib_fw_ts")
    lines.append("=== 启动判定 ===")
    lines.append(f"  wizard_done={wizard}  calib_schema={schema} (expected {CALIB_SCHEMA_VER})")
    lines.append(f"  calib_fw_ts={fw_ts!r}")
    if center:
        lines.append(f"  center=({center[0]}, {center[1]}, {center[2]})")
    else:
        lines.append("  center=缺失 → 会进 9 点向导")

    ok_wizard = wizard == 1
    ok_schema = schema == CALIB_SCHEMA_VER
    ok_center = center is not None
    ok_z = True
    for z in zones:
        zmin, zmax = z["Z"]
        span = zmax - zmin
        if span < 20 or span >= 550:
            ok_z = False
            lines.append(f"  [!] zone {z['id']} Z span={span:.0f} 不合格 (需 20~550)")

    strict = ok_wizard and ok_schema and ok_center and ok_z and len(zones) == 9
    sleep_ok = ok_wizard and ok_center and len(zones) == 9
    lines.append(f"  calibration_complete(严格)={'是' if strict else '否'}")
    lines.append(f"  深度睡眠恢复(宽松)={'是' if sleep_ok else '否'}")
    if not strict and sleep_ok:
        lines.append("  → 冷启动可能进向导，但 DEEPSLEEP 唤醒可跳过")
    return lines


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="COM7", help="UART port (monitor must be closed)")
    ap.add_argument("--bin", type=Path, help="existing nvs_dump.bin")
    args = ap.parse_args()

    root = Path(__file__).resolve().parents[1]
    bin_path = args.bin or (root / "build" / "nvs_dump.bin")

    if args.bin is None:
        try:
            run_esptool(args.port, bin_path)
        except subprocess.CalledProcessError as exc:
            raise SystemExit(
                f"读取 Flash 失败 (请关闭串口监视器): {exc}"
            ) from exc

    if not bin_path.is_file():
        raise SystemExit(f"NVS bin 不存在: {bin_path}")

    text = dump_nvs_text(bin_path)
    print("=== NVS 原始条目 (nvs_tool minimal) ===")
    print(text or "(nvs_tool 不可用，仅解析已知 blob)")

    meta: dict = {}
    center = None
    zones: list[dict] = []

    # Parse minimal dump lines: namespace:key=value
    for line in text.splitlines():
        line = line.strip()
        if "=" not in line or ":" not in line:
            continue
        # format: " mag_zones:wizard_done = 1"
        left, val = line.split("=", 1)
        left = left.strip()
        val = val.strip().strip("\x00").strip('"')
        if ":" not in left:
            continue
        ns, key = left.split(":", 1)
        key = key.strip()
        val = val.strip()
        if ns == "mag_zones":
            if key == "wizard_done":
                meta["wizard_done"] = int(val)
            elif key == "calib_schema":
                meta["calib_schema"] = int(val)
            elif key == "calib_fw_ts":
                meta["calib_fw_ts"] = val
        elif ns == "storage":
            if key == "has_center":
                meta["has_center"] = int(val)
            elif key in ("center_x", "center_y", "center_z"):
                meta[key] = int(val)

    if meta.get("has_center") == 1:
        center = (meta.get("center_x", 0), meta.get("center_y", 0), meta.get("center_z", 0))

    # Find calib_v2 blob in partition via nvs_tool blobs output
    blob_dump = subprocess.run(
        [
            sys.executable,
            str(Path(r"C:\esp\v5.5.4\esp-idf/components/nvs_flash/nvs_partition_tool/nvs_tool.py")),
            str(bin_path),
            "-d",
            "blobs",
        ],
        capture_output=True,
        text=True,
        check=False,
    ).stdout

    blob_bytes = None
    for line in blob_dump.splitlines():
        if "mag_zones" in line and "calib_v2" in line:
            # next lines may be hex; fallback read from raw partition search
            pass

    raw = bin_path.read_bytes()
    magic = ZONE_MAGIC.to_bytes(4, "little")
    idx = raw.find(magic)
    if idx >= 0 and idx + 8 <= len(raw):
        count = struct.unpack_from("<I", raw, idx + 4)[0]
        blob_len = 8 + count * struct.calcsize("<i17f")
        blob_bytes = raw[idx : idx + blob_len]

    if blob_bytes:
        parsed = parse_zone_blob(blob_bytes)
        if "error" in parsed:
            print("calib_v2:", parsed["error"])
        else:
            zones = parsed["zones"]
            print("\n=== calib_v2 九区边界 ===")
            for z in zones:
                print(
                    f"  [{z['id']}] R={z['R'][0]:.0f}~{z['R'][1]:.0f}  "
                    f"TH={z['TH'][0]:.1f}~{z['TH'][1]:.1f}  "
                    f"Z={z['Z'][0]:.0f}~{z['Z'][1]:.0f}  Q={z['Q']:.0f}"
                )

    print("\n" + "\n".join(analyze(zones, center, meta)))


if __name__ == "__main__":
    main()
