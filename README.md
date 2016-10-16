# motionVector

[FFmpeg](https://ffmpeg.org) (libavutil & libavformat) based Video Motion Vector analysis for node js

## Installation

```
npm install motion-vector
```
## Dependencies

To build this pacakge, you should have dev packages for libavutil & libavformat installed.

## Current Usage

```node
var motionVector = require("./build/Release/motion-vector.node");

motionVector.getVector("input.m4v", function(a,b,c,f){
  if(a.vectors){

  } else {
    console.log(a.framenum);
  }
});
```
## Future Usage (WIP)

```node
var motionVector = require("motion-vector");


var video = motionVector.open("input_short.m4v");

video.on("frame", function(frame){
  // data is
  // {
  //   framenum : 42
  //   vectors : [{
  //      
  //   }]
  // }
});

video.on("error", function(error){

});

video.start();

```

## Contribute

* Add an issue
* Add a Pull request
