const NodeTray = require("../js")
const path = require("path")

const tray = new NodeTray(path.join(__dirname, "colors.ico"))
const tray2 = new NodeTray(path.join(__dirname, "colors.ico"))
tray.setToolTip("123123")
tray.on("mouse-enter", () => console.log("mouse-enter"))
tray2.on("mouse-enter", () => console.log("mouse-enter2"))
tray.on("mouse-leave", () => console.log("mouse-leave"))
tray.on("click", () => console.log("click"))
tray.on("right-click", () => console.log("right-click"))
tray.on("double-click", () => console.log("double-click"))


setInterval(() => {
    console.log(tray.getBounds())
}, 2000)

setTimeout(() => {
    tray.on("double-click", () => {
        tray.destroy()
        process.exit(0)
    })

}, 4000)