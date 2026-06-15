/*!
 *  \file web_server_data.h
 *
 *  \brief
 *  Application Web Server File Data
 *
 *  \author
 *  Texas Instruments Incorporated
 *
 *  \copyright
 *  Copyright (C) 2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GSP_WS_JS_H
#define GSP_WS_JS_H

#ifdef __cplusplus
extern "C" {
#endif

    static const char main_js[] =
    "function getCpuLoad()"
    "{"
        "var cpuLoad = 0;"
        "function getCpuLoadComplete()"
        "{"
            "if(cpuLoad.readyState == 4)"
            "{"
                "if(cpuLoad.status == 200)"
                "{"
                    "if(cpuLoad.responseText != null)"
                    "{"
                        "var rowsNum = 20;"
                        "var id = \"\";"
                        "var row = 0;"
                        "var data = cpuLoad.responseText;"
                        "const dataArray = data.split(\",\");"
                        "for (let i = 0; i < dataArray.length; i = i + 2)"
                        "{"
                            "row = i/2;"
                            "if (rowsNum > i/2)"
                            "{"
                                "id = \"r\" + row + \"c0\";"
                                "document.getElementById(id).innerHTML = dataArray[i];"
                                "id = \"r\" + row + \"c1\";"
                                "document.getElementById(id).innerHTML = dataArray[i + 1];"
                            "}"
                        "}"
                    "}"
                "}"
            "}"
        "}"
        "cpuLoad = new XMLHttpRequest();"
        "if(cpuLoad)"
        "{"
            "cpuLoad.open(\"GET\", \"/cpuLoad?id=\" + Math.random(), true);"
            "cpuLoad.responseType = \"text\";"
            "cpuLoad.onreadystatechange = getCpuLoadComplete;"
            "cpuLoad.send(null);"
        "}"
        "setTimeout('getCpuLoad()', 1000);"
    "}";

    static const char iomap_js[] =
    "let svgTemplateIO=null;"
    "let svgTemplatePLC=null;"
    "let modulesCreated=false;"
    "let moduleMap={};"
    "let tooltip=null;"
    "let activeModuleId=null;" // Tracks which module is currently under the mouse
    "let scanInProgress = false;"
    
    "const LED_LEFT_X = 18;"
    "const LED_RIGHT_X = 30;"
    "const LED_Y_ROWS = [75, 84, 93, 102, 112, 121, 129, 139];"
    
    "const PIN_LEFT_X = 21;"
    "const PIN_RIGHT_X = 44;"
    "const PIN_Y_ROWS = [207, 242, 277, 309, 344, 378, 408, 443];"

    "const nmtStates = {"
    "   0: \"Boot-up\","
    "   4: \"Stopped\","
    "   5: \"Operational\","
    "   127: \"Pre-operational\""
    "};"

    "const PRODUCTS={"
    "   1:{name:'DO',channels:16,digital:true},"
    "   2:{name:'DI',channels:16,digital:true},"
    "   3:{name:'AIC',channels:8,digital:false},"
    "   4:{name:'AIV',channels:8,digital:false},"
    "   5:{name:'AOC',channels:8,digital:false},"
    "   6:{name:'AOV',channels:8,digital:false},"
    "   7:{name:'RTDY',channels:6,digital:false},"
    "   8:{name:'RTDB',channels:6,digital:false}"
    "};"
    
    "function showToast(message, type) {"
    "   const colors = { success: '#00ff66', error: '#ff4444', warn: '#ffaa00', info: '#4da6ff' };"
    "   const icons  = { success: '✅', error: '❌', warn: '⚠️', info: 'ℹ️' };"
    "   const toast = document.createElement('div');"
    "   toast.style.cssText = ["
    "       'position:fixed;top:20px;right:20px;z-index:9999;',"
    "       'background:#1e1e2e;color:#eee;border-radius:8px;',"
    "       'padding:14px 18px;max-width:320px;font-size:13px;',"
    "       'box-shadow:0 4px 16px rgba(0,0,0,0.5);',"
    "       `border-left:4px solid ${colors[type]||colors.info};`,"
    "       'transition:opacity 0.4s ease'"
    "   ].join('');"
    "   toast.innerHTML = `<b>${icons[type]||''} ${message}</b>`;"
    "   document.body.appendChild(toast);"
    "   setTimeout(() => { toast.style.opacity='0'; setTimeout(()=>toast.remove(),400); }, 3500);"
    "}"

    "function getBit(v,b){return (v>>b)&1;}"

    "function getLedPosition(ch) {"
    "   const isRight = (ch % 2) !== 0;"
    "   const rowIndex = Math.floor(ch / 2);"
    "   return { x: isRight ? LED_RIGHT_X : LED_LEFT_X, y: LED_Y_ROWS[rowIndex] };"
    "}"

    "function getPinPosition(ch) {"
    "   const isRight = (ch % 2) !== 0;"
    "   const rowIndex = Math.floor(ch / 2);"
    "   return { cx: isRight ? PIN_RIGHT_X : PIN_LEFT_X, cy: PIN_Y_ROWS[rowIndex] };"
    "}"
    
    "function normalizePin(svg, ch) {"
    "   let led = svg.querySelector(`.led-ch-${ch}`);"
    "   let io = svg.querySelector(`.io-ch-${ch}`);"
    "   const ledPos = getLedPosition(ch);"
    "   const pinPos = getPinPosition(ch);"
    "   if (!led) {"
    "       led = document.createElementNS(\"http://www.w3.org/2000/svg\", \"rect\");"
    "       led.setAttribute(\"class\", `status-indicator pin-led led-ch-${ch}`);"
    "       led.setAttribute(\"x\", ledPos.x - 2);" 
    "       led.setAttribute(\"y\", ledPos.y - 2);"
    "       led.setAttribute(\"width\", \"4\");"
    "       led.setAttribute(\"height\", \"4\");"
    "       svg.appendChild(led);"
    "   }"
    "   if (!io) {"
    "       io = document.createElementNS(\"http://www.w3.org/2000/svg\", \"circle\");"
    "       io.setAttribute(\"class\", `status-indicator pin-io io-ch-${ch}`);"
    "       io.setAttribute(\"cx\", pinPos.cx);"
    "       io.setAttribute(\"cy\", pinPos.cy);"
    "       io.setAttribute(\"r\", \"9\");"
    "       svg.appendChild(io);"
    "   }"
    "}"

    "function buildPinMap(svg) {"
    "   const map = new Map();"
    "   for (let ch = 0; ch < 16; ch++) {"
    "       normalizePin(svg, ch);"
    "       map.set(ch, {"
    "           led: svg.querySelector(`.led-ch-${ch}`),"
    "           io: svg.querySelector(`.io-ch-${ch}`)"
    "       });"
    "   }"
    "   return map;"
    "}"

    "async function loadSvgTemplate(){"
    "   try{"
    "       const r=await fetch('/iomap.svg');"
    "       if(!r.ok) throw new Error('SVG load failed');"
    "       const txt=await r.text();"
    "       svgTemplateIO=new DOMParser().parseFromString(txt,'image/svg+xml');"
    "   }catch(e){console.error(e);}"
    "   try{"
    "       const r=await fetch('/plcmap.svg');"
    "       if(!r.ok) throw new Error('SVG load failed');"
    "       const txt=await r.text();"
    "       svgTemplatePLC=new DOMParser().parseFromString(txt,'image/svg+xml');"
    "   }catch(e){console.error(e);}"
    "   poll();"
    "}"

    "function updateSummaryTable(data) {"
    "   const counts = { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 0 };"
    "   data.forEach(io => {"
    "       if (counts[io.productCode] !== undefined) {"
    "           counts[io.productCode]++;"
    "       }"
    "   });"

    "   document.getElementById('DOCount').textContent   = counts[1] || '0';" 
    "   document.getElementById('DICount').textContent   = counts[2] || '0';"
    "   document.getElementById('AICCount').textContent  = counts[3] || '0';"
    "   document.getElementById('AIVCount').textContent  = counts[4] || '0';"
    "   document.getElementById('AOCCount').textContent  = counts[5] || '0';"
    "   document.getElementById('AOVCount').textContent  = counts[6] || '0';"
    "   document.getElementById('RTDYCount').textContent = counts[7] || '0';"
    "   document.getElementById('RTDBCount').textContent = counts[8] || '0';"
    "}"

    "function renderTooltipContent(io) {"
    "   if (!tooltip) return;"
    "   const p = PRODUCTS[io.productCode] || {name:'Unknown', channels:16, digital:false};"
    "   const val = Number(io.value) || 0;"
    
    "   let chTable = '<table><tr><th>CH</th><th>Stat</th><th>CH</th><th>Stat</th></tr>';"
    "   for(let i=0; i < p.channels/2; i++) {"
    "       const chA = i * 2;"
    "       const chB = i * 2 + 1;"
    "       const b1 = getBit(val, chA);"
    "       const b2 = getBit(val, chB);"
    "       chTable += `<tr>` +"
    "           `<td>${chA+1}</td><td class=\"${b1?'ch-on':'ch-off'}\">${b1?'ON':'OFF'}</td>` +"
    "           `<td>${chB+1}</td><td class=\"${b2?'ch-on':'ch-off'}\">${b2?'ON':'OFF'}</td>` +"
    "       `</tr>`;"
    "   }"
    "   chTable += '</table>';"

    "const stateString = nmtStates[io.state] || `Unknown (${io.state})`;"
    
    "   tooltip.innerHTML = `"
    "       <b>${p.name} Module (ID: ${io.id})</b><br/>"
    "       <hr style='border:0; border-top:1px solid #555'/>"
    "       Serial Number: ${io.serialNumber || 'N/A'}<br/>"
    "       HW: ${io.hwVer || 'N/A'} | FW: ${io.fwVer || 'N/A'}<br/>"
    "       IO State: <span style=\"color: ${io.state === 5 ? '#00ff66' : '#ff4444'}\">${stateString}</span><br/>"
    "       Last Error Type: <span style='color:${io.lastErrorType !== 0 ? \"#ff4444\":\"#00ff66\"}'>${io.lastErrorType}</span><br/>"
    "       Last Error Code: <span style='color:${io.lastErrorCode !== 0 ? \"#ff4444\":\"#00ff66\"}'>${io.lastErrorCode}</span><br/>"
    "       ${chTable}"
    "   `;"
    "}"

    "function handleMouseMove(e, id) {"
    "   if(!tooltip) {"
    "       tooltip = document.createElement('div');"
    "       tooltip.id = 'io-tooltip';"
    "       document.body.appendChild(tooltip);"
    "   }"
    "   activeModuleId = id;"
    "   const m = moduleMap[id];"
    "   if (m && m.lastData) {"
    "       renderTooltipContent(m.lastData);"
    "   }"
    "   tooltip.style.display = 'block';"
    "   tooltip.style.left = (e.clientX + 15) + 'px';"
    "   tooltip.style.top = (e.clientY + 15) + 'px';"
    "}"

    "function handleMouseLeave() {"
    "   activeModuleId = null;"
    "   if(tooltip) tooltip.style.display = 'none';"
    "}"
    
    "function updateModule(io) {"
    "   const m = moduleMap[io.id];"
    "   if (!m) return;"
    "   m.lastData = io;"

    "   const p = PRODUCTS[io.productCode];"
    "   m.wrapper.classList.toggle('state-ok', !!io.state);"
    "   m.wrapper.classList.toggle('state-error', !io.state);"

    "   if (!p || !p.digital) return;"
    "   const value = Number(io.value) || 0;"

    "   for (let ch = 0; ch < p.channels; ch++) {"
    "       const bit = getBit(value, ch);"
    "       const targets = m.pins.get(ch);"
    "       if (!targets) continue;"
    "       targets.led.classList.toggle('pin-on', bit);"
    "       targets.led.classList.toggle('pin-off', !bit);"
    "       targets.io.classList.toggle('pin-on', bit);"
    "       targets.io.classList.toggle('pin-off', !bit);"
    "   }"

    "   if (activeModuleId === io.id) {"
    "       renderTooltipContent(io);"
    "   }"
    "}"

    // DOM NODE GENERATION
    "function createPlcModule(){"
    "   const div=document.createElement('div');"
    "   div.className='module module-plc';"
    "   div.innerHTML='<div class=\"module-title\">PLC CPU</div><div class=\"module-id\">&nbsp;</div>';"
    "   if(svgTemplatePLC){"
    "       const svg=svgTemplatePLC.documentElement.cloneNode(true);"
    "       div.appendChild(svg);"
    "   }"
    "   return div;"
    "}"

    "function createModule(io){"
    "  const p=PRODUCTS[io.productCode]||{name:'Unknown',channels:0,digital:false};"
    "  const div=document.createElement('div');"
    "  div.className='module';"
    "  div.dataset.id=io.id;"
    "  div.innerHTML="
    "    '<div class=\"module-title\">'+p.name+'</div>' +"
    "    '<div class=\"module-id\">ID '+io.id+'</div>';"

    "  let svg=null;"
    "  let pinMap=new Map();"
    "  if(svgTemplateIO){"
    "    svg=svgTemplateIO.documentElement.cloneNode(true);"
    "    pinMap=buildPinMap(svg);"
    "    div.appendChild(svg);"
    "  }"

    "  moduleMap[io.id]={"
    "    wrapper:div,"
    "    svg:svg,"
    "    pins:pinMap,"
    "    lastData:io"
    "  };"

    "  div.addEventListener('mousemove', (e) => handleMouseMove(e, io.id));"
    "  div.addEventListener('mouseleave', handleMouseLeave);"

    "  updateModule(io);"
    "  return div;"
    "}"

    "function createRack(data){"
    "  const rack=document.getElementById('io-rack');"
    "  if(!rack) return;"
    "  rack.innerHTML='';"
    "  moduleMap={};"
    
    "  if(svgTemplatePLC){"
    "      rack.appendChild(createPlcModule());"
    "  }"
    
    "  data.forEach(io=>rack.appendChild(createModule(io)));"
    "  modulesCreated=true;"
    "  updateSummaryTable(data);"
    "}"

    "function updateRack(data){"
    "  data.forEach(updateModule);"
    "}"

    "async function loadData(){"
    "  try{"
    "    const r=await fetch('/io-json?id='+Date.now());"
    "    if(!r.ok) throw new Error('IO request failed');"
    "    const data=await r.json();"
    "    if(!modulesCreated) createRack(data);"
    "    else updateRack(data);"
    "  }catch(e){console.error(e);}"
    "}"

    "async function poll(){"
    "  if (!scanInProgress) await loadData();"
    "  setTimeout(poll,20);"
    "}"

    "const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));"
    "async function triggerHardwareScan() {"
    "    scanInProgress = true;"
    "    const btn = document.getElementById('scanButton');"
    "    const btnText = document.getElementById('scanButtonText');"
    "    const progContainer = document.getElementById('progressContainer');"
    "    const progBar = document.getElementById('progressBar');"
    "    if (!btn || !btnText) return;"

    "    btn.disabled = true;"
    "    btn.style.opacity = '0.6';"
    "    btnText.textContent = 'Scanning...';"
    "    if (progContainer && progBar) {"
    "        progContainer.style.display = 'block';"
    "        progBar.style.width = '10%';"
    "    }"

    "    const rack = document.getElementById('io-rack');"
    "    if (rack) {"
    "        rack.innerHTML = `"
    "            <div style='"
    "                display:flex; flex-direction:column; align-items:center; justify-content:center;"
    "                width:100%; height:300px; gap:16px;"
    "                color:#888; font-family:sans-serif;"
    "            '>"
    "                <div class='scan-spinner' style='font-size:48px'>⚙️</div>"
    "                <div style='font-size:20px; font-weight:600; color:#555'>Scanning in progress...</div>"
    "                <div style='font-size:13px; color:#aaa'>Please wait while devices are being discovered</div>"
    "            </div>"
    "        `;"
    "        modulesCreated = false;"
    "        moduleMap = {};"
    "    }"

    "    try {"
             //Trigger the scan
    "        const r = await fetch('/api/scan', { method: 'POST' });"
    "        if (!r.ok) throw new Error(`Server returned status: ${r.status}`);"
    "        const res = await r.json();"

    "        if (res.status === 'busy') {"
    "            showToast('Bus scan already running, please wait.', 'warn');"
    "            return;"
    "        }"

             // Poll for completion (max 15s, 500ms interval)
    "        if (progBar) progBar.style.width = '30%';"
    "        const MAX_WAIT_MS = 60000;"
    "        const POLL_INTERVAL_MS = 1000;"
    "        const deadline = Date.now() + MAX_WAIT_MS;"
    "        let scanDone = false;"

    "        while (Date.now() < deadline) {"
    "            await sleep(POLL_INTERVAL_MS);"
    "            let pollError = null;"
    "            try {"
    "                const sr = await fetch('/api/scan/status');"
    "                if (sr.ok) {"
    "                    const s = await sr.json();"
    "                    if (s.status === 'done') {"
    "                        scanDone = true;"
    "                        break;"
    "                    } else if (s.status === 'error') {"
    "                        pollError = new Error('MCU reported scan error: ' + (s.message || ''));"
    "                    }"
    "                }"
    "            } catch(fetchErr) {"
    "                console.warn('Poll attempt failed:', fetchErr);"  // network errors only
    "            }"

    "            if (pollError) throw pollError;"  // now escapes the catch above

    "            if (progBar) {"
    "                const elapsed = MAX_WAIT_MS - (deadline - Date.now());"
    "                const pct = 30 + Math.min(60, (elapsed / MAX_WAIT_MS) * 60);"
    "                progBar.style.width = pct + '%';"
    "            }"
    "        }"
    "        if (!scanDone) {"
    "            throw new Error('Scan timed out after 60 seconds.');"
    "        }"
             //Clear rack and reload fresh data
    "        if (progBar) progBar.style.width = '95%';"
    "        const rack = document.getElementById('io-rack');"
    "        if (rack) rack.innerHTML = '';"
    "        moduleMap = {};"
    "        modulesCreated = false;"
    "        await loadData();"
    "        if (progBar) progBar.style.width = '100%';"
    "        showToast('Hardware scan complete! Modules updated.', 'success');"
    "    } catch(e) {"
    "        console.error('Scan Execution Error:', e);"
    "        showToast(`Scan failed: ${e.message}`, 'error');"
    "    } finally {"
    "        scanInProgress = false;"
    "        btn.disabled = false;"
    "        btn.style.opacity = '1.0';"
    "        btnText.textContent = 'Scan';"
             // Give user time to see 100% before hiding
    "        setTimeout(() => {"
    "            if (progContainer) progContainer.style.display = 'none';"
    "            if (progBar) progBar.style.width = '0%';"
    "        }, 800);"  // 800ms delay
    "    }"
    "}"

    "async function init(){"
    "  await loadSvgTemplate();"
    "   const scanBtn = document.getElementById('scanButton');"
    "   if (scanBtn) {"
    "       scanBtn.addEventListener('click', triggerHardwareScan);"
    "   }"
    "}"

    "window.addEventListener('load',init);";

#ifdef  __cplusplus
}
#endif

#endif // GSP_WS_JS_H
