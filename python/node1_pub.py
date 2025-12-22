import random
import socket
import time

PORT = 1111
BROADCAST_IP = "255.255.255.255"

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    print("[node1_pub] started, sending mot_vel on UDP broadcast:1111")

    while True:
        v = random.uniform(-100.0, 100.0)
        msg = f"mot_vel,{v:.1f}"
        sock.sendto(msg.encode("utf-8"), (BROADCAST_IP, PORT))
        print(f"[node1_pub] sent: {msg}")
        time.sleep(0.1)  # 10Hz

if __name__ == "__main__":
    main()
