export class BuildController {
  static logOffset = 0;
  static pollTimer = null;
  static isPolling = false;
  static currentStatus = null;
  static init() {
    BuildController.bindEvents();
    setTimeout(() => BuildController.fetchStatus(), 600);
  }
  static onTabActivated() {
    BuildController.fetchStatus();
  }
  static bindEvents() {
    document.getElementById('btn-refresh-ports')?.addEventListener('click', async () => {
      await BuildController.refreshPorts();
      GUIController.showToast('Đã làm mới danh sách cổng COM', 'info');
    });
    document.getElementById('btn-quick-set-target')?.addEventListener('click', async () => {
      await BuildController.triggerAction('set-target', { target: 'esp32s3' });
    });
    document.getElementById('btn-idf-build')?.addEventListener('click', async () => {
      await BuildController.triggerAction('build');
    });
    document.getElementById('btn-idf-flash')?.addEventListener('click', async () => {
      const port = document.getElementById('serial-port-select')?.value;
      const baud = document.getElementById('serial-baud-select')?.value || '460800';
      if (!port) {
        GUIController.showToast('Vui lòng chọn cổng COM trước khi nạp chip!', 'warning');
        return;
      }
      await BuildController.triggerAction('flash', { port, baud });
    });
    document.getElementById('btn-idf-build-flash')?.addEventListener('click', async () => {
      const port = document.getElementById('serial-port-select')?.value;
      const baud = document.getElementById('serial-baud-select')?.value || '460800';
      if (!port) {
        GUIController.showToast('Vui lòng chọn cổng COM trước khi nạp chip!', 'warning');
        return;
      }
      await BuildController.triggerAction('build-flash', { port, baud });
    });
    document.getElementById('btn-idf-clean')?.addEventListener('click', async () => {
      if (confirm('Bạn có chắc chắn muốn dọn dẹp thư mục build (clean)?')) {
        await BuildController.triggerAction('clean');
      }
    });
    document.getElementById('btn-idf-cancel')?.addEventListener('click', async () => {
      try {
        const res = await window.apiFetch('/api/idf/cancel', { method: 'POST' });
        const data = await res.json();
        GUIController.showToast(data.message || 'Đã gửi lệnh dừng tiến trình', 'warning');
      } catch (e) {
        GUIController.showToast('Lỗi gửi lệnh dừng: ' + e.message, 'error');
      }
    });
    document.getElementById('btn-copy-logs')?.addEventListener('click', () => {
      const term = document.getElementById('terminal-body');
      if (term) {
        navigator.clipboard.writeText(term.textContent).then(() => {
          GUIController.showToast('Đã sao chép nhật ký vào bộ nhớ tạm', 'success');
        }).catch(() => {
          GUIController.showToast('Không thể sao chép nhật ký', 'error');
        });
      }
    });
    document.getElementById('btn-clear-logs')?.addEventListener('click', async () => {
      const term = document.getElementById('terminal-body');
      if (term) {
        term.innerHTML = '<span class="term-dim">=== Nhật ký đã được xóa ===</span>\n';
      }
      BuildController.logOffset = 0;
      try {
        await window.apiFetch('/api/idf/clear-logs', { method: 'POST' });
      } catch (e) { }
    });
    document.getElementById('btn-toggle-idf-custom')?.addEventListener('click', () => {
      const panel = document.getElementById('idf-custom-panel');
      if (panel) {
        const isHidden = panel.style.display === 'none' || !panel.style.display;
        panel.style.display = isHidden ? 'block' : 'none';
      }
    });
    document.getElementById('btn-apply-idf-path')?.addEventListener('click', async () => {
      const input = document.getElementById('custom-idf-input');
      const val = input?.value?.trim();
      if (!val) {
        GUIController.showToast('Vui lòng nhập đường dẫn thư mục ESP-IDF!', 'warning');
        return;
      }
      await BuildController.applyCustomPath(val);
    });
    document.getElementById('btn-scan-all-idf')?.addEventListener('click', async () => {
      await BuildController.scanAllIdf();
    });
    document.getElementById('btn-reset-idf-path')?.addEventListener('click', async () => {
      await BuildController.resetCustomPath();
    });
  }
  static async applyCustomPath(customPath) {
    try {
      const res = await window.apiFetch('/api/idf/set-path', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ path: customPath })
      });
      const data = await res.json();
      if (data.success) {
        GUIController.showToast(data.message || 'Đã áp dụng đường dẫn ESP-IDF', 'success');
        await BuildController.fetchStatus();
      } else {
        GUIController.showToast(data.error || 'Đường dẫn không hợp lệ', 'error');
      }
    } catch (e) {
      GUIController.showToast('Lỗi gửi yêu cầu: ' + e.message, 'error');
    }
  }
  static async resetCustomPath() {
    try {
      const res = await window.apiFetch('/api/idf/reset-path', { method: 'POST' });
      const data = await res.json();
      if (data.success) {
        GUIController.showToast(data.message || 'Đã khôi phục chế độ tự động', 'info');
        const input = document.getElementById('custom-idf-input');
        if (input) input.value = '';
        await BuildController.fetchStatus();
      }
    } catch (e) {
      GUIController.showToast('Lỗi khôi phục mặc định: ' + e.message, 'error');
    }
  }
  static async scanAllIdf() {
    const loading = document.getElementById('idf-scan-loading');
    const container = document.getElementById('idf-scan-results');
    if (loading) loading.style.display = 'block';
    if (container) {
      container.style.display = 'none';
      container.innerHTML = '';
    }
    try {
      const res = await window.apiFetch('/api/idf/scan-all', { cache: 'no-store' });
      const data = await res.json();
      if (loading) loading.style.display = 'none';
      if (container && data.success && Array.isArray(data.installations)) {
        container.style.display = 'flex';
        if (data.installations.length === 0) {
          container.innerHTML = '<span style="font-size: 0.74rem; color: var(--text-muted); padding: 4px;">Không tìm thấy bản cài đặt ESP-IDF nào tự động trên máy tính. Hãy nhập đường dẫn thủ công phía trên.</span>';
          return;
        }
        data.installations.forEach(item => {
          const row = document.createElement('div');
          row.className = 'idf-scan-item';
          row.innerHTML = `
            <div class="idf-scan-item-info">
              <span class="idf-scan-path" title="${item.path}">${item.path}</span>
              <div class="idf-scan-meta">
                <span class="text-cyan font-bold">${item.version}</span>
                <span class="idf-scan-source-pill">${item.source}</span>
              </div>
            </div>
            <button type="button" class="btn btn-outline btn-xs btn-pick-idf" title="Chọn đường dẫn này làm ESP-IDF hoạt động">
              Chọn
            </button>
          `;
          row.querySelector('.btn-pick-idf')?.addEventListener('click', async () => {
            const input = document.getElementById('custom-idf-input');
            if (input) input.value = item.path;
            await BuildController.applyCustomPath(item.path);
          });
          container.appendChild(row);
        });
        GUIController.showToast(`Tìm thấy ${data.installations.length} phiên bản ESP-IDF`, 'success');
      }
    } catch (e) {
      if (loading) loading.style.display = 'none';
      GUIController.showToast('Lỗi khi dò tìm ESP-IDF: ' + e.message, 'error');
    }
  }
  static async fetchStatus() {
    try {
      const res = await window.apiFetch('/api/idf/status', { cache: 'no-store' });
      if (!res.ok) return;
      const data = await res.json();
      BuildController.currentStatus = data;
      BuildController.renderStatus(data);
    } catch (e) {
      console.log('Không thể lấy trạng thái IDF (chế độ offline):', e.message);
    }
  }
  static renderStatus(data) {
    const { idf, target, ports, job } = data;
    const idfBadge = document.getElementById('idf-status-badge');
    const idfVer = document.getElementById('idf-version-val');
    const idfPath = document.getElementById('idf-path-val');
    const idfPy = document.getElementById('idf-python-val');
    const idfCustomTag = document.getElementById('idf-custom-tag');
    const btnResetIdf = document.getElementById('btn-reset-idf-path');
    const customIdfInput = document.getElementById('custom-idf-input');
    if (idf && idf.installed) {
      if (idfBadge) {
        idfBadge.textContent = idf.is_custom ? 'Đã cài đặt (Tùy chỉnh)' : 'Đã cài đặt';
        idfBadge.className = 'status-badge success';
      }
      if (idfVer) idfVer.textContent = idf.version || 'v6.1';
      if (idfPath) idfPath.textContent = idf.idf_path || '--';
      if (idfPy) idfPy.textContent = idf.python_venv ? 'Python 3.11 (ESP-IDF venv)' : 'Python hệ thống';
      if (idfCustomTag) idfCustomTag.style.display = idf.is_custom ? 'inline-block' : 'none';
      if (btnResetIdf) btnResetIdf.style.display = idf.is_custom ? 'inline-flex' : 'none';
      if (customIdfInput && !customIdfInput.value && idf.idf_path) {
        customIdfInput.value = idf.idf_path;
      }
    } else {
      if (idfBadge) {
        idfBadge.textContent = 'Chưa cài đặt';
        idfBadge.className = 'status-badge danger';
      }
      if (idfVer) idfVer.textContent = 'Không tìm thấy ESP-IDF';
      if (idfPath) idfPath.textContent = 'Nhấn "Tùy chỉnh / Dò tìm IDF" để chỉ định thư mục';
      if (idfPy) idfPy.textContent = '--';
      if (idfCustomTag) idfCustomTag.style.display = 'none';
      if (btnResetIdf) btnResetIdf.style.display = 'none';
    }
    const targetBadge = document.getElementById('target-status-badge');
    const targetVal = document.getElementById('target-name-val');
    const btnSetTarget = document.getElementById('btn-quick-set-target');
    if (target && target.is_set && target.target === 'esp32s3') {
      if (targetBadge) {
        targetBadge.textContent = 'Đã thiết lập';
        targetBadge.className = 'status-badge success';
      }
      if (targetVal) targetVal.textContent = 'esp32s3 (Đã sẵn sàng build)';
      if (btnSetTarget) btnSetTarget.style.display = 'inline-flex';
    } else if (target && target.is_set) {
      if (targetBadge) {
        targetBadge.textContent = `Target: ${target.target}`;
        targetBadge.className = 'status-badge warning';
      }
      if (targetVal) targetVal.textContent = `${target.target} (Cần đổi sang esp32s3)`;
      if (btnSetTarget) btnSetTarget.style.display = 'inline-flex';
    } else {
      if (targetBadge) {
        targetBadge.textContent = 'Chưa set-target';
        targetBadge.className = 'status-badge warning';
      }
      if (targetVal) targetVal.textContent = 'Chưa xác định target';
      if (btnSetTarget) btnSetTarget.style.display = 'inline-flex';
    }
    BuildController.renderPortOptions(ports);
    BuildController.syncJobState(job);
  }
  static renderPortOptions(ports) {
    const sel = document.getElementById('serial-port-select');
    if (!sel) return;
    const currentVal = sel.value;
    sel.innerHTML = '';
    if (!ports || ports.length === 0) {
      const opt = document.createElement('option');
      opt.value = '';
      opt.textContent = '-- Không tìm thấy cổng COM nào --';
      sel.appendChild(opt);
      return;
    }
    let hasSelection = false;
    ports.forEach(p => {
      const opt = document.createElement('option');
      opt.value = p.port;
      opt.textContent = `${p.port} - ${p.desc || 'Thiết bị nối tiếp'}`;
      if (currentVal && p.port === currentVal) {
        opt.selected = true;
        hasSelection = true;
      } else if (!hasSelection && /CH343|CH340|CP210|ESP|USB-Enhanced/i.test(p.desc)) {
        opt.selected = true;
        hasSelection = true;
      }
      sel.appendChild(opt);
    });
    if (!hasSelection && ports.length > 0) {
      const nonCom1 = ports.find(p => p.port !== 'COM1');
      if (nonCom1) {
        sel.value = nonCom1.port;
      } else {
        sel.selectedIndex = 0;
      }
    }
  }
  static async refreshPorts() {
    try {
      const res = await window.apiFetch('/api/idf/ports', { cache: 'no-store' });
      if (!res.ok) return;
      const data = await res.json();
      BuildController.renderPortOptions(data.ports);
    } catch (e) {
      console.log('Lỗi refresh ports:', e.message);
    }
  }
  static async triggerAction(action, payload = {}) {
    const isRunning = BuildController.currentStatus?.job?.state === 'running';
    if (isRunning) {
      GUIController.showToast('Đang có tiến trình khác đang chạy!', 'warning');
      return;
    }
    const activePanel = document.querySelector('.tab-panel.active')?.id;
    if (activePanel && activePanel !== 'panel-build' && GUIController.dirty[activePanel]) {
      await GUIController.saveTab(activePanel);
    }
    GUIController.switchTab('panel-build');
    const term = document.getElementById('terminal-body');
    if (term) {
      term.innerHTML += `\n<span class="term-cyan">=== YÊU CẦU: ${action.toUpperCase()} ===</span>\n`;
    }
    BuildController.logOffset = 0;
    try {
      const res = await window.apiFetch(`/api/idf/${action}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
      const data = await res.json();
      if (!data.success) {
        GUIController.showToast(data.error || 'Lỗi khởi động tác vụ', 'error');
        if (term) term.innerHTML += `<span class="term-error">[LỖI] ${data.error}</span>\n`;
        return;
      }
      GUIController.showToast(data.message || `Đang chạy ${action}...`, 'info');
      BuildController.startLogPolling();
    } catch (e) {
      GUIController.showToast('Lỗi kết nối server: ' + e.message, 'error');
    }
  }
  static syncJobState(job) {
    const isRunning = job && job.state === 'running';
    const state = job ? job.state : 'idle';
    const btnBuild = document.getElementById('btn-idf-build');
    const btnFlash = document.getElementById('btn-idf-flash');
    const btnBuildFlash = document.getElementById('btn-idf-build-flash');
    const btnClean = document.getElementById('btn-idf-clean');
    const btnCancel = document.getElementById('btn-idf-cancel');
    if (btnBuild) btnBuild.disabled = isRunning;
    if (btnFlash) btnFlash.disabled = isRunning;
    if (btnBuildFlash) btnBuildFlash.disabled = isRunning;
    if (btnClean) btnClean.disabled = isRunning;
    if (btnCancel) btnCancel.disabled = !isRunning;
    const stateTag = document.getElementById('terminal-state-tag');
    const liveDot = document.getElementById('terminal-live-dot');
    const navDot = document.getElementById('nav-build-dot');
    const overallStatus = document.getElementById('build-overall-status');
    const statusLine = document.getElementById('term-status-line');
    if (stateTag) {
      stateTag.textContent = state.toUpperCase();
      stateTag.style.color = isRunning ? '#fbbf24' : (state === 'success' ? '#34d399' : (state === 'failed' ? '#f87171' : '#38bdf8'));
    }
    if (liveDot) {
      liveDot.className = `terminal-dot ${isRunning ? 'running' : (state === 'failed' ? 'error' : '')}`;
    }
    if (navDot) {
      navDot.className = `build-status-dot ${isRunning ? 'running' : (state === 'success' ? 'success' : (state === 'failed' ? 'error' : ''))}`;
    }
    if (overallStatus) {
      if (isRunning) {
        overallStatus.textContent = `Đang chạy (${job.action || 'IDF'})...`;
        overallStatus.className = 'tab-status-pill unsaved';
      } else if (state === 'success') {
        overallStatus.textContent = 'Hoàn tất thành công';
        overallStatus.className = 'tab-status-pill saved';
      } else if (state === 'failed') {
        overallStatus.textContent = 'Thất bại';
        overallStatus.className = 'tab-status-pill unsaved';
      } else {
        overallStatus.textContent = 'Sẵn sàng';
        overallStatus.className = 'tab-status-pill saved';
      }
    }
    if (statusLine) {
      if (isRunning) {
        statusLine.textContent = `Đang chạy tác vụ: ${job.action || 'idf.py'}...`;
      } else if (state === 'success') {
        statusLine.textContent = 'Tiến trình hoàn tất thành công (Exit code: 0)';
      } else if (state === 'failed') {
        statusLine.textContent = `Tiến trình kết thúc với lỗi (Mã thoát: ${job.exit_code})`;
      } else if (state === 'cancelled') {
        statusLine.textContent = 'Tiến trình đã bị người dùng hủy';
      } else {
        statusLine.textContent = 'Trạng thái: Sẵn sàng';
      }
    }
    if (isRunning && !BuildController.isPolling) {
      BuildController.startLogPolling();
    }
  }
  static startLogPolling() {
    if (BuildController.isPolling) return;
    BuildController.isPolling = true;
    if (BuildController.pollTimer) clearInterval(BuildController.pollTimer);
    BuildController.pollTimer = setInterval(async () => {
      try {
        const res = await window.apiFetch(`/api/idf/logs?offset=${BuildController.logOffset}`, { cache: 'no-store' });
        if (!res.ok) return;
        const data = await res.json();
        if (data.logs && data.logs.length > 0) {
          BuildController.appendLogs(data.logs);
          BuildController.logOffset = data.next_offset;
        }
        BuildController.syncJobState({
          state: data.state,
          action: data.action,
          exit_code: data.exit_code
        });
        if (data.state !== 'running') {
          clearInterval(BuildController.pollTimer);
          BuildController.pollTimer = null;
          BuildController.isPolling = false;
          if (data.state === 'success') {
            GUIController.showToast('Tiến trình hoàn tất thành công!', 'success');
          } else if (data.state === 'failed') {
            GUIController.showToast(`Tiến trình thất bại (Mã: ${data.exit_code})`, 'error');
          }
        }
      } catch (e) {
        console.log('Lỗi poll logs:', e.message);
      }
    }, 600);
  }
  static appendLogs(lines) {
    const term = document.getElementById('terminal-body');
    if (!term) return;
    const frag = document.createDocumentFragment();
    lines.forEach(rawLine => {
      const line = rawLine;
      const span = document.createElement('span');
      if (/^===/.test(line)) {
        span.className = 'term-cyan';
      } else if (/error:|fatal:|\[LỖI\]|FAILED|Ninja build failed/i.test(line)) {
        span.className = 'term-error';
      } else if (/warning:/i.test(line)) {
        span.className = 'term-warn';
      } else if (/THÀNH CÔNG|Successfully|Hash of data verified|Done/i.test(line)) {
        span.className = 'term-success';
      } else if (/^-- /.test(line) || /^Executing/i.test(line)) {
        span.className = 'term-dim';
      }
      span.textContent = line + '\n';
      frag.appendChild(span);
    });
    term.appendChild(frag);
    const autoScroll = document.getElementById('terminal-autoscroll')?.checked;
    if (autoScroll) {
      term.scrollTop = term.scrollHeight;
    }
  }
}
