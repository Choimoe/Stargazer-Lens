// ==UserScript==
// @name         算番器
// @namespace    http://tampermonkey.net/
// @version      12.0
// @description  通过悬浮窗UI优雅地展示C++算番结果，支持拖拽。
// @author       Choimoe
// @match        none
// @connect      localhost
// @grant        GM_addStyle
// @grant        GM_xmlhttpRequest
// @run-at       document-start
// ==/UserScript==

(function() {
    'use strict';

    const CPP_SERVER_URL = 'http://localhost:17711/calculate';

    function createResultPanel() {
        GM_addStyle(`
            #fan-calculator-panel {
                position: fixed;
                top: 50px;
                right: 20px;
                width: 380px;
                max-width: 90vw;
                background-color: #2c3e50;
                color: #ecf0f1;
                border-radius: 12px;
                box-shadow: 0 10px 30px rgba(0, 0, 0, 0.4);
                z-index: 9999;
                font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
                display: none; /* Initially hidden */
                flex-direction: column;
                overflow: hidden;
                transition: opacity 0.3s ease, transform 0.3s ease;
                opacity: 0;
                transform: scale(0.95);
            }
            #fan-calculator-panel.visible {
                display: flex;
                opacity: 1;
                transform: scale(1);
            }
            #fan-panel-header {
                padding: 10px 15px;
                background-color: #34495e;
                cursor: move;
                user-select: none;
                display: flex;
                justify-content: space-between;
                align-items: center;
                border-bottom: 1px solid #4a627a;
            }
            #fan-panel-header h3 {
                margin: 0;
                font-size: 16px;
                font-weight: 600;
                color: #ecf0f1;
            }
            #fan-panel-close {
                cursor: pointer;
                font-size: 24px;
                font-weight: bold;
                color: #95a5a6;
                transition: color 0.2s;
            }
            #fan-panel-close:hover {
                color: #ecf0f1;
            }
            #fan-panel-content {
                padding: 15px;
                max-height: 70vh;
                overflow-y: auto;
            }
            .fan-section {
                margin-bottom: 15px;
            }
            .fan-section h4 {
                margin: 0 0 8px 0;
                font-size: 14px;
                color: #95a5a6;
                text-transform: uppercase;
                border-bottom: 1px solid #34495e;
                padding-bottom: 5px;
            }
            #fan-total-score {
                font-size: 48px;
                font-weight: bold;
                color: #1abc9c;
                text-align: center;
            }
            #fan-ting-list {
                font-size: 24px;
                text-align: center;
                word-spacing: 10px;
            }
            #fan-details-table {
                width: 100%;
                border-collapse: collapse;
                font-size: 14px;
            }
            #fan-details-table th, #fan-details-table td {
                padding: 8px 5px;
                text-align: left;
                border-bottom: 1px solid #34495e;
            }
            #fan-details-table th {
                font-weight: 600;
                color: #bdc3c7;
            }
            #fan-details-table td:nth-child(2) {
                text-align: right;
                font-weight: bold;
                color: #e67e22;
            }
            #fan-details-table td .fan-packs {
                 font-family: monospace, sans-serif;
                 font-size: 13px;
                 color: #95a5a6;
            }
            #fan-panel-footer {
                padding: 8px 15px;
                background-color: #233140;
                font-size: 12px;
                color: #7f8c8d;
                text-align: center;
                font-family: monospace;
            }
        `);

        const panel = document.createElement('div');
        panel.id = 'fan-calculator-panel';
        panel.innerHTML = `
            <div id="fan-panel-header">
                <h3>算番结果</h3>
                <span id="fan-panel-close">&times;</span>
            </div>
            <div id="fan-panel-content">
                <div class="fan-section">
                    <h4>总番数</h4>
                    <div id="fan-total-score"></div>
                </div>
                <div class="fan-section">
                    <h4>听牌</h4>
                    <div id="fan-ting-list"></div>
                </div>
                <div class="fan-section">
                    <h4>番种详情</h4>
                    <table id="fan-details-table">
                        <thead><tr><th>番种</th><th>番数</th><th>组成</th></tr></thead>
                        <tbody></tbody>
                    </table>
                </div>
            </div>
            <div id="fan-panel-footer"></div>
        `;
        document.body.appendChild(panel);

        const header = panel.querySelector('#fan-panel-header');
        const closeButton = panel.querySelector('#fan-panel-close');

        closeButton.onclick = () => panel.classList.remove('visible');

        let isDragging = false;
        let offset = { x: 0, y: 0 };

        header.onmousedown = (e) => {
            isDragging = true;
            offset.x = e.clientX - panel.offsetLeft;
            offset.y = e.clientY - panel.offsetTop;
            panel.style.transition = 'none';
        };

        document.onmousemove = (e) => {
            if (!isDragging) return;
            panel.style.left = `${e.clientX - offset.x}px`;
            panel.style.top = `${e.clientY - offset.y}px`;
        };

        document.onmouseup = () => {
            isDragging = false;
            panel.style.transition = 'opacity 0.3s ease, transform 0.3s ease';
        };
    }

    function updateResultPanel(result) {
        const panel = document.getElementById('fan-calculator-panel');
        if (!panel) return;

        document.getElementById('fan-total-score').innerText = result.total_fan || '0';
        document.getElementById('fan-ting-list').innerHTML = result.ting ? result.ting.join(' ') : '<em>无 (None)</em>';
        document.getElementById('fan-panel-footer').innerText = result.parsed_hand || '...';

        const tableBody = document.querySelector('#fan-details-table tbody');
        tableBody.innerHTML = '';
        if (result.fan_details && result.fan_details.length > 0) {
            result.fan_details.forEach(fan => {
                const row = tableBody.insertRow();
                row.innerHTML = `
                    <td>${fan.name}</td>
                    <td>${fan.score}</td>
                    <td><span class="fan-packs">${fan.packs}</span></td>
                `;
            });
        } else {
            const row = tableBody.insertRow();
            row.innerHTML = `<td colspan="3" style="text-align:center;color:#7f8c8d;">无 (None)</td>`;
        }

        panel.classList.add('visible');
    }

    function forwardToServer(rawJsonString) {
        GM_xmlhttpRequest({
            method: 'POST',
            url: CPP_SERVER_URL,
            data: rawJsonString,
            headers: { 'Content-Type': 'application/json; charset=UTF-8' },
            onload: function(response) {
                try {
                    const result = JSON.parse(response.responseText);
                    if (result.status === 'success') {
                        updateResultPanel(result);
                    } else {
                        console.error('[C++服务器错误]', result.message);
                    }
                } catch (e) {
                    console.error('[脚本] 解析C++服务器响应失败:', e, response.responseText);
                }
            },
            onerror: function(response) {
                console.error('[脚本] 无法连接到本地C++服务器。', response);
            }
        });
    }

    const codeToInject = `
        (function() {
            if (window.isWsInterceptorInjected) return;
            window.isWsInterceptorInjected = true;
            const OriginalWebSocket = window.WebSocket;
            window.WebSocket = function(url, protocols) {
                const socket = protocols ? new OriginalWebSocket(url, protocols) : new OriginalWebSocket(url);
                socket.addEventListener('message', (event) => {
                    try {
                        const data = JSON.parse(event.data);
                        if (data && data.q) {
                            window.dispatchEvent(new CustomEvent('ForwardDataToSandbox', { detail: event.data }));
                        }
                    } catch(e) {}
                });
                return socket;
            };
            window.WebSocket.prototype = OriginalWebSocket.prototype;
        })();
    `;

    createResultPanel();
    const script = document.createElement('script');
    script.textContent = codeToInject;
    (document.head || document.documentElement).appendChild(script);
    script.remove();

    window.addEventListener('ForwardDataToSandbox', (event) => {
        forwardToServer(event.detail);
    });

})();
