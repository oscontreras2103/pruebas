main_path = "/Users/oscar_contreras/Documents/PlatformIO/Projects/pruebas/src/main.cpp"

func_code = """bool sendWhatsAppAlert(const String &levelName, float mag, const String &timeStr) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERR] Wi-Fi desconectado.");
    return false;
  }

  HTTPClient http;
  http.begin(EVOLUTION_API_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", EVOLUTION_API_KEY);
  http.setTimeout(3500);

  String headerTitle = (levelName == "Fuerte") 
                       ? "ALERTA MEDICA: CAIDA DETECTADA" 
                       : "AVISO MEDICO: POSIBLE CAIDA";

  String actionText  = (levelName == "Fuerte")
                       ? "Se confirma caida. Se solicita atencion inmediata!"
                       : "Posible caida detectada. Se recomienda revision preventiva.";

  String messageBody = headerTitle + "\\n\\n" +
                       "Paciente: " + String(PATIENT_NAME) + "\\n" +
                       "Ubicacion: " + String(PATIENT_ROOM) + "\\n" +
                       "Severidad: " + levelName + "\\n" +
                       "Impacto: " + String(mag, 2) + " m/s2\\n" +
                       "Fecha y Hora: " + timeStr + "\\n\\n" +
                       "Accion: " + actionText;

  String payload = "{\\"number\\":\\"" + String(EVOLUTION_TARGET_NUM) +
                   "\\",\\"text\\":\\"" + messageBody + "\\"}";

  int httpResponseCode = http.POST(payload);
  bool success = (httpResponseCode >= 200 && httpResponseCode < 300);
  
  if (!success) {
    Serial.printf("[Evolution] Error enviando WhatsApp. Codigo HTTP: %d\\n", httpResponseCode);
  } else {
    Serial.println("[Evolution WhatsApp] Alerta entregada con exito a WhatsApp!");
  }
  
  http.end();

  // Notificar tambien a n8n
  HTTPClient httpN8n;
  httpN8n.begin(N8N_WEBHOOK_URL);
  httpN8n.addHeader("Content-Type", "application/json");
  httpN8n.setTimeout(1200);
  String n8nPayload = "{\\"paciente\\":\\"" + String(PATIENT_NAME) +
                      "\\",\\"habitacion\\":\\"" + String(PATIENT_ROOM) +
                      "\\",\\"severidad\\":\\"" + levelName +
                      "\\",\\"impacto\\":" + String(mag, 2) +
                      ",\\"fecha_hora\\":\\"" + timeStr + "\\"}";
  httpN8n.POST(n8nPayload);
  httpN8n.end();

  return success;
}"""

with open(main_path, "r") as f:
    text = f.read()

import re
pattern = r"bool sendWhatsAppAlert\(.*?return success;\s*\}"
text = re.sub(pattern, func_code, text, flags=re.DOTALL)

with open(main_path, "w") as f:
    f.write(text)

print("Replacement complete.")
