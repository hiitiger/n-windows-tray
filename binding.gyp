{
  "targets": [{
    "target_name": "n_windows_tray",
    "include_dirs" : [
      "<!@(node -p \"require('node-addon-api').include\")",
      "src",
    ],
    "sources": [
      "src/n-utils.hpp",
      "src/utils.hpp",
      "src/tray.h",
      "src/tray.cc",
      "src/node_async_call.h",
      "src/node_async_call.cc",
      "src/main.cc",
    ],
    "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS", "UNICODE", "_UNICODE" ],
  }]
}