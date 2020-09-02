#ifndef MD5_H
#define MD5_H

#if defined(SIMPLE_MD5SUM)
typedef unsigned int guint32;
#endif

typedef struct MD5Context {
	guint32 buf[4];
	guint32 bits[2];
	unsigned char in[64];
} MD5Context;

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf,
			unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);

void AsciiDigest(char*, unsigned char*);

#endif /* MD5_H */
