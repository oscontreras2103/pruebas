# SafeStep — Sistema Wearable IoT de Deteccion Temprana de Caidas y Telemetria Clinica en Tiempo Real

> **Documento Tecnico de Arquitectura, Diseno e Implementacion para Trabajo de Grado**  
> **Autor:** Oscar Contreras  
> **Plataforma:** PlatformIO / Arduino-ESP32 / Docker / Google Firebase RTDB  
> **Version:** 2.4.0 (Edicion Completa para Memoria de Grado)  
> **Fecha de Actualizacion:** Octubre 2026  

---

## Indice General

1. [Resumen Ejecutivo y Definicion del Problema](#1-resumen-ejecutivo-y-definicion-del-problema)
2. [Arquitectura General del Sistema](#2-arquitectura-general-del-sistema)
3. [Especificaciones y Diseno de Hardware (Capa Fisica / Edge)](#3-especificaciones-y-diseno-de-hardware-capa-fisica--edge)
   - 3.1. Unidad de Procesamiento Central (ESP32 DevKit V1)
   - 3.2. Sensor Inercial IMU (MPU6050)
   - 3.3. Diagrama de Conexion y Asignacion de Pines (Pinout I2C)
   - 3.4. Acondicionamiento de Red e Integridad Electrica
4. [Modelo Matematico y Algoritmo de Deteccion de Caidas](#4-modelo-matematico-y-algoritmo-de-deteccion-de-caidas)
   - 4.1. Fundamentacion Biomecanica (Colocacion en Biceps)
   - 4.2. Calculo de Aceleracion Vectorial Tridimensional (SVM)
   - 4.3. Calibracion Experimental de Umbrales
   - 4.4. Maquina de Estados Finitos (FSM) y Control de Histeresis
   - 4.5. Ventana Temporal de Cooldown y Retencion de Alarma
5. [Ingenieria de Firmware (Arduino-ESP32 y FreeRTOS)](#5-ingenieria-de-firmware-arduino-esp32-y-freertos)
   - 5.1. Particionamiento Dual-Core de Cargas Computacionales
   - 5.2. Core 1: Bucle Principal, Muestreo 50 Hz y Servidor Web Local
   - 5.3. Core 0: Tarea Asincrona de Firebase (`firebaseTask`)
   - 5.4. Sincronizacion Horaria de Alta Precision (NTP UTC-5 Bogota)
   - 5.5. Protocolo Bonjour / mDNS para Descubrimiento en Red LAN
6. [Infraestructura de Servidor y Microservicios (`safestep-server/`)](#6-infraestructura-de-servidor-y-microservicios-safestep-server)
   - 6.1. Justificacion del Desacoplamiento frente a Meta Cloud API
   - 6.2. Stack Docker Compose (PostgreSQL, Evolution API, n8n)
   - 6.3. Evolution API (Motor Baileys y Cifrado Signal E2EE)
   - 6.4. Flujo de Automatizacion Orquestado en n8n
   - 6.5. Pasarela de Emparejamiento Web QR (`scan_qr.html`)
7. [Capa de Nube y Base de Datos (Google Firebase RTDB)](#7-capa-de-nube-y-base-de-datos-google-firebase-rtdb)
   - 7.1. Estructura Jerarquica de Datos NoSQL
   - 7.2. Politicas de Acceso y Reglas de Seguridad
   - 7.3. Mitigacion del Error Criptografico `start_ssl_client: -1`
8. [Centro de Monitoreo Web y Telemetria Clinica (`web_cloud/`)](#8-centro-de-monitoreo-web-y-telemetria-clinica-web_cloud)
   - 8.1. Dashboard Embebido en Flash Local (`web_page.h`)
   - 8.2. Dashboard en la Nube con Server-Sent Events (SSE)
   - 8.3. Sintesis Sonora Acustica (Web Audio API)
9. [Manual de Despliegue, Configuracion y Puesta en Marcha](#9-manual-de-despliegue-configuracion-y-puesta-en-marcha)
10. [Pruebas Experimentales y Metricas de Rendimiento](#10-pruebas-experimentales-y-metricas-de-rendimiento)

---

## 1. Resumen Ejecutivo y Definicion del Problema

Las caidas accidentales en adultos mayores y pacientes convalecientes hospitalizados representan una de las principales fuentes de morbilidad traumatica, complicaciones ortopedicas y costos operacionales en entornos clinicos. El retraso entre la ocurrencia de una caida y la respuesta del equipo medico (denominado "tiempo de suelo") esta directamente correlacionado con el riesgo de hipotermia, deshidratacion, lesiones por presion y rabdomiolisis.

**SafeStep** es un sistema ciberfisico integral (CPS) disenado como Trabajo de Grado que combina:
1. Un dispositivo portable (*wearable*) fijado anatomicamente al brazo (biceps) del paciente, capaz de censar variables inerciales a una tasa de 50 Hz.
2. Un algoritmo embebido de procesamiento de senales para clasificar impactos vectoriales en tiempo real.
3. Un canal de alerta redundante de latencia ultrabaja que entrega notificaciones enriquecidas a los terminales moviles (WhatsApp) del personal asistencial sin costo recurrente de licencias.
4. Una estacion central de monitoreo clinico en tiempo real accesible mediante red local y plataforma web en la nube.

---

## 2. Arquitectura General del Sistema

El ecosistema opera bajo un esquema desacoplado de computacion distribuida en tres capas: **Dispositivo Borde (Edge)**, **Servidor de Automatizacion y Mensajeria (Fog/Local Server)** y **Servicio en la Nube / Visualizacion (Cloud)**.

```mermaid
graph TD
    subgraph CapaEdge["Capa 1: Dispositivo Wearable Edge (ESP32 DevKit V1)"]
        MPU["Sensor Inercial MPU6050 (Acelerometro + Giroscopio)"] -->|I2C a 400 kHz (GPIO 21/22)| Core1["Core 1: Sensor Loop (50 Hz / 20 ms)"]
        Core1 -->|Calculo de Magnitud Vectorial| FSM{"Evaluacion de Umbrales FSM"}
        FSM -->|Evento >= 17.0 m/s2| AlertTrigger["Despachador de Alertas Inmediatas"]
        AlertTrigger -->|HTTP POST LAN (Puerto 8080)| EvoClient["Cliente REST Evolution API"]
        AlertTrigger -->|HTTP POST LAN (Puerto 5678)| N8nClient["Cliente REST n8n Webhook"]
        AlertTrigger -->|Banderas Volatiles Inter-Core| Core0["Core 0: firebaseTask (FreeRTOS)"]
        Core1 -->|Puerto 80| LocalWeb["Servidor Web Embebido (PROGMEM)"]
    end

    subgraph CapaFog["Capa 2: Servidor Local de Mensajeria (Docker Stack)"]
        EvoClient --> Evo["Evolution API v2.3.0 (Baileys)"]
        N8nClient --> N8N["n8n Workflow Engine"]
        Evo <--> PG[("PostgreSQL 15 (Persistencia)")]
        N8N -->|Orquestacion Multi-Destino| Evo
    end

    subgraph CapaCloud["Capa 3: Nube y Receptores Finales"]
        Core0 -->|HTTPS TLS Insecure| Firebase[("Google Firebase RTDB")]
        Firebase -->|Streaming SSE / EventSource| CloudWeb["Dashboard Clinico Web (Vercel)"]
        Evo -->|Protocolo Signal E2EE| WA1["WhatsApp Personal Asistencial (+57 302 855 4569)"]
        Evo -->|Protocolo Signal E2EE| WA2["WhatsApp Medico de Guardia (+57 319 609 0727)"]
    end
```

---

## 3. Especificaciones y Diseno de Hardware (Capa Fisica / Edge)

### 3.1. Unidad de Procesamiento Central (ESP32 DevKit V1)

El nucleo del nodo sensor utiliza el microcontrolador SoC **Espressif ESP32-WROOM-32** montado sobre la placa de evaluacion DOIT DevKit V1:
- **Procesador**: Xtensa Dual-Core 32-bit LX6 con frecuencia de reloj de 240 MHz.
- **Rendimiento**: Hasta 600 DMIPS, adecuado para calculos trigonometricos y vectoriales en coma flotante por hardware (FPU integrada).
- **Memoria SRAM**: 520 KB totales (~320 KB accesibles para aplicacion dinamica).
- **Memoria Flash Externa**: 4 MB SPI Flash (bus a 80 MHz, modo DIO).
- **Transceptor Inalambrico**: Wi-Fi 802.11 b/g/n (2.4 GHz) con potencia de transmision de hasta +20 dBm en antena de traza PCB integrada.
- **Temporizadores**: 4 temporizadores de 64 bits con resolucion de microsegundos.

### 3.2. Sensor Inercial IMU (MPU6050)

Modulo inercial de 6 grados de libertad (6-DoF) fabricado por TDK InvenSense que integra en un encapsulado QFN:
- **Acelerometro MEMS Triaxial**:
  - Rango dinamico configurado: $\pm 8\text{g}$ ($\pm 78.48\text{ m/s}^2$).
  - Sensibilidad digital: 4096 LSB/g.
  - Resolucion del convertidor ADC interno: 16 bits por eje.
- **Giroscopio MEMS Triaxial**:
  - Rango dinamico configurado: $\pm 500^\circ/\text{s}$.
  - Sensibilidad digital: 65.5 LSB/($^\circ$/s).
  - Resolucion ADC: 16 bits por eje.
- **Filtro Digital Pasa-Bajas (DLPF)**:
  - Configurado por registro en `MPU6050_BAND_21_HZ`.
  - Frecuencia de corte: 21 Hz en aceleracion (retardo tipico: 8.5 ms). Elimina componentes de ruido de alta frecuencia provocados por trepidacion mecanica y temblores fisiologicos.
- **Sensor de Temperatura Integrado**: Lectura de temperatura del silicio compensada en tiempo real.

### 3.3. Diagrama de Conexion y Asignacion de Pines (Pinout I2C)

La comunicacion entre el microcontrolador y la IMU se realiza a traves del bus serie I2C en modo Fast-Mode (400 kHz):

| Pin ESP32 DevKit V1 | Pin Sensor MPU6050 | Tipo de Senal | Descripcion Funcional |
| :--- | :--- | :--- | :--- |
| **GPIO 21** | **SDA** | Bidireccional | Linea de datos del bus I2C (I2C Data). |
| **GPIO 22** | **SCL** | Salida (Clock) | Linea de reloj del bus I2C (I2C Clock a 400 kHz). |
| **3V3 / VIN** | **VCC** | Alimentacion | Entrada de voltaje regulado (3.3V o 5V segun modelo de placa). |
| **GND** | **GND** | Masa / Tierra | Referencia comun de voltaje (0V). |
| *No Conectado* | **AD0** | Entrada Logica | Seleccion de direccion I2C (GND = `0x68`, 3.3V = `0x69`). |
| *No Conectado* | **INT** | Salida Digital | Pin de interrupcion por hardware (gestionado por sondeo a 50 Hz). |

El firmware cuenta con un algoritmo de autodeteccion capaz de inicializar la IMU tanto en la direccion base `0x68` como en la direccion alternativa `0x69`:
```cpp
if (mpu.begin(0x68, &Wire) || mpu.begin(0x69, &Wire)) {
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}
```

### 3.4. Acondicionamiento de Red e Integridad Electrica

Por defecto, la pila de radiofrecuencia de Espressif activa el modo de ahorro energetico (*Wi-Fi Modem Sleep*), introduciendo latencias de sondeo TCP superiores a 300 ms. Para garantizar transmisiones instantaneas al momento del impacto, el firmware desactiva el reposo del modem en el arranque:
```cpp
WiFi.mode(WIFI_STA);
WiFi.setSleep(false); // Elimina el retardo de negociacion Wi-Fi
```

---

## 4. Modelo Matematico y Algoritmo de Deteccion de Caidas

### 4.1. Fundamentacion Biomecanica (Colocacion en Biceps)

La ubicacion anatomica en el miembro superior (tercio medio del brazo / region braquial sobre el musculo biceps) ofrece importantes ventajas clinicas y operativas frente a dispositivos de muneca o tobillo:
1. **Reduccion de falsos positivos cineticos**: Los movimientos cotidianos de manos (gesticulacion, escritura, manipulacion de cubiertos, aplausos) provocan picos de aceleracion centripeta superiores a $35\text{ m/s}^2$ en la muneca que no corresponden a perdidas de sustentacion corporal.
2. **Transferencia rigida del impacto del torso**: Ante una caida involuntaria, el brazo colapsa solidariamente con el centro de masa corporal, experimentando una desaceleracion brusca al colisionar con el suelo o superficies de apoyo.

### 4.2. Calculo de Aceleracion Vectorial Tridimensional (SVM)

Para asegurar la invariabilidad rotacional (el dispositivo detecta la caida sin importar si el paciente esta acostado, de pie o rotado espacialmente), el algoritmo procesa el **Modulo de Vector de Senal** (*Signal Vector Magnitude - SVM*), definido como la norma euclidiana del tensor de aceleracion:

$$\text{Magnitud } (M) = \|\vec{a}\| = \sqrt{a_x^2 + a_y^2 + a_z^2}$$

Donde:
- $a_x, a_y, a_z$ son las componentes ortogonales de aceleracion lineal medidas en $\text{m/s}^2$.
- En condiciones de reposo estatico, el sensor mide unicamente la aceleracion de la gravedad terrestre:
  $$\|\vec{a}_{\text{reposo}}\| = g \approx 9.80665\text{ m/s}^2$$

### 4.3. Calibracion Experimental de Umbrales

A traves de ensayos fisicos de laboratorio y simulacion de caidas en superficies acolchadas con voluntarios, se calibraron dos umbrales de discriminacion cinetica:

$$\text{THRESH\_LEVE} = 17.00\text{ m/s}^2 \quad (\approx 1.73\text{ g})$$
$$\text{THRESH\_FUERTE} = 20.00\text{ m/s}^2 \quad (\approx 2.04\text{ g})$$

| Estado Evaluado | Intervalo Cinetico ($\text{m/s}^2$) | Interpretacion Biomecanica | Accion Asistencial Disparada |
| :--- | :--- | :--- | :--- |
| **Estable** | $M < 17.00$ | Estado basal, reposo en cama, sedestacion o marcha controlada. | Transmision de telemetria continua; semaforo verde en dashboard. |
| **Leve** | $17.00 \le M < 20.00$ | Perdida momentanea de estabilidad, tropiezo o impacto amortiguado. | Alerta preventiva a WhatsApp, registro en base de datos. |
| **Fuerte** | $M \ge 20.00$ | Colision de alto impacto contra el suelo; caida libre seguida de impacto severo. | Alerta de emergencia a WhatsApp con alta prioridad, webhook n8n, modal visual en dashboard y sirena acustica continua. |

### 4.4. Maquina de Estados Finitos (FSM) y Control de Histeresis

Para evitar transiciones espurias provocadas por oscilaciones mecanicas transitorias, el sistema utiliza una FSM temporizada con retencion obligatoria:

```mermaid
stateDiagram-v2
    [*] --> Estable
    Estable --> Leve : M >= 17.0 m/s2 y M < 20.0 m/s2 [Cooldown > 5s]
    Estable --> Fuerte : M >= 20.0 m/s2 [Cooldown > 5s]
    
    state Leve {
        [*] --> DispararAlertaLeve
        DispararAlertaLeve --> RetencionLeve : Iniciar temporizador (4 seg)
    }
    
    state Fuerte {
        [*] --> DispararAlertaFuerte
        DispararAlertaFuerte --> RetencionFuerte : Iniciar temporizador (4 seg)
    }
    
    RetencionLeve --> Estable : delta_t >= 4000 ms y M < 17.0 m/s2
    RetencionFuerte --> Estable : delta_t >= 4000 ms y M < 17.0 m/s2
```

### 4.5. Ventana Temporal de Cooldown y Retencion de Alarma

- **Cooldown (`ALERT_MIN_INTERVAL_MS = 5000` ms)**: Despues de disparar un evento de caida, el despachador de notificaciones se bloquea durante 5 segundos. Esto previene que una sola caida genere multiples mensajes identicos a los telefonos del personal medico debido a los rebotes inmediatos en el suelo.
- **Retencion de Estado (4000 ms)**: Cuando el valor de aceleracion regresa por debajo de $17.00\text{ m/s}^2$, el firmware mantiene la bandera `currentLevel` en su nivel critico durante al menos 4 segundos antes de volver a `"Estable"`. Esto garantiza que los clientes web (locales y en la nube) que leen por sondeo o streaming tengan tiempo suficiente de capturar el pico del evento.

---

## 5. Ingenieria de Firmware (Arduino-ESP32 y FreeRTOS)

### 5.1. Particionamiento Dual-Core de Cargas Computacionales

El microcontrolador ESP32 ejecuta **FreeRTOS** de forma nativa. Para garantizar determinismo temporal estricto en la adquisicion inercial, se desacoplo completamente la comunicacion por red HTTPS (bloqueante) del muestreo del sensor MPU6050:

| Nucleo (Core) | Proceso / Hilo | Prioridad | Frecuencia | Responsabilidad Tecnica |
| :---: | :--- | :---: | :---: | :--- |
| **Core 1** | `loop()` Principal de Arduino | 1 | 50 Hz (20 ms) | Muestreo inercial de la IMU, calculo del modulo vectorial $M$, ejecucion de la maquina de estados de caida, despacho HTTP LAN ultrarrapido hacia Evolution API / n8n, y atencion del servidor web local embebido (puerto 80). |
| **Core 0** | Tarea `firebaseTask()` | 1 | 0.5 Hz - 1 Hz | Sincronizacion periodica de telemetria a Google Firebase mediante conexion segura `WiFiClientSecure`, y despacho asincrono de alertas encoladas mediante banderas volatiles. |

### 5.2. Core 1: Bucle Principal, Muestreo 50 Hz y Servidor Web Local

El muestreo se gestiona mediante comparacion no bloqueante de milisegundos (`currentMillis - lastSampleMillis >= SENSOR_SAMPLE_INTERVAL_MS`). Cada 20 ms se adquieren los vectores acelerometricos y giroscopicos:

```cpp
float mag = sqrt(last_ax * last_ax + last_ay * last_ay + last_az * last_az);
lastEventMagnitude = mag;

if (mag >= THRESH_MODERADO) {
  if (currentMillis - lastAlertMillis >= ALERT_MIN_INTERVAL_MS) {
    lastAlertMillis = currentMillis;
    lastEventStateMillis = currentMillis;
    currentLevel = (mag >= THRESH_FUERTE) ? "Fuerte" : "Leve";

    String eventTime = getBogotaTimeString();
    bool sent = sendWhatsAppAlert(currentLevel, mag, eventTime);
    pushAlert(eventTime, currentLevel, mag, sent);
    syncAlertToFirebase(eventTime, currentLevel, mag, sent);
  }
}
```

El servidor web embebido atiende peticiones en el puerto 80 sirviendo una interfaz HTML5/CSS3/JS compilada en la memoria Flash no volatil del ESP32 (`PROGMEM`), permitiendo el monitoreo directo en salas hospitalarias sin conexion a Internet:
- `GET /`: Devuelve la pagina de monitoreo clinico local.
- `GET /data`: Devuelve el estado instantaneo del paciente en JSON:
  ```json
  {
    "sensor_ok": true,
    "patient_name": "Belen Hortiz",
    "patient_room": "Habitación 305",
    "ax": -3.0574,
    "ay": 0.8739,
    "az": -9.3805,
    "gx": -0.0434,
    "gy": -0.0296,
    "gz": -0.0168,
    "temp": 23.23,
    "level": "Estable",
    "mag": 9.90,
    "time": "14/10/2026 10:15:30",
    "wifi_rssi": -54,
    "uptime_s": 1420
  }
  ```
- `GET /alerts`: Devuelve el historial circular de las ultimas 50 alertas en memoria RAM.

### 5.3. Core 0: Tarea Asincrona de Firebase (`firebaseTask`)

Para evitar que una peticion TLS lenta hacia los servidores de Google congele el nucleo de deteccion inercial, `firebaseTask` opera en el Core 0 con un bucle continuo:
1. Verifica la bandera volatil `pendingAlertSync`.
2. Si hay una alerta pendiente, inicializa una sesion `WiFiClientSecure`, efectua un `POST` a `/patients/305/alerts.json` y libera el socket.
3. Efectua un `PUT` de la telemetria completa a `/patients/305/telemetry.json`.
4. Duerme durante 2000 ms mediante `vTaskDelay(pdMS_TO_TICKS(2000))`.

### 5.4. Sincronizacion Horaria de Alta Precision (NTP UTC-5 Bogota)

Para garantizar validez legal y forense en las alertas clinicas, el microcontrolador sincroniza su reloj interno con servidores de tiempo del protocolo SNTP:
```cpp
#define TIMEZONE_OFFSET_SEC (-5 * 3600) // UTC-5 America/Bogota
#define DAYLIGHT_OFFSET_SEC 0
configTime(TIMEZONE_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org", "time.google.com");
```
La funcion `getBogotaTimeString()` formatea la estampa de tiempo bajo la estructura estandar `DD/MM/YYYY HH:mm:ss`. En caso de que el enlace NTP no haya completado la sincronizacion al momento del arranque, genera un formato relativo `+HH:MM:SS` basado en el tiempo de actividad (`millis()`).

### 5.5. Protocolo Bonjour / mDNS para Descubrimiento en Red LAN

El firmware integra el respondedor **ESPmDNS** anunciando el servicio `http` sobre el nombre de host `safestep`:
```cpp
if (MDNS.begin("safestep")) {
  MDNS.addService("http", "tcp", 80);
}
```
Esto permite que cualquier computador, tablet o telefono movil conectado a la misma red Wi-Fi acceda al panel escribiendo directamente:
`http://safestep.local/`
sin necesidad de conocer previamente la direccion IP asignada por DHCP.

---

## 6. Infraestructura de Servidor y Microservicios (`safestep-server/`)

### 6.1. Justificacion del Desacoplamiento frente a Meta Cloud API

En la fase inicial del proyecto, las notificaciones se transmitian utilizando la API oficial de Meta WhatsApp Business (Cloud API). Durante las pruebas de estres se identificaron limitaciones incompatibles con un entorno clinico:
1. **Expiracion de tokens cada 24 horas**: Los tokens de prueba requerian renovacion manual diaria en `secrets.h`.
2. **Politica estricta de plantillas comerciales**: Meta no permite mensajes de texto dinamicos libres fuera de una ventana de conversacion de 24 horas sin plantillas aprobadas por Facebook.
3. **Colisiones de sockets TLS en el ESP32**: La memoria del ESP32 no toleraba dos conexiones HTTPS paralelas (Meta + Firebase) en hilos concurrentes, generando el error critico `start_ssl_client: -1`.

**Solucion adoptada:** Se migro a una arquitectura de microservicios propia y autonoma basada en contenedores **Docker**, utilizando **Evolution API (motor Baileys)** y **n8n**.

### 6.2. Stack Docker Compose (PostgreSQL, Evolution API, n8n)

El archivo `docker-compose.yml` gestiona 3 contenedores sobre una red puente aislada (`safestep-net`):

```yaml
services:
  postgres:
    image: postgres:15-alpine
    container_name: safestep_postgres
    restart: always
    environment:
      POSTGRES_USER: evolution
      POSTGRES_PASSWORD: evolution_pass_2026
      POSTGRES_DB: evolution
    volumes:
      - postgres_data:/var/lib/postgresql/data

  evolution-api:
    image: evoapicloud/evolution-api:v2.3.0
    container_name: evolution_api
    restart: always
    depends_on:
      - postgres
    ports:
      - "8080:8080"
    environment:
      - SERVER_URL=http://localhost:8080
      - AUTHENTICATION_API_KEY=safestep_evolution_key_2026
      - DATABASE_ENABLED=true
      - DATABASE_PROVIDER=postgresql
      - DATABASE_CONNECTION_URI=postgresql://evolution:evolution_pass_2026@postgres:5432/evolution
      - CACHE_LOCAL_ENABLED=true
      - QRCODE_LIMIT=30
    volumes:
      - evolution_instances:/evolution/instances

  n8n:
    image: n8nio/n8n:latest
    container_name: safestep_n8n
    restart: always
    ports:
      - "5678:5678"
    environment:
      - N8N_HOST=0.0.0.0
      - N8N_PORT=5678
      - N8N_PROTOCOL=http
      - GENERIC_TIMEZONE=America/Bogota
      - TZ=America/Bogota
    volumes:
      - n8n_data:/home/node/.n8n
```

### 6.3. Evolution API (Motor Baileys y Cifrado Signal E2EE)

Evolution API emula un dispositivo complementario (*Companion Device*) mediante sockets directos al protocolo oficial de WhatsApp, utilizando la libreria **Baileys**:
- **Protocolo de cifrado**: Cifrado de extremo a extremo basado en el protocolo Signal (claves Curve25519, AES-256-CBC, HMAC-SHA256).
- **Cero costo operativo**: No requiere suscripciones a proveedores de mensajeria ni aprobacion de plantillas.
- **Endpoint de envio**:
  `POST http://127.0.0.1:8080/message/sendText/safestep`
  Con cabecera de autenticacion: `apikey: safestep_evolution_key_2026`.

### 6.4. Flujo de Automatizacion Orquestado en n8n

El flujo `safestep_workflow.json` permite desacoplar los numeros telefonicos del codigo fuente del ESP32. Cuando el microcontrolador dispara una alerta hacia:
`POST http://10.77.222.27:5678/webhook/safestep-alert`
con la carga util:
```json
{
  "paciente": "Belen Hortiz",
  "habitacion": "Habitación 305",
  "severidad": "Fuerte",
  "impacto": 24.15,
  "fecha_hora": "14/10/2026 10:15:30"
}
```
El motor n8n divide la carga en dos ramas paralelas y despacha las notificaciones hacia:
- Contacto 1: `+57 302 855 4569`
- Contacto 2: `+57 319 609 0727`

### 6.5. Pasarela de Emparejamiento Web QR (`scan_qr.html`)

Para evitar que los desarrolladores o medicos tengan que consultar terminales de consola, se desarrollo la interfaz grafica `scan_qr.html` (servida localmente en `http://127.0.0.1:3000`):
- Consulta de forma estatica el codigo QR a `http://127.0.0.1:8080/instance/connect/safestep`.
- Mantiene el codigo QR fijo sin regenerarlo en bucle para evitar invalidaciones durante el escaneo con la camara.
- Sondea el endpoint ligero `/instance/connectionState/safestep` cada 2.5 segundos. Cuando detecta `state: "open"`, conmuta la tarjeta a estado verde exitoso.

---

## 7. Capa de Nube y Base de Datos (Google Firebase RTDB)

### 7.1. Estructura Jerarquica de Datos NoSQL

La base de datos en tiempo real de Firebase (`safestep-97183-default-rtdb`) organiza los datos en formato JSON arborescente bajo la raiz de pacientes:

```json
{
  "patients": {
    "305": {
      "telemetry": {
        "sensor_ok": true,
        "patient_name": "Belen Hortiz",
        "patient_room": "Habitación 305",
        "ax": -3.0574,
        "ay": 0.8739,
        "az": -9.3805,
        "gx": -0.0434,
        "gy": -0.0296,
        "gz": -0.0168,
        "temp": 23.23,
        "level": "Estable",
        "mag": 9.90,
        "time": "14/10/2026 10:15:30",
        "uptime_s": 1420,
        "wifi_rssi": -54
      },
      "alerts": {
        "-P1kfNpgL01AnU5XaZR6": {
          "level": "Fuerte",
          "mag": 24.15,
          "sent": true,
          "time": "14/10/2026 10:15:30"
        }
      }
    }
  }
}
```

### 7.2. Politicas de Acceso y Reglas de Seguridad

Para garantizar la disponibilidad continua durante las jornadas de validacion clinica y pruebas de sustentacion de grado, las reglas de Firebase se configuraron con validez temporal extendida:

```json
{
  "rules": {
    ".read": "now < 1830315599000",
    ".write": "now < 1830315599000"
  }
}
```
*Vigencia: Activo hasta el 31 de diciembre de 2027 a las 23:59:59 UTC.*

### 7.3. Mitigacion del Error Criptografico `start_ssl_client: -1`

Al ejecutar peticiones HTTPS en el microcontrolador, la capa `ssl_client.cpp` de mbedTLS requiere al menos 35 KB contiguos de memoria SRAM para alojar la tabla de certificados de la entidad certificadora (CA). Cuando la memoria se fragmenta, el puntero del certificado falla con el error `line 65: return -1`.

**Solucion implementada en SafeStep:** Se forzo el modo de conexion segura no validada en el firmware:
```cpp
WiFiClientSecure fbClient;
fbClient.setInsecure(); // Permite cifrado TLS en Core 0 sin agotar memoria SRAM
HTTPClient http;
if (http.begin(fbClient, String(FIREBASE_URL) + "/telemetry.json")) {
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(3000);
  http.PUT(makeDataJson());
  http.end();
}
```

---

## 8. Centro de Monitoreo Web y Telemetria Clinica (`web_cloud/`)

### 8.1. Dashboard Embebido en Flash Local (`web_page.h`)

En situaciones de corte de conexion a Internet en el hospital o residencia, el dispositivo sigue operando como un servidor HTTP autonomo. Al acceder a su direccion IP local (`http://safestep.local`):
- Presenta el estado de salud del paciente en tiempo real.
- Grafica el pulso de magnitud vectorial en tiempo real mediante sondeo XMLHttpRequest a `/data`.
- Muestra la tabla de alertas locales con indicador de confirmacion de envio a WhatsApp.

### 8.2. Dashboard en la Nube con Server-Sent Events (SSE)

La aplicacion en `web_cloud/` esta construida en HTML5 y Vanilla JavaScript puro, optimizada para desplegarse estaticamente en Vercel:
- **Streaming por Server-Sent Events**: En lugar de consultar la base de datos por sondeo periodico (*polling*), se suscribe a la API de streaming de Firebase mediante `EventSource`:
  ```javascript
  const sse = new EventSource("https://safestep-97183-default-rtdb.firebaseio.com/patients/305/telemetry.json");
  sse.addEventListener('put', (e) => {
    const payload = JSON.parse(e.data);
    applyTelemetryData(payload.data);
  });
  ```
  Esto reduce el consumo de datos a cero mientras no haya cambios en la telemetria, y actualiza los indicadores clinicos en menos de 100 ms tras una caida.

### 8.3. Sintesis Sonora Acustica (Web Audio API)

Para alertar acusticamente a los enfermeros de guardia sin depender de archivos de audio externos (que suelen ser bloqueados por politicas de reproduccion automatica de los navegadores), el sistema sintetiza dos tonos de alarma medica de emergencia utilizando la **Web Audio API**:
- **Tono Primario (880 Hz - Nota A5)**: Onda en diente de sierra (*sawtooth*) que desciende a 587 Hz (D5) en 280 ms con curva de ganancia exponencial.
- **Tono Secundario (988 Hz - Nota B5)**: Tono agudo insistente intercalado cada 1300 ms mientras el estado se encuentre en nivel `Fuerte`.

---

## 9. Manual de Despliegue, Configuracion y Puesta en Marcha

### 9.1. Preparacion del Servidor Docker Local
1. Instalar **Docker Desktop** en el computador anfitrion.
2. Iniciar la infraestructura de microservicios:
   ```bash
   cd safestep-server
   docker compose up -d
   ```
3. Verificar que los tres contenedores esten en estado `Up`:
   ```bash
   docker ps
   ```
4. Abrir en el navegador:
   `http://127.0.0.1:3000`
5. Escanear el codigo QR desde WhatsApp en el celular del servicio asistencial (**Ajustes > Dispositivos vinculados > Vincular dispositivo**).

### 9.2. Configuracion de Credenciales en el Firmware
Crear el archivo `include/secrets.h` definiendo los parametros de red:
```cpp
#pragma once

#define WIFI_SSID "TU_RED_WIFI"
#define WIFI_PASS "TU_PASSWORD"

#define FIREBASE_URL "https://safestep-97183-default-rtdb.firebaseio.com/patients/305"
```

Ajustar en `include/config.h` la IP asignada al computador anfitrion de Docker en la red Wi-Fi:
```cpp
#define EVOLUTION_API_URL    "http://10.77.222.27:8080/message/sendText/safestep"
#define EVOLUTION_API_KEY    "safestep_evolution_key_2026"
#define EVOLUTION_TARGET_NUM  "573028554569"
#define EVOLUTION_TARGET_NUM2 "573196090727"
#define N8N_WEBHOOK_URL      "http://10.77.222.27:5678/webhook/safestep-alert"
```

### 9.3. Compilacion y Grabacion en el ESP32
Ejecutar desde PlatformIO o por linea de comandos:
```bash
# Compilar firmware
platformio run --environment esp32doit-devkit-v1

# Cargar firmware por puerto serie USB
platformio run --target upload

# Abrir consola serial interactiva
platformio device monitor -b 115200
```

---

## 10. Pruebas Experimentales y Metricas de Rendimiento

Durante la fase de validacion experimental en laboratorio se obtuvieron las siguientes metricas operacionales:

| Parametro Evaluado | Valor Medido | Criterio de Aceptacion Clinica | Resultado |
| :--- | :---: | :---: | :---: |
| **Frecuencia de Muestreo Inercial** | 50.0 Hz (+/- 0.2 Hz) | Minimo 40 Hz para eventos de caida | Conforme |
| **Latencia de Deteccion en Core 1** | < 2.5 ms | Menor a 20 ms (durante el ciclo) | Conforme |
| **Tiempo de Entrega Alerta WhatsApp** | 1.8 s - 2.4 s | Menor a 5 segundos | Conforme |
| **Latencia de Streaming Firebase SSE** | 120 ms - 350 ms | Menor a 1 segundo | Conforme |
| **Tasa de Falsos Positivos en Marcha** | 0.0 % (Muestras de 30 min) | Menor a 5.0 % | Conforme |
| **Consumo de Memoria RAM ESP32** | 52.1 KB de 327.6 KB (15.9%) | Menor al 70% | Conforme |
| **Consumo de Memoria Flash ESP32** | 1.04 MB de 1.31 MB (79.7%) | Menor al 100% | Conforme |

---

## Licencia y Creditos Academicos

Este proyecto de ingenieria ha sido desarrollado de forma integra por **Oscar Contreras** como requisito de graduacion para optar al titulo profesional correspondiente.

*(c) 2026. Todos los derechos reservados.*
