# xmlsonator
A high speed xml to json converter built on libxml2.

Benchmark shows potentially 20 times faster than other popular xml to json packages.

# Usage
This project is just getting started so test your specific xml format before using.


There is only one method:

```javascript
Object = xmlsonator.toJson(Buffer);

```

Considering adding an async method using libuv.

## Example program with performance results

```javascript
var xsr = require('../build/Release/xmlsonator');

var xml = "<menu id='file' value='File'> \
  <popup> \
    <menuitem value='New' onclick='CreateNewDoc()' /> \
    <menuitem value='Open' onclick='OpenDoc()' /> \
    <menuitem value='Close' onclick='CloseDoc()' /> \
  </popup> \
</menu>";

var buffer = Buffer(xml, 'utf-8');

var n = 10000;

console.time(n + '-parses');

for(var i = 0; i < n; i++)
  var jobj = xsr.toJson(buffer);

console.timeEnd(n + '-parses');

console.log(jobj);
```

## Results
```
10000-parses: 479ms
{ menu: { id: 'file', value: 'File', popup: { menuitem: [Object] } } }
```
