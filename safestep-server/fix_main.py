import re

main_path = "/Users/oscar_contreras/Documents/PlatformIO/Projects/pruebas/src/main.cpp"
with open(main_path, "r", encoding="utf-8") as f:
    code = f.read()

correct_func = '''bool sendWhatsAppAlert(const String &levelName, float mag, const String &timeStr) {
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
                       ? "🚨 *ALERTA MÉDICA: CAÍDA DETECTADA*" 
                       : "⚠️ *AVISO MÉDICO: POSIBLE CAÍDA*";

  String actionText  = (levelName == "Fuerte")
                       ? "Se confirma caída. ¡Se solicita atención inmediata!"
                       : "Posible caída detectada. Se recomienda revisión preventiva del paciente.";

  String messageBody = headerTitle + "\\n\\n"
                       "👤 *Paciente:* " + String(PATIENT_NAME) + "\\n"
                       "📍 *Ubicación:* " + String(PATIENT_ROOM) + "\\n"
                       "⚠️ *Severidad:* " + levelName + "\\n"
                       "💥 *Impacto:* " + String(mag, 2) + " m/s²\\n"
                       "🕒 *Fecha y Hora:* " + timeStr + "\\n\\n"
                       "ℹ️ *Acción:* " + actionText;

  String payload = "{\\"number\\":\\"" + String(EVOLUTION_TARGET_NUM) +
                   "\\",\\"text\\":\\"" + messageBody + "\\"}";

  int httpResponseCode = http.POST(payload);
  bool success = (httpResponseCode >= 200 && httpResponseCode < 300);
  
  if (!success) {
    Serial.printf("[Evolution] Error enviando WhatsApp. Codigo HTTP: %d\\n", httpResponseCode);
  } else {
    Serial.println("📱 [Evolution WhatsApp] ¡Alerta entregada con éxito a WhatsApp!");
  }
  
  http.end();

  // Enviar evento también a n8n en segundo plano
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
}'''

pattern = r"bool sendWhatsAppAlert\(const String &levelName, float mag, const String &timeStr\) \{.*?\n  return success;\n\}"
new_code = re.sub(pattern, correct_func, code, flags=re.DOTALL)

with open(main_path, "w", encoding="utf-8") as f:
    f.write(new_code)
print("SUCCESS: sendWhatsAppAlert correctly written")
