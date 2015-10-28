{
  "targets": [
    {
      "target_name": "xmlsonator",
      "cflags": [
          "-std=c++11"
      ],
      "sources": [ "xmlsonator.cc" ],
      "include_dirs": [
"/usr/include/libxml2", "/usr/include/glibmm-2.4", "/usr/lib/x86_64-linux-gnu/glibmm-2.4/include", "/usr/include/sigc++-2.0", "/usr/lib/x86_64-linux-gnu/sigc++-2.0/include", "/usr/include/glib-2.0", "/usr/lib/x86_64-linux-gnu/glib-2.0/include", "/usr/include/libxml++-2.6", "/usr/lib/libxml++-2.6/include"
	],
      "libraries": [ "-lxml++-2.6", "-lxml2", "-lglibmm-2.4", "-lgobject-2.0", "-lsigc-2.0", "-lglib-2.0", "-L/usr/lib/" ]
    }
  ]
}
