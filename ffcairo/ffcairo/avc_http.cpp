
#include <ffcairo/avc_http.h>

#include <ctype.h>
#include <string>

/**
* Конструктор
*/
AVCHttp::AVCHttp(int afd, AVCEngine *e): AsyncStream(afd), engine(e)
{
	http_state = READ_METHOD;
	line.clear();
	done = false;
	peer_down = false;
}

/**
* Деструктор
*/
AVCHttp::~AVCHttp()
{
	printf("AVCHttp::~AVCHttp()\n");
	close();
}

/**
* Обработчик прочитанных данных
*/
void AVCHttp::onRead(const char *data, size_t len)
{
	const char *p = data;
	const char *lim = data + len;
	
	while ( http_state == READ_METHOD || http_state == READ_HEADERS )
	{
		while ( *data != '\n' && data < lim ) data ++;
		line += std::string(p, data);
		if ( data >= lim ) return;
		data++;
		p = data;
		if ( http_state == READ_METHOD )
		{
			method = line;
			printf("method: %s\n", method.c_str());
			http_state = READ_HEADERS;
			line.clear();
		}
		else
		{
			if ( line == "" || line == "\r" )
			{
				http_state = READ_BODY;
			}
			else
			{
				printf("header: %s\n", line.c_str());
				line.clear();
			}
		}
	}
	
	if ( http_state == READ_BODY )
	{
		printf("read body\n");
		http_state = STREAMING;
		initStreaming();
	}
	
	if ( http_state == STREAMING )
	{
	}
}

/**
* Обработчик события опустошения выходного буфера
*
* Вызывается после того как все данные были записаны в файл/сокет
*/
void AVCHttp::onEmpty()
{
	if ( done )
	{
		if ( engine->http == this )
		{
			engine->http = NULL;
		}
		leaveDaemon();
	}
}

/**
* Пир (peer) закрыл поток.
*
* Мы уже ничего не можем отправить в ответ,
* можем только корректно закрыть соединение с нашей стороны.
*/
void AVCHttp::onPeerDown()
{
	peer_down = true;
	printf("AVCHttp::onPeerDown()\n");
	// TODO
	//disableObject();
	if ( engine->http == this )
	{
		engine->http = NULL;
	}
	leaveDaemon();
}


void AVCHttp::write(const std::string &s)
{
	put(s.c_str(), s.size());
}

/**
* Инициализация стриминга
*/
void AVCHttp::initStreaming()
{
	std::string error;
	
	const int width = 1280;
	const int height = 720;
	
	FFCVideoOptions opts;
	opts.width = width;
	opts.height = height;
	opts.pix_fmt = AV_PIX_FMT_YUV420P;
	opts.bit_rate = 2000000;
	opts.time_base = (AVRational){ 1, 25 };
	opts.gop_size = 12;
	
	do
	{
		pic = FFCImage::createImage(width, height);
		if ( pic.getObject() == NULL )
		{
			error = "fail to create FFCImage";;
			break;
		}
		
		muxer = new FFCMuxer();
		
		if ( ! muxer->createContext("avi") )
		{
			error = "failed to createContext()";
			break;
		}
		
		uint8_t *avio_ctx_buffer = NULL;
		size_t avio_ctx_buffer_size = 4096;
		
		avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
		if (!avio_ctx_buffer)
		{
			error = "av_malloc failed";
			break;
		}
	
		avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, this/*user pointer*/, NULL, &write_packet, NULL);
		if ( !avio_ctx )
		{
			error = "avio_alloc_context() failed";
			break;
		}
		muxer->avFormat->pb = avio_ctx;
		
		// add video stream
		AVCodecID video_codec = muxer->defaultVideoCodec();
		if ( video_codec == AV_CODEC_ID_NONE)
		{
			error = "video_codec = NONE";
			break;
		}
		
		opts.codec_id = video_codec;
		
		vo = muxer->createVideoStream(&opts);
		
		if ( ! vo->openEncoder(&opts) )
		{
			error = "openEncoder() failed";
			break;
		}
		
		av_dump_format(muxer->avFormat, 0, 0, 1);
		
		scale = new FFCScale();
		scale->init_scale(vo->avFrame, pic->avFrame);
		
		write("HTTP/1.0 200 OK\r\n");
		write("Content-type: video/x-flv\r\n");
		write("\r\n");
		
		if ( ! muxer->openAVIO() )
		{
			error = "fail to openFile()\n";
			break;
		}
		
		frameNo = 0;
		
		engine->http = this;
		return;
	}
	while ( 0 );
	
	write("HTTP/1.0 200 OK\r\n");
	write("Content-type: text/plain\r\n");
	write("\r\n");
	if ( error != "" )
	{
		write("<error>");
		write(error);
		write("</error>");
	}
	else
	{
		write("<ok>Hello world</ok>");
	}
	done = true;
}

/**
* Обработчик записи пакета в поток
*/
int AVCHttp::write_packet(void *opaque, uint8_t *buf, int buf_size)
{
	AVCHttp *s = (AVCHttp*)opaque;
	if ( s->put((const char *)buf, buf_size) ) return buf_size;
	else return 0;
}

/**
* Временный таймер
*/
void AVCHttp::onTimer()
{
	if ( done )
	{
		printf("onTimer() after done\n");
		return;
	}
	if ( peer_down )
	{
		printf("onTimer() after peer_down\n");
		return;
	}
	//printf("AVCHttp::onTimer(), frameNo: %d\n", frameNo);
	
	for(int i = 0; i < 25; i++)
	{
		frameNo++;
		//printf("FrameNo: %d\n", frameNo);
		
		AVPacket pkt = { 0 };
		pkt.data = NULL;
		pkt.size = 0;
		av_init_packet(&pkt);
		
		int got_packet = 0;
		
		DrawPic();
		
		scale->scale(vo->avFrame, pic->avFrame);
		
		if ( ! vo->encode() )
		{
			printf("frame[%d] encode failed\n", frameNo);
			endStreaming();
			return;
		}
		
		while ( 1 )
		{
			if ( ! vo->recv_packet(&pkt, got_packet) )
			{
				printf("recv_packet() failed\n");
				endStreaming();
				return;
			}
			//printf("recv_packet ok, got_packet = %s\n", (got_packet ? "yes" : "no"));
			
			if ( got_packet )
			{
				//vo->rescale_ts(&pkt);
				
				/* Write the compressed frame to the media file. */
				//log_packet(muxer->avFormat, &pkt);
				if ( muxer->writeFrame(&pkt) )
				{
					//printf("muxer->writeFrame() ok\n");
				}
				else
				{
					printf("muxer->writeFrame() failed\n");
				}
				av_packet_unref(&pkt);
				got_packet = 0;
			}
			else
			{
				break;
			}
		}
	}
}

/**
* прервать передачу
*/
void AVCHttp::endStreaming()
{
	if ( engine->http == this )
	{
		engine->http = NULL;
	}
	done = true;
}

/**
* Отрисовать фрейм
*/
void AVCHttp::DrawPic()
{
	int iFrame = frameNo;
	
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
	
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime(sFrameId, sizeof(sFrameId),"Time %H:%M:%S",timeinfo);
	//sprintf(sFrameId, "Time: %02d:%02d", min, sec);
	
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
