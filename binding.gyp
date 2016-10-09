{
  "targets": [
    {
      "target_name": "motion-vector",
      "sources": [ "lib/extract_mvs.cc" ],
      "cflags" : [
        "-I/usr/include/x86_64-linux-gnu",
        "-Wall",
        "-g",
        "-lavdevice-ffmpeg",
        "-lavformat-ffmpeg",
        "-lavfilter-ffmpeg",
        "-lavcodec-ffmpeg",
        "-lswresample-ffmpeg",
        "-lswscale-ffmpeg",
        "-lavutil-ffmpeg"
      ]
    }
  ]
}
