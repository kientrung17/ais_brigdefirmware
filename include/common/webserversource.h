namespace SourceWebserver
{
const char *html_config = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Relay + Power Monitor</title>
  <style>
    :root{
      --bg1:#0f2027; --bg2:#203a43; --bg3:#2c5364;
      --card:#0b1116cc; --text:#eaf2f6; --muted:#9fb2bf;
      --accent:#ff8a00; --accent2:#ffd200;
      --ok:#2dd4bf; --bad:#f97316; --warn:#fbbf24;
      --radius:16px; --shadow:0 20px 50px rgba(0,0,0,.35);
    }
    *{box-sizing:border-box}
    body{
      margin:0; color:var(--text);
      font-family:"Space Grotesk","Segoe UI",sans-serif;
      background:radial-gradient(1200px 600px at 10% -10%, #2f80ed55, transparent),
                 radial-gradient(900px 700px at 110% 0%, #f2994a55, transparent),
                 linear-gradient(120deg,var(--bg1),var(--bg2) 45%,var(--bg3));
      min-height:100vh; padding:28px;
    }
    header{
      display:flex; align-items:flex-end; justify-content:space-between;
      margin-bottom:20px; gap:12px; flex-wrap:wrap;
    }
    h1{margin:0; font-size:28px; letter-spacing:.5px}
    .subtitle{color:var(--muted); font-size:13px}
    .grid{
      display:grid; gap:18px;
      grid-template-columns:repeat(12,1fr);
    }
    .card{
      background:var(--card); border-radius:var(--radius); box-shadow:var(--shadow);
      padding:18px; border:1px solid rgba(255,255,255,.06); backdrop-filter:blur(6px);
    }
    .relay-panel{grid-column:span 7}
    .power-panel{grid-column:span 5}
    @media (max-width:900px){
      .relay-panel,.power-panel{grid-column:span 12}
    }
    .section-title{display:flex; align-items:center; gap:10px; margin:0 0 12px}
    .dot{width:10px; height:10px; border-radius:50%; background:linear-gradient(120deg,var(--accent),var(--accent2));}
    .relay-grid{display:grid; grid-template-columns:repeat(3,1fr); gap:12px}
    @media (max-width:700px){ .relay-grid{grid-template-columns:repeat(2,1fr);} }
    .relay-card{padding:14px; border-radius:12px; background:rgba(255,255,255,.03); border:1px solid rgba(255,255,255,.06)}
    .relay-head{display:flex; align-items:center; justify-content:space-between; margin-bottom:8px}
    .badge{padding:4px 8px; border-radius:999px; font-size:11px; letter-spacing:.3px}
    .badge.on{background:rgba(45,212,191,.18); color:var(--ok); border:1px solid rgba(45,212,191,.35)}
    .badge.off{background:rgba(249,115,22,.15); color:var(--bad); border:1px solid rgba(249,115,22,.35)}
    .badge.na{background:rgba(148,163,184,.15); color:var(--muted); border:1px solid rgba(148,163,184,.25)}
    .relay-btn{
      width:100%; padding:10px 12px; border-radius:10px; border:none; cursor:pointer;
      background:linear-gradient(120deg,var(--accent),var(--accent2)); color:#1b1b1b; font-weight:600;
      transition:transform .12s ease, filter .12s ease;
    }
    .relay-btn:active{transform:scale(.98)}
    .relay-btn.off{background:rgba(148,163,184,.2); color:#dbe4ea}
    .metrics{display:grid; gap:12px}
    .metric{display:flex; align-items:center; justify-content:space-between; padding:10px 12px;
      background:rgba(255,255,255,.03); border-radius:12px; border:1px solid rgba(255,255,255,.06)}
    .metric .label{color:var(--muted); font-size:12px}
    .metric .value{font-size:18px; font-weight:600}
    .status-row{display:flex; gap:10px; flex-wrap:wrap; margin-top:10px}
    .status-pill{padding:6px 10px; border-radius:999px; font-size:12px; border:1px solid transparent}
    .status-pill.ok{color:var(--ok); border-color:rgba(45,212,191,.35); background:rgba(45,212,191,.12)}
    .status-pill.bad{color:var(--bad); border-color:rgba(249,115,22,.35); background:rgba(249,115,22,.12)}
    .footer{margin-top:14px; color:var(--muted); font-size:12px}
  </style>
</head>
<body>
  <header>
    <div>
      <h1>Control Hub Dashboard</h1>
      <div class="subtitle">Relay control + power monitor</div>
    </div>
    <div class="subtitle" id="lastUpdate">Last update: --</div>
  </header>

  <main class="grid">
    <section class="card relay-panel">
      <h3 class="section-title"><span class="dot"></span>Relay Control</h3>
      <div class="relay-grid" id="relayGrid"></div>
    </section>

    <section class="card power-panel">
      <h3 class="section-title"><span class="dot"></span>Power Monitor</h3>
      <div class="metrics">
        <div class="metric"><div class="label">Channel 1 Voltage</div><div class="value" id="v1">-- V</div></div>
        <div class="metric"><div class="label">Channel 2 Voltage</div><div class="value" id="v2">-- V</div></div>
        <div class="metric"><div class="label">Channel 1 Current</div><div class="value" id="a1">-- A</div></div>
        <div class="metric"><div class="label">Channel 2 Current</div><div class="value" id="a2">-- A</div></div>
      </div>
      <div class="status-row">
        <div class="status-pill" id="lostPhase">Lost phase: --</div>
        <div class="status-pill" id="lostElectric">Lost electric: --</div>
      </div>
      <div class="footer">Data refreshes automatically every 1.5 seconds.</div>
    </section>
  </main>

<script>
const RELAY_COUNT = 6;
const relayGrid = document.getElementById('relayGrid');
const relayState = Array(RELAY_COUNT).fill(-1);

function formatValue(val){
  if (typeof val !== 'number' || Number.isNaN(val)) return '--';
  return val.toFixed(2);
}

function setLastUpdate(){
  const now = new Date();
  document.getElementById('lastUpdate').textContent = `Last update: ${now.toLocaleTimeString()}`;
}

function renderRelays(){
  relayGrid.innerHTML = '';
  for(let i=0;i<RELAY_COUNT;i++){
    const card = document.createElement('div');
    card.className = 'relay-card';
    const head = document.createElement('div');
    head.className = 'relay-head';

    const title = document.createElement('div');
    title.textContent = `Relay ${i+1}`;

    const badge = document.createElement('div');
    badge.className = 'badge na';
    badge.textContent = 'Unknown';

    const btn = document.createElement('button');
    btn.className = 'relay-btn off';
    btn.textContent = 'Turn On';
    btn.addEventListener('click', () => {
      const next = relayState[i] === 1 ? 0 : 1;
      sendRelay(i, next);
    });

    head.appendChild(title);
    head.appendChild(badge);
    card.appendChild(head);
    card.appendChild(btn);
    relayGrid.appendChild(card);

    card.dataset.channel = i;
    card.dataset.badgeId = i;
    card.dataset.buttonId = i;
  }
}

function updateRelayUI(){
  const cards = relayGrid.querySelectorAll('.relay-card');
  cards.forEach((card, idx) => {
    const badge = card.querySelector('.badge');
    const btn = card.querySelector('button');
    const state = relayState[idx];
    if (state === 1){
      badge.className = 'badge on';
      badge.textContent = 'ON';
      btn.className = 'relay-btn';
      btn.textContent = 'Turn Off';
    }else if (state === 0){
      badge.className = 'badge off';
      badge.textContent = 'OFF';
      btn.className = 'relay-btn off';
      btn.textContent = 'Turn On';
    }else{
      badge.className = 'badge na';
      badge.textContent = 'Unknown';
      btn.className = 'relay-btn off';
      btn.textContent = 'Retry';
    }
  });
}

async function fetchRelays(){
  try{
    const res = await fetch('/api/relay');
    if(!res.ok) throw new Error('HTTP '+res.status);
    const data = await res.json();
    if(Array.isArray(data.relays)){
      data.relays.forEach(r => {
        if(r && r.channel >= 0 && r.channel < RELAY_COUNT){
          relayState[r.channel] = r.status;
        }
      });
      updateRelayUI();
    }
  }catch(e){
    console.warn('Relay fetch failed', e);
  }
}

async function sendRelay(channel, status){
  try{
    const body = new URLSearchParams({ channel: String(channel), status: String(status) }).toString();
    const res = await fetch('/api/relay', {
      method:'POST',
      headers:{ 'Content-Type':'application/x-www-form-urlencoded;charset=UTF-8' },
      body
    });
    if(!res.ok) throw new Error('HTTP '+res.status);
    relayState[channel] = status;
    updateRelayUI();
  }catch(e){
    console.warn('Relay update failed', e);
  }
}

async function fetchPower(){
  try{
    const res = await fetch('/api/power');
    if(!res.ok) throw new Error('HTTP '+res.status);
    const data = await res.json();
    document.getElementById('v1').textContent = `${formatValue(data.voltage?.[0])} V`;
    document.getElementById('v2').textContent = `${formatValue(data.voltage?.[1])} V`;
    document.getElementById('a1').textContent = `${formatValue(data.ampe?.[0])} A`;
    document.getElementById('a2').textContent = `${formatValue(data.ampe?.[1])} A`;

    const lostPhase = document.getElementById('lostPhase');
    lostPhase.textContent = `Lost phase: ${data.lost_phase ? 'YES' : 'NO'}`;
    lostPhase.className = data.lost_phase ? 'status-pill bad' : 'status-pill ok';

    const lostElectric = document.getElementById('lostElectric');
    lostElectric.textContent = `Lost electric: ${data.lost_electric ? 'YES' : 'NO'}`;
    lostElectric.className = data.lost_electric ? 'status-pill bad' : 'status-pill ok';

    setLastUpdate();
  }catch(e){
    console.warn('Power fetch failed', e);
  }
}

function start(){
  renderRelays();
  updateRelayUI();
  fetchRelays();
  fetchPower();
  setInterval(() => { fetchRelays(); fetchPower(); }, 1500);
}

document.addEventListener('DOMContentLoaded', start);
</script>
</body>
</html>
)rawliteral";
}
