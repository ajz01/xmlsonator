var xsr = require('../lib/build/Release/xmlsonator');

var xml = "<xsr:Xmlsonator xmlns:xsr='http://xmlsonator.org/schema'>"
      "<xsr:Sample name='test1' emptyAttribute='' xsr:ns_id='auio'>"
      "</xsr:Sample>"
      "</xsr:EmptyElem>";

xsr.parseXML(Buffer(xml, 'utf-8'));
