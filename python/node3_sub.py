import socket
import threading
import time
from queue import Queue, Empty

PORT = 1111


def make_udp_listener() -> socket.socket:
    """
    Creates a UDP socket that listens on port 1111 on all interfaces.
    SO_REUSEADDR / SO_REUSEPORT help when multiple subscribers listen on the same port.
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    if hasattr(socket, "SO_REUSEPORT"):
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        except OSError:
            pass

    sock.bind(("", PORT))
    return sock


def parse_message(raw: str):
    """
    Expected format: "<topic>,<data>"
    Returns (topic, data) or (None, None) if invalid.
    """
    raw = raw.strip()
    if "," not in raw:
        return None, None

    topic, data = raw.split(",", 1)
    topic = topic.strip()
    data = data.strip()

    if not topic or not data:
        return None, None

    return topic, data


def main() -> None:
    sock = make_udp_listener()

    # Queue for mot_vel commands (so processing can be separate from receiving)
    mot_vel_queue: Queue[float] = Queue()

    # Shared state: are we currently in STOP (estop=1)?
    state_lock = threading.Lock()
    stop_active = {"value": False}

    print("[node3_sub] listening on UDP port 1111 for mot_vel and estop...", flush=True)

    def receiver_thread():
        """
        Receives UDP packets and updates shared state or queues mot_vel values.
        """
        while True:
            data, _addr = sock.recvfrom(2048)  # blocking wait for UDP packet
            raw = data.decode("utf-8", errors="replace")
            topic, payload = parse_message(raw)

            if topic is None:
                continue

            if topic == "estop":
                if payload not in ("0", "1"):
                    continue

                with state_lock:
                    stop_active["value"] = (payload == "1")

                print(f"[node3_sub] estop updated => {payload}", flush=True)

            elif topic == "mot_vel":
                try:
                    v = float(payload)
                except ValueError:
                    continue

                mot_vel_queue.put(v)

    def processing_thread():
        """
        Processes mot_vel commands. If stop is active, prints velocity=0.
        """
        while True:
            try:
                v = mot_vel_queue.get(timeout=0.5)
            except Empty:
                continue

            with state_lock:
                stop = stop_active["value"]

            if stop:
                print("[node3_sub] STOP active -> velocity=0.0 (ignoring mot_vel)", flush=True)
            else:
                print(f"[node3_sub] velocity command received: {v:.1f}", flush=True)

    t_rx = threading.Thread(target=receiver_thread, daemon=True)
    t_proc = threading.Thread(target=processing_thread, daemon=True)

    t_rx.start()
    t_proc.start()

    # Keep main alive
    while True:
        time.sleep(1)


if __name__ == "__main__":
    main()
