#ifndef FFC_LIBAV
#define FFC_LIBAV

#if IS_LIBAV

//if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame


/**
 * Convert valid timing fields (timestamps / durations) in a packet from one timebase to another.
 *
 * Timestamps with unknown values (AV_NOPTS_VALUE) will be ignored.
 */
void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb);

/**
 * Allocate an AVFormatContext for an output format.
 *
 * avformat_free_context() can be used to free the context and everything allocated by the framework within it.
 */
int avformat_alloc_output_context2(AVFormatContext **avctx, AVOutputFormat *oformat,
                                   const char *format, const char *filename);


#endif // IS_LIBAV

#endif // FFC_LIBAV