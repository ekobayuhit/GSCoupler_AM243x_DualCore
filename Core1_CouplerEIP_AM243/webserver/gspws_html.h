#ifndef GSP_WS_HTML_H
#define GSP_WS_HTML_H

#ifdef __cplusplus
extern "C" {
#endif

static const char response_200_content_html[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-type: text/html\r\n"
    "Connection: close\r\n\r\n";

static const char response_200_content_css[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-type: text/css\r\n"
    "Connection: close\r\n\r\n";;

static const char response_200_content_js[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-type: application/javascript\r\n"
    "Connection: close\r\n\r\n";

static const char response_200_content_image[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-type: image/x-icon\r\n"
    "Connection: close\r\n\r\n";
        
static const char response_404[] = 
    "HTTP/1.1 404 Not Found\r\n"
    "Content-type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<html>"
        "<head>"
            "<style>"
                "h1 {"
                    "text-align: center;"
                    "font-family: Arial,sans-serif;"
                "}"
                "h2 {"
                    "text-align: center;"
                    "font-family: Arial,sans-serif;"
                "}"
            "</style>"
            "<meta charset=\"utf-8\"/>"
            "<title>Gespant IO Coupler - Dashboard</title>"
        "</head>"
        "<body>"
            "<h1>Gespant IO Coupler - Dashboard</h1>"
            "<h2>404 : Not Found </h2>"
        "</body>"
    "</html>";

static const char response_501[] = 
    "HTTP/1.1 501 Not Implemented\r\n"
    "Content-type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<html>"
        "<head>"
            "<style>"
                "h1 {"
                    "text-align: center;"
                    "font-family: Arial,sans-serif;"
                "}"
                "h2 {"
                    "text-align: center;"
                    "font-family: Arial,sans-serif;"
                "}"
            "</style>"
            "<meta charset=\"utf-8\"/>"
            "<title>Gespant IO Coupler - Dashboard</title>"
        "</head>"
        "<body>"
            "<h1>Gespant IO Coupler - Dashboard</h1>"
            "<h2>501 : Not Implemented </h2>"
        "</body>"
    "</html>";

static const char svg_header[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: image/svg+xml\r\n"
    "Content-Length: %u\r\n"
    "Connection: close\r\n"
    "\r\n";

static const char style_css[] =
":root{"
"--primary-orange:#fd6005;"
"--sidebar-grey:#e9ecef;"
"--sidebar-hover:#dee2e6;"
"--bg-body:#f8f9fa;"
"--white:#fff;"
"--border:#dee2e6;"
"--text-dark:#495057;"
"--success:#4caf50;"
"--danger:#f44336;"
"--transition:all .3s ease;"
"}"

/* --- SIDEBAR --- */
/* --- ORIGINAL GREY SIDEBAR STYLE --- */
".sidebar-container {"
"   width: 240px;"
"   background: #cacaca;"          /* Original Light-Grey Background */
"   border-right: 1px solid #dee2e6;"
"   color: #333;"
"   position: fixed;"
"   height: 100vh;"
"   display: flex;"
"   flex-direction: column;"
"}"

"header {"
"   padding: 20px;"
"   background: #f8f9fa;"         /* Slightly different grey for logo area */
"   border-bottom: 1px solid #dee2e6;"
"   text-align: center;"
"}"

"header img {" 
"   max-width: 100%;" 
"   height: auto;" 
    /* Remove the 'invert' filter if your logo is already dark/colored */
"}"

".sidebar-nav { flex-grow: 1; margin-top: 10px; }"
".sidebar-nav ul { list-style: none; }"

".sidebar-nav ul li a {"
"   padding: 12px 20px;"
"   display: flex;"
"   align-items: center;"
"   color: #495057;"             /* Original Text Color */
"   text-decoration: none;"
"   gap: 12px;"
"   font-weight: 500;"
"   transition: all 0.2s;"
"}"

/* Hover Effect: Original Lighter Grey */
".sidebar-nav ul li a:hover {"
"   background: #dee2e6;"
"   color: #000;"
"}"

/* Active State: Original Orange Accent */
".sidebar-nav ul li.active a {"
"   background: #fff;"
"   color: #fd6005;"             /* Your brand orange */
"   border-left: 4px solid #fd6005;"
"}"

".sidebar-footer {"
"   padding: 15px;"
"   font-size: 12px;"
"   color: #6c757d;"
"   background: #f8f9fa;"
"   border-top: 1px solid #dee2e6;"
"   text-align: center;"
"}"

"*{margin:0;padding:0;box-sizing:border-box;font-family:Arial,sans-serif;}"
"body{background:var(--bg-body);display:flex;min-height:100vh;}"

"main,#content{margin-left:240px;padding:20px;width:calc(100%-240px);}"

".sidebar{width:240px;background:var(--sidebar-grey);position:fixed;height:100%;padding:10px;}"
".sidebar img{display:block;margin:auto;margin-bottom:10px;}"
".sidebar a{display:block;padding:10px;color:#333;text-decoration:none;}"
".sidebar a:hover{background:var(--sidebar-hover);}"
".active{background:#fff;color:var(--primary-orange);font-weight:bold;}"

".section-title{font-size:1.2rem;font-weight:600;margin-bottom:15px;border-bottom:2px solid var(--border);padding-bottom:5px;}"

".grid,.config-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(400px,1fr));gap:15px;}"

".card,.themed-card{background:#fff;border:1px solid var(--border);border-radius:10px;padding:15px;box-shadow:0 2px 6px rgba(0,0,0,.05);}"

".btn{padding:10px;border:none;border-radius:6px;cursor:pointer;font-weight:600;}"
".btn-primary{background:var(--primary-orange);color:#fff;}"
".btn:hover{opacity:.9;}"

"table,.status-table{width:100%;border-collapse:collapse;font-size:13px;}"
"th{background:#2f4f6f;color:#fff;padding:6px;}"
"td{padding:6px;border:1px solid var(--border);}"
"tr:nth-child(even){background:#f2f2f2;}"

".register-grid{display:grid;grid-template-columns:repeat(auto-fit,72px);}"
".register-block{width:72px;height:610px;position:relative;box-shadow:0 4px 6px rgba(0,0,0,.3);}"

".bit{width:8px;height:8px;background:#444;}"
".bit.active{background:#44D62C;box-shadow:0 0 5px #44D62C;}"

".modal-overlay{position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,.6);display:none;align-items:center;justify-content:center;z-index:1000;}"
".modal-content{background:#fff;padding:20px;border-radius:10px;width:400px;}"

// ".led{position:absolute;width:10px;height:10px;background:#fff;}"
// ".led.on{background:lime;box-shadow:0 0 6px lime;}"

"@media(max-width:768px){"
"main,#content{margin-left:0;width:100%;}"
".sidebar{display:none;}"
"}";

static const char style_iomap_css[] =
    /* ===== BACKPLANE / RACK LAYOUT ===== */
    "#io-rack {"
    "   display: flex;"
    "   flex-direction: row;"
    "   flex-wrap: nowrap;"
    "   gap: 0px;"                     /* No space between modules */
    "   background: #f5f5f5;"
    "   padding: 10px;"
    "   overflow-x: auto;"
    "   align-items: stretch;"         /* Forces all module containers to be identical heights */
    "}"

    /* ===== MODULE CARD CONTAINER ===== */
    ".module {"
    "   display: flex;"                /* Use flex inside the card to manage header vs SVG space */
    "   flex-direction: column;"
    "   position: relative;"
    "   flex: 0 0 68px;"               /* Fixed width for standard IO modules */
    "   width: 68px;"
    "   background: #ffffff;"
    "   text-align: center;"
    "   box-sizing: border-box;"
    "   border: 1px solid #ccc;"
    "}"

    ".module-plc {"
    "   flex: 0 0 200px;"              /* Adjusted PLC width to look proportioned */
    "   width: 200px;"
    "}"

    ".module svg {"
    "   width: 100%;"
    "   height: 100%;"                 /* Forces the SVG height to scale to container bounds */
    "   flex-grow: 1;"                 /* Stretches the SVG downward to fill out any empty space */
    "   display: block;"
    "}"

    /* ===== CORE OVERLAY DESIGN ===== */
    ".status-indicator {"
    "   transition: fill 0.15s ease, filter 0.15s ease, opacity 0.15s ease;"
    "   pointer-events: none;"    /* Clicks fall through to any underlying elements */
    "}"

    /* Top Rectangle LEDs */
    ".pin-led {"
    "   rx: 1px;"                 /* FIXED: Correct syntax for rounding rect corners in SVG CSS */
    "}"

    /* Bottom Terminal Connectors (Large circles) */
    ".pin-io {"
    "   mix-blend-mode: multiply;"
    "}"

    /* ===== OFF STATES ===== */
    ".pin-led.pin-off {"
    "   fill: #1a1a1a;"
    "   opacity: 0.1;"
    "}"

    ".pin-io.pin-off {"
    "   fill: #000000;"
    "   opacity: 0.0;"            /* Hidden cleanly when off */
    "}"

    /* ===== ON STATES (Vibrant Glows) ===== */
    ".pin-led.pin-on {"
    "   fill: #00ff66;"           /* High-visibility Neon Green */
    "   opacity: 0.9;"
    "}"

    ".pin-io.pin-on {"
    "   fill: #00ff66;"
    "   filter: drop-shadow(0 0 4px rgba(0, 255, 102, 0.6));"
    "   opacity: 0.9;"
    "}"

    /* ===== TEXT LABELS ===== */
    ".module-title {"
    "   font-size: 11px;"
    "   font-weight: bold;"
    "   font-family: sans-serif;"
    "   padding-top: 4px;"
    "   line-height: 14px;"
    "}"

    ".module-id {"
    "   font-size: 10px;"
    "   color: #666;"
    "   font-family: sans-serif;"
    "   line-height: 12px;"
    "   min-height: 12px;"             /* Keeps empty space on PLC so headers take up identical room */
    "}"

    /* ===== MODULE DIAGNOSTIC STATES ===== */
    ".state-ok { border: 1px solid #4CAF50; }"
    ".state-error { border: 1px solid red; }"

    /* ===== ELEGANT HOVER POPUP (TOOLTIP) ===== */
    "#io-tooltip {"
    "   position: fixed;"
    "   display: none;"
    "   z-index: 1000;"
    "   background: #24292e;"         /* Rich charcoal dark-mode theme */
    "   color: #e1e4e8;"              /* Soft white text */
    "   padding: 12px 14px;"
    "   border-radius: 6px;"
    "   font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
    "   font-size: 12px;"
    "   line-height: 1.5;"
    "   box-shadow: 0 8px 24px rgba(0,0,0,0.5);"
    "   border: 1px solid #444d56;"   /* Clean structural outline border */
    "   pointer-events: none;"
    "   min-width: 180px;"
    "}"
    
    "#io-tooltip b {"
    "   font-size: 13px;"
    "   color: #ffffff;"
    "}"
    
    /* Elegant Flat Minimalist Table Definition */
    "#io-tooltip table {"
    "   width: 100%;"
    "   border-collapse: collapse;"
    "   margin-top: 10px;"
    "   background: #1f2327;"         /* Deep table background inset */
    "   border-radius: 4px;"
    "   overflow: hidden;"            /* Wraps corners cleanly */
    "}"
    
    /* Clean, distinct table header bar */
    "#io-tooltip th {"
    "   background: #2f363d;"         /* Distinct block for titles */
    "   color: #959da5;"              /* Muted text for structural labels */
    "   font-size: 11px;"
    "   font-weight: 600;"
    "   text-transform: uppercase;"
    "   letter-spacing: 0.5px;"
    "   padding: 5px 8px;"
    "   border-bottom: 2px solid #444d56;"
    "}"
    
    /* Uniform grid padding and subtle dividers */
    "#io-tooltip td {"
    "   padding: 6px 8px;"
    "   border-bottom: 1px solid #2b3137;"
    "   font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;"
    "   font-size: 11px;"
    "}"
    
    /* Muted Alternating Row Styling */
    "#io-tooltip tr:nth-child(even) {"
    "   background: #24292e;"
    "}"
    
    /* Status Column High Contrast Overrides */
    "#io-tooltip .ch-on {"
    "   color: #00ff66;"              /* Vibrant Neon Green text */
    "   font-weight: bold;"
    "   text-shadow: 0 0 4px rgba(0,255,102,0.3);"
    "}"
    
    "#io-tooltip .ch-off {"
    "   color: #6a737d;"              /* Flat muted grey for standby elements */
    "   font-weight: normal;"
    "}"
    
    /* Highlight module on hover wrapper hook */
    ".module:hover {"
    "   background: #f0faff;"
    "   cursor: help;"
    "}"

    /* ===== SCAN BUTTON ===== */
    "#scanButton {"
    "   background: #ff7f32;"
    "   color: white;"
    "   border: none;"
    "   padding: 8px 16px;"
    "   border-radius: 4px;"
    "   cursor: pointer;"
    "   font-size: 13px;"
    "}"

    "#scanButton:disabled {"
    "   cursor: not-allowed;"
    "}"

    /* ===== PROGRESS BAR ===== */
    "#progressContainer {"
    "   display: none;"           /* hidden by default, shown via JS */
    "   width: 100%;"
    "   height: 6px;"
    "   background: #e0e0e0;"
    "   border-radius: 4px;"
    "   overflow: hidden;"
    "   margin-top: 8px;"
    "   margin-bottom: 1px;"
    "}"

    "#progressBar {"
    "   height: 100%;"
    "   width: 0%;"
    "   background: #ff7f32;"
    "   border-radius: 3px;"
    "   transition: width 0.3s ease;"
    "}"

    "@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.4} }"
    ".scan-spinner { animation: pulse 1.5s ease-in-out infinite; }"
    ;

static const char header_html[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n\r\n";

static const char html_top[] =
"<!DOCTYPE html><html>"
"<head>"
"   <meta charset=\"utf-8\">"
"   <title>Gespant IO Coupler</title>"
"   <link rel=\"stylesheet\" href=\"/style.css\">"
"   <script src=\"/main.js\"></script>"
"</head>";

static const char html_top_iopage[] =
"<!DOCTYPE html><html>"
"<head>"
"   <meta charset=\"utf-8\">"
"   <title>Gespant IO Coupler</title>"
"   <link rel=\"stylesheet\" href=\"/style.css\">"
"   <link rel=\"stylesheet\" href=\"/iomap.css\">"
"   <script src=\"/main.js\"></script>"
// "   <script src=\"/wsio.js\"></script>"
"   <script src=\"/iomap.js\"></script>"
"</head>";

static const char html_top_body[] =
"<body onload=\"%s\">"

"   <div id=\"title\">Gespant IO Coupler EtherNet/IP</div>"

"   <div class=\"sidebar-container\">"
"       <header>"
"           <svg width=\"186px\" height=\"36px\" viewBox=\"0 0 500 100\" xmlns=\"http://www.w3.org/2000/svg\">"
"               <path d=\"M0 0 C9.57 0 19.14 0 29 0 C29 5.61 29 11.22 29 17 C23.06 17 17.12 17 11 17 C11 15.02 11 13.04 11 11 C14.63 11 18.26 11 22 11 C22 9.35 22 7.7 22 6 C17.05 6 12.1 6 7 6 C7 10.95 7 15.9 7 21 C9.25650391 20.98259766 9.25650391 20.98259766 11.55859375 20.96484375 C13.51822793 20.95546751 15.47786345 20.94636713 17.4375 20.9375 C18.92733398 20.92493164 18.92733398 20.92493164 20.44726562 20.91210938 C21.39150391 20.90888672 22.33574219 20.90566406 23.30859375 20.90234375 C24.62041626 20.89448853 24.62041626 20.89448853 25.95874023 20.88647461 C28 21 28 21 29 22 C29 23.98 29 25.96 29 28 C19.43 28 9.86 28 0 28 C0 18.76 0 9.52 0 0 Z \" fill=\"#16391E\" transform=\"translate(9,65)\"/>"
"               <path d=\"M0 0 C1.16781006 0.00523682 2.33562012 0.01047363 3.53881836 0.01586914 C4.79887695 0.0190918 6.05893555 0.02231445 7.35717773 0.02563477 C8.6976732 0.03399136 10.03816799 0.04245632 11.37866211 0.05102539 C12.72306153 0.05603879 14.06746271 0.06060197 15.41186523 0.06469727 C18.71334151 0.07653051 22.01473014 0.09301568 25.31616211 0.11352539 C25.31616211 2.09352539 25.31616211 4.07352539 25.31616211 6.11352539 C18.05616211 6.11352539 10.79616211 6.11352539 3.31616211 6.11352539 C3.31616211 11.39352539 3.31616211 16.67352539 3.31616211 22.11352539 C8.26616211 22.11352539 13.21616211 22.11352539 18.31616211 22.11352539 C18.31616211 20.46352539 18.31616211 18.81352539 18.31616211 17.11352539 C14.68616211 17.11352539 11.05616211 17.11352539 7.31616211 17.11352539 C7.31616211 15.13352539 7.31616211 13.15352539 7.31616211 11.11352539 C13.25616211 11.11352539 19.19616211 11.11352539 25.31616211 11.11352539 C25.31616211 16.72352539 25.31616211 22.33352539 25.31616211 28.11352539 C15.74616211 28.11352539 6.17616211 28.11352539 -3.68383789 28.11352539 C-3.70446289 23.67915039 -3.72508789 19.24477539 -3.74633789 14.67602539 C-3.75544189 13.27553955 -3.7645459 11.87505371 -3.77392578 10.43212891 C-3.77642334 9.33827881 -3.7789209 8.24442871 -3.78149414 7.11743164 C-3.78673096 5.99151611 -3.79196777 4.86560059 -3.79736328 3.70556641 C-3.64207883 0.16007175 -3.55501654 0.15037594 0 0 Z \" fill=\"#621211\" transform=\"translate(12.683837890625,31.886474609375)\"/>"
"               <path d=\"M0 0 C9.24 0 18.48 0 28 0 C28 9.24 28 18.48 28 28 C18.76 28 9.52 28 0 28 C0 25.69 0 23.38 0 21 C7.26 21 14.52 21 22 21 C22 16.05 22 11.1 22 6 C16.72 6 11.44 6 6 6 C6 7.65 6 9.3 6 11 C9.63 11 13.26 11 17 11 C17 12.98 17 14.96 17 17 C11.39 17 5.78 17 0 17 C0 11.39 0 5.78 0 0 Z \" fill=\"#14371F\" transform=\"translate(42,65)\"/>"
"               <path d=\"M0 0 C9.24 0 18.48 0 28 0 C28 9.24 28 18.48 28 28 C18.76 28 9.52 28 0 28 C0 22.39 0 16.78 0 11 C5.61 11 11.22 11 17 11 C17 12.98 17 14.96 17 17 C13.37 17 9.74 17 6 17 C6 18.65 6 20.3 6 22 C11.28 22 16.56 22 22 22 C22 16.72 22 11.44 22 6 C14.74 6 7.48 6 0 6 C0 4.02 0 2.04 0 0 Z \" fill=\"#14371D\" transform=\"translate(42,32)\"/>"
"               <line x1=\"110\" y1=\"20\" x2=\"110\" y2=\"80\" stroke=\"#800000\" stroke-width=\"4\"/>"
                /* Main Text: gespant */
"               <text x=\"130\" y=\"70\" font-family=\"Arial, sans-serif\" font-weight=\"bold\" font-size=\"75\" fill=\"#1B3F21\">gespant</text>"
                /* Subtext: technology */
"               <text x=\"280\" y=\"95\" font-family=\"Arial, sans-serif\" font-size=\"25\" letter-spacing=\"8\" fill=\"#800000\">technology</text>"
"           </svg>"
"       </header>"
"       <nav class=\"sidebar-nav\">"
"           <ul>"
"               <li class=\"%s\"><a href=\"/main.html\">"
"                   <svg width=\"18\" height=\"18\" viewBox=\"0 -1 22 22\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\" style=\"vertical-align:middle; margin-right:6px;\">"
"                       <path fill-rule=\"evenodd\" clip-rule=\"evenodd\" "
"                       d=\"M2 11C1.08049 11 0.648384 9.86349 1.33564 9.25259L10.3356 1.25259C10.7145 0.915803 11.2855 0.915803 11.6644 1.25259L20.6644 9.25259C21.3516 9.86349 20.9195 11 20 11H19V18C19 18.5523 18.5523 19 18 19H4C3.44772 19 3 18.5523 3 18V11H2ZM8 17V12C8 11.4477 8.44772 11 9 11H13C13.5523 11 14 11.4477 14 12V17H17V10C17 9.62477 17.2067 9.29781 17.5124 9.12674L11 3.33795L4.48762 9.12674C4.79334 9.29781 5 9.62477 5 10V17H8ZM10 17V13H12V17H10Z\" "
"                       fill=\"currentColor\"/>"
"                   </svg>"
"                   <span>Dashboard</span></a>"
"               </li>"
"               <li class=\"%s\"><a href=\"/IO_Mapping.html\">"
"                   <svg width=\"18\" height=\"18\" viewBox=\"0 0 32 32\" xmlns=\"http://www.w3.org/2000/svg\" style=\"vertical-align:middle; margin-right:6px;\">"
"                       <path d=\"M28 1.75h-24c-1.794 0.002-3.248 1.456-3.25 3.25v16c0.002 1.794 1.456 3.248 3.25 3.25h8.011l-1.685 3.5h-4.326c-0.69 0-1.25 0.56-1.25 1.25s0.56 1.25 1.25 1.25v0h20c0.69 0 1.25-0.56 1.25-1.25s-0.56-1.25-1.25-1.25v0h-4.229l-1.75-3.5h7.979c1.794-0.001 3.249-1.456 3.25-3.25v-16c-0.002-1.794-1.456-3.248-3.25-3.25h-0zM4 4.25h24c0.414 0 0.75 0.336 0.75 0.75v10.75h-25.5v-10.75c0.001-0.414 0.336-0.749 0.75-0.75h0zM18.978 27.75h-5.878l1.685-3.5h2.442zM28 21.75h-24c-0.414-0-0.75-0.336-0.75-0.75v-2.75h25.5v2.75c0 0.414-0.336 0.75-0.75 0.75v0z\" fill=\"currentColor\"/>"
"                   </svg>"
"                   <span>IO Mapping</span></a>"
"               </li>"
"               <li class=\"%s\"><a href=\"/Network.html\">"
"                   <svg width=\"18\" height=\"18\" viewBox=\"0 0 16 16\" xmlns=\"http://www.w3.org/2000/svg\" style=\"vertical-align:middle; margin-right:6px;\">"
"                       <path fill=\"currentColor\" fill-rule=\"evenodd\" clip-rule=\"evenodd\" "
"                       d=\"M6.25 0C5.56 0 5 .56 5 1.25v3.5C5 5.44 5.56 6 6.25 6H7v1H1a.75.75 0 000 1.5h2.5V10H2.25C1.56 10 1 10.56 1 11.25v3.5c0 .69.56 1.25 1.25 1.25h3.5C6.44 16 7 15.44 7 14.75v-3.5C7 10.56 6.44 10 5.75 10H5V8.5h6V10h-.75C9.56 10 9 10.56 9 11.25v3.5c0 .69.56 1.25 1.25 1.25h3.5c.69 0 1.25-.56 1.25-1.25v-3.5c0-.69-.56-1.25-1.25-1.25H12.5V8.5H15A.75.75 0 0015 7H8.5V6h1.25C10.44 6 11 5.44 11 4.75v-3.5C11 .56 10.44 0 9.75 0h-3.5zm4.25 11.5v3h3v-3h-3zm-8 0v3h3v-3h-3zm7-7v-3h-3v3h3z\"/>"
"                   </svg>"
"                   <span>Network</span></a>"
"               </li>"
"           </ul>"
"       </nav>"
"       <div style=\"margin-top: auto; padding: 20px; font-size: 0.75rem; color: #999;\">"
"           © 2026 Gespant. All rights reserved."
"       </div>"
"   </div>"

"   <div id=\"content\">";

static const char html_bottom[] =
"   </div>"
"</body></html>";

static const char page_main[] =
"<div class=\"section-title\" style=\"display:flex; align-items:center; gap:10px; font-size:18px; font-weight:600; margin-bottom:15px;\">"
"   <svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" xmlns=\"http://www.w3.org/2000/svg\" style=\"color: var(--primary-orange);\">"
"       <rect x=\"4\" y=\"4\" width=\"16\" height=\"16\" rx=\"3\" fill=\"currentColor\"/>"
"   </svg>"
"   System CPU Load"
"</div>"

"<div style=\"background:#fff; border-radius:10px; padding:15px; box-shadow:0 2px 6px rgba(0,0,0,0.08); margin-bottom:24px;\">"
"   <table id=\"cputable\" style=\"width:100%; border-collapse:collapse; font-size:13px;\">"
"       <thead>"
"           <tr style=\"background:#f5f5f5; text-align:left;\">"
"               <th style=\"padding:8px;\">Task Name</th>"
"               <th style=\"padding:8px; width:60%;\">CPU Load</th>"
"               <th style=\"padding:8px; width:60px;\">%</th>"
"           </tr>"
"       </thead>"
"       <tbody>"

#define CPU_ROW(i) \
"       <tr>" \
"           <td style=\"padding:6px;\" id=\"r" #i "c0\">-</td>" \
"           <td style=\"padding:6px;\">" \
"               <div style=\"background:#eee; border-radius:6px; height:10px; overflow:hidden;\">" \
"                   <div id=\"bar" #i "\" style=\"width:0%; height:100%; background:var(--primary-orange);\"></div>" \
"               </div>" \
"           </td>" \
"           <td style=\"padding:6px; text-align:right;\" id=\"r" #i "c1\">0%</td>" \
"       </tr>"

CPU_ROW(0)  CPU_ROW(1)  CPU_ROW(2)  CPU_ROW(3)  CPU_ROW(4)
CPU_ROW(5)  CPU_ROW(6)  CPU_ROW(7)  CPU_ROW(8)  CPU_ROW(9)
CPU_ROW(10) CPU_ROW(11) CPU_ROW(12) CPU_ROW(13) CPU_ROW(14)
CPU_ROW(15) CPU_ROW(16) CPU_ROW(17) CPU_ROW(18) CPU_ROW(19)

#undef CPU_ROW

"       </tbody>"
"   </table>"
"</div>"

/* ── Core 0 MCAN Statistics ──────────────────────────────────── */
"<div class=\"section-title\" style=\"display:flex; align-items:center; gap:10px; font-size:18px; font-weight:600; margin-bottom:15px;\">"
"   <svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" xmlns=\"http://www.w3.org/2000/svg\" style=\"color: var(--primary-orange);\">"
"       <path d=\"M4 6h16M4 12h16M4 18h10\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" fill=\"none\"/>"
"   </svg>"
"   IO \xe2\x80\x94 Protocol Statistics"
"</div>"

"<div style=\"background:#fff; border-radius:10px; padding:15px; box-shadow:0 2px 6px rgba(0,0,0,0.08); margin-bottom:24px;\">"

"   <div style=\"font-size:12px; font-weight:600; color:#888; text-transform:uppercase; letter-spacing:.05em; margin-bottom:8px;\">Error Counters</div>"
"   <table style=\"width:100%; border-collapse:collapse; font-size:13px; margin-bottom:16px;\">"
"       <thead>"
"           <tr style=\"background:#f5f5f5; text-align:left;\">"
"               <th style=\"padding:8px;\">Counter</th>"
"               <th style=\"padding:8px; text-align:right;\">Value</th>"
"               <th style=\"padding:8px; width:40%;\">Status</th>"
"           </tr>"
"       </thead>"
"       <tbody>"

#define MCAN_ERR_ROW(label, id_val, id_bar, id_pill) \
"       <tr>" \
"           <td style=\"padding:6px;\">" label "</td>" \
"           <td style=\"padding:6px; text-align:right; font-family:monospace;\" id=\"" id_val "\">0</td>" \
"           <td style=\"padding:6px;\">" \
"               <div style=\"display:flex; align-items:center; gap:8px;\">" \
"                   <div style=\"flex:1; background:#eee; border-radius:6px; height:8px; overflow:hidden;\">" \
"                       <div id=\"" id_bar "\" style=\"width:0%; height:100%; background:var(--primary-orange); transition:width .4s;\"></div>" \
"                   </div>" \
"                   <span id=\"" id_pill "\" style=\"font-size:11px; padding:2px 7px; border-radius:10px; background:#e8f5e9; color:#388e3c; white-space:nowrap;\">OK</span>" \
"               </div>" \
"           </td>" \
"       </tr>"

MCAN_ERR_ROW("REC \xe2\x80\x94 Receive Error Counter", "mcan_rec", "mcan_rec_bar", "mcan_rec_pill")
MCAN_ERR_ROW("TEC \xe2\x80\x94 Transmit Error Log",    "mcan_tec", "mcan_tec_bar", "mcan_tec_pill")
MCAN_ERR_ROW("CEL \xe2\x80\x94 CAN Error Log Count",   "mcan_cel", "mcan_cel_bar", "mcan_cel_pill")

#undef MCAN_ERR_ROW

"       </tbody>"
"   </table>"

"   <div style=\"font-size:12px; font-weight:600; color:#888; text-transform:uppercase; letter-spacing:.05em; margin-bottom:8px;\">Protocol Status</div>"
"   <table style=\"width:100%; border-collapse:collapse; font-size:13px; margin-bottom:16px;\">"
"       <thead>"
"           <tr style=\"background:#f5f5f5; text-align:left;\">"
"               <th style=\"padding:8px;\">Field</th>"
"               <th style=\"padding:8px; text-align:right;\">Raw</th>"
"               <th style=\"padding:8px;\">Meaning</th>"
"           </tr>"
"       </thead>"
"       <tbody>"

#define MCAN_PSR_ROW(label, id_raw, id_decoded) \
"       <tr>" \
"           <td style=\"padding:6px;\">" label "</td>" \
"           <td style=\"padding:6px; text-align:right; font-family:monospace;\" id=\"" id_raw "\">-</td>" \
"           <td style=\"padding:6px; color:#555;\" id=\"" id_decoded "\">-</td>" \
"       </tr>"

MCAN_PSR_ROW("CPU \xe2\x80\x94 Mcan CPU Load", "mcan_cpuload",  "mcan_cpuload_dec")
MCAN_PSR_ROW("HB \xe2\x80\x94 Mcan Heartbeat", "mcan_hb",  "mcan_hb_dec")
MCAN_PSR_ROW("LEC \xe2\x80\x94 Last Error Code", "mcan_lec",  "mcan_lec_dec")
MCAN_PSR_ROW("ACT \xe2\x80\x94 Activity",        "mcan_act",  "mcan_act_dec")
MCAN_PSR_ROW("DLEC \xe2\x80\x94 Data Phase LEC", "mcan_dlec", "mcan_dlec_dec")
MCAN_PSR_ROW("TDCV \xe2\x80\x94 TDC Value",      "mcan_tdcv", "mcan_tdcv_dec")

#undef MCAN_PSR_ROW

"       </tbody>"
"   </table>"

"   <div style=\"font-size:12px; font-weight:600; color:#888; text-transform:uppercase; letter-spacing:.05em; margin-bottom:8px;\">Bus State Flags</div>"
"   <div style=\"display:flex; gap:10px; flex-wrap:wrap; padding-bottom:4px;\">"

#define MCAN_FLAG_PILL(label, id_pill) \
"       <div style=\"display:flex; align-items:center; gap:6px; background:#f5f5f5; border-radius:8px; padding:6px 12px; font-size:13px;\">" \
"           <span id=\"" id_pill "_dot\" style=\"width:9px; height:9px; border-radius:50%; background:#ccc; display:inline-block;\"></span>" \
"           <span>" label "</span>" \
"           <span id=\"" id_pill "_val\" style=\"font-weight:600; color:#aaa;\">-</span>" \
"       </div>"

MCAN_FLAG_PILL("Error Passive", "mcan_ep")
MCAN_FLAG_PILL("Rx Passive", "mcan_rp")
MCAN_FLAG_PILL("Warning",       "mcan_warn")
MCAN_FLAG_PILL("Bus-Off",       "mcan_boff")
MCAN_FLAG_PILL("RESI",          "mcan_resi")
MCAN_FLAG_PILL("RBRS",          "mcan_rbrs")
MCAN_FLAG_PILL("RFDF",          "mcan_rfdf")
MCAN_FLAG_PILL("PXE",           "mcan_pxe")

#undef MCAN_FLAG_PILL

"   </div>"
"</div>";

static const char page_iomap[] =
"<div class=\"section-title\" style=\"display:flex; align-items:center; gap:10px; font-size:18px; font-weight:600; margin-bottom:15px;\">"
"   <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" xmlns=\"http://www.w3.org/2000/svg\" " 
"       style=\"color: var(--primary-orange);\">" 
"       <path d=\"M19.9,12.66a1,1,0,0,1,0-1.32L21.18,9.9a1,1,0,0,0,.12-1.17l-2-3.46a1,1,0,0,0-1.07-.48l-1.88.38a1,1,0,0,1-1.15-.66l-.61-1.83A1,1,0,0,0,13.64,2h-4a1,1,0,0,0-1,.68L8.08,4.51a1,1,0,0,1-1.15.66L5,4.79A1,1,0,0,0,4,5.27L2,8.73A1,1,0,0,0,2.1,9.9l1.27,1.44a1,1,0,0,1,0,1.32L2.1,14.1A1,1,0,0,0,2,15.27l2,3.46a1,1,0,0,0,1.07.48l1.88-.38a1,1,0,0,1,1.15.66l.61,1.83a1,1,0,0,0,1,.68h4a1,1,0,0,0,.95-.68l.61-1.83a1,1,0,0,1,1.15-.66l1.88.38a1,1,0,0,0,1.07-.48l2-3.46a1,1,0,0,0-.12-1.17ZM18.41,14l.8.9-1.28,2.22-1.18-.24a3,3,0,0,0-3.45,2L12.92,20H10.36L10,18.86a3,3,0,0,0-3.45-2l-1.18.24L4.07,14.89l.8-.9a3,3,0,0,0,0-4l-.8-.9L5.35,6.89l1.18.24a3,3,0,0,0,3.45-2L10.36,4h2.56l.38,1.14a3,3,0,0,0,3.45,2l1.18-.24,1.28,2.22-.8.9A3,3,0,0,0,18.41,14ZM11.64,8a4,4,0,1,0,4,4A4,4,0,0,0,11.64,8Zm0,6a2,2,0,1,1,2-2A2,2,0,0,1,11.64,14Z\" " 
"       fill=\"currentColor\"/>" 
"   </svg>"
"   IO Mapping Configuration"
"</div>"

"<div style=\"margin-bottom:20px;font-size:13px;color:#666;\">"
"   Coupler Type: <b style=\"color:var(--primary-orange);\">%s</b> | "
"   FW Version: <b>%s</b>"
"</div>"

"<div style=\"display:grid; grid-template-columns: 1fr 1fr; gap:20px;\">"

"   <div style=\"background:#fff; border-radius:10px; padding:18px; box-shadow:0 2px 6px rgba(0,0,0,0.08);\">"
"       <div style=\"display:flex; align-items:center; gap:8px; font-weight:600; margin-bottom:12px;\">"
"           <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" style=\"color:var(--primary-orange);\">"
"               <circle cx=\"12\" cy=\"12\" r=\"2\" fill=\"currentColor\"/>"
"           </svg>"
"           Scan Devices"
"       </div>"

"       <div style=\"display:flex; align-items:center; gap:15px;\">"

"           <div>"

"               <div id=\"progressContainer\">"
"                   <div id=\"progressBar\"></div>"
"               </div>"

"               <button id=\"scanButton\" style=\"width:150px; height:36px; border:none; border-radius:6px; "
"                   background:var(--primary-orange); color:white; cursor:pointer; display:flex; align-items:center; justify-content:center; gap:6px;\">"

"                   <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" xmlns=\"http://www.w3.org/2000/svg\" style=\"color: currentColor;\">" 
"                       <line x1=\"3\" y1=\"12\" x2=\"21\" y2=\"12\" stroke=\"currentColor\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\"/>" 
"                       <path d=\"M3,7V4A1,1,0,0,1,4,3H7\" stroke=\"currentColor\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" fill=\"none\"/>" 
"                       <path d=\"M21,7V4a1,1,0,0,0-1-1H17\" stroke=\"currentColor\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" fill=\"none\"/>" 
"                       <path d=\"M3,17v3a1,1,0,0,0,1,1H7\" stroke=\"currentColor\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" fill=\"none\"/>" 
"                       <path d=\"M21,17v3a1,1,0,0,1-1,1H17\" stroke=\"currentColor\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" fill=\"none\"/>" 
"                   </svg>"
"                   <span id=\"scanButtonText\">Scan</span>"
"               </button>"
"           </div>"

"           <div style=\"flex:1;\">"
"               <table style=\"width:100%; border-collapse:collapse; font-size:12px; border:1px solid #ddd;\">"
"                   <thead>"
"                       <tr style=\"background:#2f363d; color:#ffffff;\">"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">DI</th>"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">DO</th>"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">AIC</th>"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">AIV</th>"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">AOC</th>"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">AOV</th>"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">RTDY</th>"
"                           <th style=\"padding:6px; border:1px solid #ddd;\">RTDB</th>"
"                       </tr>"
"                   </thead>"
"                   <tbody>"
"                       <tr style=\"text-align:center; font-weight:bold; background:#ffffff;\">"
"                           <td id=\"DICount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                           <td id=\"DOCount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                           <td id=\"AICCount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                           <td id=\"AIVCount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                           <td id=\"AOCCount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                           <td id=\"AOVCount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                           <td id=\"RTDYCount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                           <td id=\"RTDBCount\" style=\"padding:8px; border:1px solid #ddd;\">-</td>"
"                       </tr>"
"               </tbody>"
"           </table>"
"       </div>"

"       </div>"
"   </div>"

"   <div style=\"background:#fff; border-radius:10px; padding:18px; box-shadow:0 2px 6px rgba(0,0,0,0.08);\">"
"       <div style=\"font-weight:600; margin-bottom:12px;\">Device Configuration</div>"

"       <form id=\"modbusConfigForm\" style=\"display:flex; gap:10px; align-items:flex-end;\">"

"           <div>"
"               <label style=\"font-size:12px; color:#666;\">Coupler Name</label><br>"
"               <input type=\"text\" id=\"ioCouplerName\" style=\"padding:6px; border:1px solid #ccc; border-radius:6px; width:130px;\">"
"           </div>"

"           <button type=\"submit\" style=\"height:34px; padding:0 14px; border:none; border-radius:6px; "
"               background:var(--primary-orange); color:white; cursor:pointer;\">Apply</button>"

"       </form>"

"       <div id=\"configStatus\" style=\"margin-top:10px; font-size:12px; color:#777;\"></div>"
"   </div>"

"</div>"
"<br>"
"<div class=\"section-title\" style=\"display:flex; align-items:center; gap:10px; font-size:18px; font-weight:600; margin-bottom:15px;\">"
"   IO Module Overview"
"</div>"
"<div class=\"io-container\">"
"   <div id=\"io-rack\">"
"   </div>"
"</div>";

static const char page_network[] =
"<div class=\"section-title\" style=\"display:flex; align-items:center; gap:10px; font-size:18px; font-weight:600; margin-bottom:15px;\">"
"   <svg width=\"20\" height=\"20\" viewBox=\"0 0 16 16\" xmlns=\"http://www.w3.org/2000/svg\" style=\"color: var(--primary-orange);\">"
"       <path d=\"M6.25 0C5.56 0 5 .56 5 1.25v3.5C5 5.44 5.56 6 6.25 6H7v1H1a.75.75 0 000 1.5h2.5V10H2.25C1.56 10 1 10.56 1 11.25v3.5c0 .69.56 1.25 1.25 1.25h3.5C6.44 16 7 15.44 7 14.75v-3.5C7 10.56 6.44 10 5.75 10H5V8.5h6V10h-.75C9.56 10 9 10.56 9 11.25v3.5c0 .69.56 1.25 1.25 1.25h3.5c.69 0 1.25-.56 1.25-1.25v-3.5c0-.69-.56-1.25-1.25-1.25H12.5V8.5H15A.75.75 0 0015 7H8.5V6h1.25C10.44 6 11 5.44 11 4.75v-3.5C11 .56 10.44 0 9.75 0h-3.5z\" fill=\"currentColor\"/>"
"   </svg>"
"   Network Configuration"
"</div>"

"<div style=\"margin-bottom:20px;font-size:13px;color:#666;\">"
"   Configure Ethernet interface settings"
"</div>"

"<div style=\"display:grid; grid-template-columns: 1fr; gap:20px;\">"

"   <div style=\"background:#fff; border-radius:10px; padding:18px; box-shadow:0 2px 6px rgba(0,0,0,0.08);\" id=\"eth1-config\">"

"       <div style=\"display:flex; align-items:center; gap:8px; font-weight:600; margin-bottom:12px;\">"
"           <svg width=\"18\" height=\"18\" viewBox=\"0 0 32 32\" xmlns=\"http://www.w3.org/2000/svg\" style=\"color: var(--primary-orange);\">"
"               <path d=\"M28 1.75h-24c-1.794 0.002-3.248 1.456-3.25 3.25v16c0.002 1.794 1.456 3.248 3.25 3.25h8.011l-1.685 3.5h-4.326c-0.69 0-1.25 0.56-1.25 1.25s0.56 1.25 1.25 1.25h20c0.69 0 1.25-0.56 1.25-1.25s-0.56-1.25-1.25-1.25h-4.229l-1.75-3.5h7.979c1.794-0.001 3.249-1.456 3.25-3.25v-16c-0.002-1.794-1.456-3.248-3.25-3.25z\" fill=\"currentColor\"/>"
"           </svg>"
"           Ethernet Interface"
"       </div>"

"       <div style=\"display:flex; gap:20px; flex-wrap:wrap;\">"

"           <!-- STATUS CARD -->"
"           <div style=\"flex:1; min-width:240px; background:#f7f7f7; padding:14px; border-radius:8px;\">"
"               <div style=\"font-weight:600; margin-bottom:10px;\">Current Status</div>"

"               <div style=\"display:flex; justify-content:space-between; margin-bottom:6px;\">"
"                   <span>IP Address</span><b id=\"eth1_ipv4\" style=\"color:var(--primary-orange);\">-</b>"
"               </div>"
"               <div style=\"display:flex; justify-content:space-between; margin-bottom:6px;\">"
"                   <span>Subnet Mask</span><b id=\"eth1_subnet\">-</b>"
"               </div>"
"               <div style=\"display:flex; justify-content:space-between; margin-bottom:6px;\">"
"                   <span>Gateway</span><b id=\"eth1_gateway\">-</b>"
"               </div>"
"               <div style=\"display:flex; justify-content:space-between;\">"
"                   <span>MAC</span><b id=\"eth1_mac\">-</b>"
"               </div>"
"           </div>"

"           <!-- CONFIG CARD -->"
"           <div style=\"flex:1; min-width:240px;\">"
"               <div style=\"font-weight:600; margin-bottom:10px;\">Configure IP</div>"

"               <div style=\"margin-bottom:8px;\">"
"                   <label style=\"font-size:12px; color:#666;\">New IP Address</label><br>"
"                   <input type=\"text\" id=\"new_eth1_ip\" style=\"width:100%; padding:6px; border:1px solid #ccc; border-radius:6px;\" placeholder=\"192.168.1.100\">"
"               </div>"

"               <div style=\"margin-bottom:8px;\">"
"                   <label style=\"font-size:12px; color:#666;\">New Gateway</label><br>"
"                   <input type=\"text\" id=\"new_eth1_gateway\" style=\"width:100%; padding:6px; border:1px solid #ccc; border-radius:6px;\" placeholder=\"192.168.1.1\">"
"               </div>"

"               <div style=\"margin-bottom:10px;\">"
"                   <label style=\"font-size:12px; color:#666;\">New Subnet Mask</label><br>"
"                   <input type=\"text\" id=\"new_eth1_subnet\" style=\"width:100%; padding:6px; border:1px solid #ccc; border-radius:6px;\" placeholder=\"255.255.255.0\">"
"               </div>"

"               <button id=\"set_eth1_ip\" "
"                   style=\"width:160px; height:36px; border:none; border-radius:6px; "
"                   background:var(--primary-orange); color:white; cursor:pointer; "
"                   display:flex; align-items:center; justify-content:center; gap:6px;\">"

"                   <svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" style=\"color:white;\">"
"                       <path d=\"M5 12h14M13 6l6 6-6 6\" stroke=\"currentColor\" stroke-width=\"2\" fill=\"none\" stroke-linecap=\"round\"/>"
"                   </svg>"
"                   Apply"
"               </button>"

"               <div id=\"netStatus\" style=\"font-size:12px; margin-top:8px; color:#777;\"></div>"
"           </div>"

"       </div>"
"   </div>"

"</div>";

// __attribute__((aligned(128), section(".web_assets"))) 
static const char svg_io_template[] =
"<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" viewBox=\"0 0 60 513\"><style>.B{fill:#ff7f2a}.C{fill:#f95}.D{fill:#b3b3b3}.E{fill:#999}.F{fill:#1a1a1a}.G{fill:#f2f2f2}</style>"
"<path fill=\"#c87137\" d=\"M8.8 0h35.5v25.7H8.8z\"/>"
"<path d=\"M.1 492.1h48.2V513H0z\" class=\"D\"/>"
"<path fill=\"#666\" d=\"M54.4 493.3H60v3.8h-5.6zm-54.3 0h48.2v7.4H0z\"/>"
"<path fill=\"#ccc\" d=\"M.1 505.9h48.2v7H0zm0-473.5h48v127H0z\"/>"
"<path d=\"M48 40.4h12v119.9H48z\" class=\"D\"/>"
"<path fill=\"#666\" d=\"M48.2 34H60v6.4H48.2z\"/>"
"<path fill=\"#ccc\" d=\"M1.9 160.2h58.2V493H1.9z\"/>"
"<path fill=\"#666\" d=\"M0 158.6h3.4v334.8H0z\"/>"
"<path d=\"M3.6 178.8h6V492h-6z\" class=\"D\"/>"
"<path d=\"M14 144.5h32v3.6H14z\" class=\"E\"/>"
"<path d=\"M13.6 164.6H60v9.4H13.6z\" class=\"F\"/>"
"<path d=\"M3.6 175.8H59v8H3.6z\" class=\"D\"/>"
"<path d=\"M14 147.5h2.6v15.4H14z\" class=\"E\"/>"
"<path d=\"M55.3 167.3H60v6.9h-4.6z\" class=\"D\"/>"
"<path d=\"M5 42.9h36.3v100.6H5z\" class=\"F\"/>"
"<path d=\"M41 36.2h1.7v7.1H41z\"/>"
"<path d=\"M47.1 115.9H60v6.4H47zm0-14.3H60v6.5H47zm0-14h12.8v6.5H47zm0-14.3H60v6.4H47zM47 59.1h12.9v6.4h-13zm0 70.9h12.8v6.4H47zm0 14.3h13v6.4H47z\" class=\"F\"/>"
"<path fill=\"#a05a2c\" d=\"M9.7 5.5H43v9.7H9.7zm0 13.4h33.5v7.3H9.8z\"/>"
"<path fill=\"#d38d5f\" d=\"M9.6.8h33.5v1.4H9.6z\"/>"
"<path fill=\"#c87137\" d=\"M9 12.7h33.5v3.6H9.1z\"/>"
"<path d=\"M.2 25.6h48.6v7H.2z\" class=\"D\"/>"
"<path fill=\"#784421\" d=\"M24 25.5h13.3v5.8H23.9z\"/>"
"<g fill=\"#020201\">"
"<path d=\"M48 503.2v-9.7h-5.3l-.1 6.3-.1 6.2q-.2.3-8 .4l-6.3.1v3.9l-2.5-.1v-3.8h-4c-4.8-.2-4.7-.1-4.7-2.4v-1.4h-.6l-.6.1c0 1.8-.2 3.9-.3 4l-.8.1h-.5l-.1-2.1-.1-6.8v-4.5H8c-5.5 0-5.9 0-6.2-.4l-.5-.3v-318l.4-.5c.5-.4.5-.4 2.4-.4h2V162H1.2v-1.8H0v-2h12V145H3.9V93.8L4 44.4v-2.6l.7-.7c.4-.3 6.2-5.8 6.8-5.8s-1.2 4.3-3.6 5.6c-.3.2-.1.8-.3 1l-.7.3 17.5-.2h18v-5.7h-15l-15 .2-.4.2q-.3 0-.3.2c0 .4-1 1.1-1.5 1.1q-.7.1-.6-.5c0-.6 1-1.7 1.4-1.7q.2 0 .4-.3v-.3h32.3l.5.1V89l.1 53.6h1.5c1.7.1 2 .2 2 1l.1.4h12.1v1.5l-2 .1-5.2.1h-2.9v2.9H60v2H48v-.5l-.2-2.1V158h6.1l6 .1v1.8l-1.3.1-6 .1h-4.7v2.9c-.2 2.6-.8 7.2-1 7.5l-1 .2H45v-.6l.6-5c.1-.3-.7-.4-15.8-.4H14v8.9h40v-2.9l.1-3.7.2-.9H60v.8l-.1 1h-2l-1.7.2v5.6H58l2 .2v319.5h-2.2l-.4 1.3-.5 1.6 1.6.1H60v2h-2.6q-2.7 0-2.8-.3-.3-.2-.2-2.2l-.1-2.3c-.2-.4-4-.4-4.1 0l-.1 9.7v9.3h-2zm-8.4-4.3v-5.4H19v10.6c0 .2 2.2.3 10.4.2h10.2zM17 496.7v-3H16v5.8q0 .3.6.3h.5zm41.6-162.8-.1-157.7c-.2-.2-4.7-.3-27.5-.2H3.7v157.7l-.1 157.7h55zM11.1 163v-2.9H8.5v5.8h2.6zm34.7-9.4v-9l-13.7-.1H16.2l-2.2.1v18h31.9zM12.2 143l.2-.4h28.8V43H5.8v100.2h6z\"/>"
"<path fill=\"#88958e\" d=\"M9.3 481.5c-.9-1.7-1-3-1-5.7.1-2 .2-2.6.6-4l.6-1.6v-20.6L9 448a15 15 0 0 1 0-10.3l.5-1.3v-20.7l-.5-1.5A15 15 0 0 1 9 404l.5-1.2v-20.6l-.4-1.2q-2-5.4 0-10.6l.4-1.3v-20.3l-.5-1.6A15 15 0 0 1 9 337l.5-1.1v-20.9l-.5-1.4q-1.8-5.3 0-10.2l.5-1.1v-20.9l-.4-1.3a15 15 0 0 1 0-10.4l.4-1.1V247l-.5-1.5q-1.6-4.8-.1-9.4l.6-1.9v-20.8L9 212c-1.2-3.6-1.1-6.5.4-10.7l.3-1h1.6v.8q0 .9-.3 1.9a9 9 0 0 0-.3 7.4l.6 1.5v23.4l-.6 1.8c-.5 1.6-.6 2-.6 3.6s0 2 .6 3.5l.7 1.9v11.7c0 10.1 0 11.9-.3 12.4-.9 2.4-1 2.8-1 4.6 0 1.6 0 1.9.6 3.6l.7 1.8v23l-.7 1.6a8 8 0 0 0-.6 3.6c0 1.7 0 2 .6 3.4l.6 1.6v23.3l-.6 1.8c-.6 1.7-.6 2-.6 3.7s0 2 .6 3.4l.6 1.5v23.3l-.4 1.2a10 10 0 0 0-.2 7.7l.7 1.8v22.6l-.7 1.8c-.6 1.7-.6 2-.6 3.8s0 2 .6 3.6l.7 1.7v22.8l-.7 1.8c-.6 1.7-.6 2-.6 3.7s0 2 .6 3.7l.7 1.8v22.7l-.6 1.4c-.6 1.6-1 4-.9 4.9q.3 2.6.8 3.6l.3 1.2q.2.6-.6.5c-.5 0-.6 0-1.2-1.2z\"/>"
"<path d=\"M42 155v-5.2h1.5v5.1l-.1 5-.7.1h-.8zm6-21.7v-3.4c0-.1 2.7-.2 6-.2h6v1.7H49.9l-.2 1.4v1.5q.1.2 5 .2h5q.3.2.3 1v1H48zm0-14.2v-3.5h12v1.7H49.8v3.4H60v1.9H48zm0-14.3v-3.4h12v.7q0 .7-.3.8h-5l-4.9.1v3.5H60v1.7H48zm0-14v-3.4h12V89l-5.1.2h-5.1v3.1h2.8l5 .1 2.4.1V94l-2.1.1-6 .2H48zm0-14.3v-3.2h12v.7q0 .7-.3.7l-4.2.1-4.8.1-1 .1v3.1H60v1.7H48zm0-14V59h3.6c3.5 0 3.5 0 3.6-.4 0-.4.1-.4 2.4-.4H60v1.9h-1.5c-1.4 0-1.6 0-1.6.3s0 .4-3.5.4h-3.6V64H60v1.7H48zM48 37v-4.5H24l-.5-.1v-6.6h-1.8c-2.3 0-2.4 0-2.4.8v.7h-7.6c-7.4 0-7.6 0-7.6-.4-.1-.3-.3-.3-2.1-.3H0v-2H4l3.9-.1V0h2v25.3h3.7l3.7.1V25c0-1 0-1 4.2-1h4v3.3l.2 3.2h10.9v-4.5c.2-.3.5-.3 2.2-.3h2V25q-.2-1 1.3-.8h.8V0h1.9v8.6c0 8.2 0 15.1.2 15.6q0 .2 2.7.2t2.8.2q.3 0 .2 4.3v4.2H60v.9q.1 1.2-.5 1l-5 .1h-4.6v4.6H60v1.9H48zm.6-8.6v-2.1h-6v1.1l-4.2.1v2.8c0 .2 1 .2 5.1.2h5zM13 16q-.4 0-.5-.7c-.1-.7 0-9.8.2-10h26.9l.7.2v5.2l.1 5.2H27.8l-13.6.1zm26-1.7V6.9H14v7.5c0 .2 2.5.2 12.4.2 11.6 0 12.3 0 12.4-.2\"/></g>"
"<use class=\"G\" href=\"#B\"/>"
"<path d=\"M4.3 144.6h9.6v7.3H4.3z\" class=\"F\"/>"
"<path d=\"M15.1 82h5.1v5.1h-5z\" class=\"G\"/>"
"<path fill=\"#705b38\" d=\"m55.4 149-1.3-.1v-.5q0-.5-.3-.6c-.4 0-.4-1 0-1l.3-.8v-.7h2c2 0 2 0 2 .3q.1.3-.5.3h-.5v2.6h1q.9 0 .9.2c0 .5-.8.5-3.6.3zm-.1-14.1H54v-.7q0-.6-.2-.6t-.3-.5.3-.5.2-.8v-.7h3.1v3.2h1.2q1.4 0 1.2.3c0 .4-.1.4-1.5.4zm2.8-3.6q0-.3 1.2-.4.4 0 .4.3 0 .4-.8.4t-.8-.2m-4-10.4v-.8q0-.6-.3-.6t-.3-.6q0-.7.3-.6t.2-.6q0-.5.2-.7l2.8-.2c2.6-.1 2.7-.1 2.7.3 0 .3-.1.3-1.3.3H57v3.6h-1.4zm1.3-14-1.3-.1v-.7q0-.8-.3-.8t-.3-.5.4-.7.3-.7v-.7h1.4l1.5-.1v3.8h1.3q1.5 0 1.3.3c0 .3-.1.3-1.5.3zm-1.2-14.3-.2-.8q0-.6-.2-.6c-.4 0-.4-1 0-1q.4-.2.3-.6c-.1-.9 0-1 1.6-1H57v4.1h-2.9zm0-14.6q0-1-.4-1c-.4 0-.3-1 0-1q.4 0 .3-.7v-.7h1.5l1.5-.1v4h-1.4c-1.5 0-1.5-.1-1.5-.5m4.1-3.2c0-.3.6-.5 1-.4q.9.4-.3.6-.7 0-.7-.2m-3-10.3H54v-.8q0-.6-.2-.7-.3-.2-.3-.5 0-.5.3-.5t.2-.7v-.8h1.2q1.3 0 1.3-.3 0-.4.4-.4.3-.1.1 1.2v2l.2 1h1.1q1.4 0 1.2.2c0 .4 0 .4-1.5.4z\"/>"
"<g fill=\"#9c8761\">"
"<path d=\"M56.6 147.2v-1.8h.8l1.5-.2h.8l-.1.6-.1 1.8v1.4h-2.9zm1.2-12.3h-1.2V131h2.9v1.4l.1 2q.1.5-.2.5zm-4.3-1.9.2-.4q.6 0 .4.5-.5.7-.6 0\"/><use href=\"#C\"/>"
"<path d=\"M56.6 104.7v-2h3v4.2h-3z\"/><use y=\"-28.3\" href=\"#C\"/>"
"<path d=\"M56.6 76.5v-2h3.1l-.1.5-.1 2v1.4h-2.9zm0-14.4v-2.3h2.9v4.6h-2.9z\"/></g><g fill=\"#b7ae83\">"
"<path d=\"M59 147.2c0-1.6.1-1.9.4-2 .2 0 .3.2.3 1.9 0 1.8 0 1.9-.4 1.9-.3 0-.3-.1-.3-1.9m0-14.2c0-1.8 0-2 .3-2s.4.2.4 2 0 1.8-.4 1.8-.3 0-.3-1.8\"/><use href=\"#D\"/>"
"<path d=\"M59 106.7v-2.1c0-1.6.1-2 .4-2 .2 0 .3.2.3 2.1 0 1.8 0 2.2-.3 2.2z\"/><use y=\"-28.3\" href=\"#D\"/>"
"<path d=\"M59 76.5v-2h.5q.5 0 .5.3 0 .4-.2.4t-.1 1.6c0 1.5 0 1.6-.4 1.6-.3 0-.3 0-.3-1.9m0-14.4c0-2.1 0-2.3.3-2.3s.4.2.4 2.3 0 2.3-.4 2.3-.3 0-.3-2.3\"/></g>"
"<path d=\"M23.3 42.7H5.7l3-3 3.2-3.2c.3-.1 4.3-.2 14.9-.2h14.4v6.5z\" class=\"E\"/>"
"<g fill=\"#bebba3\"><use href=\"#E\"/><use y=\"-14.2\" href=\"#E\"/><use href=\"#F\"/>"
"<path d=\"M59.2 104.7v-2h.8v4.2h-.9z\"/><use y=\"-28.3\" href=\"#F\"/><use y=\"-70.6\" href=\"#E\"/>"
"<path d=\"M59.2 62.1v-2.3h.8v4.6h-.9z\"/></g>"
"<path fill-opacity=\"0\" d=\"M13 144.2h33.2v3.4H12.9z\"/><g class=\"G\">"
"<path d=\"M15 100h5v5h-5zm0 9h5v5h-5z\"/><g stroke=\"#000\"><use y=\"44.8\" href=\"#B\"/><use y=\"26.8\" href=\"#B\"/><use y=\"35.5\" href=\"#B\"/><use y=\"18.1\" href=\"#B\"/><use y=\"8.8\" href=\"#B\"/></g></g><g class=\"E\"><circle cx=\"20.8\" cy=\"477.4\" r=\"9.1\"/><circle cx=\"42.7\" cy=\"477.6\" r=\"9.1\"/><circle cx=\"43.4\" cy=\"443.6\" r=\"9.1\"/><circle cx=\"20\" cy=\"443.4\" r=\"9.1\"/>"
"<circle cx=\"43.4\" cy=\"409.6\" r=\"9.1\"/><circle cx=\"19.8\" cy=\"410.3\" r=\"9.1\"/><circle cx=\"19.7\" cy=\"376.5\" r=\"9.1\"/><circle cx=\"43.2\" cy=\"377.2\" r=\"9.1\"/><circle cx=\"43\" cy=\"342.9\" r=\"9.1\"/><circle cx=\"19.9\" cy=\"343.3\" r=\"9.1\"/><circle cx=\"20.1\" cy=\"309.6\" r=\"9.1\"/><circle cx=\"43.1\" cy=\"309.4\" r=\"9.1\"/><circle cx=\"20.2\" cy=\"275.9\" r=\"9.1\"/><circle cx=\"43.4\" cy=\"275.9\" r=\"9.1\"/>"
"<circle cx=\"43.1\" cy=\"241.8\" r=\"9.1\"/><circle cx=\"19.6\" cy=\"240.8\" r=\"9.1\"/><circle cx=\"19.7\" cy=\"207.5\" r=\"9.1\"/><circle cx=\"42.4\" cy=\"207.4\" r=\"9.1\"/></g>"
"<g stroke=\"#000\" class=\"G\"><use y=\"-.1\" href=\"#B\"/><use y=\"-9.1\" href=\"#B\"/>"
"<path d=\"M15 73h5v5h-5z\"/><use y=\"-27.1\" href=\"#B\"/>"
"<path d=\"M14.9 54.8h5v5h-5z\"/><use x=\"12.9\" y=\"44.8\" href=\"#B\"/><use x=\"12.8\" y=\"26.8\" href=\"#B\"/><use x=\"12.9\" y=\"35.4\" href=\"#B\"/><use x=\"12.8\" y=\"18\" href=\"#B\"/><use x=\"12.8\" y=\"8.8\" href=\"#B\"/>"
"<path d=\"M27.8 90.9h5v5h-5z\"/><use x=\"12.8\" y=\"-9.1\" href=\"#B\"/><use x=\"12.9\" y=\"-18.1\" href=\"#B\"/><use x=\"12.9\" y=\"-27.2\" href=\"#B\"/><use x=\"12.8\" y=\"-36.3\" href=\"#B\"/></g>"
"<path d=\"M11.6 185.8H28v10.6H11.6z\" class=\"B\"/><use class=\"C\" href=\"#G\"/>"
"<circle cx=\"19.7\" cy=\"190.9\" r=\"2.2\"/>"
"<path d=\"M17.5 216.9a10 10 0 0 1-7.4-6.5c-.5-1.4-.5-4.5-.1-5.8a10 10 0 0 1 4.2-5.6q2.4-1.1-1.2-1H11v-13.2h17.8V198h-2c-1.2 0-2.2.1-2.2.2l.8.5c2 1 3.7 3.2 4.4 5.4.5 1.6.5 4.6 0 6.2a9 9 0 0 1-6.5 6.4c-1.5.4-4 .5-5.7.2m4.8-2c3.7-1.2 6.5-5.3 5.9-8.6a9 9 0 0 0-4.3-5.9 9 9 0 0 0-8.6.4 9 9 0 0 0-3.6 5.8c-.4 3.4 2 6.9 5.5 8.2q2.5 1 5.1.1zm.5-17.5v-.5h-6.1v1h6.1zm-9-1.5q-.1-.6 1.4-.7l5.7.1 4.8.1v.5q-.2.6.7.5h.7v-9.8h-7.3l-7.2-.1v9.9h.6q.8.1.6-.5\"/>"
"<path d=\"M11.7 219.6h16.2v10.6H11.7z\" class=\"B\"/><use y=\"33.8\" class=\"C\" href=\"#G\"/><circle cx=\"19.7\" cy=\"224.6\" r=\"2.2\"/>"
"<use href=\"#H\"/>"
"<path d=\"M12.4 254h16.3v10.6H12.4z\" class=\"B\"/>"
"<use x=\".8\" y=\"68.1\" class=\"C\" href=\"#G\"/><circle cx=\"20.5\" cy=\"259\" r=\"2.2\"/><use y=\"34.4\" href=\"#H\"/>"
"<path d=\"M11.7 287.7h16.2v10.6H11.7z\" class=\"B\"/><use y=\"101.9\" class=\"C\" href=\"#G\"/><circle cx=\"19.8\" cy=\"292.8\" r=\"2.2\"/><use y=\"68.1\" href=\"#H\"/>"
"<path d=\"M11.7 321.2h16.2v10.6H11.7z\" class=\"B\"/><use y=\"135.4\" class=\"C\" href=\"#G\"/><circle cx=\"19.8\" cy=\"326.3\" r=\"2.2\"/><use y=\"101.6\" href=\"#H\"/>"
"<path d=\"M11.7 355h16.2v10.6H11.7z\" class=\"B\"/><use y=\"169.1\" class=\"C\" href=\"#G\"/><circle cx=\"19.8\" cy=\"360\" r=\"2.2\"/><use y=\"135.4\" href=\"#H\"/>"
"<path d=\"M11.7 388.2h16.2v10.6H11.7z\" class=\"B\"/><use y=\"202.3\" class=\"C\" href=\"#G\"/><circle cx=\"19.7\" cy=\"393.2\" r=\"2.2\"/><use y=\"168.6\" href=\"#H\"/>"
"<path d=\"M12.1 422h16.3v10.6H12z\" class=\"B\"/>"
"<path d=\"M19.4 422.7H21v8.7h-1.6z\" class=\"C\"/><circle cx=\"20.2\" cy=\"427\" r=\"2.2\"/><use y=\"202.4\" href=\"#H\"/>"
"<path d=\"M11.7 456h16.2v10.5H11.7z\" class=\"B\"/><use y=\"270.1\" class=\"C\" href=\"#G\"/><circle cx=\"19.7\" cy=\"461\" r=\"2.2\"/><use y=\"236.3\" href=\"#H\"/>"
"<path d=\"M34.8 185.7h16.3v10.6H34.8z\" class=\"B\"/><use x=\"23.2\" y=\"-.1\" class=\"C\" href=\"#G\"/><circle cx=\"42.9\" cy=\"190.8\" r=\"2.2\"/><use x=\"23.2\" y=\"-33.9\" href=\"#H\"/>"
"<path d=\"M34.8 219.5h16.3V230H34.8z\" class=\"B\"/><use x=\"23.2\" y=\"33.6\" class=\"C\" href=\"#G\"/><circle cx=\"42.9\" cy=\"224.5\" r=\"2.2\"/><use x=\"23.2\" y=\"-.1\" href=\"#H\"/>"
"<path d=\"M35.6 253.8h16.2v10.7H35.6z\" class=\"B\"/><use x=\"24\" y=\"68\" class=\"C\" href=\"#G\"/><circle cx=\"43.7\" cy=\"258.9\" r=\"2.2\"/><use x=\"23.2\" y=\"34.3\" href=\"#H\"/>"
"<path d=\"M34.8 287.6h16.3v10.6H34.8z\" class=\"B\"/><use x=\"23.2\" y=\"101.8\" class=\"C\" href=\"#G\"/><circle cx=\"42.9\" cy=\"292.6\" r=\"2.2\"/>"
"<path d=\"M40.7 318.7a10 10 0 0 1-7.4-6.6c-.4-1.3-.5-4.4-.1-5.7a10 10 0 0 1 4.2-5.7q2.4-1-1.1-.9H34v-13.3h9l8.8.1v13.1h-2l-2.1.3.8.5c1.9 1 3.6 3.2 4.3 5.4.5 1.5.5 4.6 0 6.1a9 9 0 0 1-6.5 6.5c-1.5.4-4 .5-5.7.2zm4.8-2c3.7-1.2 6.5-5.3 5.9-8.6a9 9 0 0 0-4.3-6 9 9 0 0 0-8.5.4 9 9 0 0 0-3.7 5.9c-.3 3.4 2 6.8 5.5 8.1q2.6 1 5.1.2zM46 299v-.5H40v1H46zm-9-1.5q0-.6 1.4-.6h5.7l4.8.2v.4q-.2.6.7.5h.7v-9.8H35.7v9.8h.6q.8.1.6-.5z\"/>"
"<path d=\"M34.8 321.1h16.3v10.6H34.8z\" class=\"B\"/><use x=\"23.2\" y=\"135.3\" class=\"C\" href=\"#G\"/>"
"<circle cx=\"42.9\" cy=\"326.1\" r=\"2.2\"/><use x=\"23.2\" y=\"101.5\" href=\"#H\"/>"
"<path d=\"M34.8 354.9h16.3v10.6H34.8z\" class=\"B\"/><use x=\"23.2\" y=\"169\" class=\"C\" href=\"#G\"/><circle cx=\"42.9\" cy=\"359.9\" r=\"2.2\"/><use x=\"23.2\" y=\"135.3\" href=\"#H\"/>"
"<path d=\"M34.8 388h16.3v10.7H34.8z\" class=\"B\"/><use x=\"23.2\" y=\"202.2\" class=\"C\" href=\"#G\"/><circle cx=\"42.9\" cy=\"393.1\" r=\"2.2\"/><use x=\"23.2\" y=\"168.5\" href=\"#H\"/>"
"<path d=\"M35.3 421.9h16.2v10.6H35.3z\" class=\"B\"/><use x=\"23.6\" y=\"236\" class=\"C\" href=\"#G\"/><circle cx=\"43.4\" cy=\"426.9\" r=\"2.2\"/>"
"<path d=\"M40.7 453a10 10 0 0 1-7.5-6.6c-.4-1.3-.4-4.4 0-5.7a10 10 0 0 1 4.2-5.7q2.4-1.1-1.2-1h-2.1L34 428v-7.1h18V434h-2.2q-1.8.1-2 .2c-.3.1.3.3.7.6 2 1 3.7 3.2 4.4 5.3.5 1.6.5 4.6 0 6.2a9 9 0 0 1-6.5 6.5c-1.5.4-4.1.4-5.7.1zm4.8-2c3.7-1.2 6.5-5.3 5.9-8.7a9 9 0 0 0-4.3-5.9 9 9 0 0 0-8.6.4 9 9 0 0 0-3.6 5.8c-.4 3.4 2 6.9 5.5 8.2q2.6 1 5.1.1zm.5-17.6v-.5H40v1H46zm-9-1.5q-.1-.7 1.4-.6H44l5 .1v.5q-.2.6.6.5h.7v-9.8H35.7v9.8h.6q.8.1.6-.5z\"/>"
"<path d=\"M34.8 455.8h16.3v10.6H34.8z\" class=\"B\"/><use x=\"23.2\" y=\"270\" class=\"C\" href=\"#G\"/><circle cx=\"42.9\" cy=\"460.9\" r=\"2.2\"/>"
"<path d=\"M40.7 486.9a10 10 0 0 1-7.5-6.6c-.4-1.3-.4-4.4 0-5.7a10 10 0 0 1 4.2-5.6q2.4-1.1-1.2-1h-2.1l-.1-6.2v-7.1h9l9 .1V468l-2.2.1q-1.8 0-2 .2c-.3.2.3.3.7.5 2 1 3.7 3.2 4.4 5.4.5 1.6.5 4.6 0 6.1a9 9 0 0 1-6.5 6.5c-1.5.4-4.1.5-5.7.2zm4.8-2c3.7-1.2 6.5-5.3 5.9-8.6a9 9 0 0 0-4.3-6 9 9 0 0 0-8.6.5 9 9 0 0 0-3.6 5.8c-.4 3.4 2 6.8 5.5 8.1q2.6 1 5.1.2zm.5-17.6v-.5H40v1H46zm-9-1.4q-.1-.6 1.4-.7H44l5 .2v.5q-.2.5.6.5h.7v-9.8l-7.3-.1h-7.2v9.9h.6q.8 0 .6-.5z\"/>"
"<path d=\"M4.8 145h7.4v4.7H4.8z\" class=\"E\"/>"
"<path d=\"M14.2 492.9h4.5v14h-4.5zM37.7 6.5h2V15h-2z\"/>"
"<path d=\"M5.7 42.2h35.5V52H5.7z\" class=\"E\"/><defs>"
"<path id=\"B\" d=\"M15 91h5v5h-5z\"/>"
"<path id=\"C\" d=\"M56.6 119v-2h2.9v3.5l.2.5h-3.1z\"/>"
"<path id=\"D\" d=\"m59.1 120.9-.1-2c0-1.9 0-2 .3-2s.4.1.4 2c0 1.7 0 2.1-.3 2.1z\"/>"
"<path id=\"E\" d=\"M59.2 147.1v-1.9h.8v3.8h-.9z\"/>"
"<path id=\"F\" d=\"M59.2 119v-2h.8v4h-.9z\"/>"
"<path id=\"G\" d=\"M19 186.5h1.5v8.8H19z\"/>"
"<path id=\"H\" d=\"M17.5 250.7A10 10 0 0 1 10 244c-.4-1.3-.5-4.4-.1-5.7a10 10 0 0 1 4.3-5.7q2.2-1-1.2-.9h-2.2v-13.3h9l8.9.1v13.1h-2.1q-2 .1-2.1.2 0 .2.8.6c1.9 1 3.7 3.2 4.3 5.4.5 1.5.6 4.6 0 6.1a9 9 0 0 1-6.5 6.5c-1.5.4-4 .5-5.7.2m4.8-2c3.8-1.2 6.5-5.3 5.9-8.7a9 9 0 0 0-4.3-5.8 9 9 0 0 0-8.5.3 9 9 0 0 0-3.7 5.9c-.3 3.4 2 6.8 5.5 8.1q2.5 1 5.2.2zM23 231v-.5h-6.2v1H23zm-9-1.5q-.1-.6 1.3-.6H21l4.9.2v.4q-.1.6.6.5h.7v-9.8H12.5v9.8h.6q.8.1.6-.5\"/>"
"</defs></svg>";

// __attribute__((aligned(128), section(".web_assets"))) 
static const char svg_plc_template[] =
"<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" viewBox=\"0 0 265 839\">"
"<path d=\"M108 22h98v50h-98zM0 85h265v720H0z\"/>"
"<path d=\"M115 14h96v71h-96z\"/>"
"<path fill=\"#ccc\" d=\"M105 326h104v350H105z\"/>"
"<path fill=\"#1a1a1a\" d=\"M146 87h106v236H146z\"/><g fill=\"#081003\">"
"<path d=\"M160 766v-2l1-1v1h23l23-1 1 3z\"/><use href=\"#a\"/>"
"<path d=\"M186 755h-3v-11h12v11h-9m7-5-1-4v3l-1 4q-2 2 0 0h2zm-7 3-1-1q0-2 0 0zm4-4-2-1q-2 0-1 1l1 2q2 1 2-2m17-10v-5l1 6-1 4zm-47-11 1-10v20zM183 731l-1-10 7-1h6v11h-13m10-5q0-6-1-4l-1 1 1 3v3h-4q-4 0-3-2v1l-1 1h9zm-3-1-2-1-1 1 1 2q3 1 2-2m-5-3-1 2 1 1v-3m22-10v-2l1 2-1 2z\"/><use y=\"-51\" href=\"#a\"/>"
"<path d=\"M183 702v-6h12v11h-12zm10 0v-4h-2v7h2zm-3 0q0-2-2-2l-1 2q0 2 2 1z\"/><use href=\"#b\"/>"
"<path d=\"m169 686 1-1 2 1-2 1q-2 0-1-1m5 0 17-1 17 1-17 1zM21 153l-1-1v-15l1-17 9-1 10 1-8 1h-9l-1 16v15l8 1 6 1-7 1c-7 0-7 0-8-2m148-16v-3q2 0 1 4v3zm85-7h4q3 1-1 1zm-85-4q0-6-2-5v-1h3l1 4q0 6-2 5zm-127-6 5-1 5 1-5 1zm28 0 3-1q2 1-1 2z\"/><use href=\"#c\"/><use x=\"7\" href=\"#c\"/>"
"<path d=\"m92 120 13-1 14 1-13 1zm30 0 10-1 9 1-10 1zm33 0zq8-1 9 1l-5 1z\"/></g><g fill=\"#091e03\">"
"<path d=\"M160 765v-11h1v4l1 6-1 2zm21-4-6-1v-5h-4v3l-1 2h-7v-21h7l1 2v2h4v-1c0-4-1-4 16-4h16v3l-2 5q-3 2-3 4t3 4l2 4v3h-26m13-11v-5l-10 1v8h9z\"/><use href=\"#d\"/><use y=\"-5\" href=\"#d\"/>"
"<path d=\"M160 741q-1-4 1-6v7zm15-7v-2h-4v4h-8v-21h7l1 2v3h4v-5h16l16-1v3q0 3-2 4-2 2-3 5l2 3q3 2 3 5v3h-32zm19-8v-5h-5l-5 1v7l5 1h5zm-9-3 3-1 3 1-3 1zM160 720v-11l1-1v13zm15-10v-2h-4v2l-1 2h-7v-21h7l1 2v2h4v-5h31v3q0 3-2 5l-2 4 3 3q3 3 1 5v2l-15 1h-16zm18-8v-4h-4l-5-1v9h9zm-7 3-1-1 3-1 3 1-2 1zm-1-5q0-2 2-1t0 1zm-25-3 1-4v3q-1 3-1 1M39 153l1-1 2 1-2 1q-2 0-1-1\"/></g><g fill=\"#1f4506\">"
"<path d=\"M160 765v-39l1-38 1 38v38h44v-3h-31v-1l-1-3-1-1h-2v5h-8v-4l1 1 3 1h2v-5h7v5h7l22-1-2-3q-2-2-2-5l2-6 2-3v-1h-28l-1 1q-1 3-1-1v-2h31l-16-1h-15v-5h-4v4l-4 1c-5 0-5 0-5-9l1-8 1 8v7h5l1-5h6v3l1 2h28v-1l-2-3q-2-2-2-5 0-4 3-7l1-2v-1h-14l-15 1q0 2-1-1v-2h31l-15-1h-16v-2q0-4-2-3h-2v4l-4 1-4-1v-21l3-1 5 1v2q0 3 2 2h1l1-5h31v-2l-1-1-1-1h4v79h-48m46-15v-5l-1 1q-4 4-1 7l2 2zm0-25v-4l-1 1q-4 4-1 7l2 1zm-1-15q1-1-2-3l-2-2v-3q0-5 2-6l2-3v-2l-14 1h-15v2l-1 3h-3l-3-1v-4h-5v19h5v-5h7v3l1 2h28zm1-9v-5l-1 2q-4 3-1 7l2 1zm-21 1v-3h6v5h-6zm5 0-2-1-1 1 1 1zm-28 47 1-5 1 5-1 6z\"/><use href=\"#e\"/><use x=\"4\" href=\"#e\"/>"
"<path d=\"M169 743v-3h-2q-3-1-2 2l-1 1-1-2v-3h8v4l3 1 2 1-3 1h-4zm16-17 1-3 1 2 1 1 2-1v-2l1 3v2h-6zm-16-7v-2h-5v1l-1 2-1-3 1-2h8v4h3l2 1-3 1h-4zm14-33h11l-6 1zM29 154l-8-1-1-17 1-16h141l7 1 1 16v15l-2 2H38l-1 1 2-1H29m13-7-1-2v4zm47-2v-5h3l-1-1q-2-1-2-5v-2 18zm-49-4h-3q-6 0 0 1l3 1v-2m56 0h-2l4 1zm-35 0h-5l2 1zm18 0h-7zm42 0h-5zm13-5q0-6-1-5v10zm15 0v-5h-1l1 6v4zm11 5h-2zm-33-4q0-10-1-3l-1 4-1 2h1l1 1h1zm-45 0-1-1v3zm77-10 4-1h-7v-5l-1 8q1 16 1 4v-6zm-100 7q10 1 7-3l-1-1v3H51v-3l-1 2v3l1 2v-3zm-15-1v-2zq1 5 1-1m61 1-1-3zv2zm6-1v-3l-1 2v3l1 2zm-83 1v-2 4zm45-4v-2zl1 3zm70-4zl-3 1h4m-106 0h-2 2m22 0h-2 2m23 0h-6 6m16 0h-3 3m22 0h-3 3m133 6v-2h12v4h-12z\"/></g>"
"<path fill=\"#362a10\" d=\"M252 264v-1l7-1 6 1-6 1zm0-23zh7l6 1-6 1zm1-22zh6l6 1-4 1-7 1zm-1-19v-1l1-2 1 1 5 1 6 1-6 1zm0-22v-2l7-1 6 1-6 1-5 1zm-60-20 1-3 1 1h9v-46h-2q-4-1-7 1-2 1-2-2-1-4 7-3h5v52l-6 1zm60-4zh7l6 1-6 1z\"/>"
"<path fill=\"#4b4b49\" d=\"m120 800-1-1h6l7 1h-12m140-559 3-1 2 1-2 1zm-7-21 1-1 1 1h-1zm-40-61 1-3v2z\"/><g fill=\"#817038\"><use href=\"#f\"/>"
"<path d=\"m253 263 6-1 6 1-6 1zm0-22v-1h4l7 1-4 1h-6zm0-22h12l-6 1c-6 0-5 0-5-1\"/><use y=\"-86\" href=\"#f\"/><use y=\"-107\" href=\"#f\"/>"
"<path d=\"m193 157-1-24 1-24h11v2l-1 2-1-2-3-1h-3v12h6v7l-1 2 1 2h-3q-4 0-4-2l-1-2v8h8v7h-7l1 6v6h8q1 2-7 2z\"/></g>"
"<path fill=\"#21d04b\" d=\"m161 738-1-39v-13h47v79h-46zm9 19v-2h3l3 1v4h29v-2q0-1-2-3l-1-5 1-5 2-4v-2h-29v5h-3q-4 1-4-3l-2-1h-3v4l1 15h5zm35-23-2-4-1-4 1-5 2-4v-1l-14-1h-15v5h-6v-4h-6v19h6v-4h6v5h29zm0-24-2-4-1-5 1-5q3-1 2-3v-2h-29v6h-6v-2q0-4-3-4l-3 1v19h6v-4h6v4l15 1h14zm-19 39v-2h2q4-1 3 3v2h-5zm0-24v-2h5v5h-5zm0-23v-3h5v5h-5z\"/><g fill=\"#8c7b41\">"
"<path d=\"m256 285 4-1 5 1-5 1zm-3-22 4-1 5 1-5 1zm11 0q-2 0 0-1l1 1zm-11-44h2q2 1 0 1-2 1-2-1m7 0h5l-2 1z\"/><use x=\"42\" y=\"-530\" href=\"#b\"/>"
"<path d=\"m253 155 1-1 6 1 5 1h-12zm-51-7v-4h-1q-4 0-1-2h1-1l-1-1h-1l-1 1h-2 2q3 2 0 2l-1 3-1 3-1-3q0-7-1-1l-1-13 1-15v3q0 3 1-2l1-3 1 3q-1 4 2 4l2-1h2v-12l-1-1q1-2 3 1v38l-1 5zm-1-10h-5q-1 1 2 1zm-2-1h-1zm1-6h-3zm0-2v-1l1-1h-1q-1 0 0 0l-2-1-2 1h-1l1 1v1h4m1-5-3-1-3 1h6m-54 9 1-2 1 1q-1 4-2 1\"/></g><g fill=\"#9f8656\">"
"<path d=\"m253 285 6-1 6 1-6 1zm0-22 6-1 6 1h-12m0-22 6-1 6 1-6 1zm0-21 6-1h6l-5 1h-7\"/><use href=\"#g\"/><use y=\"-22\" href=\"#g\"/>"
"<path d=\"M253 155v-1h12v2h-12z\"/></g>"
"<path fill=\"#b29c50\" d=\"M195 150v-7h6v12l-2 1h-3l-1 1zm0-14 2-1 3 1v1l-3 1q-2 0-2-2m0-2q-1-3 5-2l1 1-3 1zm0-4zl3-1 4 1q0 2-4 2zm0-4zh4q2 2-3 2zm0-8 1-6 4-1h1v12h-6z\"/><g fill=\"#979792\">"
"<path d=\"M118 800H5V87h191v6l-1 6-3 1q-5 1-6 6v55q3 5 8 5h2v6h-34v143l23 1h22v6h-96v116h96v15h-95v222h95v5h-51v90h51v17l20 1h20v12H118m54-642c3-2 3-2 3-21v-17l-1-2-3-2-76-1H20l-3 3-2 1v35l2 1 2 3 76 1h75zm43 620h-2V313h28q5-1 7-6l1-7v-10h12v488h-46m-44-474-1-2-1-60c0-66 0-61 3-63h31v-12l2-1 5-2c3-3 3-2 3-32v-27l-1-2q-2-4-6-4l-2-1-1-3v-3h18c19 0 20 1 21 3l1 103-1 104-2 2-34 1zm26-2v-5c-4-3-8 2-5 6h4zm6-21-1-8-5-1h-8v18h12zm0-23v-7h-11v11h11zm35-1v-6h-11v11h11zm-35-20c0-7 1-7-6-7h-5v11h11zm35-1v-6h-11v11h11zm-35-21v-5h-11v10h11zm35 0v-6h-11v11h11zm-35-21v-5h-11v11h6l5-1zm35 0v-5h-11v11h11z\"/><use href=\"#h\"/>"
"<path d=\"M249 251v-3h16v7h-16z\"/><use y=\"-43\" href=\"#h\"/><use y=\"-65\" href=\"#h\"/>"
"<path d=\"m250 190-1-4v-3h16v7h-15\"/><use href=\"#i\"/><use y=\"-22\" href=\"#i\"/>"
"<path d=\"M249 114v-11h15v-8q1-1 1 14v16h-16z\"/></g>"
"<path fill=\"#fefefe\" d=\"M0 808v-2h252v-15h7v3l1 4 4 2h1v7zm35-655 1-2q4-1 4-4t-4-4q-5 0-8-5-3-6 0-10c5-5 13-5 16 1q3 6-1 11l-2 2 1 1v8q-4 5-7 2m4-14q5-3 2-9-2-3-6-3-5 2-5 5c-1 6 5 10 9 7m49 1v-10l1-2q5-6 12-3c6 3 7 11 2 15q-5 5-10 2l-3-1v10h-3zm12-1q6-6-1-11c-4-2-10 2-9 7q3 6 10 4m-45 3q-6-2-6-8 1-9 9-10 8 1 9 9v2H52v1l2 2q3 3 7 1h2c2 3-4 5-8 3m9-11q-2-3-6-4-3 0-5 3l-1 2h6zm52 11c-6-1-9-9-5-14 3-5 12-5 15 0q3 1 2 8v7h-1l-2-1h-9m5-3q4-2 4-5 0-4-4-6h-5q-4 2-4 6l1 4 3 1zm-50 2v-1h4q7 0 5-3-1-2-4-2-5-2-5-5l2-4q0-2 5-2h5v3h-3q-5 0-5 2-1 3 3 3l4 2 1 4-1 3q-2 3-6 2h-5zm61 0v-6l1-7q8-7 15-1 3 3 2 10v6h-1l-2-1v-5q1-9-3-9h-6q-4 1-3 9v6h-2zm24 1c-2-2-2-3-2-13v-9h2l1 2v2h3q4 0 4 2v1h-7v6l1 6q2 1 3-1h3l-2 4zm-4-61q0-3 3-3l2 2v2h-2z\"/><g fill=\"#b3b3b3\">"
"<path d=\"M5 87h7v713H5z\"/>"
"<path d=\"M9 87h187v5H9zm207 228h4v462h-4zm-48-136h4v125h-4z\"/>"
"<path d=\"M171 179h32v4h-32z\"/><rect width=\"38.7\" height=\"2.8\" x=\"204\" y=\"93\" rx=\"0\" ry=\"1.4\"/>"
"<path d=\"M239 95h3v209h-3zm-35-2h3v3h-3zm-89 366h10v217h-10z\"/></g><use href=\"#j\"/><use x=\"35\" href=\"#j\"/><use y=\"21\" href=\"#j\"/><use x=\"35\" y=\"21\" href=\"#j\"/><use y=\"41\" href=\"#j\"/><use x=\"35\" y=\"42\" href=\"#j\"/>"
"<path fill=\"#333\" d=\"M198 18h7v54h-7z\"/>"
"<path fill=\"#1a1a1a\" d=\"M109 23h89v48h-89z\"/>"
"<path fill=\"#333\" d=\"M117 16h88v4h-88zm-1 55h82v3h-82z\"/><use x=\"35\" y=\"62\" href=\"#j\"/><use y=\"62\" href=\"#j\"/>"
"<path d=\"M189 272h14v19h-14z\"/>"
"<path fill=\"#87090c\" d=\"M176 76h11v7h-11z\"/>"
"<path fill=\"#979792\" d=\"M113 459h3v217h-3z\"/>"
"<path fill=\"#b3b3b3\" d=\"M115 328h10v105h-10z\"/>"
"<path fill=\"#979792\" d=\"M112 328h4v105h-4z\"/>"
"<path d=\"M142 570h-37V453h104v117h-67m60-24v-18h-10v7h-11v21h-21v-10h-12l-1 5v5h-25v-90h25v10h13v-10h21v21h12v8h9v-36h-89l-1 52v52h90zm-61 2v-1h-12v3h12zm12-36v-29h-12l-12 1 9 1h9v7h-9l-9 1v1h18v7h-18v1l9 1 9 1v6h-9l-9 1 3 2h15v7h-9l-9 1h18v8h-9l-9 1 9 1h9v7h-18v2h24zm-12-37v-2h-12v3h12z\"/><use href=\"#k\"/><use fill=\"#313231\" href=\"#l\"/><use href=\"#m\"/><g fill=\"#313231\"><use href=\"#n\"/><use x=\"1\" y=\"64\" href=\"#n\"/></g>"
"<path fill=\"#f9f9f9\" stroke=\"#000\" stroke-width=\"2.8\" d=\"M184 468h15v17h-15zm0 70h15v17h-15z\"/>"
"<path d=\"M142 682h-36V564h103v118h-67m60-24v-18h-9v7h-12v21h-21v-10h-12v5l-1 5h-25v-90h25v10h13v-10h21v21h12v7h9v-35h-89v104h89zm-61 2v-1h-12v3h12zm12-37v-28h-12l-12 1 9 1h9v7h-9l-9 1v1h18v7h-18v1l9 1h9v7h-9l-9 1 3 1h15v7h-9l-9 1 9 1h9v8h-9l-9 1h18v7h-18v3h12l12-1zm-12-37v-1h-12v3h12z\"/><use y=\"112\" href=\"#k\"/><use y=\"112\" fill=\"#313231\" href=\"#l\"/><use y=\"112\" href=\"#m\"/><g fill=\"#313231\"><use y=\"112\" href=\"#n\"/><use x=\"1\" y=\"176\" href=\"#n\"/></g>"
"<path fill=\"#f9f9f9\" stroke=\"#000\" stroke-width=\"2.8\" d=\"M184 580h15v16h-15zm1 70h14v17h-14z\"/>"
"<path d=\"M142 439h-36V322h103v117h-67m60-24v-18h-9v7h-12v22h-21v-11h-12v5l-1 6h-25v-91h25v10h13v-10h21v22h12v7h9v-35l-45-1h-44v104h89zm-61 2v-1h-12v3h12zm12-36v-29h-12l-12 1 9 1h9v7h-9l-9 1v1h18v7h-18v1l9 2h9v7h-18l3 2h15v7h-9l-9 1 9 1h9v7h-9l-9 1 9 1h9v7h-18v2h24zm-12-37v-2h-12v4l6-1h6z\"/><use y=\"-131\" href=\"#k\"/><use y=\"-131\" fill=\"#313231\" href=\"#l\"/><use y=\"-131\" href=\"#m\"/><g fill=\"#313231\"><use y=\"-131\" href=\"#n\"/><use x=\"1\" y=\"-67\" href=\"#n\"/></g>"
"<path fill=\"#f9f9f9\" stroke=\"#000\" stroke-width=\"2.8\" d=\"M184 337h15v17h-15zm1 70h14v17h-14z\"/><rect width=\"23.3\" height=\"63.8\" x=\"188\" y=\"101\" fill=\"none\" stroke=\"#6e6607\" stroke-width=\"1.8\" ry=\"8.8\"/><defs>"
"<path id=\"a\" d=\"m160 750 1-11v22z\"/>"
"<path id=\"b\" d=\"M160 686zl1 1-1 1z\"/>"
"<path id=\"c\" d=\"m79 120 3-1q2 1-1 2z\"/>"
"<path id=\"d\" d=\"m185 752 3-1 3 1-3 1z\"/>"
"<path id=\"e\" d=\"m185 749 1-2 1 2-1 3z\"/>"
"<path id=\"f\" d=\"m253 284 6-1 6 1-6 1z\"/>"
"<path id=\"g\" d=\"M253 199v-1h12v2h-12z\"/>"
"<path id=\"h\" d=\"M249 273v-3h16v7h-16z\"/>"
"<path id=\"i\" d=\"M249 164v-3h16v7h-16z\"/>"
"<path id=\"j\" d=\"M192 189h11v11h-11z\"/>"
"<path id=\"k\" d=\"M128 551v-2q-2-4 7-3h7v5h-14\"/>"
"<path id=\"l\" d=\"M128 539v-2h18v-5h-18v-3l9-1h9v-5h-18v-4h18v-5h-18v-4h18v-5h-18v-4h18v-5h-9c-10 0-9 0-9-3v-1l9-1h9v-4l-9-1h-9v-3h8l11-1 5 1h2v58h-26z\"/>"
"<path id=\"m\" d=\"M128 475v-2l7-1h7v6h-14z\"/>"
"<path id=\"n\" d=\"M173 471h5v17h-5z\"/>"
"</defs></svg>";

#ifdef  __cplusplus
}
#endif

#endif // GSP_WS_HTML_H