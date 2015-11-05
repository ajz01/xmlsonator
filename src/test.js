Object.keys(require.cache).forEach(function(key) { delete require.cache[key] });
var xsr = require('./xmlsonator.js');
var fs = require('fs');

var xml = "<menu id='file' value='File'> \
  <popup> \
    <menuitem value='New' onclick='CreateNewDoc()' /> \
    <menuitem value='Open' onclick='OpenDoc()' /> \
    <menuitem value='Close' onclick='CloseDoc()' /> \
  </popup> \
</menu>";

var file1 = 'xml/xml1';
var file2 = 'xml/xml2';
var file3 = 'xml/xml3';
var file4 = 'xml/xml4';
var file5 = 'xml/xml5';
var file6 = 'xml/xml6';
var file7 = 'xml/xml7';
var file8 = 'xml/xml8';
var file9 = 'xml/xml9';
var file10 = 'xml/xml10';
var file11 = 'xml/xml11';
var file12 = 'xml/xml12';

// number of times to convert xml to json Object
var n = 1;

function testFile(file) {
  fs.readFile(file, function(err, data) {
    if(err)
      throw err;

    test(data);
  });
}

// performance test
function test(buffer) {
  //console.log('\n**** Start Test ****\n');

  //console.time(n + '-parses');

  for(var i = 0; i < n; i++)
    var jobj = xsr.toJson(buffer);

  //console.timeEnd(n + '-parses');

  //console.log("\njson Object\n\n");

  //console.log(jobj);

  var jstr = JSON.stringify(jobj);

  //console.log("\njson string\n\n");

  console.log('result: ' + jstr);

  //console.log('\n**** End Test ****\n');
}

/*var buffer = Buffer(xml, 'utf-8');

var n = 10000;

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer);

console.timeEnd(n + '-parses');

//console.log(jobj);

console.log(JSON.stringify(jobj));*/

//testFile(file1);
//testFile(file2);
//testFile(file3);
//testFile(file4);
//testFile(file5);
testFile(file6);
testFile(file7);
testFile(file8);
testFile(file9);
testFile(file10);
testFile(file11);
testFile(file12);
