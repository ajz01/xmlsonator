var xsr = require('./build/Release/xmlsonator');

exports.toJson = function(buffer) {
  return xsr.toJson(buffer);
}
