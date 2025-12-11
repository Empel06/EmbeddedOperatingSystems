# Pynq-Z2 HDMI + UDP Project met FreeRTOS

## Overzicht
Dit project heeft als doel een **Pynq-Z2-board** te gebruiken om een systeem te bouwen dat:
- **HDMI-video** uitstuurt naar een extern scherm.
- **Twee UDP-verbindingen** ontvangt als invoer.
- **FreeRTOS** gebruikt als onderliggende softwarestructuur voor taakbeheer en synchronisatie.  

De einddemo zal plaatsvinden tijdens de laatste les. De upload bestaat uit:
- Demo filmpje  
- Vivado projectbestanden  
- Vitis projectbestanden (C/C++ en FreeRTOS)  
- Eventuele extra documenten of tools  

---

## 1. Systeemarchitectuur

### Hardware
Het project maakt gebruik van het **Pynq-Z2-board** met de volgende kerncomponenten:
- Zynq-7000 SoC met ARM Cortex-A9 processoren en FPGA-logica.  
- HDMI-uitgang voor video-output (dedicated HDMI TX-poort op de Pynq-Z2).  
- Gigabit Ethernet-interface voor UDP-communicatie.  

---

## 2. Functionele Beschrijving

Het systeem voert de volgende hoofdfuncties uit:

1. **Ontvangen van data via UDP**  
   Twee onafhankelijke UDP-sockets luisteren naar inkomende datastromen.  
   Elke socket draait in een aparte FreeRTOS-taak.

2. **Verwerken van netwerkdata**  
   De ontvangen data wordt in buffers opgeslagen en eventueel samengevoegd, gefilterd of geherstructureerd.  
   Synchronisatie gebeurt via FreeRTOS-queues of semaforen.

3. **Visualisatie via HDMI-uitgang**  
   De verwerkte data wordt weergegeven op een HDMI-scherm, bijvoorbeeld als tekst, grafieken of statusindicatoren.  
   De HDMI-controller wordt aangestuurd vanuit Vivado (hardware-IP) en Vitis (C-code voor rendering).

---

## 3. Softwareontwerp

### Realtime besturing met FreeRTOS
Om multitasking mogelijk te maken, is het systeem opgebouwd rond FreeRTOS. Typische taakverdeling:
- **Task 1:** Verwerk UDP-invoer van verbinding 1  
- **Task 2:** Verwerk UDP-invoer van verbinding 2  
- **Task 3:** Update de HDMI-framebuffer of weergave   

Gebruik van FreeRTOS-elementen:
- **Queues** voor datatransfer tussen netwerk-taken en de HDMI-taak.  
- **Semaphores** voor toegang tot gedeelde buffers en hardwarebronnen.  
- **Timers** voor periodieke schermupdates of timeouts op netwerkverbindingen.  

### Video Output
De HDMI-controller wordt in **Vivado** opgebouwd met geschikte video-IP.  
De rendering van de data naar het scherm gebeurt in de Vitis-toepassing, die de framebuffer of video-API’s aanstuurt vanuit de FreeRTOS-taken.  

---

## 4. Deliverables

De uiteindelijke oplevering bevat:
- **Demo filmpje**: toont UDP-ontvangst en HDMI-uitvoer in real-time.  
- **Vivado project**: met IP-integratie voor Pynq-Z2, inclusief HDMI en Ethernet.  
- **Vitis project**: met FreeRTOS-taken, UDP-functionaliteit (lwIP) en HDMI-aansturing.  
