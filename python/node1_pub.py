import random
import socket
import time

PORT = 1111
BROADCAST_IP = "255.255.255.255"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

print("[node1_pub] sending mot_vel on UDP broadcast:1111")

while True:
    v = random.uniform(-100.0, 100.0)
    msg = f"mot_vel,{v:.1f}".encode("utf-8")
    sock.sendto(msg, (BROADCAST_IP, PORT))
    time.sleep(0.1)  # ~10Hz
