

from __future__ import annotations

import argparse
import time
from typing import Optional

import cv2
import numpy as np
import requests


def limit(value: float, lower: int, upper: int) -> int:
    return max(lower, min(upper, int(round(value))))


def choose_person(hog: cv2.HOGDescriptor, frame: np.ndarray) -> Optional[tuple[int, int, int, int]]:
    rectangles, weights = hog.detectMultiScale(frame, winStride=(8, 8), padding=(8, 8), scale=1.05)
    if not len(rectangles):
        return None
    # Largest rectangle is NOT a persistent identity, just a prototype target policy.
    i = max(range(len(rectangles)), key=lambda j: int(rectangles[j][2]) * int(rectangles[j][3]))
    return tuple(int(x) for x in rectangles[i])


class RoverClient:
    def __init__(self, url: str, key: str):
        self.url = url.rstrip("/")
        self.session = requests.Session()
        self.headers = {"X-Api-Key": key}

    def snapshot(self) -> np.ndarray:
        response = self.session.get(self.url + "/capture", timeout=2.5)
        response.raise_for_status()
        image = cv2.imdecode(np.frombuffer(response.content, dtype=np.uint8), cv2.IMREAD_COLOR)
        if image is None:
            raise ValueError("Unable to decode ESP32 JPEG")
        return image

    def status(self) -> dict:
        response = self.session.get(self.url + "/status", timeout=1.0)
        response.raise_for_status()
        return response.json()

    def control(self, speed: int, steer: int, pan: int = 90, tilt: int = 90) -> None:
        response = self.session.post(self.url + "/control", headers=self.headers, data={
            "speed": speed, "steer": steer, "pan": pan, "tilt": tilt,
        }, timeout=1.0)
        response.raise_for_status()

    def stop(self) -> None:
        self.session.post(self.url + "/stop", headers=self.headers, timeout=1.0).raise_for_status()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", required=True, help="ESP32 URL, e.g. http://192.168.1.42")
    parser.add_argument("--key", required=True, help="Same control token as firmware")
    parser.add_argument("--allow-drive", action="store_true", help="Enable low-speed movement commands")
    args = parser.parse_args()
    rover = RoverClient(args.url, args.key)
    hog = cv2.HOGDescriptor()
    hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())
    last_status = {}
    last_status_update = 0.0
    print("Starting in", "motion-request" if args.allow_drive else "NO DRIVE", "mode. Press Q to stop.")
    try:
        rover.stop()
        while True:
            frame = rover.snapshot()
            now = time.monotonic()
            if now - last_status_update > 0.4:
                last_status = rover.status()
                last_status_update = now

            selected = choose_person(hog, frame)
            steering, pan, tilt, speed = 90, 90, 90, 0
            label = "No person — stopped"
            if selected is not None:
                x, y, width, height = selected
                midpoint = (x + width * 0.5) / frame.shape[1]
                error = (midpoint - 0.5) * 2.0  # -1 left, +1 right
                # Sign can be reversed based on steering linkage geometry.
                steering = limit(90 + error * 25, 65, 115)
                # Keep the camera/LiDAR roughly forward for range safety checks.
                pan = 90
                cv2.rectangle(frame, (x, y), (x + width, y + height), (0, 255, 0), 2)
                lidar = last_status.get("lidar_cm", 0)
                lidar_age = last_status.get("lidar_age_ms", -1)
                fresh = isinstance(lidar_age, int) and 0 <= lidar_age < 300
                # Range is measured along the LiDAR beam, not guaranteed to hit person.
                if args.allow_drive and fresh and lidar >= 250 and abs(error) < 0.75:
                    speed = 45  # Approx. 18% duty; NOT a physical speed guarantee
                label = f"person x={midpoint:.2f}, lidar={lidar}cm, speed={speed}"

            # Always send zero drive when there is no valid person/range estimate.
            rover.control(speed=speed, steer=steering, pan=pan, tilt=tilt)
            cv2.putText(frame, label, (8, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 2)
            cv2.imshow("TerrainTrack Reference", frame)
            if cv2.waitKey(1) & 0xFF in (ord("q"), 27):
                break
    except (KeyboardInterrupt, requests.RequestException, ValueError) as error:
        print("Stopping after error:", error)
    finally:
        try:
            rover.stop()
        except requests.RequestException:
            print("Network stop failed: Arduino/ESP32 watchdog must stop drive")
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
