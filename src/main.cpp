#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <time.h>

#include "secrets.h"
#include "config.h"
#include "web_page.h"

Adafruit_MPU6050 mpu;
WebServer server(80);

bool mpuConnected = false;
unsigned long lastMpuRetryMillis = 0;

volatile float last_ax = 0.0f, last_ay = 0.0f, last_az = 0.0f;
volatile float last_gx = 0.0f, last_gy = 0.0f, last_gz = 0.0f;
volatile float last_temp = 0.0f;
volatile float lastEventMagnitude = 0.0f;
String currentLevel = "Estable";

struct AlertRecord {
  String timeStr;
  String level;
  float magnitude;
  bool whatsappSent;
};

AlertRecord alerts[MAX_ALERTS];
int alertCount = 0;
unsigned long lastAlertMillis = 0;
unsigned long lastSampleMillis = 0;
unsigned long lastEventStateMillis = 0;
unsigned long lastFirebaseSyncMillis = 0;

String getBogotaTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 50)) {
    unsigned long s = millis() / 1000;
    char buf[32];
    snprintf(buf, sizeof(buf), "+%02lu:%02lu:%02lu", (s / 3600), (s % 3600) / 60, (s % 60));
    return String(buf);
  }
  char timeStr[32];
  strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(timeStr);
}

void pushAlert(const String &timeStr, const String &level, float mag, bool sent) {
  if (alertCount < MAX_ALERTS) {
    alerts[alertCount++] = {timeStr, level, mag, sent};
  } else {
    for (int i = 1; i < MAX_ALERTS; i++) {
      alerts[i - 1] = alerts[i];
    }
    alerts[MAX_ALERTS - 1] = {timeStr, level, mag, sent};
  }
}

bool sendWhatsAppAlert(const String &levelName, float mag, const String &timeStr) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERR] Wi-Fi desconectado.");
    return false;
  }

  HTTPClient http;
  http.begin(EVOLUTION_API_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", EVOLUTION_API_KEY);

  String headerTitle = (levelName == "Fuerte") 
                       ? "🚨 *ALERTA MÉDICA: CAÍDA DETECTADA*" 
                       : "⚠️ *AVISO MÉDICO: POSIBLE CAÍDA*";

  String actionText  = (levelName == "Fuerte")
                       ? "Se confirma caída. ¡Se solicita atención inmediata!"
                       : "Posible caída detectada. Se recomienda revisión del paciente.";

  String messageBody = headerTitle + "\\n"
                       "👤 *Paciente:* " + String(PATIENT_NAME) + "\\n"
                       "📍 *Ubicación:* " + String(PATIENT_ROOM) + "\\n"
                       "⚠️ *Severidad:* " + levelName + "\\n"
                       "💥 *Impacto:* " + String(mag, 2) + " m/s²\\n"
                       "🕒 *Fecha y Hora (Bogotá):* " + timeStr + "\\n"
                       "ℹ️ *Acción:* " + actionText;

  String payload = "{\"number\":\"" + String(EVOLUTION_TARGET_NUM) + "\",\"text\":\"" + messageBody + "\"}";

  int httpResponseCode = http.POST(payload);
  bool success = (httpResponseCode > 0 && httpResponseCode < 300);
  
  if (!success) {
    Serial.printf("[Evolution] Error enviando WhatsApp. Código: %d\n", httpResponseCode);
  } else {
    Serial.println("📱 [Evolution WhatsApp] ¡Alerta enviada exitosamente a WhatsApp!");
  }
  
  http.end();
  return success;
}

