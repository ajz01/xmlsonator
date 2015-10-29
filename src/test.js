var xsr = require('xmlsonator');

var xml = "<xsr:Xmlsonator xmlns:xsr='http://xmlsonator.org/schema'>" +
      "<xsr:Sample name='test1' emptyAttribute='' xsr:ns_id='xmlson'>" +
      "</xsr:Sample>" +
      "</xsr:Xmlsonator>";

console.time('n-parses');

for(var i = 0; i < 10000; i++)
  var jobj = xsr.parseXML(Buffer(xml, 'utf-8'));

console.timeEnd('n-parses');

console.log(jobj);
