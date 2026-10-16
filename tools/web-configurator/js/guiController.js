import { ESP32S3 } from './hardware.js';
import { DEFAULT_CONFIG } from './configState.js';
import { SdkconfigParser } from './parser.js';
import { SdkconfigGenerator } from './generator.js';
import { PinValidator } from './pinValidator.js';
import { BuildController } from './buildController.js';
import { BINDINGS } from './bindings.js';
import { applyProfileToDocument, isP4Profile, linesForProfile } from './profile.js';
export class GUIController {
  static snapshots   = {};   
  static dirty       = {};   
  static pendingTab  = null;
  static TAB_NAMES = {
    'panel-assistant':   'Trợ lý',
    'panel-display':     'Màn hình',
    'panel-audio':       'Âm thanh',
    'panel-network':     'Kết nối',
    'panel-peripherals': 'Ngoại vi',
    'panel-power':       'Nguồn & Pin',
    'panel-system':      'Hệ thống',
    'panel-build':       'Biên dịch & Nạp',
  };
  static async init() {
    GUIController.bindNav();
    GUIController.bindInputs();
    GUIController.bindSaveBtns();
    GUIController.bindUnsavedModal();
    GUIController.bindOpenDir();
    GUIController.bindSupportModal();
    BuildController.init();
    GUIController.syncStateToUI();
    GUIController.renderPinMatrix();
    const fromHash = window.location.hash.slice(1);
    const startTab = /^panel-[\w-]+$/.test(fromHash) &&
                     document.querySelector(`.nav-link[data-target="${fromHash}"]`)
      ? fromHash
      : document.querySelector('.nav-link.active[data-target]')?.dataset.target || 'panel-assistant';
    GUIController.switchTab(startTab);
    GUIController.initSnapshots();
    await GUIController.autoConnect();
  }
  static applyProfile(profile) {
    if (!profile) return;
    window.buildProfile = profile;
    applyProfileToDocument(profile);
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
        GUIController.applyProfile(cfgData.profile);
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
    p.querySelectorAll('input:not([data-no-config]), select:not([data-no-config]), textarea:not([data-no-config])').forEach(el => {
      if (el.type === 'checkbox') parts.push(`${el.id}:${el.checked}`);
      else if (el.type === 'radio') { if (el.checked) parts.push(`${el.name}:${el.value}`); }
      else parts.push(`${el.id}:${el.value}`);
    });
    return parts.join('|');
  }
  static initSnapshots() {
    GUIController.savedState = JSON.parse(JSON.stringify(window.configState));
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
        const panel = document.getElementById(active);
        for (const [id, path] of BINDINGS) {
          if (panel?.contains(document.getElementById(id))) {
            GUIController._setPath(window.configState, path,
              GUIController._getPath(GUIController.savedState, path));
          }
        }
        GUIController.syncStateToUI();
        GUIController.updateValidation();
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
      if (active && !(await GUIController.saveTab(active))) return;
      const t = GUIController.pendingTab;
      close();
      if (t) GUIController.switchTab(t);
    });
  }
  static bindInputs() {
    document.querySelectorAll('.tab-panel').forEach(panel => {
      panel.addEventListener('input',  () => GUIController.checkDirty(panel.id));
      panel.addEventListener('change', () => {
        GUIController.applyVisibility();
        GUIController.checkDirty(panel.id);
      });
    });
    const on = (id, fn) => document.getElementById(id)?.addEventListener('change', fn);
    on('display_type',   () => GUIController._autoDisplayResolution());
    ['display_width', 'display_height'].forEach(id =>
      document.getElementById(id)?.addEventListener('input', () => { GUIController._resEdited = true; }));
    on('sys_flash_size', () => GUIController._syncFlashPartition());
    window.addEventListener('beforeunload', e => {
      if (Object.values(GUIController.dirty).some(Boolean)) {
        e.preventDefault(); e.returnValue = '';
      }
    });
  }
  static async saveTab(panelId) {
    GUIController.syncUIToState();
    const { errors } = isP4Profile(window.buildProfile) ? { errors: [] } : PinValidator.validate(window.configState);
    if (errors.length) {
      GUIController.showToast('Có xung đột GPIO! Kiểm tra lại.', 'error');
      GUIController.updateValidation();
      return false;
    }
    if (window.isServerMode) {
      try {
        const lines = linesForProfile(SdkconfigGenerator.build(window.configState), window.buildProfile);
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
    GUIController.initSnapshots();
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
      const lines = linesForProfile(SdkconfigGenerator.build(window.configState), window.buildProfile);
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
  static bindSupportModal() {
    const modal = document.getElementById('support-modal');
    const open = (e) => { e?.currentTarget?.blur(); modal?.classList.add('active', 'visible'); };
    const close = () => modal?.classList.remove('active', 'visible');
    document.getElementById('btn-open-support')?.addEventListener('click', open);
    document.getElementById('btn-close-support')?.addEventListener('click', close);
    modal?.addEventListener('click', (e) => { if (e.target === modal) close(); });
    document.addEventListener('keydown', (e) => { if (e.key === 'Escape') close(); });
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
              <span class="project-scan-path"></span>
              <div class="project-scan-meta">
                <span class="idf-scan-source-pill"></span>
                ${item.is_current ? '<span class="text-emerald font-bold">● Đang hoạt động</span>' : ''}
              </div>
            </div>
            ${!item.is_current ? `
              <button type="button" class="btn btn-outline btn-xs btn-pick-project">
                Chọn
              </button>
            ` : '<span class="text-emerald" style="font-size:0.75rem;">✓ Hiện tại</span>'}
          `;
          const pathEl = row.querySelector('.project-scan-path');
          pathEl.textContent = item.path; pathEl.title = item.path;
          row.querySelector('.idf-scan-source-pill').textContent = item.source;
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
          GUIController.applyProfile(cfgData.profile);
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
          GUIController.applyProfile(cfgData.profile);
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
  static _getPath(obj, path) {
    return path.split('.').reduce((o, k) => (o == null ? undefined : o[k]), obj);
  }
  static _setPath(obj, path, val) {
    const keys = path.split('.');
    const last = keys.pop();
    const target = keys.reduce((o, k) => (o[k] ??= {}), obj);
    target[last] = val;
  }
  static syncStateToUI() {
    const s = window.configState;
    GUIController._resEdited = false;
    for (const [id, path, kind] of BINDINGS) {
      const el = document.getElementById(id);
      if (!el) continue;
      const val = GUIController._getPath(s, path);
      if (kind === 'bool') el.checked = !!val;
      else if (el.tagName === 'SELECT' && val != null && val !== '' &&
               ![...el.options].some(o => o.value === String(val))) {
        const opt = document.createElement('option');
        opt.value = String(val);
        opt.textContent = `${val} (từ sdkconfig)`;
        el.add(opt);
        el.value = String(val);
      } else el.value = val ?? '';
    }
    GUIController.applyVisibility();
  }
  static syncUIToState() {
    const s = window.configState;
    for (const [id, path, kind, def = -1] of BINDINGS) {
      const el = document.getElementById(id);
      if (!el) continue;
      let val;
      if (kind === 'bool') val = !!el.checked;
      else if (kind === 'int') { const n = parseInt(el.value, 10); val = Number.isNaN(n) ? def : n; }
      else val = el.value ?? '';
      GUIController._setPath(s, path, val);
    }
  }
  static _evalShowIf(expr) {
    if (expr.includes('||')) return expr.split('||').some(e => GUIController._evalShowIf(e));
    if (expr.trim() === 'false') return false;
    return expr.split('&').every(raw => {
      const cond = raw.trim();
      const m = /^(!?)([\w-]+)(?:(\^=|!=|=)(.*))?$/.exec(cond);
      if (!m) return true;
      const [, neg, id, op, rhs] = m;
      const el = document.getElementById(id);
      if (!el) return false;
      const shown = GUIController._isShown(el);
      let ok;
      if (!op) ok = shown && (el.type === 'checkbox' ? el.checked : !!el.value);
      else if (op === '^=') ok = shown && el.value.startsWith(rhs);
      else {
        const hit = shown && rhs.split('|').includes(el.value);
        ok = op === '=' ? hit : !hit;
      }
      return neg ? !ok : ok;
    });
  }
  static _isShown(el) {
    for (let n = el; n; n = n.parentElement) if (n.style?.display === 'none') return false;
    return true;
  }
  static applyVisibility() {
    document.querySelectorAll('[data-show-if]').forEach(el => {
      el.style.display = GUIController._evalShowIf(el.dataset.showIf) ? '' : 'none';
    });
  }
  static _autoDisplayResolution() {
    const type = document.getElementById('display_type')?.value || '';
    const autoRes = {
      'CUSTOM_DISPLAY_ST7796':[320,480],'CUSTOM_DISPLAY_ILI9486':[320,480],
      'CUSTOM_DISPLAY_GC9A01':[240,240],'CUSTOM_DISPLAY_GC9107':[128,128],
      'CUSTOM_DISPLAY_OLED_SSD1306':[128,64],'CUSTOM_DISPLAY_OLED_SH1106':[128,64],
      'CUSTOM_DISPLAY_ST7735':[128,160],
    };
    if (type in autoRes && !GUIController._resEdited) {
      const [w,h] = autoRes[type];
      const we = document.getElementById('display_width');
      const he = document.getElementById('display_height');
      if (we && we.value !== String(w)) we.value = w;
      if (he && he.value !== String(h)) he.value = h;
    }
  }
  static _syncFlashPartition() {
    const size = document.getElementById('sys_flash_size')?.value;
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
    GUIController._setConflictChip(errors.length);
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
    const icon = document.createElement('span');
    icon.textContent = {'success':'✓','error':'✕','warning':'⚠️'}[type] || 'ℹ';
    const text = document.createElement('span');
    text.textContent = msg;
    t.append(icon, text);
    c.appendChild(t);
    setTimeout(() => { t.style.opacity='0'; setTimeout(()=>t.remove(),300); }, 3500);
  }
  static _setConflictChip(count) {
    const chip = document.getElementById('status-conflicts');
    if (!chip) return;
    const dot = chip.querySelector('.status-chip-dot');
    const value = chip.querySelector('.status-chip-value');
    if (count) {
      if (value) value.textContent = `${count} trùng chân`;
      dot?.classList.remove('ok');
      dot?.classList.add('warn');
    } else {
      if (value) value.textContent = 'Không xung đột';
      dot?.classList.remove('warn');
      dot?.classList.add('ok');
    }
  }
  static _setStatus(state, text) {
    const dot  = document.getElementById('fs-status-dot');
    const label= document.getElementById('fs-status-text');
    dot?.classList.toggle('connected', state === 'connected');
    dot?.classList.toggle('error',     state === 'error');
    if (label) label.textContent = text;
  }
}
