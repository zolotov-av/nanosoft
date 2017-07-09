
#include <ffcairo/ffcimage.h>

#include <stdio.h>

/**
* Конструктор
*
* Автоматически создает и настраивает буфер для совместной
* работы FFMPEG и Cairo
*/
FFCImage::FFCImage(int w, int h)
{
	width = w;
	height = h;
	stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
	int size = stride * height;
	
	int av_size = avpicture_get_size(AV_PIX_FMT_BGRA, width, height);
	printf("cairo size=%d, ffmpeg size=%d\n", size, av_size);
	if ( av_size > size ) size = av_size;
	printf("actual size: %d\n", size);

	data = (uint8_t *)av_malloc( size * sizeof(uint8_t) );
	
	surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, width, height, stride);
	if ( cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS )
	{
		printf("cairo_image_surface_create_for_data failed\n");
	}
	
	// Allocate an AVFrame structure
	avFrame = av_frame_alloc();
	if( avFrame == NULL )
	{
		printf("avFrame alloc failed\n");
		
	}
	
	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)avFrame, data, AV_PIX_FMT_BGRA, width, height);
}

/**
* Деструктор
*
* Автоматически высвобождает буфер
*/
FFCImage::~FFCImage()
{
	av_free(data);
}

/**
* Сохранить в файл в формате PNG
*/
bool FFCImage::savePNG(const char *fname) const
{
	cairo_surface_write_to_png(surface, fname);
}
