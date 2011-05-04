#ifndef __HEXFILE_H__
#define __HEXFILE_H__

#define HEX_BUF_SIZ		(136*1024)

/* Internal struct for managing HEX records */
typedef struct hex_record // Intel HEX file record
{
	unsigned char length; // Record length in number of data bytes.
	unsigned long offset; // Offset address.
	unsigned char type; // Record type.
	unsigned char * data; // Optional data bytes.
}hex_record_t;

typedef  struct hexfile{
	unsigned int size;
	unsigned int start;
	unsigned int end;
	unsigned char data[HEX_BUF_SIZ];
}hexfile_t;


int hexfile_read_file(hexfile_t *hex, char *filename);


#endif
