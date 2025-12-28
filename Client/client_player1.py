import socket
import time
import sys
import threading

BOARD_IP = "192.168.1.10"   # <-- jouw Pynq IP
PORT = 5001                # UDP poort paddle
SEND_INTERVAL = 0.05       # 20 Hz (vloeiend)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

running = True
direction = None  # 'u' of 'd'

def send_loop():
    while running:
        if direction:
            sock.sendto(direction.encode(), (BOARD_IP, PORT))
        time.sleep(SEND_INTERVAL)

def input_loop():
    global direction, running
    print("Gebruik:")
    print("  w = omhoog")
    print("  s = omlaag")
    print("  x = stop")
    print("  q = quit\n")

    while running:
        key = input().strip().lower()
        if key == 'w':
            direction = 'u'
        elif key == 's':
            direction = 'd'
        elif key == 'x':
            direction = None
        elif key == 'q':
            running = False
            break

# Start threads
threading.Thread(target=send_loop, daemon=True).start()
input_loop()

sock.close()
print("Gestopt.")
