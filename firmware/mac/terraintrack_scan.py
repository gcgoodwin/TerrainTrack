
from __future__ import annotations

import argparse
import csv
import math
import time
from pathlib import Path

from terraintrack_host import RoverClient


def point_from_range(distance_cm: int, pan_deg: int, tilt_deg: int) -> tuple[float,float,float]:
    """+X right, +Y forward, +Z up. 90deg pan/tilt = straight ahead."""
    h = math.radians(pan_deg - 90)
    v = math.radians(tilt_deg - 90)
    r = distance_cm / 100.0
    return r * math.cos(v) * math.sin(h), r * math.cos(v) * math.cos(h), r * math.sin(v)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", required=True)
    parser.add_argument("--key", required=True)
    parser.add_argument("--output", default="terraintrack_stationary_points.csv")
    args = parser.parse_args()
    client = RoverClient(args.url, args.key)
    rows = []
    # Start with a narrow FOV and verify that the gimbal never hits end stops.
    try:
        client.stop()
        for tilt in range(70, 111, 10):
            for pan in range(60, 121, 10):
                client.control(speed=0, steer=90, pan=pan, tilt=tilt)
                time.sleep(0.35)  # Servo may require more time under load.
                data = client.status()
                age = data.get("lidar_age_ms", -1)
                cm = data.get("lidar_cm", 0)
                if not isinstance(age,int) or age<0 or age>300 or not isinstance(cm,int) or cm<20:
                    continue
                xyz = point_from_range(cm, pan, tilt)
                rows.append((pan,tilt,cm,data.get("strength",0),*xyz))
                print(f"pan={pan:3} tilt={tilt:3}: {cm:4}cm")
    finally:
        try:
            client.control(speed=0,steer=90,pan=90,tilt=90)
            client.stop()
        except Exception:
            pass
    out = Path(args.output)
    with out.open("w",newline="") as f:
        writer=csv.writer(f)
        writer.writerow(["pan_deg","tilt_deg","range_cm","strength","x_m","y_m","z_m"])
        writer.writerows(rows)
    print(f"Wrote {len(rows)} unregistered points to {out}")

if __name__ == "__main__":
    main()
