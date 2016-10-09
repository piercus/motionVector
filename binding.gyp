{
  "targets": [
    {
      "target_name": "motion-vector",
      "sources": [ "lib/extract_mvs.cc" ],
      "link_settings": {
        "libraries": [
            "-lavformat-ffmpeg"
        ],
      }
    }
  ]
}
#cc -I/usr/include/x86_64-linux-gnu   -Wall -g    src/extract_mvs.c  -lavdevice-ffmpeg -lavformat-ffmpeg -lavfilter-ffmpeg -lavcodec-ffmpeg -lswresample-ffmpeg -lswscale-ffmpeg -lavutil-ffmpeg    -o buile/extract_mvs
#cc lib/extract_mvs.cc -o build/test
