# ESP Web Server — Battery Dashboard & Multi‑Charger (25‑bay)

An ESP8266/ESP32 project that exposes battery telemetry over an on‑device **web server**, renders a **real‑time chart** in the browser (AJAX), and lets you **export CSV** for analysis. The repository also includes the **hardware for a 25‑slot charging dock** with per‑slot status LEDs.

> Frontend lives in **`data/index.html`** so it can be uploaded with SPIFFS/LittleFS. When the ESP web server runs, the page fetches `/battery` periodically and updates the chart; CSV can be downloaded from the same page or a dedicated endpoint.

---

## Highlights
- **Edge → UI** pattern (industrial/OT friendly): local telemetry, simple controls, CSV export.
- **HTTP endpoints** served by the MCU; browser uses **AJAX** for 1‑second sampling.
- **Battery sensing** via ADC and resistor divider; optional LED control endpoints.
- **25‑bay charging dock** with per‑slot **CHARGE**/**FULL** LED indication.

---

## Architecture

```mermaid
flowchart LR
  S[Battery / Divider → ADC] --> M[ESP8266/ESP32 WebServer]
  M -->|HTTP JSON + CSV| U[data/index.html]
  U -->|AJAX (1 s)| M
  M -.optional.-> LEDs[RGB LED control endpoints]
```

**Firmware**
- Main server sketch: **`ESP_Grafica_WebServer.ino`** (serves routes and exposes battery data).
- Battery helper (if needed): **`Lectura_Bateria.ino`** (ADC read → voltage/percentage).  
- Frontend: **`data/index.html`** (served by SPIFFS/LittleFS).

> Endpoints to expose in the firmware (names may already exist in your sketch; keep them stable):
>
> - `GET /battery` → e.g. `{ "voltage": 3.95, "percent": 78 }` (or a plain value if preferred)
> - `GET /export.csv` → `Content-Type: text/csv` with header `timestamp,voltage,percent`
> - Optional LED control: `/ledverde?intensidadverde=...`, `/ledamarillo?...`, `/ledrojo?...`, `/ledblanco?...`, `/ledfiesta?intensidadfiesta=...&delayfiesta=...`
>
> The **web page polls `/battery`** every second; CSV can be downloaded from the UI.

---

## Hardware — Wearable board (ESP‑12F)

The wearable board uses an **ESP‑12F (ESP8266MOD)** with a 3.3V regulator and RGB LED chain.

- **MCU**: ESP‑12F / ESP8266MOD (Wi‑Fi + ADC).  
- **Regulator**: AP2112K‑3.3 (LDO) to power the ESP from the Li‑ion cell.  
- **RGB LEDs**: WS2812B chain driven by a single **Data pin**.  
- **Battery measurement**: resistor divider **R7=402k / R8=100k** with **C5=10µF** (low‑pass) into **ADC** (see schematic).  
- **Connector**: battery input + programming pads.

> Schematic previews (stored under **`Simulacion 3D/`**):
>
> - Wearable schematic: `Simulacion 3D/Esquematico-De-Pulsera.png`  
> - LED section: `Simulacion 3D/Esquematico-De-Pulsera.png` (upper‑right block)  
> - ESP‑12F pins overview: `Simulacion 3D/Captura1.PNG`

**Images**
- PCB back: `Simulacion 3D/Pulsera-PCB-Parte-Atras.png`  
- 3D views: `Simulacion 3D/Pulsera-3D.jpg`, `Simulacion 3D/Pulsera-Vista-3D(1).jpg`, `Simulacion 3D/Pulsera-Vista-3D(2).jpg`, `Simulacion 3D/Vista-Pulsera3D.jpg`

You can embed them in the README if you prefer inline visuals:
```md
![Wearable schematic](Simulacion%203D/Esquematico-De-Pulsera.png)
![3D view](Simulacion%203D/Pulsera-3D.jpg)
```

---

## Hardware — 25‑Bay Charging Dock

A multi‑slot charger that can **charge up to 25 bracelets simultaneously**, each slot with two status LEDs:
- **CHARGE** (charging in progress)
- **FULL** (fully charged)

> Based on the schematic labels in `Simulacion 3D/Esquematico-De-Cargador.png`, the design uses a **single‑cell Li‑ion charger IC of the TP4056 class** (constant‑current/constant‑voltage) and a **DW01A‑class protection IC** with a **dual MOSFET pack (e.g., FS8205A)**. These parts are commonly used together for safe charge/protect functions.
>
> - **Charger IC** (TP4056‑compatible): provides CC/CV, exposed **CHRG**/**STDBY** pins for LED status.  
> - **Protection**: **DW01A** + **dual MOSFET** (battery over‑/under‑voltage and over‑current protection).  
> - **Connector**: 2‑pin battery header per slot.  
> - Add‑on filters/caps near power path to reduce noise during simultaneous charging.

