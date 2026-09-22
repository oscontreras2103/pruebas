#pragma once
#include <Arduino.h>

const char htmlPage[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>SafeStep | Centro de Monitoreo</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&family=JetBrains+Mono:wght@500;700&display=swap" rel="stylesheet">
<style>
:root {
  --bg-main: #f1f5f9;
  --surface: #ffffff;
  --surface-subtle: #f8fafc;
  --border: #e2e8f0;
  --text-primary: #0f172a;
  --text-secondary: #475569;
  --text-muted: #94a3b8;
  --primary: #0284c7;
  --primary-dark: #0369a1;
  --primary-light: #e0f2fe;
  --success: #10b981;
  --success-bg: #ecfdf5;
  --warning: #f59e0b;
  --warning-bg: #fffbeb;
  --orange: #f97316;
  --orange-bg: #fff7ed;
  --danger: #ef4444;
  --danger-bg: #fef2f2;
  --shadow-sm: 0 1px 2px 0 rgba(0, 0, 0, 0.05);
  --shadow: 0 4px 12px -1px rgba(15, 23, 42, 0.08);
  --shadow-lg: 0 12px 24px -4px rgba(15, 23, 42, 0.12);
  --radius: 14px;
}

* { box-sizing: border-box; margin: 0; padding: 0; }

body {
  font-family: 'Plus Jakarta Sans', -apple-system, BlinkMacSystemFont, sans-serif;
  background-color: var(--bg-main);
  color: var(--text-primary);
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}

/* NAVBAR SUPERIOR */
.navbar {
  background: #0f172a;
  color: #ffffff;
  padding: 14px 28px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  box-shadow: var(--shadow);
}
.brand-group {
  display: flex;
  align-items: center;
  gap: 12px;
}
.brand-icon {
  width: 40px;
  height: 40px;
  background: linear-gradient(135deg, #0284c7, #38bdf8);
  border-radius: 10px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 22px;
  box-shadow: 0 2px 8px rgba(2, 132, 199, 0.4);
}
.brand-text h1 {
  font-size: 1.15rem;
  font-weight: 700;
  letter-spacing: -0.02em;
  color: #f8fafc;
}
.brand-text p {
  font-size: 0.75rem;
  color: #94a3b8;
  font-weight: 500;
}
.navbar-meta {
  display: flex;
  align-items: center;
  gap: 20px;
}
.clock-widget {
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.12);
  padding: 6px 14px;
  border-radius: 8px;
  font-family: 'JetBrains Mono', monospace;
  font-size: 0.85rem;
  color: #e2e8f0;
  display: flex;
  align-items: center;
  gap: 8px;
}
.pulse-dot {
  width: 9px;
  height: 9px;
  background: var(--success);
  border-radius: 50%;
  box-shadow: 0 0 0 0 rgba(16, 185, 129, 0.7);
  animation: pulse-green 2s infinite;
}
@keyframes pulse-green {
  0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(16, 185, 129, 0.7); }
  70% { transform: scale(1); box-shadow: 0 0 0 8px rgba(16, 185, 129, 0); }
  100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(16, 185, 129, 0); }
}

/* CONTENEDOR PRINCIPAL */
.main-wrapper {
  max-width: 1240px;
  width: 100%;
  margin: 0 auto;
  padding: 24px 20px;
  display: flex;
  flex-direction: column;
  gap: 22px;
  flex: 1;
}

