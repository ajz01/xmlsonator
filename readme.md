# xmlsonator
A high speed xml to json converter built on libxml2.

Benchmark shows potentially 20 times faster than other popular xml to json packages.

# Usage
This project is just getting started so test your specific xml format before using.

Currently, test cases are being developed to check for both correctness

and performance.

## Example program with performance results

```javascript
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
```
