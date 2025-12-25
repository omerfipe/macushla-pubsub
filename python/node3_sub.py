import socket

PORT = 1111

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(("", PORT))

estop = 0
velocity = 0.0

print("[node3_sub] listening on UDP :1111 (mot_vel + estop)")

while True:
    raw, _ = sock.recvfrom(2048)
    try:
        s = raw.decode("utf-8", errors="ignore").strip()
        topic, data = s.split(",", 1)
    except ValueError:
        continue

    if topic == "estop":
        if data in ("0", "1"):
            estop = int(data)
            if estop == 1:
                velocity = 0.0
                print("[node3_sub] ESTOP=1 -> velocity forced to 0 (ignore mot_vel)")
            else:
                print("[node3_sub] ESTOP=0 -> motor commands allowed")
        continue

    if topic == "mot_vel":
        if estop == 1:
            continue
        try:
            velocity = float(data)
        except ValueError:
            continue
        print(f"[node3_sub] motor velocity = {velocity:.1f}")