/* BANNER FICHA DEL PACIENTE */
.patient-card {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 22px 28px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-wrap: wrap;
  gap: 20px;
  box-shadow: var(--shadow);
  position: relative;
  overflow: hidden;
}
.patient-card::before {
  content: '';
  position: absolute;
  left: 0;
  top: 0;
  bottom: 0;
  width: 6px;
  background: var(--primary);
  border-radius: var(--radius) 0 0 var(--radius);
}
.patient-profile {
  display: flex;
  align-items: center;
  gap: 18px;
}
.patient-avatar {
  width: 58px;
  height: 58px;
  background: var(--primary-light);
  color: var(--primary);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 28px;
  border: 2px solid #bae6fd;
}
.patient-details .tag {
  font-size: 0.72rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.05em;
  color: var(--primary);
  background: var(--primary-light);
  padding: 2px 8px;
  border-radius: 4px;
  display: inline-block;
  margin-bottom: 4px;
}
.patient-name {
  font-size: 1.45rem;
  font-weight: 800;
  color: var(--text-primary);
  letter-spacing: -0.02em;
}
.patient-room {
  font-size: 0.95rem;
  color: var(--text-secondary);
  display: flex;
  align-items: center;
  gap: 6px;
  margin-top: 2px;
}
.patient-status-box {
  display: flex;
  align-items: center;
  gap: 16px;
}
.status-pill {
  padding: 10px 22px;
  border-radius: 30px;
  font-weight: 700;
  font-size: 1rem;
  display: flex;
  align-items: center;
  gap: 10px;
  box-shadow: var(--shadow-sm);
  transition: all 0.3s ease;
}
.status-estable {
  background: var(--success-bg);
  color: #065f46;
  border: 1.5px solid #a7f3d0;
}
.status-leve {
  background: var(--warning-bg);
  color: #92400e;
  border: 1.5px solid #fde68a;
}
.status-moderado {
  background: var(--orange-bg);
  color: #9a3412;
  border: 1.5px solid #fed7aa;
}
.status-fuerte {
  background: var(--danger-bg);
  color: #991b1b;
  border: 1.5px solid #fecaca;
  animation: pulse-red 1s infinite;
}
@keyframes pulse-red {
  0% { transform: scale(1); box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.7); }
  50% { transform: scale(1.03); box-shadow: 0 0 0 10px rgba(239, 68, 68, 0); }
  100% { transform: scale(1); box-shadow: 0 0 0 0 rgba(239, 68, 68, 0); }
}

/* GRID TELEMETRÍA */
.dashboard-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
  gap: 18px;
}
.card {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 20px;
  box-shadow: var(--shadow);
  display: flex;
  flex-direction: column;
}
.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 14px;
}
.card-title {
  font-size: 0.88rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.04em;
  color: var(--text-secondary);
}
.badge-unit {
  font-size: 0.72rem;
  font-weight: 600;
  color: var(--text-muted);
  background: var(--surface-subtle);
  padding: 2px 7px;
  border-radius: 4px;
  border: 1px solid var(--border);
}

