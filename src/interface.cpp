#include "interface.h"
#include <WiFi.h>
#include <WebServer.h>

static WebServer server(80);
static VehicleData* dataPtr = nullptr;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Touareg Dashboard</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@500;600;700&display=swap" rel="stylesheet">
<style>
  :root {
    --bg: #07090c;
    --panel: #0d1015;
    --line: #202630;
    --text: #f4f7fb;
    --muted: #7f8998;
    --blue: #66dcff;
    --amber: #ffbd4a;
    --red: #ff4d57;
    --green: #55e69b;
  }
  * { box-sizing: border-box; }
  body {
    margin: 0; min-height: 100vh;
    display: grid; place-items: center;
    padding: 20px;
    background: radial-gradient(circle at 50% 15%, #1a2028 0, #0b0e12 38%, #050608 80%);
    color: var(--text);
    font-family: Inter, ui-sans-serif, system-ui, sans-serif;
  }
  .cluster {
    width: min(720px, 100%);
    padding: 28px 24px;
    border: 1px solid #262d37;
    border-radius: 30px;
    background: linear-gradient(135deg, rgba(255,255,255,.025), transparent 35%), var(--panel);
    box-shadow: 0 40px 100px #000a, inset 0 1px 0 #ffffff0b;
    display: grid; gap: 24px;
  }

  .shift-wrap { display: grid; gap: 10px; }
  .shift-labels {
    display: flex; justify-content: space-between; align-items: baseline;
    color: var(--muted); font-size: 11px; font-weight: 650;
    letter-spacing: .14em; text-transform: uppercase;
  }
  .shift-labels strong { color: var(--text); font-size: 14px; letter-spacing: .04em; }
  .shift-lights { height: 10px; display: grid; grid-template-columns: repeat(20, 1fr); gap: 5px; }
  .shift-light { border-radius: 3px; background: #242a33; transition: background .08s, box-shadow .08s; }
  .shift-light.on-blue { background: var(--blue); box-shadow: 0 0 10px #66dcff88; }
  .shift-light.on-amber { background: var(--amber); box-shadow: 0 0 10px #ffbd4a88; }
  .shift-light.on-red { background: var(--red); box-shadow: 0 0 12px #ff4d5788; }

  .main-grid {
    display: grid;
    grid-template-columns: 1fr minmax(160px, 1.3fr) 1fr;
    align-items: center;
    gap: 20px;
  }
  .telemetry { display: grid; gap: 18px; }
  .metric { padding-bottom: 14px; border-bottom: 1px solid var(--line); }
  .metric:last-child { border-bottom: 0; padding-bottom: 0; }
  .metric-label {
    display: block; margin-bottom: 5px; color: var(--muted);
    font-size: 10px; font-weight: 650; letter-spacing: .14em; text-transform: uppercase;
  }
  .metric-value {
    display: flex; align-items: baseline; gap: 4px;
    font-size: clamp(18px, 2vw, 24px); font-weight: 500;
    letter-spacing: -.03em; font-variant-numeric: tabular-nums;
  }
  .metric-value small { color: var(--muted); font-size: .5em; font-weight: 600; }

  .speed-block { text-align: center; }
  .speed {
    font-size: clamp(64px, 12vw, 96px); font-weight: 500; line-height: .8;
    letter-spacing: -.06em; font-variant-numeric: tabular-nums;
  }
  .unit {
    margin-top: 10px; color: var(--muted); font-size: 12px;
    font-weight: 650; letter-spacing: .16em; text-transform: uppercase;
  }

  .bottom {
    display: grid; grid-template-columns: 1fr auto;
    align-items: end; gap: 20px;
    padding-top: 18px; border-top: 1px solid var(--line);
  }
  .status {
    display: flex; align-items: center; gap: 8px;
    color: var(--muted); font-size: 11px; font-weight: 650;
    letter-spacing: .08em; text-transform: uppercase;
  }
  .status-dot {
    width: 7px; height: 7px; border-radius: 50%;
    background: var(--muted); transition: background .2s, box-shadow .2s;
  }
  .status-dot.live { background: var(--green); box-shadow: 0 0 10px #55e69b88; }
  .status-dot.lost { background: var(--red); box-shadow: 0 0 10px #ff4d5788; }

  .pedal-wrap { justify-self: end; min-width: 150px; }
  .pedal-copy {
    display: flex; justify-content: space-between; margin-bottom: 6px;
    color: var(--muted); font-size: 10px; font-weight: 650;
    letter-spacing: .1em; text-transform: uppercase;
  }
  .pedal-track { height: 5px; overflow: hidden; border-radius: 4px; background: #262c34; }
  .pedal-fill { width: 0; height: 100%; border-radius: inherit; background: var(--blue); box-shadow: 0 0 10px #66dcff88; transition: width .1s linear; }

  @media (max-width: 480px) {
    .main-grid { grid-template-columns: 1fr 1fr; }
    .speed-block { grid-column: 1 / -1; grid-row: 1; }
  }
</style>
</head>
<body>
  <main class="cluster">
    <section class="shift-wrap">
      <div class="shift-labels"><span>RPM</span><strong id="rpmText">0</strong><span id="redlineLabel">-- redline</span></div>
      <div class="shift-lights" id="shiftLights"></div>
    </section>

    <section class="main-grid">
      <div class="telemetry">
        <div class="metric"><span class="metric-label">Coolant</span><span class="metric-value"><span id="coolant">--</span><small>&deg;C</small></span></div>
        <div class="metric"><span class="metric-label">Intake</span><span class="metric-value"><span id="intake">--</span><small>&deg;C</small></span></div>
        <div class="metric" id="manifoldRow" style="display:none;">
          <span class="metric-label">Manifold Pressure</span>
          <span class="metric-value"><span id="manifold">--</span><small>kPa</small></span>
        </div>
      </div>

      <div class="speed-block">
        <div class="speed" id="speed">0</div>
        <div class="unit">km/h</div>
      </div>

      <div class="telemetry">
        <div class="metric"><span class="metric-label">Engine Load</span><span class="metric-value"><span id="load">--</span><small>%</small></span></div>
        <div class="metric" id="mafRow" style="display:none;">
          <span class="metric-label">Air Flow</span>
          <span class="metric-value"><span id="maf">--</span><small>g/s</small></span>
        </div>
      </div>
    </section>

    <footer class="bottom">
      <div class="status"><span class="status-dot" id="statusDot"></span><span id="statusText">Connecting</span></div>
      <div class="pedal-wrap">
        <div class="pedal-copy"><span>Throttle</span><span id="throttleText">0%</span></div>
        <div class="pedal-track"><div class="pedal-fill" id="pedalFill"></div></div>
      </div>
    </footer>
  </main>

<script>
const REDLINE_RPM = 6500; // set to your car's actual redline
const MAX_RPM = 7000;
const LED_COUNT = 20;

const lightBar = document.getElementById('shiftLights');
for (let i = 0; i < LED_COUNT; i++) {
  const seg = document.createElement('div');
  seg.className = 'shift-light';
  seg.id = 'led' + i;
  lightBar.appendChild(seg);
}
document.getElementById('redlineLabel').innerText = REDLINE_RPM.toLocaleString() + ' redline';

function updateLeds(rpm) {
  const litCount = Math.round((rpm / MAX_RPM) * LED_COUNT);
  for (let i = 0; i < LED_COUNT; i++) {
    const led = document.getElementById('led' + i);
    led.classList.remove('on-blue', 'on-amber', 'on-red');
    if (i < litCount) {
      const ledRpmValue = (i / LED_COUNT) * MAX_RPM;
      if (ledRpmValue >= REDLINE_RPM) led.classList.add('on-red');
      else if (ledRpmValue >= REDLINE_RPM * 0.7) led.classList.add('on-amber');
      else led.classList.add('on-blue');
    }
  }
}

let lastSuccessfulFetch = 0;

async function update() {
  try {
    const res = await fetch('/data');
    const d = await res.json();
    lastSuccessfulFetch = Date.now();

    document.getElementById('rpmText').innerText = d.rpm.toLocaleString();
    document.getElementById('speed').innerText = d.speed;
    document.getElementById('coolant').innerText = d.coolant;
    document.getElementById('intake').innerText = d.intake;
    document.getElementById('load').innerText = d.load;
    document.getElementById('throttleText').innerText = d.throttle + '%';
    document.getElementById('pedalFill').style.width = d.throttle + '%';

    if (d.mafSupported) {
      document.getElementById('mafRow').style.display = '';
      document.getElementById('maf').innerText = d.maf.toFixed(1);
    }

    if (d.manifoldSupported) {
      document.getElementById('manifoldRow').style.display = '';
      document.getElementById('manifold').innerText = d.manifold;
    }

    if (d.fuelSupported) {
      document.getElementById('statusText').innerText = d.fuel + '% Fuel';
    }

    updateLeds(d.rpm);
  } catch (e) { /* handled by the connection-health check below */ }
}

function checkConnection() {
  const dot = document.getElementById('statusDot');
  const msSinceUpdate = Date.now() - lastSuccessfulFetch;

  if (lastSuccessfulFetch === 0) {
    dot.className = 'status-dot';
  } else if (msSinceUpdate < 1000) {
    dot.className = 'status-dot live';
  } else {
    dot.className = 'status-dot lost';
    document.getElementById('statusText').innerText = 'No signal';
  }
}

setInterval(update, 150);
setInterval(checkConnection, 300);
update();
</script>
</body>
</html>
)rawliteral";

static void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

static void handleData() {
  String json = "{";
  json += "\"rpm\":" + String(dataPtr->rpm) + ",";
  json += "\"speed\":" + String(dataPtr->speedKph) + ",";
  json += "\"coolant\":" + String(dataPtr->coolantTempC) + ",";
  json += "\"intake\":" + String(dataPtr->intakeTempC) + ",";
  json += "\"load\":" + String(dataPtr->engineLoadPct) + ",";
  json += "\"throttle\":" + String(dataPtr->throttlePct) + ",";
  json += "\"maf\":" + String(dataPtr->mafGramsPerSec) + ",";
  json += "\"mafSupported\":" + String(dataPtr->mafSupported ? "true" : "false") + ",";
  json += "\"manifold\":" + String(dataPtr->manifoldPressureKpa) + ",";
  json += "\"manifoldSupported\":" + String(dataPtr->manifoldPressureSupported ? "true" : "false") + ",";
  json += "\"fuel\":" + String(dataPtr->fuelLevelPct) + ",";
  json += "\"fuelSupported\":" + String(dataPtr->fuelLevelSupported ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void WebInterface::setData(VehicleData* data) {
  dataPtr = data;
}

void WebInterface::begin(const char* apSsid, const char* apPassword) {
  // channel 1, not hidden, max 2 devices connected at once
  if (!WiFi.softAP(apSsid, apPassword, 1, 0, 2)) {
    Serial.println("Wi-Fi failed to start (the password needs to be at least 8 characters)");
    return;
  }
  Serial.print("Wi-Fi started, connect to: ");
  Serial.println(apSsid);
  Serial.print("then open http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void WebInterface::handle() {
  server.handleClient();
}
