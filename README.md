# ESP Web Server — Battery Dashboard + CSV

On‑device **web server** that exposes battery telemetry and serves a **real‑time dashboard**. The UI polls `/battery` and plots the values; you can also **export CSV** for analysis.

## Why it’s relevant (IT/OT & IoT)
- Patrón clásico **edge → UI** para manufactura: telemetría local, control simple y exportación de datos.
- Demuestra **HTTP endpoints**, **AJAX**, medición por **ADC** y una UI embebida/externa.

## Architecture
```mermaid
flowchart LR
  Sensor[Battery/ADC] --> MCU[ESP8266/ESP32 + WebServer]
  MCU -->|HTTP (JSON/CSV)| UI[index.html]
  UI -->|AJAX 1s| MCU
```

## Features
- Web server en el ESP (firmware `.ino`).
- Dashboard en tiempo real (Highcharts + Bootstrap).
- Endpoints HTTP para **batería** y **control de LEDs**.
- Exportación de datos a **CSV** (cliente o endpoint dedicado).
- Lista para demo: LAN local sin dependencias cloud.

## Repo layout (sugerido)
```
/firmware
  ESP_Grafica_WebServer.ino        # servidor y API
  Lectura_Bateria.ino              # utilidades de ADC (opcional, integrar)
/dashboard
  index.html                       # UI con Highcharts/AJAX
/docs
  wiring.png (divisor resistivo)   # opcional
  metrics.md                       # latencia, % pérdida, etc.
```

## Firmware
- **Base**: `ESP_Grafica_WebServer.ino`
- **Lectura**: si tu sketch no trae ADC completo, integra la rutina de `Lectura_Bateria.ino` (normaliza a volt/%) y expón:
  - `GET /battery` → JSON o texto plano (ej: `3.95` o `{ "voltage": 3.95, "percent": 78 }`)
  - `GET /export.csv` → `Content-Type: text/csv` con `timestamp,voltage,percent`

## UI (index.html)
- La página **consulta `/battery` cada segundo** y grafica los puntos en tiempo real.
- Usa **Highcharts** y **Bootstrap** vía CDN. Ajusta la IP/base URL si la sirves fuera del ESP.
- La UI incluye **acciones de LEDs** y modos (verde/amarillo/rojo/blanco/fiesta, dormir/hibernar) llamando rutas HTTP.
  - Ejemplos esperados por la UI:
    - `/ledverde?intensidadverde=VAL`
    - `/ledamarillo?intensidadamarillo=VAL`
    - `/ledrojo?intensidadrojo=VAL`
    - `/ledblanco?intensidadblanco=VAL`
    - `/ledfiesta?intensidadfiesta=VAL&delayfiesta=MS`
    - `/dormir`
    - `/hibernar?tiempo=MS`

> Nota: si hosteas `index.html` fuera del ESP, **reemplaza `http://192.168.100.16`** por la IP del dispositivo o usa **rutas relativas** (`/ledverde?...`).

## Wiring (batería → ADC)
- Conecta la batería al ADC con **divisor resistivo** (ej. 100k/100k).  
  Fórmula ejemplo: `Vbat = Vadc * (R1 + R2) / R2`. Documenta en `/docs/wiring.png`.

## Métricas (rellenar)
- Latencia de actualización: `__ ms`  
- Pérdida de muestras: `__ %`  
- Rango de voltaje y % (calibración): `__`  
- Export/CSV: `__ registros` por `__ minutos`

## Seguridad / producción (quick notes)
- Añade **auth/token** a endpoints si va a red compartida.
- Limita origenes (CORS) si sirves la UI fuera del dispositivo.
- Considera **HTTPS** (WiFiClientSecure) o **VPN** en redes OT.

## Cómo correr (resumen)
1. Flashea `ESP_Grafica_WebServer.ino` con tus credenciales WiFi.  
2. Sirve `dashboard/index.html` **desde el ESP (SPIFFS/LittleFS)** o desde tu PC (ajusta URL/IP).  
3. Navega a la página y verifica la gráfica; usa los controles de LED para probar endpoints.

## Licencia
MIT (o la que prefieras).