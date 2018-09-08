{
  "targets": [{
    "target_name": "n_windows_tray",
    "include_dirs" : [
      "<!@(node -p \"require('node-addon-api').include\")",
      "src",
    ],
    "sources": [
      "src/utils/n-utils.h",
      "src/utils/win-utils.h",
      "src/utils/node_async_call.h",
      "src/utils/node_async_call.cc",
      "src/tray.h",
      "src/tray.cc",
      "src/main.cc",
    ],
    "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS", "UNICODE"],
    "cflags!": [
        "-fno-exceptions"
    ],
    "cflags_cc!": [
        "-fno-exceptions"
    ],
    "conditions": [
      [
        "OS=='win'", {
          "defines": [
              "_UNICODE",
              "_WIN32_WINNT=0x0601"
            ],
          "configurations": {
            "Release": {
              "msvs_settings": {
                "VCCLCompilerTool" : {
                  "ExceptionHandling": 1,
                }
              }
            },
            "Debug": {
              "msvs_settings": {
                "VCCLCompilerTool" : {
                  "ExceptionHandling": 1,
                }
              }
            }
          }
        }
      ]
    ]
  }]
}