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