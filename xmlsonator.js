var xsr = require('xmlsonator');

exports.toJson = function(buffer) {
  return xsr.parseXML(buffer);
}
