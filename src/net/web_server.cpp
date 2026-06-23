#include "web_server.h"

#include "../config.h"
#include "../core/homing.h"
#include "../core/planner.h"
#include "../core/protocol.h"
#include "../drivers/drv_endstop.h"
#include "../net/wifi_manager.h"

#include <stdio.h>

#if defined(ARDUINO)
#include <WebServer.h>
#include <WiFi.h>

static WebServer server(80);

static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Plotter CNC</title>
<style>
:root{font-family:system-ui,sans-serif;color:#111;background:#f6f7f9}
body{margin:0}
main{max-width:760px;margin:0 auto;padding:18px}
h1{font-size:1.35rem;margin:0 0 12px}
h2{font-size:1rem;margin:0 0 10px}
section{border:1px solid #d7dbe2;background:#fff;border-radius:8px;padding:14px;margin:12px 0}
.row{display:flex;gap:8px;flex-wrap:wrap;align-items:center}
button,input,select{font:inherit;min-height:36px}
button{border:1px solid #1f2937;background:#1f2937;color:white;border-radius:6px;padding:0 12px}
button.secondary{background:#fff;color:#1f2937}
button:disabled{opacity:.5}
progress{width:100%;height:18px}
pre{white-space:pre-wrap;background:#101418;color:#d8f3dc;padding:10px;border-radius:6px;min-height:120px;max-height:260px;overflow:auto}
.stat{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.pill{background:#eef2f7;border-radius:6px;padding:8px}
</style>
</head>
<body>
<main>
<h1>Plotter CNC</h1>
<section class="stat">
<div class="pill">State <strong id="state">-</strong></div>
<div class="pill">Position <strong id="pos">-</strong></div>
</section>
<section>
<div class="row">
<input id="file" type="file" accept=".gcode,.nc,.txt">
<button id="start">Start Drawing</button>
<button id="pause" class="secondary">Pause</button>
<button id="stop" class="secondary">Stop</button>
</div>
<progress id="progress" value="0" max="100"></progress>
</section>
<section>
<div class="row">
<input id="cmd" placeholder="G92 X0 Y0 Z0" style="flex:1">
<button id="send">Send</button>
<button id="unlock" class="secondary">M17</button>
<button id="release" class="secondary">M18</button>
</div>
</section>
<section>
<h2>Calibration</h2>
<div class="row">
<input id="stepsX" placeholder="X steps/mm">
<input id="stepsY" placeholder="Y steps/mm">
<input id="stepsZ" placeholder="Z steps/mm">
<button id="saveCal">Save Manual</button>
<button id="loadCal" class="secondary">Reload</button>
</div>
<div class="row">
<select id="calAxis">
<option value="X">X axis</option>
<option value="Y">Y axis</option>
<option value="Z">Z axis</option>
</select>
<input id="calStep" value="1" placeholder="Jog step mm">
<input id="calFeed" value="100" placeholder="Feedrate">
<button id="markStart" class="secondary">Mark Start</button>
<button id="jogMinus" class="secondary">Jog -</button>
<button id="jogPlus" class="secondary">Jog +</button>
<button id="markEnd">This is the last it can go</button>
</div>
<div class="row">
<input id="measuredDistance" placeholder="Real travel distance mm">
<button id="applyAutoCal">Apply Button Calibration</button>
</div>
<div class="pill">Calibration: <strong id="calStatus">mark the start, jog to the physical end, then mark the last point</strong></div>
</section>
<section>
<h2>Endstop Button System</h2>
<div class="stat">
<div class="pill">X home button <strong id="btnX">-</strong></div>
<div class="pill">Y home button <strong id="btnY">-</strong></div>
<div class="pill">Z home button <strong id="btnZ">-</strong></div>
<div class="pill">Max/end buttons <strong id="btnMax">-</strong></div>
</div>
<div class="row">
<button id="readButtons" class="secondary">Read Buttons</button>
<button id="homeX">Home X Button</button>
<button id="homeY">Home Y Button</button>
<button id="homeZ">Home Z Button</button>
<button id="homeAll" class="secondary">Home All</button>
</div>
<div class="row">
<button id="buttonCal" class="secondary">Try End-Button Calibration</button>
</div>
</section>
<section><pre id="log"></pre></section>
</main>
<script>
const $=id=>document.getElementById(id);
let running=false, paused=false;
let lastStatus=null, calStart=null, calEnd=null;
function log(t){$('log').textContent+=(t.endsWith('\n')?t:t+'\n');$('log').scrollTop=$('log').scrollHeight}
function sleep(ms){return new Promise(r=>setTimeout(r,ms))}
async function command(line){
  while(running){
    const r=await fetch('/api/command',{method:'POST',headers:{'Content-Type':'text/plain'},body:line});
    const t=await r.text();
    if(r.status===409){await sleep(80);continue}
    if(t.trim()) log('> '+line+'\n'+t.trim());
    if(!r.ok) throw new Error(t.trim()||'command failed');
    return t;
  }
}
async function refresh(){
  try{
    const r=await fetch('/api/status');
    const s=await r.json();
    lastStatus=s;
    $('state').textContent=s.state;
    $('pos').textContent=s.mpos;
  }catch(e){}
}
async function statusNow(){
  await refresh();
  return lastStatus;
}
function axisPosition(status, axis){
  if(!status||!status.mpos) return 0;
  const p=status.mpos.split(',').map(Number);
  return axis==='X'?p[0]:axis==='Y'?p[1]:p[2];
}
async function loadCalibration(){
  const r=await fetch('/api/calibration');
  const c=await r.json();
  $('stepsX').value=c.x.toFixed(3);
  $('stepsY').value=c.y.toFixed(3);
  $('stepsZ').value=c.z.toFixed(3);
}
async function readButtons(){
  const r=await fetch('/api/buttons');
  const b=await r.json();
  $('btnX').textContent=b.x_min?'pressed':'open';
  $('btnY').textContent=b.y_min?'pressed':'open';
  $('btnZ').textContent=b.z_min?'pressed':'open';
  $('btnMax').textContent=(b.x_max_available||b.y_max_available||b.z_max_available)?'available':'not wired';
}
async function homeAxis(axis){
  const r=await fetch('/api/buttons/home',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({axis})});
  log(await r.text());
}
async function buttonCalibration(){
  const axis=$('calAxis').value;
  const r=await fetch('/api/buttons/calibrate',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({axis})});
  log(await r.text());
}
async function saveCalibration(){
  const body={x:parseFloat($('stepsX').value),y:parseFloat($('stepsY').value),z:parseFloat($('stepsZ').value)};
  const r=await fetch('/api/calibration',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
  log(await r.text());
  loadCalibration();
}
async function moveCalibration(){
  const axis=$('calAxis').value;
  const distance=parseFloat($('calStep').value||'0');
  const feed=parseFloat($('calFeed').value||'100');
  if(!(distance>0)){log('Enter commanded distance');return}
  running=true;
  try{
    await command('G91');
    await command(`G1 ${axis}${distance} F${feed}`);
    await command('G90');
  } finally { running=false }
}
async function jogCalibration(sign){
  const axis=$('calAxis').value;
  const step=parseFloat($('calStep').value||'0');
  const feed=parseFloat($('calFeed').value||'100');
  if(!(step>0)){log('Enter jog step');return}
  running=true;
  try{
    await command('G91');
    await command(`G1 ${axis}${sign*step} F${feed}`);
    await command('G90');
    await statusNow();
  } finally { running=false }
}
async function markCalibrationStart(){
  const axis=$('calAxis').value;
  const s=await statusNow();
  calStart={axis,pos:axisPosition(s,axis)};
  calEnd=null;
  $('calStatus').textContent=`start marked on ${axis} at ${calStart.pos.toFixed(3)} mm`;
  log(`Calibration start marked: ${axis} ${calStart.pos.toFixed(3)} mm`);
}
async function markCalibrationEnd(){
  const axis=$('calAxis').value;
  if(!calStart||calStart.axis!==axis){log('Mark the start first');return}
  const s=await statusNow();
  calEnd={axis,pos:axisPosition(s,axis)};
  const commanded=Math.abs(calEnd.pos-calStart.pos);
  $('calStatus').textContent=`last point marked on ${axis}; controller travel ${commanded.toFixed(3)} mm`;
  log(`This is the last it can go: ${axis} ${calEnd.pos.toFixed(3)} mm`);
}
async function applyAutoCalibration(){
  const axis=$('calAxis').value;
  if(!calStart||!calEnd||calStart.axis!==axis||calEnd.axis!==axis){log('Mark start and last point first');return}
  const commanded=Math.abs(calEnd.pos-calStart.pos);
  const measured=parseFloat($('measuredDistance').value);
  if(!(commanded>0)){log('Jog before applying calibration');return}
  if(!(measured>0)){log('Enter the real travel distance in mm');return}
  const body={axis,commanded,measured};
  const r=await fetch('/api/calibration/auto',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
  log(await r.text());
  $('calStatus').textContent=`calibrated ${axis}: controller ${commanded.toFixed(3)} mm, real ${measured.toFixed(3)} mm`;
  loadCalibration();
}
async function streamFile(file){
  running=true; paused=false; $('start').disabled=true; $('progress').value=0;
  const reader=file.stream().getReader();
  const dec=new TextDecoder();
  let pending='', sent=0;
  try{
    while(running){
      while(paused&&running) await sleep(100);
      const {value,done}=await reader.read();
      pending+=dec.decode(value||new Uint8Array(),{stream:!done});
      const lines=pending.split(/\r?\n/);
      pending=lines.pop();
      for(const raw of lines){
        while(paused&&running) await sleep(100);
        const line=raw.trim();
        sent+=raw.length+1;
        $('progress').value=Math.min(100,(sent/file.size)*100);
        if(line) await command(line);
        if(!running) break;
      }
      if(done) break;
    }
    if(running&&pending.trim()) await command(pending.trim());
    if(running) log('Job complete');
  }catch(e){log('Error: '+e.message)}
  running=false; $('start').disabled=false;
}
$('start').onclick=()=>{const f=$('file').files[0];if(!f){log('Choose a G-code file first');return}streamFile(f)}
$('pause').onclick=()=>{paused=!paused;$('pause').textContent=paused?'Resume':'Pause'}
$('stop').onclick=()=>{running=false;log('Stopped')}
$('send').onclick=()=>{running=true;command($('cmd').value).finally(()=>running=false)}
$('unlock').onclick=()=>{running=true;command('M17').finally(()=>running=false)}
$('release').onclick=()=>{running=true;command('M18').finally(()=>running=false)}
$('loadCal').onclick=loadCalibration
$('saveCal').onclick=saveCalibration
$('markStart').onclick=markCalibrationStart
$('jogMinus').onclick=()=>jogCalibration(-1)
$('jogPlus').onclick=()=>jogCalibration(1)
$('markEnd').onclick=markCalibrationEnd
$('applyAutoCal').onclick=applyAutoCalibration
$('readButtons').onclick=readButtons
$('homeX').onclick=()=>homeAxis('X')
$('homeY').onclick=()=>homeAxis('Y')
$('homeZ').onclick=()=>homeAxis('Z')
$('homeAll').onclick=()=>homeAxis('ALL')
$('buttonCal').onclick=buttonCalibration
setInterval(refresh,1000); refresh(); loadCalibration(); readButtons();
</script>
</body>
</html>
)HTML";

static void send_no_cache() {
    server.sendHeader("Cache-Control", "no-store");
}

static void handle_index() {
    send_no_cache();
    server.send_P(200, "text/html", INDEX_HTML);
}

static void handle_status() {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    planner_current_position(&x, &y, &z);

    char json[160];
    snprintf(json, sizeof(json),
             "{\"state\":\"%s\",\"mpos\":\"%.3f,%.3f,%.3f\",\"buffer\":%u,\"wifi\":\"%s\"}",
             planner_has_work() ? "Run" : "Idle",
             x,
             y,
             z,
             planner_buffer_available(),
             wifi_manager_state_text());
    send_no_cache();
    server.send(200, "application/json", json);
}

static void handle_command() {
    if (planner_buffer_available() == 0) {
        server.send(409, "text/plain", "busy\n");
        return;
    }

    const String body = server.arg("plain");
    char response[224];
    const bool ok = protocol_execute_command(body.c_str(), response, sizeof(response));
    server.send(ok ? 200 : 400, "text/plain", response);
}

static String json_value(const String &body, const char *key) {
    String pattern = String("\"") + key + "\"";
    int at = body.indexOf(pattern);
    if (at < 0) {
        return "";
    }
    at = body.indexOf(':', at);
    if (at < 0) {
        return "";
    }
    int first_quote = body.indexOf('"', at + 1);
    if (first_quote < 0) {
        return "";
    }
    int second_quote = body.indexOf('"', first_quote + 1);
    if (second_quote < 0) {
        return "";
    }
    return body.substring(first_quote + 1, second_quote);
}

static float json_number(const String &body, const char *key, float fallback) {
    String pattern = String("\"") + key + "\"";
    int at = body.indexOf(pattern);
    if (at < 0) {
        return fallback;
    }
    at = body.indexOf(':', at);
    if (at < 0) {
        return fallback;
    }
    int end = at + 1;
    while (end < body.length() && body[end] != ',' && body[end] != '}') {
        ++end;
    }
    return body.substring(at + 1, end).toFloat();
}

static MotorAxis axis_from_text(const String &axis) {
    if (axis == "Y" || axis == "y") {
        return MOTOR_AXIS_Y;
    }
    if (axis == "Z" || axis == "z") {
        return MOTOR_AXIS_Z;
    }
    return MOTOR_AXIS_X;
}

static float travel_for_axis(MotorAxis axis) {
    if (axis == MOTOR_AXIS_Y) {
        return Y_TRAVEL_MM;
    }
    if (axis == MOTOR_AXIS_Z) {
        return Z_TRAVEL_MM;
    }
    return X_TRAVEL_MM;
}

static void handle_calibration_get() {
    char json[120];
    snprintf(json, sizeof(json),
             "{\"x\":%.3f,\"y\":%.3f,\"z\":%.3f}",
             planner_steps_per_mm(MOTOR_AXIS_X),
             planner_steps_per_mm(MOTOR_AXIS_Y),
             planner_steps_per_mm(MOTOR_AXIS_Z));
    send_no_cache();
    server.send(200, "application/json", json);
}

static void handle_calibration_set() {
    const String body = server.arg("plain");
    const float x = json_number(body, "x", planner_steps_per_mm(MOTOR_AXIS_X));
    const float y = json_number(body, "y", planner_steps_per_mm(MOTOR_AXIS_Y));
    const float z = json_number(body, "z", planner_steps_per_mm(MOTOR_AXIS_Z));

    const bool ok = planner_set_steps_per_mm(MOTOR_AXIS_X, x) &&
                    planner_set_steps_per_mm(MOTOR_AXIS_Y, y) &&
                    planner_set_steps_per_mm(MOTOR_AXIS_Z, z);
    send_no_cache();
    server.send(ok ? 200 : 400, "text/plain", ok ? "ok\n" : "error:invalid calibration\n");
}

static void handle_calibration_auto() {
    const String body = server.arg("plain");
    const MotorAxis axis = axis_from_text(json_value(body, "axis"));
    const float commanded = json_number(body, "commanded", 0.0f);
    const float measured = json_number(body, "measured", 0.0f);
    float updated = 0.0f;

    const bool ok = planner_calibrate_steps_per_mm(axis, commanded, measured, &updated);
    char response[80];
    snprintf(response, sizeof(response), ok ? "ok:steps_per_mm=%.3f\n" : "error:invalid measured distance\n", updated);
    send_no_cache();
    server.send(ok ? 200 : 400, "text/plain", response);
}

static void handle_buttons_status() {
    char json[240];
    snprintf(json, sizeof(json),
             "{\"x_min\":%s,\"y_min\":%s,\"z_min\":%s,"
             "\"x_max_available\":%s,\"y_max_available\":%s,\"z_max_available\":%s,"
             "\"x_max\":%s,\"y_max\":%s,\"z_max\":%s}",
             drv_endstop_is_triggered(MOTOR_AXIS_X) ? "true" : "false",
             drv_endstop_is_triggered(MOTOR_AXIS_Y) ? "true" : "false",
             drv_endstop_is_triggered(MOTOR_AXIS_Z) ? "true" : "false",
             drv_endstop_max_is_available(MOTOR_AXIS_X) ? "true" : "false",
             drv_endstop_max_is_available(MOTOR_AXIS_Y) ? "true" : "false",
             drv_endstop_max_is_available(MOTOR_AXIS_Z) ? "true" : "false",
             drv_endstop_max_is_triggered(MOTOR_AXIS_X) ? "true" : "false",
             drv_endstop_max_is_triggered(MOTOR_AXIS_Y) ? "true" : "false",
             drv_endstop_max_is_triggered(MOTOR_AXIS_Z) ? "true" : "false");
    send_no_cache();
    server.send(200, "application/json", json);
}

static void handle_buttons_home() {
    const String body = server.arg("plain");
    const String axis_text = json_value(body, "axis");
    if (axis_text == "ALL" || axis_text == "all") {
        homing_start();
    } else {
        homing_start_axis(axis_from_text(axis_text));
    }
    send_no_cache();
    server.send(200, "text/plain", "ok:homing started\n");
}

static void handle_buttons_calibrate() {
    const String body = server.arg("plain");
    const MotorAxis axis = axis_from_text(json_value(body, "axis"));
    send_no_cache();

    if (!drv_endstop_max_is_available(axis)) {
        server.send(400, "text/plain",
                    "error:max/end button not configured for this axis. Wire a second end button and set *_MAX_LIMIT_PIN in hal_esp32.h\n");
        return;
    }

    if (!drv_endstop_is_triggered(axis)) {
        homing_start_axis(axis);
        server.send(409, "text/plain", "busy:home button is not pressed yet. Homing started; retry after it reaches the home button.\n");
        return;
    }

    if (!drv_endstop_max_is_triggered(axis)) {
        server.send(409, "text/plain", "busy:move toward the far end until the max/end button is pressed, then retry.\n");
        return;
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    planner_current_position(&x, &y, &z);

    const float controller_travel = fabs(axis == MOTOR_AXIS_X ? x : axis == MOTOR_AXIS_Y ? y : z);
    const float physical_travel = travel_for_axis(axis);
    if (controller_travel <= 0.0f || physical_travel <= 0.0f) {
        server.send(400, "text/plain", "error:invalid travel distance for end-button calibration\n");
        return;
    }

    float updated = 0.0f;
    const bool ok = planner_calibrate_steps_per_mm(axis, controller_travel, physical_travel, &updated);
    char response[96];
    snprintf(response, sizeof(response), ok ? "ok:end-button calibration confirmed steps_per_mm=%.3f\n" : "error:button calibration failed\n", updated);
    server.send(ok ? 200 : 400, "text/plain", response);
}

#endif

void web_server_init() {
#if defined(ARDUINO)
    server.on("/", HTTP_GET, handle_index);
    server.on("/api/status", HTTP_GET, handle_status);
    server.on("/api/command", HTTP_POST, handle_command);
    server.on("/api/calibration", HTTP_GET, handle_calibration_get);
    server.on("/api/calibration", HTTP_POST, handle_calibration_set);
    server.on("/api/calibration/auto", HTTP_POST, handle_calibration_auto);
    server.on("/api/buttons", HTTP_GET, handle_buttons_status);
    server.on("/api/buttons/home", HTTP_POST, handle_buttons_home);
    server.on("/api/buttons/calibrate", HTTP_POST, handle_buttons_calibrate);
    server.begin();
#endif
}

void web_server_task() {
#if defined(ARDUINO)
    server.handleClient();
#endif
}
