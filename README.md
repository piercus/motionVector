# motionVector
Video Motion Vector analysis for node js

## Installation


## Usage

var motionVector = require("./build/Release/motion-vector.node");

motionVector.getVector("input.m4v", function(a,b,c,f){
    console.log(a,b,c,f);
});
