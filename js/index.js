const EventEmitter = require('events').EventEmitter;
const NodeTray = require("../build/Release/n_windows_tray").NodeTray

const util = require('util')
util.inherits(NodeTray, EventEmitter)

module.exports = NodeTray