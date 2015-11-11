{
  "targets": [
    {
      "target_name": "xmlsonator",
      "cflags": [
          "-std=c++11"
      ],
      "dependencies": [
        "./lib/vendor/libxml/libxml.gyp:libxml"
      ],
      "conditions": [
        ["OS=='mac'", {
          # node-gyp 2.x doesn't add this anymore
          # https://github.com/TooTallNate/node-gyp/pull/612
          "xcode_settings": {
            "OTHER_LDFLAGS": [
              "-undefined dynamic_lookup"
            ],
          },
        }]
      ],
      "sources": [ "./src/xmlsonator.cc" ]
    }
  ]
}
