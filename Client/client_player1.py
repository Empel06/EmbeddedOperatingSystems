#!/usr/bin/env python3
"""
Pynq-Z2 Pong Game - PLAYER 1 (Rood Paddel)
UDP Client voor Speler 1
"""

import socket
import sys
import time

# Configuratie
PYNQ_HOST = "192.168.1.10"  # IP van Pynq-Z2
PYNQ_PORT_P1 = 5001          # Port voor Speler 1
SCREEN_HEIGHT = 1080         # Schermhoogte
PADDLE_HEIGHT = 120          # Paddel hoogte

def send_paddle_position(y_position):
    """Stuur paddel Y-positie naar Pynq-Z2"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        message = str(y_position).encode()
        sock.sendto(message, (PYNQ_HOST, PYNQ_PORT_P1))
        sock.close()
        return True
    except Exception as e:
        print(f"[ERROR] Kan niet connecteren naar {PYNQ_HOST}:{PYNQ_PORT_P1}")
        print(f"[ERROR] {e}")
        return False

def main():
    print("\n" + "="*50)
    print("  Pynq-Z2 Pong Game - PLAYER 1 (Rood)")
    print("="*50)
    print(f"Verbinding met Pynq-Z2: {PYNQ_HOST}:{PYNQ_PORT_P1}")
    print(f"Scherm hoogte: {SCREEN_HEIGHT} pixels")
    print(f"Paddel hoogte: {PADDLE_HEIGHT} pixels")
    print(f"Geldige Y waarden: 0 - {SCREEN_HEIGHT - PADDLE_HEIGHT}")
    print("\nBedieningstoetsen:")
    print("  W/↑  : Paddel omhoog")
    print("  S/↓  : Paddel omlaag")
    print("  Q    : Afsluiten")
    print("="*50 + "\n")

    # Initiële positie (midden scherm)
    paddle_y = (SCREEN_HEIGHT - PADDLE_HEIGHT) // 2
    step = 15  # Hoeveel pixels per druk

    # Eerste verbindingstest
    if not send_paddle_position(paddle_y):
        print("Kan geen verbinding maken met Pynq-Z2")
        print("Controleer:")
        print(f"  1. Pynq-Z2 IP adres: {PYNQ_HOST}")
        print(f"  2. Ethernet kabel verbonden")
        print(f"  3. Vitis programma draait op Pynq-Z2")
        return

    print(f"✓ Verbonden! Paddel Y = {paddle_y}\n")

    try:
        while True:
            # Eenvoudige input via terminal
            print(f"\nPaddel Y-positie: {paddle_y}")
            print("Voer in: W(omhoog), S(omlaag), Q(quit)")
            
            command = input(">>> ").upper().strip()

            if command == 'Q':
                print("Programma afgesloten.")
                break
            elif command == 'W':
                paddle_y = max(0, paddle_y - step)
                send_paddle_position(paddle_y)
                print(f"↑ Omhoog -> Y = {paddle_y}")
            elif command == 'S':
                paddle_y = min(SCREEN_HEIGHT - PADDLE_HEIGHT, paddle_y + step)
                send_paddle_position(paddle_y)
                print(f"↓ Omlaag -> Y = {paddle_y}")
            else:
                print("Onbekend commando. Probeer W, S of Q")

    except KeyboardInterrupt:
        print("\n\nProgramma onderbroken.")
    except Exception as e:
        print(f"Fout: {e}")

if __name__ == "__main__":
    main()
