var motionVector = require("../build/Release/motion-vector.node");

motionVector.getVector("../input.m4v", function(a,b,c,f){
  if(a.vectors){

  } else {
    console.log(a.framenum);
  }
});
/*

var video = motionVector.open("input_short.m4v");

video.on("frame", function(frame){
  // data is
  // {
  //   framenum : 42
  //   vectors : [{
  //
  //   }]
  // }
  console.log("frame");
});

video.on("error", function(error){

});

video.start();
*/