String makeDataJson() {
  String s = "{";
  s += "\"sensor_ok\":" + String(mpuConnected ? "true" : "false") + ",";
  s += "\"patient_name\":\"" + String(PATIENT_NAME) + "\",";
  s += "\"patient_room\":\"" + String(PATIENT_ROOM) + "\",";
  s += "\"ax\":" + String(last_ax, 4) + ",";
  s += "\"ay\":" + String(last_ay, 4) + ",";
  s += "\"az\":" + String(last_az, 4) + ",";
  s += "\"gx\":" + String(last_gx, 4) + ",";
  s += "\"gy\":" + String(last_gy, 4) + ",";
  s += "\"gz\":" + String(last_gz, 4) + ",";
  s += "\"temp\":" + String(last_temp, 2) + ",";
  s += "\"level\":\"" + (mpuConnected ? currentLevel : "Sensor Desconectado") + "\",";
  s += "\"mag\":" + String(lastEventMagnitude, 2) + ",";
  s += "\"time\":\"" + getBogotaTimeString() + "\",";
  s += "\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
  s += "\"uptime_s\":" + String(millis() / 1000);
  s += "}";
  return s;
}

String makeAlertsJson() {
  String s = "[";
  for (int i = alertCount - 1; i >= 0; i--) {
    s += "{";
    s += "\"time\":\"" + alerts[i].timeStr + "\",";
    s += "\"level\":\"" + alerts[i].level + "\",";
    s += "\"mag\":" + String(alerts[i].magnitude, 2) + ",";
    s += "\"sent\":" + String(alerts[i].whatsappSent ? "true" : "false");
    s += "}";
    if (i > 0) s += ",";
  }
  s += "]";
  return s;
}

void syncTelemetryToFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(String(FIREBASE_URL) + "/telemetry.json");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(2000);
  http.PUT(makeDataJson());
  http.end();
}

void firebaseTask(void *pvParameters) {
  // Esperar a que el sistema inicialice
  vTaskDelay(pdMS_TO_TICKS(2000));
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(String(FIREBASE_URL) + "/telemetry.json");
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(2000);
      http.PUT(makeDataJson());
      http.end();
    }
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}

void syncAlertToFirebase(const String &timeStr, const String &level, float mag, bool sent) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(String(FIREBASE_URL) + "/alerts.json");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(1500);
  String payload = "{\"time\":\"" + timeStr + "\",\"level\":\"" + level + "\",\"mag\":" + String(mag, 2) + ",\"sent\":" + (sent ? "true" : "false") + "}";
  http.POST(payload);
  http.end();
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  if (server.method() == HTTP_HEAD) {
    server.send(200, "text/html", "");
  } else {
    server.send_P(200, "text/html", htmlPage);
  }
}

void setupWebServer() {
  server.on("/", HTTP_ANY, handleRoot);
  server.on("/index.html", HTTP_ANY, handleRoot);
  server.on("/data", HTTP_ANY, []() {
    if (server.method() == HTTP_HEAD) {
      server.send(200, "application/json", "");
    } else {
      server.send(200, "application/json", makeDataJson());
    }
  });
  server.on("/alerts", HTTP_ANY, []() {
    if (server.method() == HTTP_HEAD) {
      server.send(200, "application/json", "");
    } else {
      server.send(200, "application/json", makeAlertsJson());
    }
  });
  server.begin();
}

bool initMPU() {
  Wire.begin(21, 22);
  // Intentar dirección 0x68 (por defecto) o 0x69 (si AD0 está a 3.3V)
  if (mpu.begin(0x68, &Wire) || mpu.begin(0x69, &Wire)) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    mpuConnected = true;
    Serial.println("[OK] Sensor MPU6050 detectado e inicializado.");
    return true;
  }
  mpuConnected = false;
  return false;
}

