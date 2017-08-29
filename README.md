# motionVector

[FFmpeg](https://ffmpeg.org) (libavutil & libavformat) based Video Motion Vector analysis for node js

## Installation

```
sudo apt-get install libavutil-dev
npm install motion-vector
```
## Warning

This is a personnal project, not working on all platform, not tested deeply, provide your help if you want to use it in production environment.

## Dependencies

To build this pacakge, you should have dev packages for libavutil & libavformat installed.

## Usage

```node
var motionVector = require("./build/Release/motion-vector.node");

motionVector.getVector("input.m4v", [42], function(a,b,c,f){
  if(a.vectors){

  } else {
    console.log(a.framenum);//42
  }
});
```

## Contribute

* Add an issue
* Add a Pull request
