var xsr = require('./xmlsonator.js');

/*var xml = "<xsr:Xmlsonator xmlns:xsr='http://xmlsonator.org/schema'>" +
      "<xsr:Sample name='test1' emptyAttribute='' xsr:ns_id='xmlson'>" +
      "</xsr:Sample>" +
      "</xsr:Xmlsonator>";*/

/*var xml = "<menu id='file' value='File'> \
  <popup> \
    <menuitem value='New' onclick='CreateNewDoc()' /> \
    <menuitem value='Open' onclick='OpenDoc()' /> \
    <menuitem value='Close' onclick='CloseDoc()' /> \
  </popup> \
</menu>";*/

var xml = '<widget> \
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

var buffer = Buffer(xml, 'utf-8');

var n = 10000

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer);

console.timeEnd(n + '-parses');

console.log(jobj);
