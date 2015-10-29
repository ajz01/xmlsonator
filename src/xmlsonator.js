var xsr = require('../lib/build/Release/xmlsonator');

exports.parse = function(buffer) {
  return xsr.parse(buffer);
}
