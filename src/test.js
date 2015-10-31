var xsr = require('./xmlsonator.js');

var xml1 = "<xsr:Xmlsonator xmlns:xsr='http://xmlsonator.org/schema'>" +
      "<xsr:Sample name='test1' emptyAttribute='' xsr:ns_id='xmlson'>" +
      "</xsr:Sample>" +
      "</xsr:Xmlsonator>";

var xml2 = "<menu id='file' value='File'> \
  <popup> \
    <menuitem value='New' onclick='CreateNewDoc()' /> \
    <menuitem value='Open' onclick='OpenDoc()' /> \
    <menuitem value='Close' onclick='CloseDoc()' /> \
  </popup> \
</menu>";

var xml3 = '<widget> \
    <debug>on</debug> \
    <window title="Sample Konfabulator Widget"> \
        <name>main_window</name> \
        <width>500</width> \
        <height>500</height> \
    </window> \
    <image src="Images/Sun.png" name="sun1"> \
        <hOffset>250</hOffset> \
        <vOffset>250</vOffset> \
        <alignment>center</alignment> \
    </image> \
    <text data="Click Here" size="36" style="bold"> \
        <name>text1</name> \
        <hOffset>250</hOffset> \
        <vOffset>100</vOffset> \
        <alignment>center</alignment> \
        <onMouseUp> \
            sun1.opacity = (sun1.opacity / 100) * 90; \
        </onMouseUp> \
    </text> \
</widget>';

xml4 = '<menu> \
    <header>Adobe SVG Viewer</header> \
    <item action="Open" id="Open">Open</item> \
    <item action="OpenNew" id="OpenNew">Open New</item> \
    <separator/> \
    <item action="ZoomIn" id="ZoomIn">Zoom In</item> \
    <item action="ZoomOut" id="ZoomOut">Zoom Out</item> \
    <item action="OriginalView" id="OriginalView">Original View</item> \
    <separator/> \
    <item action="Quality" id="Quality">Quality</item> \
    <item action="Pause" id="Pause">Pause</item> \
    <item action="Mute" id="Mute">Mute</item> \
    <separator/> \
    <item action="Find" id="Find">Find...</item> \
    <item action="FindAgain" id="FindAgain">Find Again</item> \
    <item action="Copy" id="Copy">Copy</item> \
    <item action="CopyAgain" id="CopyAgain">Copy Again</item> \
    <item action="CopySVG" id="CopySVG">Copy SVG</item> \
    <item action="ViewSVG" id="ViewSVG">View SVG</item> \
    <item action="ViewSource" id="ViewSource">View Source</item> \
    <item action="SaveAs" id="SaveAs">Save As</item> \
    <separator/> \
    <item action="Help" id="Help">Help</item> \
    <item action="About" id="About">About Adobe CVG Viewer...</item> \
</menu>';

var buffer1 = Buffer(xml1, 'utf-8');
var buffer2 = Buffer(xml2, 'utf-8');
var buffer3 = Buffer(xml3, 'utf-8');
var buffer4 = Buffer(xml4, 'utf-8');

var n = 10000

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer1);

console.timeEnd(n + '-parses');

console.log(jobj);

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer2);

console.timeEnd(n + '-parses');

console.log(jobj);

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer3);

console.timeEnd(n + '-parses');

console.log(jobj);

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer4);

console.timeEnd(n + '-parses');

console.log(jobj);
