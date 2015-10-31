var xsr = require('../lib/build/Release/xmlsonator');

exports.toJson = function(buffer) {
  return xsr.parseXML(buffer);
}
