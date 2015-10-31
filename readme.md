# xmlsonator
A high speed xml to json converter built on libxml2.

Benchmark shows potentially 20 times faster than other popular xml to json packages.

# Prerequisites
install libxml2. If you don't already have it.

[libxml](http://www.xmlsoft.org)

apt-get install libxml2

# Usage

## Installation
>npm install xmlsonator  

## API
This project is just getting started and is not tested with all valid xml.

Please wait until release is 1.0 or > before using.

Better yet, look for the github repo and contribute.

Currently, test cases are being developed to check for both correctness

and performance.

## Example program with performance results

'''javascript
var xsr = require('xmlsonator');

var xml = "<menu id='file' value='File'> \
  <popup> \
    <menuitem value='New' onclick='CreateNewDoc()' /> \
    <menuitem value='Open' onclick='OpenDoc()' /> \
    <menuitem value='Close' onclick='CloseDoc()' /> \
  </popup> \
</menu>";

var buffer = Buffer(xml, 'utf-8');

var n = 10000

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer);

console.timeEnd(n + '-parses');

console.log(jobj);
'''
