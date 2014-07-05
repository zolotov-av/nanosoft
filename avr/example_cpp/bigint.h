#ifndef __BIGINT_H_
#define __BIGINT_H_

class BigInt
{
public:
	static const int maxlen = 16;
private:
	unsigned char num[maxlen];
public:
	BigInt& operator += (const BigInt &b);
	BigInt& operator -= (const BigInt &b);
	const unsigned char* c_str() const { return num; }
	unsigned char operator [] (int i) const { return num[i]; }
	int count() const { return maxlen; }
	BigInt& operator = (unsigned char b);
};

#endif // __BIGINT_H_
