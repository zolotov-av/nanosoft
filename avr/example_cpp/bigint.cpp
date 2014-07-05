
#include "bigint.h"

BigInt& BigInt::operator += (const BigInt &b)
{
	unsigned int sum = 0;
	unsigned char carry = 0;
	for(int i = 0; i < maxlen; i++)
	{
		sum = num[i] + b.num[i] + carry;
		num[i] = sum;
		carry = sum / 256;
	}
	return *this;
}

BigInt& BigInt::operator -= (const BigInt &b)
{
	unsigned int sub = 0;
	unsigned int carry = 0;
	for(int i = 0; i < maxlen; i++)
	{
		sub = num[i] - b.num[i] - carry;
		num[i] = sub;
		carry = (sub / 256) & 1;
	}
}

BigInt& BigInt::operator = (unsigned char b)
{
	num[0] = b;
	for(int i = 1; i < maxlen; i++) num[i] = 0;
	return *this;
}

#ifdef DEBUG_AT_PC

#include <stdio.h>
#include <string.h>


const char * c_str(char *buf, const BigInt &b)
{
	char *p = buf;
	for(int i = b.count()-1; i >= 0; i--)
	{
		p += sprintf(p, "%02X", b[i]);
	}
	return buf;
}

int main()
{
	BigInt a, b;
	a = 0x0;
	b = 0x1;
	char buf[80];
	printf("Hello world\n");
	printf("a = %s\n", c_str(buf, a));
	printf("b = %s\n", c_str(buf, b));
	a -= b;
	printf("a = %s\n", c_str(buf, a));
	b = 4;
	a += b;
	printf("a = %s\n", c_str(buf, a));
	return 0;
}

#endif // DEBUG_AT_PC
