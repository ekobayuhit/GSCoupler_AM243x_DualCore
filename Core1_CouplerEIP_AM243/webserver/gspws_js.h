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

    static const char wsio_js[] =
    "function get_IOdata()"
    "{"
    // "    var ioReq = new XMLHttpRequest();"

    // "    ioReq.onreadystatechange = function()"
    // "    {"
    // "        if(ioReq.readyState == 4)"
    // "        {"
    // "            if(ioReq.status == 200)"
    // "            {"
    // "                if(ioReq.responseText != null)"
    // "                {"
    // "                    var container = document.getElementById('iotable_container');"
    // "                    if(container)"
    // "                    {"
    // "                        container.innerHTML = ioReq.responseText;"
    // "                    }"
    // "                }"
    // "            }"
    // "        }"
    // "    };"

    // "    ioReq.open('GET', '/io-data?id=' + Math.random(), true);"
    // "    ioReq.send(null);"

    // "    setTimeout(get_IOdata, 500);"
    "}";

    static const char iomap_js[] =
    "let svgTemplateDoc=null;"
    "let modulesCreated=false;"
    "let moduleMap={};"
    
    "const LED_BASE_Y = 100;"   // Shifted down to land on the black DI8 display grid
    "const LED_STEP_Y = 11.8;"  // Finely adjusted vertical stepping for the LED grid
    "const LED_LEFT_X = 29;"    // Centered with the left column of square indicators
    "const LED_RIGHT_X = 41;"   // Centered with the right column of square indicators

    "const PIN_BASE_Y = 206;"   // Y-coordinate of the first terminal pin row
    "const PIN_STEP_Y = 37;"    // Vertical spacing between terminal pin rows
    "const PIN_LEFT_X = 21;"    // X-coordinate for left terminal column
    "const PIN_RIGHT_X = 44;"   // X-coordinate for right terminal column

    "const PRODUCTS={"
    "1:{name:'DO16',channels:16,digital:true},"
    "2:{name:'DI16',channels:16,digital:true},"
    "3:{name:'AIC',channels:8,digital:false},"
    "4:{name:'AIV',channels:8,digital:false},"
    "5:{name:'AOC',channels:8,digital:false},"
    "6:{name:'AOV',channels:8,digital:false},"
    "7:{name:'RTDY',channels:6,digital:false},"
    "8:{name:'RTDB',channels:6,digital:false}"
    "};"

    "function getBit(v,b){return (v>>b)&1;}"

    "function getLedPosition(ch) {"
    "   const isRight = (ch % 2) !== 0;"
    "   const rowIndex = Math.floor(ch / 2);"// Drops down 1 row every 2 channels
    "   return {"
    "       cx: isRight ? LED_RIGHT_X : LED_LEFT_X,"
    "       cy: LED_BASE_Y + (rowIndex * LED_STEP_Y)"
    "   };"
    "}"

    "function getPinPosition(ch) {"
    "   const isRight = (ch % 2) !== 0;"
    "   const rowIndex = Math.floor(ch / 2);"// Drops down 1 row every 2 channels
    "   return {"
    "       cx: isRight ? PIN_RIGHT_X : PIN_LEFT_X,"
    "       cy: PIN_BASE_Y + (rowIndex * PIN_STEP_Y)"
    "   };"
    "}"
    
    // Instead of nesting elements, we populate or inject explicit positions
    "function normalizePin(svg, ch) {"
    // Look for existing overlays, create if absent
    "   let led = svg.querySelector(`.led-ch-${ch}`);"
    "   let io = svg.querySelector(`.io-ch-${ch}`);"
    "   const ledPos = getLedPosition(ch);"
    "   const pinPos = getPinPosition(ch);"
    // Render LED as a Rectangle (using SVG <rect>)
    "   if (!led) {"
    "       led = document.createElementNS(\"http://www.w3.org/2000/svg\", \"rect\");"
    "       led.setAttribute(\"class\", `status-indicator pin-led led-ch-${ch}`);"
    // Center the rectangle by subtracting half its width(5)/height(4) from coordinates
    "       led.setAttribute(\"x\", ledPos.x - 2);" 
    "       led.setAttribute(\"y\", ledPos.y - 2);"
    "       led.setAttribute(\"width\", \"4\");"
    "       led.setAttribute(\"height\", \"4\");"
    "       svg.appendChild(led);"
    "   }"
    // Render Terminal Pin Connector as a larger Circle
    "   if (!io) {"
    "       io = document.createElementNS(\"http://www.w3.org/2000/svg\", \"circle\");"
    "       io.setAttribute(\"class\", `status-indicator pin-io io-ch-${ch}`);"
    "       io.setAttribute(\"cx\", pinPos.cx);"
    "       io.setAttribute(\"cy\", pinPos.cy);"
    // Radius is set explicitly here or scaled via CSS
    "       io.setAttribute(\"r\", \"9\");"
    "       svg.appendChild(io);"
    "   }"
    "}"

    "function buildPinMap(svg) {"
    "   const map = new Map();"
        // Assume 16 channels max for DI16 / DO16
    "   for (let ch = 0; ch < 16; ch++) {"
    "       normalizePin(svg, ch);"
            // Group references by channel ID for easy updates
    "       map.set(ch, {"
    "           led: svg.querySelector(`.led-ch-${ch}`),"
    "           io: svg.querySelector(`.io-ch-${ch}`)"
    "       });"
    "   }"
    "   return map;"
    "}"

    "async function loadSvgTemplate(){"
    "  try{"
    "    const r=await fetch('/iomap.svg');"
    "    if(!r.ok) throw new Error('SVG load failed');"
    "    const txt=await r.text();"
    "    svgTemplateDoc=new DOMParser().parseFromString(txt,'image/svg+xml');"
    "    poll();"
    "  }catch(e){console.error(e);}"
    "}"

    // Adjust your update loop to toggle classes on both target nodes
    "function updateModule(io) {"
    "   const m = moduleMap[io.id];"
    "   if (!m) return;"

    "   const p = PRODUCTS[io.productCode];"

    "   m.wrapper.classList.toggle('state-ok', !!io.state);"
    "   m.wrapper.classList.toggle('state-error', !io.state);"

    "   if (!p || !p.digital) return;"

    "   const value = Number(io.value) || 0;"

    "   for (let ch = 0; ch < p.channels; ch++) {"
    "       const bit = getBit(value, ch);"
    "       const targets = m.pins.get(ch);"
    "       if (!targets) continue;"

            // Apply state classes directly to the elements
    "       targets.led.classList.toggle('pin-on', bit);"
    "       targets.led.classList.toggle('pin-off', !bit);"
    "       targets.io.classList.toggle('pin-on', bit);"
    "       targets.io.classList.toggle('pin-off', !bit);"
    "   }"
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

    "  if(svgTemplateDoc){"
    "    svg=svgTemplateDoc.documentElement.cloneNode(true);"
    "    pinMap=buildPinMap(svg);"
    "    div.appendChild(svg);"
    "  }"

    "  moduleMap[io.id]={"
    "    wrapper:div,"
    "    svg:svg,"
    "    pins:pinMap"
    "  };"

    "  updateModule(io);"
    "  return div;"
    "}"

    "function createRack(data){"
    "  const rack=document.getElementById('io-rack');"
    "  if(!rack) return;"

    "  rack.innerHTML='';"
    "  moduleMap={};"

    "  data.forEach(io=>rack.appendChild(createModule(io)));"

    "  modulesCreated=true;"
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
    "  await loadData();"
    "  setTimeout(poll,50);"
    "}"

    "async function init(){"
    "  await loadSvgTemplate();"
    "}"

    "window.addEventListener('load',init);";

#ifdef  __cplusplus
}
#endif

#endif // GSP_WS_JS_H
