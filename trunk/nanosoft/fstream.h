#ifndef NANO_FSTREAM_H
#define NANO_FSTREAM_H

#include <sys/types.h>
#include <fcntl.h>
#include <nanosoft/stream.h>

namespace nanosoft
{
	
	class fstream: public stream
	{
	private:
		int fd;
	public:
		enum {
			ro = O_RDONLY,
			rw = O_RDWR
		};
		
		fstream(int afd = 0);
		fstream(const char *fileName, int flags);
		fstream(const char *fileName, int flags, mode_t mode);
		bool open(const char *fileName, int flags);
		bool open(const char *fileName, int flags, mode_t mode);
		void bind(int afd = 0);
		void close();
		virtual ~fstream();
		virtual int read(void *buffer, size_t size);
		virtual int write(const void *buffer, size_t size);
	};
	
}

#endif // NANO_FSTREAM_H
