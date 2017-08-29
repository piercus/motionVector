var mVec = require('../build/Release/motion-vector.node')

var frames = [232];

mVec.getVector("./input_short.m4v", frames, function(a,b){
  console.log(a,b);
})
