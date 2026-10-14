# SafeStep

> "Usar SafeStep es dar pasos seguros"

Sistema asistencial de telemetria y deteccion temprana de caidas para pacientes hospitalarios y adultos mayores, basado en microcontrolador **ESP32**, sensor inercial **MPU6050**, infraestructura de microservicios con **Evolution API** y **n8n** en Docker, y sincronizacion en la nube con **Google Firebase Realtime Database**.

---

## Descripcion del Proyecto

SafeStep es un dispositivo IoT wearable y plataforma clinica disenada como Trabajo de Grado. El sistema monitorea en tiempo real la cinematica del paciente a una frecuencia de muestreo de 50 Hz (intervalos de 20 ms), clasifica eventos anomalos mediante umbrales de aceleracion vectorial calibrados experimentalmente para su colocacion en el bicep, y emite alertas criticas a traves de multiples canales:

1. **Notificacion directa a WhatsApp** a traves de Evolution API (motor Baileys en Docker) a los contactos medicos y familiares registrados, transmitiendo nombre del paciente, habitacion, magnitud del impacto en m/s2 y hora oficial (NTP America/Bogota, UTC-5).
2. **Orquestacion de alertas via n8n** mediante webhooks para distribucion paralela y automatizaciones clinicas.
3. **Telemetria continua en la nube** con Google Firebase Realtime Database mediante peticiones HTTP asincronas en el Core 0 de FreeRTOS.
4. **Centro de Monitoreo Web** tanto de forma local embebida en el ESP32 (puerto 80) como en la aplicacion cloud (`web_cloud/`) con soporte de Server-Sent Events (SSE), modal visual de emergencia y alarma sonora de advertencia.

---

## Umbrales de Deteccion Calibrados

Los umbrales se calculan a partir de la magnitud de aceleracion vectorial tridimensional:

$$\text{Magnitud} = \sqrt{a_x^2 + a_y^2 + a_z^2}$$

Calibrados experimentalmente para la cinematica del brazo (bicep):

| Nivel | Rango de Magnitud | Clasificacion Clinica | Accion del Sistema |
| :--- | :--- | :--- | :--- |
| **Estable** | $< 17.00\text{ m/s}^2$ | Reposo seguro / Actividad cotidiana normal | Monitoreo continuo y transmision de telemetria base. |
| **Leve** | $17.00\text{ m/s}^2 \le \text{mag} < 20.00\text{ m/s}^2$ | Posible caida / Impacto o perdida de equilibrio leve | Alerta preventiva a WhatsApp y registro en base de datos. |
| **Fuerte** | $\ge 20.00\text{ m/s}^2$ | Caida fuerte confirmada / Impacto critico | Alerta de emergencia prioritaria a WhatsApp, notificacion a n8n, modal de caida y alarma sonora en el panel. |

---

## Arquitectura de Hardware y Pines

- **Microcontrolador**: DOIT ESP32 DevKit V1 (Xtensa Dual-Core 240 MHz, 320 KB SRAM, 4 MB Flash)
- **Sensor Inercial**: MPU6050 (Acelerometro +/- 8g, Giroscopio +/- 500 deg/s)
- **Conexion I2C**:
  - GPIO 21 -> SDA
  - GPIO 22 -> SCL
  - 3.3V / 5V -> VCC
  - GND -> GND

---

## Arquitectura de Software y Distribucion por Nucleos

El firmware implementa particionamiento de tareas mediante FreeRTOS para garantizar que las peticiones de red no interrumpan la medicion inercial:

- **Core 1 (Hilo Principal)**:
  - Muestreo periodico del sensor MPU6050 a 50 Hz.
  - Calculo de magnitud vectorial y evaluacion de umbrales.
  - Servidor web HTTP embebido (puerto 80).
  - Despacho directo de alertas HTTP LAN hacia Evolution API y webhook de n8n.
- **Core 0 (`firebaseTask`)**:
  - Tarea en segundo plano para sincronizacion periodica de telemetria (cada 1000 - 2000 ms) hacia Firebase Realtime Database.
  - Despacho asincrono de alertas pendientes mediante banderas volatiles para evitar bloqueos del procesador.

---

## Microservicios del Servidor (`safestep-server/`)

La infraestructura de comunicacion opera bajo Docker Compose:

- **`safestep_postgres`** (PostgreSQL 15): Persistencia de estados de sesion y metadatos.
- **`evolution_api`** (Puerto 8080): Pasarela autonoma de WhatsApp basada en Baileys, eliminando limitaciones de plantillas y expiracion de tokens diarios.
- **`safestep_n8n`** (Puerto 5678): Motor de integracion para recepcion de webhooks y entrega paralela a multiples destinatarios.

### Puesta en Marcha del Servidor

```bash
cd safestep-server
docker compose up -d
```

Para vincular la sesion de WhatsApp con Evolution API:
- Abrir la interfaz de vinculacion en `http://127.0.0.1:3000` o ejecutar `scan_qr.html`.
- Escanear el codigo QR desde WhatsApp (Dispositivos vinculados).

---

## Puesta en Marcha del Firmware

### 1. Requisitos
- Visual Studio Code con PlatformIO IDE instalado.
- Cable USB de datos para conexion con el ESP32.

### 2. Configuracion de Credenciales
Crear el archivo `include/secrets.h` definiendo los parametros de red:
```cpp
#pragma once

#define WIFI_SSID "TU_RED_WIFI"
#define WIFI_PASS "TU_PASSWORD"

#define FIREBASE_URL "https://safestep-97183-default-rtdb.firebaseio.com/patients/305"
```

Verificar en `include/config.h` la IP del host donde corre Evolution API y n8n, asi como los numeros destinatarios:
```cpp
#define EVOLUTION_API_URL    "http://IP_DEL_SERVIDOR:8080/message/sendText/safestep"
#define EVOLUTION_API_KEY    "safestep_evolution_key_2026"
#define EVOLUTION_TARGET_NUM  "573028554569"
#define EVOLUTION_TARGET_NUM2 "573196090727"
#define N8N_WEBHOOK_URL      "http://IP_DEL_SERVIDOR:5678/webhook/safestep-alert"
```

### 3. Compilacion y Carga

```bash
# Compilar firmware
platformio run --environment esp32doit-devkit-v1

# Cargar al microcontrolador
platformio run --target upload

# Monitorear salida serial (115200 baud)
platformio device monitor
```

---

## Aplicacion Web en la Nube (`web_cloud/`)

El directorio `web_cloud/` contiene el panel clinico desacoplado, optimizado para ejecucion estatica o despliegue en Vercel:
- `index.html`: Estructura semantica del panel de enfermeria.
- `styles.css`: Diseno de alto contraste para visibilidad en estaciones medicas.
- `app.js`: Conexion en tiempo real a Firebase Realtime Database mediante Server-Sent Events (SSE) y gestion de alarmas sonoras asistenciales.
- `vercel.json`: Configuracion de cabeceras de despliegue.

---

## Licencia y Creditos

Proyecto desarrollado como Trabajo de Grado (c) 2026. Todos los derechos reservados.
