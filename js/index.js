const EventEmitter = require('events').EventEmitter;
const nWindowTray = require("../build/Release/n_windows_tray").NodeTray

class NodeTray extends EventEmitter {
    constructor(icon) {
        super();
        this.nWindowTray = new nWindowTray(icon);
        this.initEvents();
    }

    initEvents() {
        this.nWindowTray.setEventCallback('mouse-enter', () => {
            this.emit('mouse-enter');
        });

        this.nWindowTray.setEventCallback('mouse-leave', () => {
            this.emit('mouse-leave');
        });

        this.nWindowTray.setEventCallback('click', () => {
            this.emit('click');
        });

        this.nWindowTray.setEventCallback('right-click', () => {
            this.emit('right-click');
        });

        this.nWindowTray.setEventCallback('double-click', () => {
            this.emit('double-click');
        });
    }


    destroy() {
        this.nWindowTray.destroy();
    }

    /**
     * 
     * @param {string} icon 
     */
    setIcon(icon) {
        this.nWindowTray.setIcon(icon);
    }

    /**
     * 
     * @param {string} text 
     */
    setToolTip(text) {
        this.nWindowTray.setToolTip(text);
    }

    /**
     * @return {object} bounds
     */
    getBounds() {
        return this.nWindowTray.getBounds();
    }
}

module.exports = NodeTray