**Images**
- Charger schematic: `Simulacion 3D/Esquematico-De-Cargador.png`  
- Charger board views: `Simulacion 3D/Cargador-De-Pulsera.png`, `Simulacion 3D/Cargador-De-Pulsera-Parte-Atras.png`

Inline example:
```md
![Charger schematic](Simulacion%203D/Esquematico-De-Cargador.png)
```

---

## Battery Telemetry — Results & Analysis

The UI plots battery values in real time. Example captures (stored under **`Graficas bateria/Analisis de las graficas/`**):

```md
![Battery live chart](Graficas%20bateria/Analisis%20de%20las%20graficas/bateria.PNG)
![Annotated run](Graficas%20bateria/Analisis%20de%20las%20graficas/Grafica%20de%20comparacion.jpg)
```

There’s also a short analysis and raw PDF:
- Notes: `Graficas bateria/Analisis de las graficas/Notas primera lectura sin grafica.txt`  
- PDF: `Graficas bateria/Analisis de las graficas/lectura-bateria.pdf`

**Recommended metrics to report** (fill with your measurements):
- Update latency: `__ ms` (UI round‑trip)  
- Packet loss / dropped samples: `__ %`  
- Voltage range and ADC scaling: `__ V` → `__ %`  
- CSV sample size: `__ rows` over `__ minutes`

---

## Repository Layout (proposed)

```
/data                          # SPIFFS/LittleFS content (served by the ESP)
  index.html
/Simulacion 3D                 # Schematics, PCBs, 3D previews
/Graficas bateria/Analisis de las graficas
/Datasheets                    # charger ICs, LEDs, regulators
/Documentos y proveedores      # vendor notes / BoM
/Logos
/Pulseras PCB
ESP_Grafica_WebServer.ino      # main firmware (web server + endpoints)
Lectura_Bateria.ino            # ADC → voltage/percent helpers
README.md
LICENSE
```

> **Tip**: If you prefer the dashboard outside SPIFFS/LittleFS, you can also host `index.html` on a PC and point it at the ESP IP, or make the requests relative to `/` if served by the device.

---

## Build & Upload

1. **Configure Wi‑Fi** in `ESP_Grafica_WebServer.ino` (SSID/password).  
2. Upload **SPIFFS/LittleFS** data so `data/index.html` is available (use the Arduino/PlatformIO data upload tool).  
3. Flash the firmware.  
4. Open the device IP in the browser → the page requests **`/battery`** every second and plots the points.  
5. Download **CSV** from the UI or via **`/export.csv`**.

---

## Security & Production Notes
- Prefer a **token/auth** on control endpoints if the device sits on a shared network.
- If the dashboard is hosted off‑device, restrict **CORS** to trusted origins.
- For OT networks, consider **VPN** or device isolation; **HTTPS** is possible with `WiFiClientSecure`, noting memory limits on small MCUs.

---

## OTA Support?

This repository currently focuses on the web server, AJAX dashboard and telemetry. If your sketch includes OTA, it will typically have references to **`ArduinoOTA`** or HTTP update code. If not present, you can add ArduinoOTA in a few lines (Wi‑Fi + OTA setup + `ArduinoOTA.handle()` in loop).

> If you want, open an issue and I can add a minimal **OTA** variant in a separate branch (ArduinoOTA + basic password).

---

## License
MIT (or your preferred license).