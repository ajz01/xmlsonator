var xsr = require('../lib/build/Release/xmlsonator');

exports.parseXML = function(buffer) {
  return xsr.parseXML(buffer);
}