/* MAGNITUD GRANDE */
.metric-hero {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 10px 0;
}
.metric-value-huge {
  font-size: 3rem;
  font-weight: 800;
  font-family: 'JetBrains Mono', monospace;
  color: var(--text-primary);
  line-height: 1;
}
.metric-unit {
  font-size: 0.9rem;
  font-weight: 600;
  color: var(--text-muted);
  margin-top: 4px;
}
.impact-bar-track {
  width: 100%;
  height: 10px;
  background: #e2e8f0;
  border-radius: 5px;
  overflow: hidden;
  margin-top: 16px;
  position: relative;
}
.impact-bar-fill {
  height: 100%;
  width: 0%;
  background: linear-gradient(90deg, #10b981, #f59e0b, #ef4444);
  transition: width 0.25s ease-out;
}

/* BARRAS DE EJES */
.axis-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
  margin-top: 6px;
}
.axis-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 10px;
}
.axis-tag {
  width: 24px;
  height: 24px;
  border-radius: 6px;
  font-weight: 700;
  font-size: 0.75rem;
  display: flex;
  align-items: center;
  justify-content: center;
}
.axis-tag-x { background: #fee2e2; color: #b91c1c; }
.axis-tag-y { background: #dcfce7; color: #15803d; }
.axis-tag-z { background: #e0f2fe; color: #0369a1; }
.axis-val {
  font-family: 'JetBrains Mono', monospace;
  font-size: 1.05rem;
  font-weight: 700;
  color: var(--text-primary);
  min-width: 80px;
  text-align: right;
}

/* METADATOS Y DIAGNÓSTICO */
.diagnostics-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}
.diag-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 8px 10px;
  background: var(--surface-subtle);
  border-radius: 8px;
  font-size: 0.88rem;
}
.diag-label {
  color: var(--text-secondary);
  font-weight: 500;
}
.diag-val {
  font-family: 'JetBrains Mono', monospace;
  font-weight: 700;
  color: var(--text-primary);
}

/* HISTORIAL DE ALERTAS / TABLA CLÍNICA */
.history-card {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 22px;
  box-shadow: var(--shadow);
}
.history-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 18px;
  flex-wrap: wrap;
  gap: 12px;
}
.history-title-group h2 {
  font-size: 1.15rem;
  font-weight: 700;
  color: var(--text-primary);
}
.history-title-group p {
  font-size: 0.8rem;
  color: var(--text-muted);
}
.btn-sound {
  background: var(--surface-subtle);
  border: 1px solid var(--border);
  padding: 8px 14px;
  border-radius: 8px;
  font-size: 0.82rem;
  font-weight: 600;
  color: var(--text-secondary);
  cursor: pointer;
  display: flex;
  align-items: center;
  gap: 6px;
  transition: all 0.2s;
}
.btn-sound:hover {
  background: #e2e8f0;
}