void setup() {
  Serial.begin(115200);

  // Inicialización I2C para DOIT DevKit V1 (SDA: GPIO 21, SCL: GPIO 22)
  Wire.begin(21, 22);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConectado. IP asignada: %s\n", WiFi.localIP().toString().c_str());

  // Anuncio mDNS Bonjour para descubrimiento automático en Net Analyzer y red local
  if (MDNS.begin("safestep")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("[OK] Servicio mDNS activo: http://safestep.local/");
  }

  // Sincronización horaria con servidor NTP (Zona horaria Bogotá UTC-5)
  configTime(TIMEZONE_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);
  Serial.println("Sincronizando hora con servidor NTP (Bogotá UTC-5)...");

  // Iniciar servidor web INMEDIATAMENTE para garantizar disponibilidad
  setupWebServer();
  Serial.printf("Servidor SafeStep disponible en: http://%s/\n", WiFi.localIP().toString().c_str());

  // Inicialización no bloqueante del MPU6050
  if (!initMPU()) {
    Serial.println("[ERR] No se encontró el sensor MPU6050 en GPIO 21/22.");
    Serial.println("[INFO] El servidor web está ACTIVO. Verifique los cables VCC, GND, SDA (21) y SCL (22).");
  }

  // Tarea en segundo plano en Core 0 para sincronizar a Firebase sin interferir con el servidor web (Core 1)
  xTaskCreatePinnedToCore(
    firebaseTask,
    "firebaseTask",
    8192,
    NULL,
    1,
    NULL,
    0
  );
}

void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();

  // Si el sensor no se detectó o se desconectó, reintentar cada 2 segundos sin congelar la web
  if (!mpuConnected) {
    if (currentMillis - lastMpuRetryMillis >= 2000) {
      lastMpuRetryMillis = currentMillis;
      initMPU();
    }
    return;
  }

  // Muestreo periódico a 50 Hz sin delay bloqueante
  if (currentMillis - lastSampleMillis >= SENSOR_SAMPLE_INTERVAL_MS) {
    lastSampleMillis = currentMillis;

    sensors_event_t a, g, temp;
    if (!mpu.getEvent(&a, &g, &temp)) {
      mpuConnected = false;
      Serial.println("[WARN] Falla de lectura I2C en MPU6050. Intentando reconectar...");
      return;
    }

    last_ax = a.acceleration.x;
    last_ay = a.acceleration.y;
    last_az = a.acceleration.z;
    last_gx = g.gyro.x;
    last_gy = g.gyro.y;
    last_gz = g.gyro.z;
    last_temp = temp.temperature;

    float mag = sqrt(last_ax * last_ax + last_ay * last_ay + last_az * last_az);
    lastEventMagnitude = mag;

    // Impresión en monitor serial (5 veces por segundo)
    static unsigned long lastSerialPrintMillis = 0;
    if (currentMillis - lastSerialPrintMillis >= 200) {
      lastSerialPrintMillis = currentMillis;
      Serial.printf("[MPU6050] Ax:%6.2f Ay:%6.2f Az:%6.2f m/s² | Mag:%6.2f m/s² | Gx:%6.2f Gy:%6.2f Gz:%6.2f | T:%4.1f°C | %s\n",
                    last_ax, last_ay, last_az, mag, last_gx, last_gy, last_gz, last_temp, currentLevel.c_str());
    }

    if (mag >= THRESH_MODERADO) {
      if (currentMillis - lastAlertMillis >= ALERT_MIN_INTERVAL_MS) {
        lastAlertMillis = currentMillis;
        lastEventStateMillis = currentMillis;

        if (mag >= THRESH_FUERTE) {
          currentLevel = "Fuerte";
        } else {
          currentLevel = "Leve";
        }

        Serial.printf("💥 [ALERTA DISPARADA] Magnitud: %.2f m/s² | Severidad: %s\n", mag, currentLevel.c_str());

        String eventTime = getBogotaTimeString();
        bool sent = sendWhatsAppAlert(currentLevel, mag, eventTime);

        pushAlert(eventTime, currentLevel, mag, sent);

        // Notificar en tiempo real inmediato a Firebase
        syncAlertToFirebase(eventTime, currentLevel, mag, sent);
        syncTelemetryToFirebase();
      }
    } else {
      // Retener el estado durante 4 segundos para garantizar sincronización en todos los navegadores
      if (currentMillis - lastEventStateMillis >= 4000) {
        currentLevel = "Estable";
      }
    }
  }
}
