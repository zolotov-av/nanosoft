/****************************************************************************

Демо-программа №2

Демонстрирует возможности создания видео из отдельных кадров

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/ffcmuxer.h>
#include <math.h>

void DrawPic(ptr<FFCImage> pic, int iFrame)
{
	// создаем контекст рисования Cairo
	cairo_t *cr = cairo_create(pic->surface);
	
	int width = pic->width;
	int height = pic->height;
	
	int cx = width / 2;
	int cy = height / 2;
	
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
	
	double r0 = (width > height ? height : width) / 2.0;
	double r1 = r0 * 0.75;
	double r2 = r0 * 0.65;
	
	cairo_set_line_width(cr, r0 * 0.04);
	
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_arc(cr, cx, cy, r1, 0, 2 * M_PI);
	cairo_stroke (cr);
	
	double x, y;
	const double k = M_PI * (2.0 / 250);
	const double f = - M_PI / 2.0;
	x = r2 * cos(iFrame * k + f) + cx;
	y = r2 * sin(iFrame * k + f) + cy;
	cairo_move_to(cr, cx, cy);
	cairo_line_to(cr, x, y);
	cairo_stroke (cr);
	
	char sFrameId[48];
	int t = iFrame / 25;
	int sec = t % 60;
	int min = t / 60;
	sprintf(sFrameId, "Time: %02d:%02d", min, sec);
	
	double hbox = pic->height * 0.1;
	double shift = pic->height * 0.05;
	
	cairo_set_source_rgba (cr, 0x5a /255.0, 0xe8/255.0, 0xf9/255.0, 96/255.0);
	cairo_rectangle (cr, shift, pic->height - hbox - shift, pic->width - 2*shift, hbox);
	cairo_fill (cr);
	
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, hbox * 0.8);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	
	cairo_move_to (cr, shift*1.5, pic->height - shift*1.5);
	cairo_show_text (cr, sFrameId);
	
	// Освобождаем контекст рисования Cairo
	cairo_destroy(cr);
}

int main(int argc, char *argv[])
{
	const char *fname = argc > 1 ? argv[1] : "out.avi";
	printf("output filename = %s\n", fname);
	
	const int width = 1280;
	const int height = 720;
		
	// INIT
	av_register_all();
	
	ptr<FFCMuxer> muxer = new FFCMuxer();
	ptr<FFCImage> pic = new FFCImage(width, height);
	
	if ( ! muxer->createFile(fname) )
	{
		printf("failed to createFile(%s)\n", fname);
		return -1;
	}
	
	AVFormatContext *avFormatCtx = muxer->avFormat;
	AVOutputFormat *oformat = avFormatCtx->oformat;
	printf("oformat->name: %s\n", oformat->name);
	printf("oformat->long_name: %s\n", oformat->long_name);
	
	// add video stream
	AVCodecID video_codec = muxer->defaultVideoCodec();
	if ( video_codec == AV_CODEC_ID_NONE)
	{
		printf("video_codec = NONE\n");
		return -1;
	}
	
	AVStream *avStream = muxer->createStream(video_codec);
	if ( !avStream < 0 )
	{
		printf("fail to create video stream\n");
		return -1;
	}
	
	AVCodecContext *avCodecCtx = avStream->codec;
	avCodecCtx->codec_id = oformat->video_codec;
	avCodecCtx->bit_rate = 2000000;
	avCodecCtx->width    = width;
	avCodecCtx->height   = height;
	/* timebase: This is the fundamental unit of time (in seconds) in terms
	 * of which frame timestamps are represented. For fixed-fps content,
	 * timebase should be 1/framerate and timestamp increments should be
	 * identical to 1. */
	avStream->time_base = (AVRational){ 1, 25 };
	avCodecCtx->time_base       = avStream->time_base;
	avCodecCtx->gop_size = 12; /* emit one intra frame every twelve frames at most */
	avCodecCtx->pix_fmt = PIX_FMT_YUV420P;
	if ( avCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO )
	{
		/* just for testing, we also add B frames */
		avCodecCtx->max_b_frames = 2;
	}
	if ( avCodecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO )
	{
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		 * This does not happen with normal video, it just happens here as
		 * the motion of the chroma plane does not match the luma plane. */
		avCodecCtx->mb_decision = 2;
	}
	
	// open video stream
	
	int ret = avcodec_open2(avCodecCtx, NULL, NULL);
	if ( ret < 0 )
	{
		printf("avcodec_open2() failed\n");
		return -1;
	}
	
	AVFrame *avFrame = av_frame_alloc();
	if ( !avFrame )
	{
		printf("av_frame_alloc() failed\n");
		return -1;
	}
	avFrame->format = avCodecCtx->pix_fmt;
	avFrame->width = avCodecCtx->width;
	avFrame->height = avCodecCtx->height;
	
	/* allocate the buffers for the frame data */
	ret = avpicture_alloc((AVPicture *)avFrame, AV_PIX_FMT_YUV420P, avFrame->width, avFrame->height);
	if ( ret < 0 )
	{
		printf("avpicture_alloc() failed\n");
		return -1;
	}
	
	av_dump_format(avFormatCtx, 0, fname, 1);
	
	SwsContext *sws_ctx = sws_getContext(width, height, PIX_FMT_BGRA,
		width, height, avCodecCtx->pix_fmt, SWS_BILINEAR,
		NULL, NULL, NULL);
	
	if ( ! muxer->openFile(fname) )
	{
		printf("fail to openFile(%s)\n", fname);
		return -1;
	}
	
	int frameNo = 0;
	while ( 1 )
	{
		frameNo++;
		if ( frameNo > (25 * 30) ) break;
		
		AVPacket pkt = { 0 };
		int got_packet = 0;
		av_init_packet(&pkt);
		
		DrawPic(pic, frameNo);
		
		sws_scale(sws_ctx,
			pic->avFrame->data, pic->avFrame->linesize,
			0, height,
			avFrame->data, avFrame->linesize);
		
		/* encode the image */
		ret = avcodec_encode_video2(avCodecCtx, &pkt, avFrame, &got_packet);
		if ( ret < 0 )
		{
			printf("frame[%d] avcodec_encode_video2() failed\n", frameNo);
			break;
		}
		
		if ( got_packet )
		{
			// write frame
			
			/* rescale output packet timestamp values from codec to stream timebase */
			av_packet_rescale_ts(&pkt, avCodecCtx->time_base, avStream->time_base);
			pkt.stream_index = avStream->index;
			
			/* Write the compressed frame to the media file. */
			//log_packet(avFormatCtx, &pkt);
			
			ret = av_interleaved_write_frame(avFormatCtx, &pkt);
		} else {
			ret = 0;
		}
	}
	
	muxer->closeFile();
	
	return 0;
}
