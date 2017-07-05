
#include <ffcairo/config.h>

#ifdef IS_LIBAV

/*****************************************************************************
  libav в Ubuntu по всей видимости битая и в ней отсутствуют некоторые важные
  символы. Этот файл восполняет потерю. Код данных функций почти один в один
  взят из исходников ffmpeg.

  Данный код не претендует на полноценную замену, но он позволяет решить наши
  задачи.
 ****************************************************************************/

/**
 * Convert valid timing fields (timestamps / durations) in a packet from one timebase to another.
 *
 * Timestamps with unknown values (AV_NOPTS_VALUE) will be ignored.
 */
void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb)
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

/**
 * Allocate an AVFormatContext for an output format.
 *
 * avformat_free_context() can be used to free the context and everything allocated by the framework within it.
 */
int avformat_alloc_output_context2(AVFormatContext **avctx, AVOutputFormat *oformat,
                                   const char *format, const char *filename)
{
    AVFormatContext *s = avformat_alloc_context();
    int ret = 0;

    *avctx = NULL;
    if (!s)
        goto nomem;

    if (!oformat) {
        if (format) {
            oformat = av_guess_format(format, NULL, NULL);
            if (!oformat) {
                av_log(s, AV_LOG_ERROR, "Requested output format '%s' is not a suitable output format\n", format);
                ret = AVERROR(EINVAL);
                goto error;
            }
        } else {
            oformat = av_guess_format(NULL, filename, NULL);
            if (!oformat) {
                ret = AVERROR(EINVAL);
                av_log(s, AV_LOG_ERROR, "Unable to find a suitable output format for '%s'\n",
                       filename);
                goto error;
            }
        }
    }

    s->oformat = oformat;
    if (s->oformat->priv_data_size > 0) {
        s->priv_data = av_mallocz(s->oformat->priv_data_size);
        if (!s->priv_data)
            goto nomem;
        if (s->oformat->priv_class) {
            *(const AVClass**)s->priv_data= s->oformat->priv_class;
            av_opt_set_defaults(s->priv_data);
        }
    } else
        s->priv_data = NULL;

    if (filename)
        av_strlcpy(s->filename, filename, sizeof(s->filename));
    *avctx = s;
    return 0;
nomem:
    av_log(s, AV_LOG_ERROR, "Out of memory\n");
    ret = AVERROR(ENOMEM);
error:
    avformat_free_context(s);
    return ret;
}

#endif // IS_LIBAV
