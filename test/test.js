
var mVec = require('../build/Release/motion-vector.node')

var fps = 30;
var duration = 60;

var start = new Date();
var frames = [];
for (var i =0; i < duration; i++){
  frames.push(i*fps)
}

mVec.getVector("./input.m4v", frames, function(a,b,c,f){
  var end = new Date();
  //console.log(end-start);
  if(a.vectors){

    //console.log(a.vectors, a.vectors.length)
    //var types = {};
    var sum = 0;
    var values = {
      '16x16' : 0,
      '8x8' : 1,
      '16x8' : 0.2,
      '8x16' : 0.2,
    };
    a.vectors.forEach(function(v){
      var type = v.w+"x"+v.h;
      sum+=values[type];
    })

    console.log("hasVectors", a.framenum/fps, sum);
  } else {
    console.log(a.framenum);
  }
})


console.log("ok")
