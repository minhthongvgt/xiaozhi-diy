import { ESP32S3 } from './hardware.js';
import { DEFAULT_CONFIG } from './configState.js';
import { SdkconfigParser } from './parser.js';
import { SdkconfigGenerator } from './generator.js';
import { PinValidator } from './pinValidator.js';
import { BuildController } from './buildController.js';
export class GUIController {
  static snapshots   = {};   
  static dirty       = {};   
  static pendingTab  = null;
  static TAB_NAMES = {
    'panel-assistant':     'Cấu hình Trợ lý chung',
    'panel-system':        'Hệ thống & Flash',
    'panel-peripherals':   'Ngoại vi tổng hợp',
    'panel-general':       'Cơ bản & Ngôn ngữ',
    'panel-audio':         'Âm thanh (Loa & Micro)',
    'panel-display':       'Màn hình & Cảm ứng',
    'panel-wakeword':      'Từ khóa & Giọng nói',
    'panel-network':       'Cấp mạng Wi-Fi',
    'panel-mcp':           'MCP & AI',
    'panel-buttons':       'Phím bấm & Input',
    'panel-camera':        'Camera',
    'panel-led':           'LED Trạng thái',
    'panel-uart':          'UART mở rộng',
    'panel-actuators':     'Servo / Buzzer / Haptic',
    'panel-motor':         'Relay & Motor DC',
    'panel-sensors':       'Cảm biến',
    'panel-power':         'Pin & Nguồn',
    'panel-flash':         'Flash & ESP32-S3',
    'panel-build':         'Biên dịch & Nạp',
  };
  static async init() {
    GUIController.bindNav();
    GUIController.bindInputs();
    GUIController.bindSaveBtns();
    GUIController.bindUnsavedModal();
    GUIController.bindOpenDir();
    BuildController.init();
    GUIController.syncStateToUI();
    GUIController.renderPinMatrix();
    GUIController.initSnapshots();
    await GUIController.autoConnect();
  }
  static async autoConnect() {
    try {
      const res = await window.apiFetch('/api/project', { cache: 'no-store' });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const info = await res.json();
      if (!info.success) throw new Error(info.error || 'Server không xác nhận');
      window.isServerMode = true;
      window.serverProjectInfo = info;
      GUIController._setStatus('connected', `Dự án: ${info.root_path}`);
      const cfgRes = await window.apiFetch('/api/config', { cache: 'no-store' });
      if (cfgRes.ok) {
        const cfgData = await cfgRes.json();
        if (cfgData.content && cfgData.content.trim()) {
          window.configState = SdkconfigParser.parse(cfgData.content);
          GUIController.syncStateToUI();
          GUIController.initSnapshots();
          GUIController.updateValidation();
          GUIController.showToast(`Đã nạp cấu hình từ sdkconfig.defaults`, 'success');
        }
      }
    } catch (e) {
      window.isServerMode = false;
      console.warn('Không thể kết nối Configurator Server:', e.message);
      if (typeof window !== 'undefined' && window.location.protocol === 'file:') {
        GUIController._setStatus('error', 'Chưa chạy run_configurator.bat');
      } else {
        GUIController._setStatus('error', 'Chưa kết nối Server');
      }
    }
  }
  static snapshot(panelId) {
    const p = document.getElementById(panelId);
    if (!p) return '';
    const parts = [];
    p.querySelectorAll('input, select, textarea').forEach(el => {
      if (el.type === 'checkbox') parts.push(`${el.id}:${el.checked}`);
      else if (el.type === 'radio') { if (el.checked) parts.push(`${el.name}:${el.value}`); }
      else parts.push(`${el.id}:${el.value}`);
    });
    return parts.join('|');
  }
  static initSnapshots() {
    document.querySelectorAll('.tab-panel').forEach(p => {
      GUIController.snapshots[p.id] = GUIController.snapshot(p.id);
      GUIController.dirty[p.id]     = false;
      GUIController._updateTabBadge(p.id);
    });
  }
  static checkDirty(panelId) {
    GUIController.syncUIToState();
    const now   = GUIController.snapshot(panelId);
    const isDirty = now !== (GUIController.snapshots[panelId] || '');
    GUIController.dirty[panelId] = isDirty;
    GUIController._updateTabBadge(panelId);
    document.querySelectorAll('.dynamic-list').forEach(list => {
      const hidden = list.querySelector('input[type="hidden"]');
      const listItems = list.querySelector('.list-items');
      if (hidden && listItems) {
        listItems.innerHTML = '';
        const vals = hidden.value.split(',').filter(x => x.trim() !== '');
        vals.forEach(val => {
            let gpioPart = val;
            let labelPart = '';
            if (val.includes('|')) {
                const s = val.split('|');
                gpioPart = s[0];
                labelPart = s.length > 1 ? s[1] : '';
            }
            const div = document.createElement('div');
            if (gpioPart.includes(':')) {
                const parts = gpioPart.split(':');
                div.className = 'dynamic-pair-row';
                div.style = "display:flex; gap:5px; margin-bottom:5px;";
                div.innerHTML = `<input type="number" class="dynamic-input form-control" value="${parts[0]}" style="flex:1;" oninput="updateHiddenExtrasPair(this)">
                                 <input type="number" class="dynamic-input form-control" value="${parts[1]}" style="flex:1;" oninput="updateHiddenExtrasPair(this)">
                                 <input type="text" class="dynamic-input form-control" value="${labelPart}" placeholder="Nhãn (Tùy chọn)" style="flex:2;" oninput="updateHiddenExtrasPair(this)">
                                 <button type="button" class="btn btn-sm" style="background:#ff4d4f; color:white; border:none; padding:0 8px; border-radius:4px; cursor:pointer;" onclick="var p=this.parentElement; p.parentElement.removeChild(p); updateHiddenExtrasPair(p.previousElementSibling || list.querySelector('.btn-add'))">X</button>`;
            } else {
                div.style = "display:flex; gap:5px; margin-bottom:5px;";
                div.innerHTML = `<input type="number" class="dynamic-input form-control" value="${gpioPart}" style="flex:1;" oninput="updateHiddenExtras(this)">
                                 <input type="text" class="dynamic-input form-control" value="${labelPart}" placeholder="Nhãn (Tùy chọn)" style="flex:2;" oninput="updateHiddenExtras(this)">
                                 <button type="button" class="btn btn-sm" style="background:#ff4d4f; color:white; border:none; padding:0 8px; border-radius:4px; cursor:pointer;" onclick="var p=this.parentElement; p.parentElement.removeChild(p); updateHiddenExtras(p.previousElementSibling || list.querySelector('.btn-add'))">X</button>`;
            }
            listItems.appendChild(div);
        });
      }
    });
    GUIController.updateValidation();
  }
  static _updateTabBadge(panelId) {
    const isDirty = !!GUIController.dirty[panelId];
    document.querySelector(`.nav-link[data-target="${panelId}"]`)
      ?.classList.toggle('has-unsaved', isDirty);
    const pill = document.querySelector(`.tab-status-pill[data-tab-status="${panelId}"]`);
    if (pill) {
      pill.className = `tab-status-pill ${isDirty ? 'unsaved' : 'saved'}`;
      pill.textContent = isDirty ? 'Chưa lưu' : 'Đã lưu';
    }
    document.querySelector(`.btn-save-tab[data-tab="${panelId}"]`)
      ?.classList.toggle('btn-highlight-save', isDirty);
  }
  static switchTab(targetId) {
    document.querySelectorAll('.nav-link[data-target]').forEach(b =>
      b.classList.toggle('active', b.dataset.target === targetId));
    document.querySelectorAll('.tab-panel').forEach(p =>
      p.classList.toggle('active', p.id === targetId));
    if (targetId === 'panel-build') {
      BuildController.onTabActivated();
    }
  }
  static bindNav() {
    document.querySelectorAll('.nav-link[data-target]').forEach(btn => {
      btn.addEventListener('click', e => {
        const target  = btn.dataset.target;
        const active  = document.querySelector('.tab-panel.active');
        const current = active?.id;
        if (current === target) return;
        if (current && GUIController.dirty[current]) {
          e.preventDefault();
          GUIController.pendingTab = target;
          GUIController._openUnsavedModal(current);
          return;
        }
        GUIController.switchTab(target);
      });
    });
  }
  static _openUnsavedModal(panelId) {
    const modal = document.getElementById('unsaved-modal');
    const desc  = document.getElementById('unsaved-modal-desc');
    const name  = GUIController.TAB_NAMES[panelId] || panelId;
    if (desc) desc.innerHTML =
      `Bạn có thay đổi chưa lưu trong mục <strong>"${name}"</strong>.`;
    modal?.classList.add('active');
  }
  static bindUnsavedModal() {
    const close = () => {
      document.getElementById('unsaved-modal')?.classList.remove('active');
      GUIController.pendingTab = null;
    };
    document.getElementById('unsaved-btn-stay')?.addEventListener('click', close);
    document.getElementById('unsaved-btn-close')?.addEventListener('click', close);
    document.getElementById('unsaved-btn-discard')?.addEventListener('click', () => {
      const active = document.querySelector('.tab-panel.active')?.id;
      if (active) {
        GUIController.syncStateToUI();
        GUIController.dirty[active] = false;
        GUIController.snapshots[active] = GUIController.snapshot(active);
        GUIController._updateTabBadge(active);
      }
      const t = GUIController.pendingTab;
      close();
      if (t) GUIController.switchTab(t);
    });
    document.getElementById('unsaved-btn-save')?.addEventListener('click', async () => {
      const active = document.querySelector('.tab-panel.active')?.id;
      if (active) await GUIController.saveTab(active);
      const t = GUIController.pendingTab;
      close();
      if (t) GUIController.switchTab(t);
    });
  }
  static bindInputs() {
    document.querySelectorAll('.tab-panel').forEach(panel => {
      panel.addEventListener('input',  () => GUIController.checkDirty(panel.id));
      panel.addEventListener('change', () => GUIController.checkDirty(panel.id));
    });
    const on = (id, fn) => document.getElementById(id)?.addEventListener('change', fn);
    on('display_enable', () => GUIController._toggleDisplayGroups());
    on('display_type',   () => GUIController._toggleDisplayGroups());
    on('display_uart_secondary', () => GUIController._toggleDisplayGroups());
    on('general_display_style',  () => GUIController._toggleMultilineChat());
    on('touch_enable',   () => GUIController._toggleGroup('.touch-fields', 'touch_enable'));
    on('speaker_enable', () => GUIController._toggleGroup('.spk-fields',   'speaker_enable'));
    on('speaker_type',   () => GUIController._toggleCodecGroup('spk'));
    on('mic_enable',     () => GUIController._toggleGroup('.mic-fields',   'mic_enable'));
    on('mic_type',       () => GUIController._toggleCodecGroup('mic'));
    on('camera_enable',  () => GUIController._toggleCameraUsb());
    on('camera_sensor',  () => GUIController._toggleCameraUsb());
    on('led_enable',     () => GUIController._toggleGroup('.led-fields',   'led_enable'));
    on('led_type',       () => GUIController._toggleLedRainbow());
    on('mcp_enable',     () => GUIController._toggleGroup('.mcp-fields',   'mcp_enable'));
    on('uart_enable',    () => GUIController._toggleGroup('.uart-fields',  'uart_enable'));
    on('uart_flow',      () => GUIController._toggleGroup('.uart-flow-fields','uart_flow'));
    on('buttons_boot',   () => GUIController._toggleGroup('.boot-fields',  'buttons_boot'));
    on('buttons_touch',  () => GUIController._toggleGroup('.touch-btn-fields','buttons_touch'));
    on('buttons_vol',    () => GUIController._toggleGroup('.vol-fields',   'buttons_vol'));
    on('buttons_slider', () => GUIController._toggleGroup('.slider-fields','buttons_slider'));
    on('buttons_rotary', () => GUIController._toggleGroup('.rotary-fields','buttons_rotary'));
    on('servo_enable',   () => GUIController._toggleGroup('.servo-fields', 'servo_enable'));
    on('buzzer_enable',  () => GUIController._toggleGroup('.buzzer-fields','buzzer_enable'));
    on('haptic_enable',  () => GUIController._toggleGroup('.haptic-fields','haptic_enable'));
    on('relay_enable',   () => GUIController._toggleGroup('.relay-fields', 'relay_enable'));
    on('motor_dc_enable',() => GUIController._toggleGroup('.motor-fields', 'motor_dc_enable'));
    on('battery_enable', () => GUIController._toggleGroup('.battery-fields','battery_enable'));
    on('ina2xx_enable',  () => GUIController._toggleGroup('.ina-fields',   'ina2xx_enable'));
    on('tp4056_enable',  () => GUIController._toggleGroup('.tp4056-fields','tp4056_enable'));
    on('flash_size',     () => GUIController._syncFlashPartition());
    on('wakeword_type',  () => GUIController._toggleWakewordCustom());
    on('mcp_lamp',       () => GUIController._toggleGroup('.lamp-gpio-field','mcp_lamp'));
    ['dht','temp','bmp280','bh1750','ldr','gas_co2','gas_mq',
     'hcsr04','pir','vibration','flame'].forEach(sens => {
      on(`sensor_${sens}`, () =>
        GUIController._toggleGroup(`.sensor-${sens}-fields`, `sensor_${sens}`));
    });
    window.addEventListener('beforeunload', e => {
      if (Object.values(GUIController.dirty).some(Boolean)) {
        e.preventDefault(); e.returnValue = '';
      }
    });
  }
  static async saveTab(panelId) {
    GUIController.syncUIToState();
    const { errors } = PinValidator.validate(window.configState);
    if (errors.length) {
      GUIController.showToast('Có xung đột GPIO! Kiểm tra lại.', 'error');
      GUIController.updateValidation();
      return false;
    }
    if (window.isServerMode) {
      try {
        const lines = SdkconfigGenerator.build(window.configState);
        const res = await window.apiFetch('/api/save', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ sdkconfig_lines: lines }),
        });
        const data = await res.json();
        if (!data.success) throw new Error(data.error);
        GUIController.showToast(
          `Đã ghi "${GUIController.TAB_NAMES[panelId]||panelId}" vào sdkconfig.defaults`, 'success');
      } catch (e) {
        GUIController.showToast(`Lỗi khi lưu: ${e.message}`, 'error');
        return false;
      }
    } else {
      GUIController.showToast(
        `Đã lưu "${GUIController.TAB_NAMES[panelId]||panelId}" (chạy run_configurator.bat để ghi file)`,
        'success');
    }
    GUIController.snapshots[panelId] = GUIController.snapshot(panelId);
    GUIController.dirty[panelId]     = false;
    GUIController._updateTabBadge(panelId);
    return true;
  }
  static bindSaveBtns() {
    document.querySelectorAll('.btn-save-tab').forEach(btn => {
      btn.addEventListener('click', async () => {
        const tabId = btn.dataset.tab;
        if (tabId) {
          const ok = await GUIController.saveTab(tabId);
          if (ok) {
            btn.classList.add('btn-saved-success');
            const orig = btn.innerHTML;
            btn.innerHTML = '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="20 6 9 17 4 12"></polyline></svg><span>Đã lưu ✓</span>';
            setTimeout(() => { btn.innerHTML = orig; btn.classList.remove('btn-saved-success'); }, 1500);
          }
        }
      });
    });
    document.getElementById('btn-download-config')?.addEventListener('click', () => {
      GUIController.syncUIToState();
      const lines = SdkconfigGenerator.build(window.configState);
      const blob  = new Blob([lines.join('\n')], { type: 'text/plain' });
      const a     = Object.assign(document.createElement('a'), {
        href: URL.createObjectURL(blob),
        download: 'sdkconfig.defaults',
      });
      a.click();
      GUIController.showToast('Đã tải xuống sdkconfig.defaults', 'success');
    });
  }
  static async openProjectModal() {
    const modal = document.getElementById('project-modal');
    if (!modal) return;
    modal.classList.add('active');
    modal.classList.add('visible');
    const input = document.getElementById('input-project-root');
    if (input && window.serverProjectInfo) {
      input.value = window.serverProjectInfo.root_path || '';
    }
    await GUIController.loadDetectedProjects();
  }
  static closeProjectModal() {
    const modal = document.getElementById('project-modal');
    modal?.classList.remove('active');
    modal?.classList.remove('visible');
  }
  static async loadDetectedProjects() {
    const container = document.getElementById('modal-detected-projects-list');
    if (!container) return;
    container.innerHTML = '<span style="font-size:0.75rem; color:var(--text-muted); padding:4px;">Đang quét dự án Xiaozhi trên máy tính...</span>';
    try {
      const res = await window.apiFetch('/api/project/scan-all', { cache: 'no-store' });
      const data = await res.json();
      if (data.success && Array.isArray(data.projects)) {
        container.innerHTML = '';
        if (data.projects.length === 0) {
          container.innerHTML = '<span style="font-size:0.75rem; color:var(--text-muted); padding:4px;">Không tìm thấy dự án Xiaozhi khác. Hãy nhập đường dẫn thư mục vào ô trên.</span>';
          return;
        }
        data.projects.forEach(item => {
          const row = document.createElement('div');
          row.className = `project-scan-item ${item.is_current ? 'is-current' : ''}`;
          row.innerHTML = `
            <div class="project-scan-item-info">
              <span class="project-scan-path" title="${item.path}">${item.path}</span>
              <div class="project-scan-meta">
                <span class="idf-scan-source-pill">${item.source}</span>
                ${item.is_current ? '<span class="text-emerald font-bold">● Đang hoạt động</span>' : ''}
              </div>
            </div>
            ${!item.is_current ? `
              <button type="button" class="btn btn-outline btn-xs btn-pick-project">
                Chọn
              </button>
            ` : '<span class="text-emerald" style="font-size:0.75rem;">✓ Hiện tại</span>'}
          `;
          row.querySelector('.btn-pick-project')?.addEventListener('click', async () => {
            const input = document.getElementById('input-project-root');
            if (input) input.value = item.path;
            await GUIController.applyProjectRoot(item.path);
          });
          container.appendChild(row);
        });
      }
    } catch (e) {
      container.innerHTML = `
        <div style="font-size:0.75rem; color:var(--accent-rose); padding:8px; background:rgba(244,63,94,0.1); border:1px solid rgba(244,63,94,0.25); border-radius:4px; line-height:1.45;">
          <strong>Chưa kết nối Backend Server (HTTP 8080):</strong><br>
          <span style="color:var(--text-secondary);">Vui lòng khởi động <code>run_configurator.bat</code> từ thư mục dự án để kích hoạt máy chủ cấu hình tại <code>http:
        </div>
      `;
    }
  }
  static async applyProjectRoot(newPath) {
    if (!newPath) {
      GUIController.showToast('Vui lòng nhập đường dẫn thư mục dự án!', 'warning');
      return;
    }
    try {
      const res = await window.apiFetch('/api/project/set-root', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ root_path: newPath })
      });
      const data = await res.json();
      if (data.success) {
        window.serverProjectInfo = data.project_info || { root_path: data.root_path };
        GUIController._setStatus('connected', `Dự án: ${data.root_path}`);
        GUIController.showToast(data.message || 'Đã chuyển sang dự án mới', 'success');
        GUIController.closeProjectModal();
        const cfgRes = await window.apiFetch('/api/config', { cache: 'no-store' });
        if (cfgRes.ok) {
          const cfgData = await cfgRes.json();
          if (cfgData.content) {
            window.configState = SdkconfigParser.parse(cfgData.content);
            GUIController.syncStateToUI();
            GUIController.initSnapshots();
            GUIController.updateValidation();
          }
        }
        BuildController.fetchStatus();
      } else {
        GUIController.showToast(data.error || 'Thư mục không hợp lệ!', 'error');
      }
    } catch (e) {
      GUIController.showToast('Lỗi gửi yêu cầu: ' + e.message, 'error');
    }
  }
  static async resetProjectRoot() {
    try {
      const res = await window.apiFetch('/api/project/reset-root', { method: 'POST' });
      const data = await res.json();
      if (data.success) {
        window.serverProjectInfo = data.project_info || { root_path: data.root_path };
        GUIController._setStatus('connected', `Dự án: ${data.root_path}`);
        GUIController.showToast(data.message || 'Đã khôi phục chế độ tự động', 'info');
        GUIController.closeProjectModal();
        const cfgRes = await window.apiFetch('/api/config', { cache: 'no-store' });
        if (cfgRes.ok) {
          const cfgData = await cfgRes.json();
          if (cfgData.content) {
            window.configState = SdkconfigParser.parse(cfgData.content);
            GUIController.syncStateToUI();
            GUIController.initSnapshots();
            GUIController.updateValidation();
          }
        }
        BuildController.fetchStatus();
      }
    } catch (e) {
      GUIController.showToast('Lỗi: ' + e.message, 'error');
    }
  }
  static bindOpenDir() {
    const triggerModal = async () => {
      if (!window.isServerMode) {
        await GUIController.autoConnect();
      }
      await GUIController.openProjectModal();
    };
    document.getElementById('btn-open-dir')?.addEventListener('click', triggerModal);
    document.getElementById('project-status-bar')?.addEventListener('click', triggerModal);
    document.getElementById('project-modal')?.addEventListener('click', (e) => {
      if (e.target.id === 'project-modal') {
        GUIController.closeProjectModal();
      }
    });
    document.getElementById('btn-close-project-modal')?.addEventListener('click', () => GUIController.closeProjectModal());
    document.getElementById('btn-close-project-modal-footer')?.addEventListener('click', () => GUIController.closeProjectModal());
    document.getElementById('btn-scan-projects')?.addEventListener('click', () => GUIController.loadDetectedProjects());
    document.getElementById('btn-apply-project-root')?.addEventListener('click', () => {
      const val = document.getElementById('input-project-root')?.value?.trim();
      GUIController.applyProjectRoot(val);
    });
    document.getElementById('btn-reset-project-root')?.addEventListener('click', () => GUIController.resetProjectRoot());
  }
  static syncStateToUI() {
    const s = window.configState;
    const v  = (id, val) => { const el = document.getElementById(id); if (el) el.value = val; };
    const c  = (id, val) => { const el = document.getElementById(id); if (el) el.checked = !!val; };
    const sv = (id, val) => { const el = document.getElementById(id); if (el) el.value = String(val); };
    sv('general_language',      s.general.language);
    v('general_ota_url',        s.general.ota_url);
    sv('general_flash_assets',  s.general.flash_assets);
    v('general_custom_assets',  s.general.custom_assets_file);
    sv('general_display_style', s.general.display_style);
    c('general_multiline',      s.general.multiline_chat);
    sv('sys_cpu_freq',        s.system.cpu_freq);
    sv('sys_flash_size',      s.system.flash_size);
    sv('sys_flash_mode',      s.system.flash_mode);
    sv('sys_flash_freq',      s.system.flash_freq);
    c('sys_spiram',           s.system.spiram);
    sv('sys_spiram_mode',     s.system.spiram_mode);
    sv('sys_spiram_speed',    s.system.spiram_speed);
    sv('sys_partition_table', s.system.partition_table);
    c('display_enable',     s.display.enable);
    sv('display_type',      s.display.type);
    v('display_width',      s.display.width);
    v('display_height',     s.display.height);
    v('display_mosi',       s.display.pin_mosi);
    v('display_clk',        s.display.pin_clk);
    v('display_cs',         s.display.pin_cs);
    v('display_dc',         s.display.pin_dc);
    c('display_use_rst',    s.display.use_rst);
    v('display_rst',        s.display.pin_rst);
    c('display_use_blk',    s.display.use_blk);
    v('display_blk',        s.display.pin_blk);
    v('display_i2c_sda',    s.display.pin_i2c_sda);
    v('display_i2c_scl',    s.display.pin_i2c_scl);
    v('display_offset_x',   s.display.offset_x);
    v('display_offset_y',   s.display.offset_y);
    c('display_mirror_x',   s.display.mirror_x);
    c('display_mirror_y',   s.display.mirror_y);
    c('display_swap_xy',    s.display.swap_xy);
    c('display_invert',     s.display.invert_color);
    sv('display_amoled_chip', s.display.amoled_chip);
    v('display_qspi_cs',    s.display.qspi_cs);
    v('display_qspi_clk',   s.display.qspi_clk);
    v('display_qspi_d0',    s.display.qspi_d0); v('display_qspi_d1', s.display.qspi_d1);
    v('display_qspi_d2',    s.display.qspi_d2); v('display_qspi_d3', s.display.qspi_d3);
    v('display_qspi_rst',   s.display.qspi_rst);
    v('display_epaper_busy',s.display.epaper_busy);
    v('display_rgb_pclk',   s.display.rgb_pclk);  v('display_rgb_de',   s.display.rgb_de);
    v('display_rgb_vsync',  s.display.rgb_vsync);  v('display_rgb_hsync',s.display.rgb_hsync);
    for (let n=0;n<=15;n++) v(`display_rgb_d${n}`, s.display[`rgb_d${n}`]);
    c('display_uart_secondary', s.display.uart_secondary);
    sv('display_uart_port',     s.display.uart_port);
    v('display_uart_tx',        s.display.uart_tx);
    v('display_uart_rx',        s.display.uart_rx);
    sv('display_uart_baud',     s.display.uart_baud);
    sv('display_uart_proto',    s.display.uart_proto);
    c('touch_enable',   s.touch.enable);
    sv('touch_type',    s.touch.type);
    v('touch_sda',      s.touch.pin_sda); v('touch_scl', s.touch.pin_scl);
    v('touch_int',      s.touch.pin_int); v('touch_rst', s.touch.pin_rst);
    c('speaker_enable',   s.speaker.enable);
    sv('speaker_type',    s.speaker.type);
    v('spk_dout',         s.speaker.pin_dout); v('spk_bclk', s.speaker.pin_bclk);
    v('spk_lrck',         s.speaker.pin_lrck);
    v('spk_codec_sda',    s.speaker.codec_sda); v('spk_codec_scl', s.speaker.codec_scl);
    c('mic_enable',   s.mic.enable);
    sv('mic_type',    s.mic.type);
    v('mic_din',      s.mic.pin_din); v('mic_sck', s.mic.pin_sck); v('mic_ws', s.mic.pin_ws);
    v('mic_codec_sda',s.mic.codec_sda); v('mic_codec_scl', s.mic.codec_scl);
    sv('audio_i2s_mode', s.audio.i2s_mode);
    sv('wakeword_type',      s.wakeword.type);
    v('wakeword_word',       s.wakeword.custom_word);
    v('wakeword_display',    s.wakeword.custom_display);
    v('wakeword_threshold',  s.wakeword.threshold);
    c('wakeword_device_aec', s.wakeword.device_aec);
    c('wakeword_server_aec', s.wakeword.server_aec);
    c('wakeword_send_data',  s.wakeword.send_data);
    c('wakeword_in_listen',  s.wakeword.detection_in_listening);
    sv('wifi_method', s.wifi.method);
    c('uart_ext_enable', s.network.uart_ext_enable);
    sv('uart_ext_port', s.network.uart_ext_port);
    v('uart_ext_tx', s.network.uart_ext_tx);
    v('uart_ext_rx', s.network.uart_ext_rx);
    v('uart_ext_baud', s.network.uart_ext_baud);
    c('network_mcp_enable', s.network.mcp_enable);
    c('buttons_boot',    s.buttons.boot_enable);
    v('boot_gpio',       s.buttons.boot_gpio);
    c('buttons_touch',   s.buttons.touch_enable);
    v('touch_btn_gpio',  s.buttons.touch_gpio);
    c('buttons_vol',     s.buttons.vol_enable);
    v('vol_up_gpio',     s.buttons.vol_up_gpio);
    v('vol_down_gpio',   s.buttons.vol_down_gpio);
    c('buttons_slider',  s.buttons.slider_enable);
    v('slider_pad1',     s.buttons.slider_pad1);
    v('slider_pad2',     s.buttons.slider_pad2);
    v('slider_pad3',     s.buttons.slider_pad3);
    c('buttons_rotary',  s.buttons.rotary_enable);
    v('rotary_a',        s.buttons.rotary_a);
    v('rotary_b',        s.buttons.rotary_b);
    v('rotary_key',      s.buttons.rotary_key);
    c('user_custom_sensors', s.buttons.user_custom_sensors);
    c('camera_enable',    s.camera.enable);
    sv('camera_sensor',   s.camera.sensor);
    v('cam_xclk',s.camera.pin_xclk); v('cam_pclk',s.camera.pin_pclk);
    v('cam_vsync',s.camera.pin_vsync); v('cam_href',s.camera.pin_href);
    v('cam_siod',s.camera.pin_siod); v('cam_sioc',s.camera.pin_sioc);
    c('cam_use_reset',s.camera.use_reset); v('cam_reset',s.camera.pin_reset);
    c('cam_use_pwdn',s.camera.use_pwdn);   v('cam_pwdn',s.camera.pin_pwdn);
    for (let n=0;n<=7;n++) v(`cam_d${n}`, s.camera[`pin_d${n}`]);
    c('cam_hmirror', s.camera.hmirror);
    c('cam_vflip',   s.camera.vflip);
    c('led_enable',    s.led.enable);
    sv('led_type',     s.led.type);
    v('led_gpio',      s.led.gpio);
    v('led_count',     s.led.count);
    c('led_rainbow',   s.led.rainbow);
    c('mcp_enable',    s.mcp.enable);
    c('mcp_lamp',      s.mcp.lamp);
    v('mcp_lamp_gpio', s.mcp.lamp_gpio);
    c('mcp_sensor',    s.mcp.sensor);
    c('mcp_actuator',  s.mcp.actuator);
    c('uart_enable',   s.uart.enable);
    sv('uart_port',    s.uart.port);
    v('uart_tx',       s.uart.pin_tx); v('uart_rx', s.uart.pin_rx);
    v('uart_baud',     s.uart.baudrate);
    c('uart_flow',     s.uart.flow_ctrl);
    v('uart_rts',      s.uart.pin_rts); v('uart_cts', s.uart.pin_cts);
    c('servo_enable',  s.actuators.servo_enable);
    v('servo_gpio',    s.actuators.servo_gpio);
    c('buzzer_enable', s.actuators.buzzer_enable);
    v('buzzer_gpio',   s.actuators.buzzer_gpio);
    c('haptic_enable', s.actuators.haptic_enable);
    v('haptic_gpio',   s.actuators.haptic_gpio);
    c('relay_enable',    s.motor.relay_enable);
    v('relay_gpio',      s.motor.relay_gpio);
    c('motor_dc_enable', s.motor.dc_enable);
    sv('motor_driver',   s.motor.dc_driver);
    v('motor_pwma',s.motor.pwma); v('motor_dira',s.motor.dira);
    v('motor_pwmb',s.motor.pwmb); v('motor_dirb',s.motor.dirb);
    c('sensor_dht',  s.sensors.dht_enable);  v('dht_gpio',  s.sensors.dht_gpio);
    c('sensor_temp', s.sensors.temp_enable); c('sensor_aht20', s.sensors.aht20);
    v('temp_sda', s.sensors.temp_sda);       v('temp_scl', s.sensors.temp_scl);
    c('sensor_bmp280', s.sensors.bmp280);
    v('bmp280_sda', s.sensors.bmp280_sda);   v('bmp280_scl', s.sensors.bmp280_scl);
    c('sensor_bh1750', s.sensors.bh1750);
    v('bh1750_sda', s.sensors.bh1750_sda);   v('bh1750_scl', s.sensors.bh1750_scl);
    c('sensor_ldr', s.sensors.ldr);          sv('ldr_ch', s.sensors.ldr_ch);
    c('sensor_gas_co2', s.sensors.gas_co2);  c('gas_scd4x', s.sensors.gas_scd4x);
    v('gas_sda', s.sensors.gas_sda);         v('gas_scl', s.sensors.gas_scl);
    c('sensor_gas_mq', s.sensors.gas_mq);    v('gas_mq_gpio', s.sensors.gas_mq_gpio);
    c('sensor_hcsr04', s.sensors.hcsr04);
    v('hcsr04_trig', s.sensors.hcsr04_trig); v('hcsr04_echo', s.sensors.hcsr04_echo);
    c('sensor_pir', s.sensors.pir);          v('pir_gpio', s.sensors.pir_gpio);
    c('sensor_vibration', s.sensors.vibration); v('vibration_gpio', s.sensors.vibration_gpio);
    c('sensor_flame', s.sensors.flame);      v('flame_gpio', s.sensors.flame_gpio);
    c('battery_enable', s.power.battery_enable);
    c('battery_adc',    s.power.battery_adc);
    sv('battery_ch',    s.power.battery_ch);
    v('battery_r1',     s.power.battery_r1); v('battery_r2', s.power.battery_r2);
    c('ina2xx_enable',  s.power.ina2xx);     c('ina219', s.power.ina219);
    v('ina_sda',        s.power.ina_sda);    v('ina_scl', s.power.ina_scl);
    c('tp4056_enable',  s.power.tp4056);     v('tp4056_gpio', s.power.tp4056_gpio);
    GUIController._toggleMultilineChat();
    GUIController._toggleDisplayGroups();
    GUIController._toggleWakewordCustom();
    GUIController._toggleCameraUsb();
    document.getElementById('touch_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('speaker_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('mic_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('camera_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('led_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('mcp_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('uart_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_boot')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_touch')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_vol')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_slider')?.dispatchEvent(new Event('change'));
    document.getElementById('buttons_rotary')?.dispatchEvent(new Event('change'));
    document.getElementById('servo_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('buzzer_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('haptic_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('relay_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('motor_dc_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('battery_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('ina2xx_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('tp4056_enable')?.dispatchEvent(new Event('change'));
    document.getElementById('uart_ext_enable')?.dispatchEvent(new Event('change'));
  }
  static syncUIToState() {
    const s = window.configState;
    const gv = id => document.getElementById(id)?.value ?? '';
    const gi = (id, def=-1) => { const v = parseInt(gv(id),10); return isNaN(v) ? def : v; };
    const gc = id => !!document.getElementById(id)?.checked;
    s.general.language         = gv('general_language');
    s.general.ota_url          = gv('general_ota_url');
    s.general.flash_assets     = gv('general_flash_assets');
    s.general.custom_assets_file = gv('general_custom_assets');
    s.general.display_style    = gv('general_display_style');
    s.general.multiline_chat   = gc('general_multiline');
    s.system.cpu_freq          = gv('sys_cpu_freq');
    s.system.flash_mode        = gv('sys_flash_mode');
    s.system.flash_freq        = gv('sys_flash_freq');
    s.system.spiram            = gc('sys_spiram');
    s.system.spiram_mode       = gv('sys_spiram_mode');
    s.system.spiram_speed      = gv('sys_spiram_speed');
    s.system.partition_table   = gv('sys_partition_table');
    s.display.enable           = gc('display_enable');
    s.display.type             = gv('display_type');
    s.display.width            = gi('display_width', 240);
    s.display.height           = gi('display_height', 320);
    s.display.pin_mosi         = gi('display_mosi');
    s.display.pin_clk          = gi('display_clk');
    s.display.pin_cs           = gi('display_cs');
    s.display.pin_dc           = gi('display_dc');
    s.display.use_rst          = gc('display_use_rst');
    s.display.pin_rst          = gi('display_rst');
    s.display.use_blk          = gc('display_use_blk');
    s.display.pin_blk          = gi('display_blk');
    s.display.pin_i2c_sda      = gi('display_i2c_sda');
    s.display.pin_i2c_scl      = gi('display_i2c_scl');
    s.display.offset_x         = gi('display_offset_x', 0);
    s.display.offset_y         = gi('display_offset_y', 0);
    s.display.mirror_x         = gc('display_mirror_x');
    s.display.mirror_y         = gc('display_mirror_y');
    s.display.swap_xy          = gc('display_swap_xy');
    s.display.invert_color     = gc('display_invert');
    s.display.amoled_chip      = gv('display_amoled_chip');
    s.display.qspi_cs          = gi('display_qspi_cs');
    s.display.qspi_clk         = gi('display_qspi_clk');
    for (let n=0;n<=3;n++) s.display[`qspi_d${n}`] = gi(`display_qspi_d${n}`);
    s.display.qspi_rst         = gi('display_qspi_rst');
    s.display.epaper_busy      = gi('display_epaper_busy');
    s.display.rgb_pclk         = gi('display_rgb_pclk');
    s.display.rgb_de           = gi('display_rgb_de');
    s.display.rgb_vsync        = gi('display_rgb_vsync');
    s.display.rgb_hsync        = gi('display_rgb_hsync');
    for (let n=0;n<=15;n++) s.display[`rgb_d${n}`] = gi(`display_rgb_d${n}`);
    s.display.uart_secondary   = gc('display_uart_secondary');
    s.display.uart_port        = gi('display_uart_port', 1);
    s.display.uart_tx          = gi('display_uart_tx');
    s.display.uart_rx          = gi('display_uart_rx');
    s.display.uart_baud        = gi('display_uart_baud', 115200);
    s.display.uart_proto       = gv('display_uart_proto');
    s.touch.enable             = gc('touch_enable');
    s.touch.type               = gv('touch_type');
    s.touch.pin_sda            = gi('touch_sda'); s.touch.pin_scl = gi('touch_scl');
    s.touch.pin_int            = gi('touch_int'); s.touch.pin_rst = gi('touch_rst');
    s.speaker.enable           = gc('speaker_enable');
    s.speaker.type             = gv('speaker_type');
    s.speaker.pin_dout         = gi('spk_dout'); s.speaker.pin_bclk = gi('spk_bclk');
    s.speaker.pin_lrck         = gi('spk_lrck');
    s.speaker.codec_sda        = gi('spk_codec_sda'); s.speaker.codec_scl = gi('spk_codec_scl');
    s.mic.enable               = gc('mic_enable');
    s.mic.type                 = gv('mic_type');
    s.mic.pin_din              = gi('mic_din'); s.mic.pin_sck = gi('mic_sck');
    s.mic.pin_ws               = gi('mic_ws');
    s.mic.codec_sda            = gi('mic_codec_sda'); s.mic.codec_scl = gi('mic_codec_scl');
    s.audio.i2s_mode           = gv('audio_i2s_mode');
    s.wakeword.type            = gv('wakeword_type');
    s.wakeword.custom_word     = gv('wakeword_word');
    s.wakeword.custom_display  = gv('wakeword_display');
    s.wakeword.threshold       = gi('wakeword_threshold', 20);
    s.wakeword.device_aec      = gc('wakeword_device_aec');
    s.wakeword.server_aec      = gc('wakeword_server_aec');
    s.wakeword.send_data       = gc('wakeword_send_data');
    s.wakeword.detection_in_listening = gc('wakeword_in_listen');
    s.wifi.method              = gv('wifi_method');
    s.network.uart_ext_enable  = gc('uart_ext_enable');
    s.network.uart_ext_port    = gv('uart_ext_port');
    s.network.uart_ext_tx      = gi('uart_ext_tx');
    s.network.uart_ext_rx      = gi('uart_ext_rx');
    s.network.uart_ext_baud    = gi('uart_ext_baud', 115200);
    s.network.mcp_enable       = gc('network_mcp_enable');
    s.buttons.boot_enable      = gc('buttons_boot');
    s.buttons.boot_gpio        = gi('boot_gpio');
    s.buttons.touch_enable     = gc('buttons_touch');
    s.buttons.touch_gpio       = gi('touch_btn_gpio');
    s.buttons.vol_enable       = gc('buttons_vol');
    s.buttons.vol_up_gpio      = gi('vol_up_gpio');
    s.buttons.vol_down_gpio    = gi('vol_down_gpio');
    s.buttons.slider_enable    = gc('buttons_slider');
    s.buttons.slider_pad1      = gi('slider_pad1');
    s.buttons.slider_pad2      = gi('slider_pad2');
    s.buttons.slider_pad3      = gi('slider_pad3');
    s.buttons.rotary_enable    = gc('buttons_rotary');
    s.buttons.rotary_a         = gi('rotary_a');
    s.buttons.rotary_b         = gi('rotary_b');
    s.buttons.rotary_key       = gi('rotary_key');
    s.buttons.user_custom_sensors = gc('user_custom_sensors');
    s.camera.enable            = gc('camera_enable');
    s.camera.sensor            = gv('camera_sensor');
    s.camera.pin_xclk=gi('cam_xclk'); s.camera.pin_pclk=gi('cam_pclk');
    s.camera.pin_vsync=gi('cam_vsync'); s.camera.pin_href=gi('cam_href');
    s.camera.pin_siod=gi('cam_siod'); s.camera.pin_sioc=gi('cam_sioc');
    s.camera.use_reset=gc('cam_use_reset'); s.camera.pin_reset=gi('cam_reset');
    s.camera.use_pwdn=gc('cam_use_pwdn');   s.camera.pin_pwdn=gi('cam_pwdn');
    for (let n=0;n<=7;n++) s.camera[`pin_d${n}`]=gi(`cam_d${n}`);
    s.camera.hmirror=gc('cam_hmirror'); s.camera.vflip=gc('cam_vflip');
    s.led.enable=gc('led_enable'); s.led.type=gv('led_type');
    s.led.gpio=gi('led_gpio'); s.led.count=gi('led_count',1);
    s.led.rainbow=gc('led_rainbow');
    s.mcp.enable=gc('mcp_enable'); s.mcp.lamp=gc('mcp_lamp');
    s.mcp.lamp_gpio=gi('mcp_lamp_gpio');
    s.mcp.sensor=gc('mcp_sensor'); s.mcp.actuator=gc('mcp_actuator');
    s.uart.enable=gc('uart_enable'); s.uart.port=gv('uart_port');
    s.uart.pin_tx=gi('uart_tx'); s.uart.pin_rx=gi('uart_rx');
    s.uart.baudrate=gi('uart_baud',115200); s.uart.flow_ctrl=gc('uart_flow');
    s.uart.pin_rts=gi('uart_rts'); s.uart.pin_cts=gi('uart_cts');
    s.actuators.servo_enable=gc('servo_enable');
    s.actuators.servo_gpio=gi('servo_gpio');
    s.actuators.buzzer_enable=gc('buzzer_enable');
    s.actuators.buzzer_gpio=gi('buzzer_gpio');
    s.actuators.haptic_enable=gc('haptic_enable');
    s.actuators.haptic_gpio=gi('haptic_gpio');
    s.motor.relay_enable=gc('relay_enable'); s.motor.relay_gpio=gi('relay_gpio');
    s.motor.dc_enable=gc('motor_dc_enable'); s.motor.dc_driver=gv('motor_driver');
    s.motor.pwma=gi('motor_pwma'); s.motor.dira=gi('motor_dira');
    s.motor.pwmb=gi('motor_pwmb'); s.motor.dirb=gi('motor_dirb');
    s.sensors.dht_enable=gc('sensor_dht'); s.sensors.dht_gpio=gi('dht_gpio');
    s.sensors.temp_enable=gc('sensor_temp'); s.sensors.aht20=gc('sensor_aht20');
    s.sensors.temp_sda=gi('temp_sda'); s.sensors.temp_scl=gi('temp_scl');
    s.sensors.bmp280=gc('sensor_bmp280');
    s.sensors.bmp280_sda=gi('bmp280_sda'); s.sensors.bmp280_scl=gi('bmp280_scl');
    s.sensors.bh1750=gc('sensor_bh1750');
    s.sensors.bh1750_sda=gi('bh1750_sda'); s.sensors.bh1750_scl=gi('bh1750_scl');
    s.sensors.ldr=gc('sensor_ldr'); s.sensors.ldr_ch=gi('ldr_ch',1);
    s.sensors.gas_co2=gc('sensor_gas_co2'); s.sensors.gas_scd4x=gc('gas_scd4x');
    s.sensors.gas_sda=gi('gas_sda'); s.sensors.gas_scl=gi('gas_scl');
    s.sensors.gas_mq=gc('sensor_gas_mq'); s.sensors.gas_mq_gpio=gi('gas_mq_gpio');
    s.sensors.hcsr04=gc('sensor_hcsr04');
    s.sensors.hcsr04_trig=gi('hcsr04_trig'); s.sensors.hcsr04_echo=gi('hcsr04_echo');
    s.sensors.pir=gc('sensor_pir'); s.sensors.pir_gpio=gi('pir_gpio');
    s.sensors.vibration=gc('sensor_vibration');
    s.sensors.vibration_gpio=gi('vibration_gpio');
    s.sensors.flame=gc('sensor_flame'); s.sensors.flame_gpio=gi('flame_gpio');
    s.power.battery_enable=gc('battery_enable'); s.power.battery_adc=gc('battery_adc');
    s.power.battery_ch=gi('battery_ch',0);
    s.power.battery_r1=gi('battery_r1',100); s.power.battery_r2=gi('battery_r2',100);
    s.power.ina2xx=gc('ina2xx_enable'); s.power.ina219=gc('ina219');
    s.power.ina_sda=gi('ina_sda'); s.power.ina_scl=gi('ina_scl');
    s.power.tp4056=gc('tp4056_enable'); s.power.tp4056_gpio=gi('tp4056_gpio');
  }
  static _show(sel, show) {
    document.querySelectorAll(sel).forEach(el =>
      el.style.display = show ? '' : 'none');
  }
  static _toggleGroup(sel, checkboxId) {
    const checked = !!document.getElementById(checkboxId)?.checked;
    GUIController._show(sel, checked);
  }
  static _toggleDisplayGroups() {
    const enabled = !!document.getElementById('display_enable')?.checked;
    GUIController._show('.display-common', enabled);
    const type = document.getElementById('display_type')?.value || '';
    const isSpi   = enabled && /ST7789|ST7796|ST7735|ILI9341|ILI9486|GC9A01|GC9107|NV3023|JD9853|ST7701/.test(type);
    const isOled  = enabled && /OLED_SSD1306|OLED_SH1106/.test(type);
    const isQspi  = enabled && type.includes('QSPI_AMOLED');
    const isEpaper= enabled && type.includes('EPAPER');
    const isRgb   = enabled && type.includes('ST7701');
    const isUart  = enabled && (type.includes('UART') || !!document.getElementById('display_uart_secondary')?.checked);
    GUIController._show('.display-spi',      isSpi);
    GUIController._show('.display-oled',     isOled);
    GUIController._show('.display-qspi',     isQspi);
    GUIController._show('.display-epaper',   isEpaper);
    GUIController._show('.display-rgb',      isRgb);
    GUIController._show('.display-uart-cfg', isUart);
    const autoRes = {
      'CUSTOM_DISPLAY_ST7796':[320,480],'CUSTOM_DISPLAY_ILI9486':[320,480],
      'CUSTOM_DISPLAY_GC9A01':[240,240],'CUSTOM_DISPLAY_GC9107':[128,128],
      'CUSTOM_DISPLAY_OLED_SSD1306':[128,64],'CUSTOM_DISPLAY_OLED_SH1106':[128,64],
      'CUSTOM_DISPLAY_ST7735':[128,160],
    };
    if (enabled && type in autoRes && !GUIController.dirty['panel-display']) {
      const [w,h] = autoRes[type];
      const we = document.getElementById('display_width');
      const he = document.getElementById('display_height');
      if (we && we.value !== String(w)) we.value = w;
      if (he && he.value !== String(h)) he.value = h;
    }
  }
  static _toggleMultilineChat() {
    const style = document.getElementById('general_display_style')?.value;
    const isDefault = style === 'USE_DEFAULT_MESSAGE_STYLE';
    const el = document.getElementById('general_multiline');
    if (el) {
      el.disabled = !isDefault;
      if (!isDefault) el.checked = false;
      const parent = el.closest('.form-check') || el.parentElement;
      if (parent) parent.style.opacity = isDefault ? '1' : '0.4';
    }
  }
  static _toggleCodecGroup(prefix) {
    const type = document.getElementById(`${prefix === 'spk' ? 'speaker' : 'mic'}_type`)?.value || '';
    const isCodec = type.includes('CODEC');
    GUIController._show(`.${prefix}-codec-fields`, isCodec);
  }
  static _toggleCameraUsb() {
    const enabled = !!document.getElementById('camera_enable')?.checked;
    const isUsb = document.getElementById('camera_sensor')?.value === 'CUSTOM_CAMERA_USB_UVC';
    GUIController._show('.cam-fields', enabled);
    GUIController._show('.cam-dvp-fields', enabled && !isUsb);
  }
  static _toggleLedRainbow() {
    const type = document.getElementById('led_type')?.value || '';
    const hasRainbow = /WS2812|CIRCULAR/.test(type);
    GUIController._show('.led-rainbow-field', hasRainbow);
  }
  static _toggleWakewordCustom() {
    const type = document.getElementById('wakeword_type')?.value || '';
    const isCustom = type === 'USE_CUSTOM_WAKE_WORD';
    GUIController._show('.wakeword-custom-fields', isCustom);
    const canListen = ['USE_AFE_WAKE_WORD', 'USE_CUSTOM_WAKE_WORD'].includes(type);
    const inListen = document.getElementById('wakeword_in_listen');
    if (inListen) {
      inListen.disabled = !canListen;
      if (!canListen) inListen.checked = false;
      const p = inListen.closest('.form-check') || inListen.parentElement;
      if (p) p.style.opacity = canListen ? '1' : '0.4';
    }
  }
  static _syncFlashPartition() {
    const size = document.getElementById('flash_size')?.value;
    const pt   = document.getElementById('sys_partition_table');
    if (pt && size === '16MB' && !pt.value.includes('16')) {
      pt.value = 'partitions/16m.csv';
    }
  }
  static renderPinMatrix() {
    const grid = document.getElementById('pin-matrix-grid');
    if (!grid) return;
    grid.innerHTML = '';
    for (let pin = 0; pin <= 48; pin++) {
      const cell = document.createElement('div');
      cell.className = 'pin-cell';
      cell.id = `pin-cell-${pin}`;
      cell.textContent = pin;
      if (ESP32S3.LOCKED.includes(pin)) {
        cell.classList.add('locked');
        cell.title = `GPIO ${pin}: KHÓA (Flash/PSRAM)`;
      } else {
        cell.title = `GPIO ${pin}: Trống`;
      }
      grid.appendChild(cell);
    }
  }
  static updateValidation() {
    const { errors, usage } = PinValidator.validate(window.configState);
    window.conflictErrors = errors;
    document.querySelectorAll('.form-control').forEach(el =>
      el.classList.remove('has-conflict','is-locked'));
    for (let p=0; p<=48; p++) {
      const cell = document.getElementById(`pin-cell-${p}`);
      if (cell && !ESP32S3.LOCKED.includes(p)) {
        cell.classList.remove('used','conflict');
        cell.title = `GPIO ${p}: Trống`;
      }
    }
    for (const [pin, users] of usage.entries()) {
      const cell = document.getElementById(`pin-cell-${pin}`);
      if (cell) {
        cell.classList.add('used');
        cell.title = `GPIO ${pin}: ${users.map(u=>u.name).join(', ')}`;
      }
    }
    const bar = document.getElementById('conflict-alert-bar');
    const msg = document.getElementById('conflict-message');
    if (errors.length) {
      bar?.classList.add('visible');
      if (msg) msg.innerHTML = errors.map(e=>`• ${e.message}`).join('<br>');
      errors.forEach(e => {
        document.getElementById(`pin-cell-${e.pin}`)?.classList.add('conflict');
      });
      document.querySelectorAll('.btn-save-tab').forEach(b => b.disabled = true);
    } else {
      bar?.classList.remove('visible');
      document.querySelectorAll('.btn-save-tab').forEach(b => b.disabled = false);
    }
  }
  static showToast(msg, type = 'success') {
    const c = document.getElementById('toast-container');
    if (!c) return;
    const t = document.createElement('div');
    t.className = `toast-item toast-${type}`;
    t.innerHTML = `<span>${{'success':'✓','error':'✕','warning':'⚠️'}[type]||'ℹ'}</span><span>${msg}</span>`;
    c.appendChild(t);
    setTimeout(() => { t.style.opacity='0'; setTimeout(()=>t.remove(),300); }, 3500);
  }
  static _setStatus(state, text) {
    const dot  = document.getElementById('fs-status-dot');
    const label= document.getElementById('fs-status-text');
    dot?.classList.toggle('connected', state === 'connected');
    dot?.classList.toggle('error',     state === 'error');
    if (label) label.textContent = text;
  }
}
