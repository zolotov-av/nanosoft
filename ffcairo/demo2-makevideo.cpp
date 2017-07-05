/****************************************************************************

Демо-программа №2

Демонстрирует возможности создания видео из отдельных кадров

****************************************************************************/

#include <ffcairo/config.h>
#include <ffcairo/ffcimage.h>
#include <ffcairo/ffcstream.h>

/**
 * libav почемуто потерял эту функцию...
 */
static void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb)
{
    if (pkt->pts != AV_NOPTS_VALUE)
        pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
    if (pkt->dts != AV_NOPTS_VALUE)
        pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
    if (pkt->duration > 0)
        pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
    if (pkt->convergence_duration > 0)
        pkt->convergence_duration = av_rescale_q(pkt->convergence_duration, src_tb, dst_tb);
FF_ENABLE_DEPRECATION_WARNINGS
#endif
}

int main(int argc, char *argv[])
{
	const char *fname = argc > 1 ? argv[1] : "out.avi";
	printf("output filename = %s\n", fname);
		
	// INIT
	av_register_all();
	
	AVFormatContext *avFormatCtx = avformat_alloc_context();
	
	if ( !avFormatCtx )
	{
		printf("avformat_alloc_context() failed\n");
		return -1;
	}
	
	AVOutputFormat *oformat = av_guess_format(NULL, fname, NULL);
	if ( !oformat )
	{
		printf("av_guess_format() failed\n");
		return -1;
	}
	
	avFormatCtx->oformat = oformat;
	printf("oformat->priv_data_size=%d\n", oformat->priv_data_size);
	
	if (avFormatCtx->oformat->priv_data_size > 0)
	{
		avFormatCtx->priv_data = av_mallocz(avFormatCtx->oformat->priv_data_size);
		if (!avFormatCtx->priv_data) {
			printf("no memory\n");
			return -1;
		}
		if ( avFormatCtx->oformat->priv_class )
		{
			*(const AVClass**)avFormatCtx->priv_data = avFormatCtx->oformat->priv_class;
			//av_opt_set_defaults(avFormatCtx->priv_data);
		}
	} else avFormatCtx->priv_data = NULL;	
	
	av_strlcpy(avFormatCtx->filename, fname, sizeof(avFormatCtx->filename));
	
	
	// add video stream
	
	if ( oformat->video_codec == AV_CODEC_ID_NONE)
	{
		printf("video_codec = NONE\n");
		return -1;
	}
	
	AVCodec *codec = avcodec_find_encoder( oformat->video_codec );
	if ( !codec )
	{
		printf("encoder not found\n");
		return -1;
	}
	
	AVStream *avStream = avformat_new_stream(avFormatCtx, codec);
	if ( ! avStream )
	{
		printf("avformat_new_stream() failed\n");
		return -1;
	}
	
	int stream_id = avFormatCtx->nb_streams - 1;
	printf("stream[%d] created\n", stream_id);
	
	AVCodecContext *avCodecCtx = avStream->codec;
	avCodecCtx->codec_id = oformat->video_codec;
	avCodecCtx->bit_rate = 2000000;
	avCodecCtx->width    = 1280;
	avCodecCtx->height   = 720;
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
	
	int ret = avcodec_open2(avCodecCtx, codec, NULL);
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
	
	/* open the output file, if needed */
	if (!(oformat->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&avFormatCtx->pb, fname, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			fprintf(stderr, "Could not open '%s'\n", fname);
			return -1;
		}
	}
  
	ret = avformat_write_header(avFormatCtx, NULL);
	if ( ret < 0 )
	{
		printf("avformat_write_header() failed\n");
		return -1;
	}
	
	int frameNo = 0;
	while ( 1 )
	{
		frameNo++;
		if ( frameNo > 250 ) break;
		
		AVPacket pkt = { 0 };
		int got_packet = 0;
		av_init_packet(&pkt);
		
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
	/* Write the trailer, if any. The trailer must be written before you
	 * close the CodecContexts open when you wrote the header; otherwise
	 * av_write_trailer() may try to use memory that was freed on
	 * av_codec_close(). */
	av_write_trailer(avFormatCtx);
	
	avformat_free_context(avFormatCtx);
	
	return 0;
}
