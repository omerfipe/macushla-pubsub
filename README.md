# Pub/Sub Messaging System

A distributed messaging system with 4 nodes that communicate over UDP. Two nodes publish messages, and two nodes subscribe to those messages.

## Project Structure

```
macushla_pubsub/
├── README.md
├── python/
│   ├── node1_pub.py       # Python publisher (motor velocity commands)
│   └── node3_sub.py       # Python subscriber (motor controller)
└── cpp/
    ├── CMakeLists.txt     # Build configuration for C++
    ├── node2_pub.cpp      # C++ publisher (emergency stop)
    └── node4_sub.cpp      # C++ subscriber (monitor)
```

## Building instructions

### C++ Code (node2 and node4)

From the `cpp/` directory:

**Linux / macOS:**
```bash
cd cpp
mkdir build
cd build
cmake ..
cmake --build .
```

**Windows (PowerShell or Command Prompt):**
```bash
cd cpp
mkdir build
cd build
cmake ..
cmake --build .
```

This creates executable files for the two C++ nodes.

### Python Code (node1 and node3)

No build needed! Python scripts run directly.

## Running the Nodes

You need to run all 4 nodes. Open 4 different terminal windows and run:

**Terminal 1 - Node 1 (Python Publisher):**

Linux / macOS:
```bash
cd python
python3 node1_pub.py
```

Windows:
```bash
cd python
python node1_pub.py
```

**Terminal 2 - Node 2 (C++ Publisher):**

Linux / macOS:
```bash
cd cpp/build
./node2_pub
```

Windows:
```bash
cd cpp\build
node2_pub.exe
```

**Terminal 3 - Node 3 (Python Subscriber):**

Linux / macOS:
```bash
cd python
python3 node3_sub.py
```

Windows:
```bash
cd python
python node3_sub.py
```

**Terminal 4 - Node 4 (C++ Subscriber):**

Linux / macOS:
```bash
cd cpp/build
./node4_sub
```

Windows:
```bash
cd cpp\build
node4_sub.exe
```

## Node Descriptions

### Node 1 (Python Publisher)
- Publishes "motor_command" messages on the `mot_vel` topic
- Generates random motor velocity values between -100.0 and 100.0
- Example message: `mot_vel,-39.47`

### Node 2 (C++ Publisher)
- Publishes "emergency_stop" messages on the `estop` topic
- Sends either "1" (stop) or "0" (safe) at random intervals
- Example message: `estop,0`

### Node 3 (Python Subscriber)
- Listens to both `mot_vel` and `estop` topics
- When it receives a motor command: prints the velocity to stdout
- When it receives a stop command: sets velocity to 0 and ignores future motor commands until reset
- Handles motor commands and emergency stop messages concurrently using separate threads

### Node 4 (C++ Subscriber)
- Listens only to the `estop` topic
- Logs state transitions between "stop" and "safe" to a file called `safety_log.txt`
- Includes a timestamp (milliseconds since node startup) with each log entry

## Communication Details

- **Protocol:** UDP
- **Port:** 1111
- **Message Format:** `<topic>,<data>`
- Messages are sent via UDP broadcast on the local network (port 1111)
- All nodes listen on the same UDP port
