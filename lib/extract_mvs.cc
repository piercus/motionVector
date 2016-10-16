
/*
 * Copyright (c) 2012 Stefano Sabatini
 * Copyright (c) 2014 Clément Bœsch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <node.h>
#include <v8.h>
#include <iostream>
#include <nan.h>

extern "C" {

#include <libavutil/motion_vector.h>
#include <libavformat/avformat.h>

}
namespace jsscope {

  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Array;
  using v8::Handle;
  using v8::Null;
  using v8::Object;
  using v8::String;
  using v8::Value;
  using v8::Number;
  using namespace std;

  static AVFormatContext *fmt_ctx = NULL;
  static AVCodecContext *video_dec_ctx = NULL;
  static AVStream *video_stream = NULL;
  static const char *src_filename = NULL;

  static int video_stream_idx = -1;
  static AVFrame *frame = NULL;
  static AVPacket pkt;
  static int video_frame_count = 0;

  static int decode_packet(int *got_frame, int cached, v8::Local<v8::Function> cb, Isolate* isolate)
  {
      int decoded = pkt.size;

      *got_frame = 0;

      if (pkt.stream_index == video_stream_idx) {
          int ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
          if (ret < 0) {
              //fprintf(stderr, "Error decoding video frame (%s)\n", av_err2str(ret));
              return ret;
          }

          if (*got_frame) {
              int i;
              AVFrameSideData *sd;

              video_frame_count++;
              sd = av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
              if (sd) {
                  const AVMotionVector *mvs = (const AVMotionVector *)sd->data;
                  int size = sd->size / sizeof(*mvs);
                  Handle<Array> array = Array::New(isolate, size);

                  for (i = 0; i < size; i++) {
                      const AVMotionVector *mv = &mvs[i];

                      // "framenum,source,blockw,blockh,srcx,srcy,dstx,dsty,flags\n"

                      Local<Object> objItem = Object::New(isolate);

                      objItem->Set(String::NewFromUtf8(isolate, "w"),  Number::New(isolate, mv->w));
                      objItem->Set(String::NewFromUtf8(isolate, "h"),  Number::New(isolate, mv->h));
                      objItem->Set(String::NewFromUtf8(isolate, "srcx"),  Number::New(isolate, mv->src_x));
                      objItem->Set(String::NewFromUtf8(isolate, "srcy"),  Number::New(isolate, mv->src_y));
                      objItem->Set(String::NewFromUtf8(isolate, "dstx"),  Number::New(isolate, mv->dst_x));
                      objItem->Set(String::NewFromUtf8(isolate, "dsty"),  Number::New(isolate, mv->dst_y));
                      objItem->Set(String::NewFromUtf8(isolate, "flags"),  Number::New(isolate, mv->flags));

                      array->Set(i, objItem);

                      //printf("%d,%2d,%2d,%2d,%4d,%4d,%4d,%4d,0x%"PRIx64"\n",
                      //       video_frame_count, mv->source,
                      //       mv->w, mv->h, mv->src_x, mv->src_y,
                      //       mv->dst_x, mv->dst_y, mv->flags);
                  }

                  Local<Object> obj = Object::New(isolate);

                  obj->Set(String::NewFromUtf8(isolate, "framenum"), Number::New(isolate, video_frame_count));
                  obj->Set(String::NewFromUtf8(isolate, "vectors"), array);
                  const unsigned argc = 1;

                  Local<Value> argv[argc] = { obj };

                  cb->Call(Null(isolate), argc, argv);
              } else {
                Local<Object> obj = Object::New(isolate);
                obj->Set(String::NewFromUtf8(isolate, "framenum"), Number::New(isolate, video_frame_count));

                const unsigned argc = 1;

                Local<Value> argv[argc] = { obj };
                cb->Call(Null(isolate), argc, argv);
              }
          }
      }

      return decoded;
  }

  static int open_codec_context(int *stream_idx,
                                AVFormatContext *fmt_ctx, enum AVMediaType type)
  {
      int ret;
      AVStream *st;
      AVCodecContext *dec_ctx = NULL;
      AVCodec *dec = NULL;
      AVDictionary *opts = NULL;

      ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
      if (ret < 0) {
          fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                  av_get_media_type_string(type), src_filename);
          return ret;
      } else {
          *stream_idx = ret;
          st = fmt_ctx->streams[*stream_idx];

          /* find decoder for the stream */
          dec_ctx = st->codec;
          dec = avcodec_find_decoder(dec_ctx->codec_id);
          if (!dec) {
              fprintf(stderr, "Failed to find %s codec\n",
                      av_get_media_type_string(type));
              return AVERROR(EINVAL);
          }

          /* Init the video decoder */
          av_dict_set(&opts, "flags2", "+export_mvs", 0);
          if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
              fprintf(stderr, "Failed to open %s codec\n",
                      av_get_media_type_string(type));
              return ret;
          }
      }

      return 0;
  }

  int c_function(v8::Local<v8::String> src_filename_in, v8::Local<v8::Function> cb, Isolate* isolate)
  {
      int ret = 0, got_frame;

      av_register_all();

      v8::String::Utf8Value filename2(src_filename_in->ToString());
      src_filename = std::string(*filename2).c_str();


      if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
          fprintf(stderr, "Could not open source file %s\n", src_filename);
          exit(1);
      }

      if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
          fprintf(stderr, "Could not find stream information\n");
          exit(1);
      }

      if (open_codec_context(&video_stream_idx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
          video_stream = fmt_ctx->streams[video_stream_idx];
          video_dec_ctx = video_stream->codec;
      }

      av_dump_format(fmt_ctx, 0, src_filename, 0);

      if (!video_stream) {
          fprintf(stderr, "Could not find video stream in the input, aborting\n");
          ret = 1;
          avcodec_close(video_dec_ctx);
          avformat_close_input(&fmt_ctx);
          av_frame_free(&frame);
          return ret < 0;
      }

      frame = av_frame_alloc();
      if (!frame) {
          fprintf(stderr, "Could not allocate frame\n");
          ret = AVERROR(ENOMEM);
          avcodec_close(video_dec_ctx);
          avformat_close_input(&fmt_ctx);
          av_frame_free(&frame);
          return ret < 0;
      }

      //printf("framenum,source,blockw,blockh,srcx,srcy,dstx,dsty,flags\n");

      /* initialize packet, set data to NULL, let the demuxer fill it */
      av_init_packet(&pkt);
      pkt.data = NULL;
      pkt.size = 0;

      /* read frames from the file */
      while (av_read_frame(fmt_ctx, &pkt) >= 0) {
          AVPacket orig_pkt = pkt;
          do {
              ret = decode_packet(&got_frame, 0, cb, isolate);
              if (ret < 0)
                  break;
              pkt.data += ret;
              pkt.size -= ret;
          } while (pkt.size > 0);
          av_packet_unref(&orig_pkt);
      }

      /* flush cached frames */
      pkt.data = NULL;
      pkt.size = 0;
      do {
          decode_packet(&got_frame, 1, cb, isolate);
      } while (got_frame);

  //end:
      avcodec_close(video_dec_ctx);
      avformat_close_input(&fmt_ctx);
      av_frame_free(&frame);
      return ret < 0;
  }




  void GetVectors(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    //Isolate* isolate = args.GetIsolate();
    //HandleScope scope(isolate);

    Isolate* isolate = info.GetIsolate();
    Local<Function> cb = info[1].As<v8::Function>();
    Local<String> str = info[0]->ToString();
    //Local<String> filename = Local<String>::Cast(args[0]);
    //NanSymbol(args[0])
    //Handle<Value> filename = String::New( args[0].c_str() );

    info.GetReturnValue().Set(c_function(str, cb, isolate));
  }

  void Init(Local<Object> exports, Local<Object> module) {
    //NODE_SET_METHOD(module, "exports", GetVectors);
    exports->Set(Nan::New("getVector").ToLocalChecked(),
                   Nan::New<v8::FunctionTemplate>(GetVectors)->GetFunction());
  }

  NODE_MODULE(binding, Init);
}