.table-responsive {
  width: 100%;
  overflow-x: auto;
}
.events-table {
  width: 100%;
  border-collapse: collapse;
  text-align: left;
  font-size: 0.88rem;
}
.events-table th {
  background: var(--surface-subtle);
  color: var(--text-secondary);
  padding: 12px 16px;
  font-weight: 700;
  font-size: 0.76rem;
  text-transform: uppercase;
  letter-spacing: 0.04em;
  border-bottom: 1.5px solid var(--border);
}
.events-table td {
  padding: 14px 16px;
  border-bottom: 1px solid var(--border);
  color: var(--text-primary);
  vertical-align: middle;
}
.events-table tr:hover td {
  background-color: #f8fafc;
}
.badge-level {
  padding: 4px 10px;
  border-radius: 20px;
  font-weight: 700;
  font-size: 0.75rem;
  display: inline-block;
  text-transform: uppercase;
}
.badge-fuerte { background: #fee2e2; color: #b91c1c; border: 1px solid #fca5a5; }
.badge-moderado { background: #ffedd5; color: #c2410c; border: 1px solid #fdba74; }
.badge-leve { background: #fef3c7; color: #b45309; border: 1px solid #fde68a; }
.badge-estable { background: #dcfce7; color: #15803d; border: 1px solid #86efac; }

.badge-wa {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  font-size: 0.75rem;
  font-weight: 600;
  padding: 3px 8px;
  border-radius: 4px;
  background: #ecfdf5;
  color: #047857;
  border: 1px solid #a7f3d0;
}
.badge-wa-err {
  background: #fef2f2;
  color: #b91c1c;
  border: 1px solid #fecaca;
}

.empty-state {
  text-align: center;
  padding: 36px 12px;
  color: var(--text-muted);
  font-size: 0.9rem;
}

/* FOOTER */
footer {
  text-align: center;
  padding: 18px;
  color: var(--text-muted);
  font-size: 0.78rem;
  border-top: 1px solid var(--border);
  background: var(--surface);
  margin-top: auto;
}

/* MODAL DE EMERGENCIA / POP-UP DE CAÍDA FUERTE */
.modal-overlay {
  position: fixed;
  inset: 0;
  background: rgba(15, 23, 42, 0.82);
  backdrop-filter: blur(8px);
  z-index: 10000;
  display: none;
  align-items: center;
  justify-content: center;
  padding: 20px;
  animation: modalFadeIn 0.25s ease-out;
}
@keyframes modalFadeIn {
  from { opacity: 0; transform: scale(0.95); }
  to { opacity: 1; transform: scale(1); }
}
.modal-card {
  background: #ffffff;
  border-radius: 20px;
  max-width: 520px;
  width: 100%;
  padding: 32px 28px;
  box-shadow: 0 0 50px rgba(239, 68, 68, 0.55);
  border: 3px solid #ef4444;
  text-align: center;
  animation: modalGlow 1.2s infinite alternate;
}
@keyframes modalGlow {
  0% { box-shadow: 0 0 25px rgba(239, 68, 68, 0.4); }
  100% { box-shadow: 0 0 55px rgba(239, 68, 68, 0.75); }
}
.modal-badge {
  display: inline-block;
  background: #fee2e2;
  color: #b91c1c;
  border: 1.5px solid #fca5a5;
  font-size: 0.8rem;
  font-weight: 800;
  letter-spacing: 0.08em;
  padding: 6px 14px;
  border-radius: 20px;
  text-transform: uppercase;
  margin-bottom: 12px;
}
.modal-title {
  font-size: 1.65rem;
  font-weight: 800;
  color: #991b1b;
  margin-bottom: 6px;
  letter-spacing: -0.02em;
}
.modal-subtitle {
  font-size: 0.92rem;
  color: var(--text-secondary);
  margin-bottom: 20px;
}
.modal-info-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 12px;
  background: #f8fafc;
  border: 1px solid var(--border);
  border-radius: 12px;
  padding: 16px;
  margin-bottom: 20px;
  text-align: left;
}
.modal-info-item .m-label {
  display: block;
  font-size: 0.72rem;
  font-weight: 700;
  text-transform: uppercase;
  color: var(--text-muted);
  letter-spacing: 0.04em;
}
.modal-info-item .m-val {
  font-size: 1.05rem;
  font-weight: 700;
  color: var(--text-primary);
  margin-top: 2px;
}
.modal-info-item .highlight {
  color: #b91c1c;
  font-family: 'JetBrains Mono', monospace;
  font-weight: 800;
}
.modal-alert-action {
  background: #fef2f2;
  border: 1.5px solid #fecaca;
  color: #991b1b;
  padding: 12px 16px;
  border-radius: 10px;
  font-size: 0.88rem;
  line-height: 1.4;
  margin-bottom: 24px;
  font-weight: 600;
}
.btn-dismiss {
  width: 100%;
  background: #ef4444;
  color: #ffffff;
  border: none;
  padding: 14px 22px;
  font-size: 1.05rem;
  font-weight: 700;
  border-radius: 12px;
  cursor: pointer;
  box-shadow: 0 4px 16px rgba(239, 68, 68, 0.4);
  transition: all 0.2s ease;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
}
.btn-dismiss:hover {
  background: #dc2626;
  transform: translateY(-1px);
}
</style>
</head>
<body>

<header class="navbar">
  <div class="brand-group">
    <div class="brand-icon">🛡️</div>
    <div class="brand-text">
      <h1>SafeStep</h1>
      <p>Usar SafeStep es dar pasos seguros</p>
    </div>
  </div>
  <div class="navbar-meta">
    <div class="clock-widget">
      <div class="pulse-dot"></div>
      <span id="navBogotaTime">Bogotá: Sincronizando...</span>
    </div>
  </div>
</header>

<main class="main-wrapper">

  <!-- FICHA DEL PACIENTE -->
  <section class="patient-card">
    <div class="patient-profile">
      <div class="patient-avatar">🛏️</div>
      <div class="patient-details">
        <span class="tag">Paciente Asignado</span>
        <div id="patientName" class="patient-name">Cargando paciente...</div>
        <div class="patient-room">
          📍 <span id="patientRoom">Habitación --</span>
        </div>
      </div>
    </div>
    <div class="patient-status-box">
      <div id="statusPill" class="status-pill status-estable">
        <span id="statusIcon">●</span>
        <span id="statusText">Estable</span>
      </div>
    </div>
  </section>

  <!-- CUADRO DE TELEMETRÍA -->
  <section class="dashboard-grid">
    <!-- MAGNITUD VECTORIAL -->
    <div class="card">
      <div class="card-header">
        <span class="card-title">Magnitud de Impacto</span>
        <span class="badge-unit">Resultante</span>
      </div>
      <div class="metric-hero">
        <div id="magVal" class="metric-value-huge">0.00</div>
        <div class="metric-unit">m/s²</div>
      </div>
      <div class="impact-bar-track">
        <div id="magBar" class="impact-bar-fill"></div>
      </div>
      <div style="display:flex; justify-content:space-between; font-size:0.7rem; color:var(--text-muted); margin-top:6px;">
        <span>0 (Reposo ~9.8)</span>
        <span>Leve (17.0)</span>
        <span>Caída Fuerte (20.0+)</span>
      </div>
    </div>

    <!-- ACELERACIÓN TRIAXIAL -->
    <div class="card">
      <div class="card-header">
        <span class="card-title">Aceleración por Eje</span>
        <span class="badge-unit">m/s²</span>
      </div>
      <div class="axis-list">
        <div class="axis-row">
          <div style="display:flex; align-items:center; gap:8px;">
            <div class="axis-tag axis-tag-x">X</div>
            <span style="font-size:0.85rem; color:var(--text-secondary);">Longitudinal</span>
          </div>
          <span id="ax" class="axis-val">0.000</span>
        </div>
        <div class="axis-row">
          <div style="display:flex; align-items:center; gap:8px;">
            <div class="axis-tag axis-tag-y">Y</div>
            <span style="font-size:0.85rem; color:var(--text-secondary);">Lateral</span>
          </div>
          <span id="ay" class="axis-val">0.000</span>
        </div>
        <div class="axis-row">
          <div style="display:flex; align-items:center; gap:8px;">
            <div class="axis-tag axis-tag-z">Z</div>
            <span style="font-size:0.85rem; color:var(--text-secondary);">Vertical</span>
          </div>
          <span id="az" class="axis-val">0.000</span>
        </div>
      </div>
    </div>

    <!-- GIROSCOPIO -->
    <div class="card">
      <div class="card-header">
        <span class="card-title">Velocidad Angular</span>
        <span class="badge-unit">rad/s</span>
      </div>
      <div class="axis-list">
        <div class="axis-row">
          <div style="display:flex; align-items:center; gap:8px;">
            <div class="axis-tag axis-tag-x">X</div>
            <span style="font-size:0.85rem; color:var(--text-secondary);">Roll (Balanceo)</span>
          </div>
          <span id="gx" class="axis-val">0.000</span>
        </div>
        <div class="axis-row">
          <div style="display:flex; align-items:center; gap:8px;">
            <div class="axis-tag axis-tag-y">Y</div>
            <span style="font-size:0.85rem; color:var(--text-secondary);">Pitch (Cabeceo)</span>
          </div>
          <span id="gy" class="axis-val">0.000</span>
        </div>
        <div class="axis-row">
          <div style="display:flex; align-items:center; gap:8px;">
            <div class="axis-tag axis-tag-z">Z</div>
            <span style="font-size:0.85rem; color:var(--text-secondary);">Yaw (Guiñada)</span>
          </div>
          <span id="gz" class="axis-val">0.000</span>
        </div>
      </div>
    </div>

    <!-- SISTEMA Y DIAGNÓSTICO -->
    <div class="card">
      <div class="card-header">
        <span class="card-title">Diagnóstico del Enlace</span>
        <span class="badge-unit">ESP32</span>
      </div>
      <div class="diagnostics-list">
        <div class="diag-row">
          <span class="diag-label">Temperatura Sensor</span>
          <span class="diag-val"><span id="temp">--</span> °C</span>
        </div>
        <div class="diag-row">
          <span class="diag-label">Dirección IP</span>
          <span class="diag-val" id="ip">192.168.x.x</span>
        </div>
        <div class="diag-row">
          <span class="diag-label">Señal Wi-Fi</span>
          <span class="diag-val"><span id="rssi">--</span> dBm</span>
        </div>
        <div class="diag-row">
          <span class="diag-label">Tiempo Activo</span>
          <span class="diag-val" id="uptime">0s</span>
        </div>
      </div>
    </div>
  </section>

  <!-- TABLA DE HISTORIAL Y EVENTOS BOGOTÁ -->
  <section class="history-card">
    <div class="history-header">
      <div class="history-title-group">
        <h2>Registro Clínico de Eventos y Caídas</h2>
        <p>Historial cronológico con fecha y hora oficial de Bogotá (UTC-5)</p>
      </div>
      <button id="btnSoundToggle" class="btn-sound" onclick="toggleAudio()">
        <span id="soundIcon">🔔</span>
        <span id="soundLabel">Alarma Sonora Activa</span>
      </button>
    </div>

    <div class="table-responsive">
      <table class="events-table">
        <thead>
          <tr>
            <th>Fecha y Hora (Bogotá)</th>
            <th>Nivel de Evento</th>
            <th>Aceleración Pico</th>
            <th>Notificación WhatsApp</th>
            <th>Estado Clínico</th>
          </tr>
        </thead>
        <tbody id="alertsBody">
          <tr>
            <td colspan="5" class="empty-state">Esperando sincronización de registros...</td>
          </tr>
        </tbody>
      </table>
    </div>
  </section>

</main>

<!-- POP-UP MODAL DE ALERTA DE CAÍDA FUERTE -->
<div id="fallModal" class="modal-overlay">
  <div class="modal-card">
    <div class="modal-badge">🚨 EMERGENCIA CLÍNICA SAFESTEP</div>
    <h2 class="modal-title">¡CAÍDA FUERTE DETECTADA!</h2>
    <p class="modal-subtitle">Se ha registrado un impacto severo que supera el umbral de seguridad.</p>
    
    <div class="modal-info-grid">
      <div class="modal-info-item">
        <span class="m-label">Paciente</span>
        <span class="m-val" id="modalPatient">Belen Hortiz</span>
      </div>
      <div class="modal-info-item">
        <span class="m-label">Habitación</span>
        <span class="m-val" id="modalRoom">Habitación 305</span>
      </div>
      <div class="modal-info-item">
        <span class="m-label">Impacto Registrado</span>
        <span class="m-val highlight" id="modalImpact">-- m/s²</span>
      </div>
      <div class="modal-info-item">
        <span class="m-label">Hora Bogotá</span>
        <span class="m-val" id="modalTime">--:--:--</span>
      </div>
    </div>

    <div class="modal-alert-action">
      ⚠️ <strong>ACCIÓN INMEDIATA:</strong> Personal asistencial debe acudir de urgencia al cuarto para valorar y asistir a la paciente.
    </div>

    <button class="btn-dismiss" onclick="dismissFallAlert()">
      <span>✓ Atender y Silenciar Alarma</span>
    </button>
  </div>
</div>

<footer>
  SafeStep &copy; 2026 — Usar SafeStep es dar pasos seguros
</footer>

<script>
let soundEnabled = true;
let audioCtx = null;
let previousLevel = "Estable";
let isModalOpen = false;
let alarmInterval = null;

// Desbloquear AudioContext al interactuar con la página (clic, tecla, toque o scroll)
['click', 'keydown', 'touchstart', 'scroll'].forEach(evt => {
  window.addEventListener(evt, () => {
    if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    if (audioCtx && audioCtx.state === 'suspended') audioCtx.resume();
  }, { once: false, passive: true });
});
let lastHandledAlertTime = "";

function toggleAudio() {
  soundEnabled = !soundEnabled;
  if (!soundEnabled) stopUrgentAlarm();
  document.getElementById("soundIcon").innerText = soundEnabled ? "🔔" : "🔕";
  document.getElementById("soundLabel").innerText = soundEnabled ? "Alarma Sonora Activa" : "Alarma Silenciada";
}

function startUrgentAlarm() {
  if (!soundEnabled) return;
  stopUrgentAlarm();
  playAlarmBurst();
  alarmInterval = setInterval(playAlarmBurst, 1300);
}

function stopUrgentAlarm() {
  if (alarmInterval) {
    clearInterval(alarmInterval);
    alarmInterval = null;
  }
}

function playAlarmBurst() {
  if (!soundEnabled) return;
  try {
    if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    if (audioCtx.state === 'suspended') audioCtx.resume();

    let now = audioCtx.currentTime;

    // Tono 1 sirena de emergencia (880 Hz)
    let osc1 = audioCtx.createOscillator();
    let g1 = audioCtx.createGain();
    osc1.type = "sawtooth";
    osc1.frequency.setValueAtTime(880, now);
    osc1.frequency.linearRampToValueAtTime(587, now + 0.28);
    g1.gain.setValueAtTime(0.4, now);
    g1.gain.exponentialRampToValueAtTime(0.001, now + 0.28);
    osc1.connect(g1);
    g1.connect(audioCtx.destination);
    osc1.start(now);
    osc1.stop(now + 0.28);

    // Tono 2 más agudo e insistente (988 Hz)
    let osc2 = audioCtx.createOscillator();
    let g2 = audioCtx.createGain();
    osc2.type = "sawtooth";
    osc2.frequency.setValueAtTime(988, now + 0.32);
    osc2.frequency.linearRampToValueAtTime(659, now + 0.6);
    g2.gain.setValueAtTime(0.45, now + 0.32);
    g2.gain.exponentialRampToValueAtTime(0.001, now + 0.6);
    osc2.connect(g2);
    g2.connect(audioCtx.destination);
    osc2.start(now + 0.32);
    osc2.stop(now + 0.6);
  } catch(e) {
    console.error("Error reproduciendo alarma:", e);
  }
}

function showFallModal(data) {
  isModalOpen = true;
  document.getElementById("modalPatient").innerText = data.patient_name || document.getElementById("patientName").innerText;
  document.getElementById("modalRoom").innerText = data.patient_room || document.getElementById("patientRoom").innerText;
  document.getElementById("modalImpact").innerText = (data.mag ? Number(data.mag).toFixed(2) : "--") + " m/s²";
  document.getElementById("modalTime").innerText = data.time || new Date().toLocaleTimeString();

  document.getElementById("fallModal").style.display = "flex";
  startUrgentAlarm();
}

function dismissFallAlert() {
  isModalOpen = false;
  stopUrgentAlarm();
  document.getElementById("fallModal").style.display = "none";
}

function formatUptime(seconds) {
  let h = Math.floor(seconds / 3600);
  let m = Math.floor((seconds % 3600) / 60);
  let s = seconds % 60;
  return (h > 0 ? h + "h " : "") + m + "m " + s + "s";
}

function updateData() {
  fetch('/data')
    .then(r => r.json())
    .then(j => {
      // Paciente
      if (j.patient_name) document.getElementById("patientName").innerText = j.patient_name;
      if (j.patient_room) document.getElementById("patientRoom").innerText = j.patient_room;

      // Reloj Bogotá
      if (j.time) document.getElementById("navBogotaTime").innerText = "Bogotá: " + j.time;

      // Valores numéricos
      document.getElementById("ax").innerText = j.ax.toFixed(3);
      document.getElementById("ay").innerText = j.ay.toFixed(3);
      document.getElementById("az").innerText = j.az.toFixed(3);
      document.getElementById("gx").innerText = j.gx.toFixed(3);
      document.getElementById("gy").innerText = j.gy.toFixed(3);
      document.getElementById("gz").innerText = j.gz.toFixed(3);
      document.getElementById("temp").innerText = j.temp.toFixed(1);
      document.getElementById("magVal").innerText = j.mag.toFixed(2);
      document.getElementById("rssi").innerText = j.wifi_rssi || "--";
      document.getElementById("uptime").innerText = formatUptime(j.uptime_s || 0);

      // Barra de impacto escalada a 50 m/s2 (30 m/s2 = 60%)
      let pct = Math.min(100, Math.max(0, (j.mag / 50.0) * 100));
      document.getElementById("magBar").style.width = pct + "%";

      // Pill de estado
      let pill = document.getElementById("statusPill");
      let icon = document.getElementById("statusIcon");
      let txt = document.getElementById("statusText");
      txt.innerText = j.level;

      if (j.level === "Fuerte") {
        pill.className = "status-pill status-fuerte";
        icon.innerText = "🚨";
        txt.innerText = "¡Caída Detectada! (Atención Inmediata)";

        // Disparar Pop-up y sonido solo si no estaba ya abierto
        if (!isModalOpen) {
          showFallModal(j);
        }
      } else if (j.level === "Moderado" || j.level === "Leve") {
        pill.className = "status-pill status-moderado";
        icon.innerText = "⚠️";
        txt.innerText = "Posible Caída / Leve (Revisión Sugerida)";
      } else {
        pill.className = "status-pill status-estable";
        icon.innerText = "●";
        txt.innerText = "Estable / Normal";
      }
      previousLevel = j.level;
    })
    .catch(e => console.error("Error al obtener /data:", e));

  fetch('/alerts')
    .then(r => r.json())
    .then(list => {
      let tbody = document.getElementById("alertsBody");
      if (!list || list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="5" class="empty-state">No se han registrado caídas ni eventos anómalos.</td></tr>';
        return;
      }

      // Si el evento más reciente es Fuerte y no se ha notificado aún en esta pantalla
      if (list[0].level === "Fuerte" && list[0].time !== lastHandledAlertTime) {
        lastHandledAlertTime = list[0].time;
        if (!isModalOpen) {
          showFallModal({
            patient_name: document.getElementById("patientName").innerText,
            patient_room: document.getElementById("patientRoom").innerText,
            mag: list[0].mag,
            time: list[0].time
          });
        }
      }

      let html = "";
      list.forEach(item => {
        let levelClass = "badge-estable";
        let actionTxt = "En reposo seguro";
        if (item.level === "Fuerte") {
          levelClass = "badge-fuerte";
          actionTxt = "🚨 Caída detectada: Se solicita atención inmediata";
        } else if (item.level === "Moderado" || item.level === "Leve") {
          levelClass = "badge-moderado";
          actionTxt = "⚠️ Posible caída: Se recomienda revisión";
        }

        let waBadge = item.sent 
          ? '<span class="badge-wa">✓ Enviado a WhatsApp</span>'
          : '<span class="badge-wa badge-wa-err">✕ No enviado</span>';

        html += `
          <tr>
            <td style="font-family:'JetBrains Mono',monospace; font-weight:600;">${item.time}</td>
            <td><span class="badge-level ${levelClass}">${item.level}</span></td>
            <td style="font-family:'JetBrains Mono',monospace; font-weight:700;">${Number(item.mag).toFixed(2)} m/s²</td>
            <td>${waBadge}</td>
            <td style="font-size:0.83rem; color:var(--text-secondary);">${actionTxt}</td>
          </tr>
        `;
      });
      tbody.innerHTML = html;
    })
    .catch(e => console.error("Error al obtener /alerts:", e));
}

document.getElementById("ip").innerText = location.hostname;
setInterval(updateData, 250);
updateData();
</script>

</body>
</html>
)rawliteral";
