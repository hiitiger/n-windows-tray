declare module "n-windows-tray" {
  export = NodeTray;
  type Listener = (...args: any[]) => void;

  type Rectangle = {
    x: number;
    y: number;
    width: number;
    height: number;
  };

  class NodeTray {
    constructor(icon: string);

    destroy(): void;
    setIcon(icon: string): void;
    setToolTip(text: string): void;
    getBounds(): Rectangle;

    on(type: "mouse-enter", listener: Listener): this;
    once(type: "mouse-enter", listener: Listener): this;
    addListener(type: "mouse-enter", listener: Listener): this;
    removeListener(type: "mouse-enter", listener: Listener): this;

    on(type: "mouse-leave", listener: Listener): this;
    once(type: "mouse-leave", listener: Listener): this;
    addListener(type: "mouse-leave", listener: Listener): this;
    removeListener(type: "mouse-leave", listener: Listener): this;

    on(type: "click", listener: Listener): this;
    once(type: "click", listener: Listener): this;
    addListener(type: "click", listener: Listener): this;
    removeListener(type: "click", listener: Listener): this;

    on(type: "right-click", listener: Listener): this;
    once(type: "right-click", listener: Listener): this;
    addListener(type: "right-click", listener: Listener): this;
    removeListener(type: "right-click", listener: Listener): this;

    on(type: "double-click", listener: Listener): this;
    once(type: "double-click", listener: Listener): this;
    addListener(type: "double-click", listener: Listener): this;
    removeListener(type: "double-click", listener: Listener): this;

    removeAllListeners(type?: string | number): this;
  }
}
