#ifndef __RINGBUF_H_
#define __RINGBUF_H_

class RingBuf
{
private:
	char buf[16];
	unsigned char len;
	unsigned char offset;
public:
	RingBuf();
	bool put(char ch);
	char fetch();
	bool isEmpty() const { return len == 0; }
	bool isFull() const { return len == sizeof(buf); }
};

#endif // __RINGBUF_H_
