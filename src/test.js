var xsr = require('./xmlsonator.js');

/*var xml = "<xsr:Xmlsonator xmlns:xsr='http://xmlsonator.org/schema'>" +
      "<xsr:Sample name='test1' emptyAttribute='' xsr:ns_id='xmlson'>" +
      "</xsr:Sample>" +
      "</xsr:Xmlsonator>";*/

var xml = "<menu id='file' value='File'> \
  <popup> \
    <menuitem value='New' onclick='CreateNewDoc()' /> \
    <menuitem value='Open' onclick='OpenDoc()' /> \
    <menuitem value='Close' onclick='CloseDoc()' /> \
  </popup> \
</menu>";

var buffer = Buffer(xml, 'utf-8');

console.time('n-parses');

for(var i = 0; i < 10000; i++)
  var jobj = xsr.toJson(buffer);

console.timeEnd('n-parses');

console.log(jobj);
