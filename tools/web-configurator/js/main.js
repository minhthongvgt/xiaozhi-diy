import './configState.js';
import { GUIController } from './guiController.js';
import { BuildController } from './buildController.js';
async function loadComponents() {
    const placeholders = document.querySelectorAll('[id^="placeholder-panel-"]');
    for (let p of placeholders) {
        const panelId = p.id.replace('placeholder-', '');
        try {
            const res = await fetch(`components/${panelId}.html`);
            if (res.ok) {
                const html = await res.text();
                                const temp = document.createElement('div');
                temp.innerHTML = html;
                const scripts = temp.querySelectorAll('script');
                p.replaceWith(...temp.childNodes);
                scripts.forEach(s => {
                    const newScript = document.createElement('script');
                    newScript.textContent = s.textContent;
                    document.body.appendChild(newScript);
                });
            } else {
                console.error(`Failed to load ${panelId}.html`);
            }
        } catch (e) {
            console.error(`Error loading ${panelId}.html`, e);
        }
    }
}
async function bootstrap() {
    await loadComponents();
    GUIController.init();
}
if (typeof window !== 'undefined') {
    if (document.readyState === 'loading') {
        window.addEventListener('DOMContentLoaded', bootstrap);
    } else {
        bootstrap();
    }
}
