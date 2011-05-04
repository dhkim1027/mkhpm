#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hexfile.h"

#define MAX_BUF_SIZ	128

int
convert_hex( char *txt, int len)
{
	int result = 0;
	int digit;
	int i;

	if( txt == NULL ){
		printf( "Cannot convert zero-length hex-string to number!\n" );
		return -1;
	}

	if( len > 8 ){
		printf( "Hex conversion overflow! Too many hex digits in string.\n" );
		return -1;
	}


	for( i = 0; i < len; i++ )
	{
		/* Convert hex digit */
		if( txt[i] >= '0' && txt[i] <= '9' )
			digit = txt[i] - '0';
		else if( txt[i] >= 'a' && txt[i] <= 'f' )
			digit = txt[i] - 'a' + 10;
		else if( txt[i] >= 'A' && txt[i] <= 'F' )
			digit = txt[i] - 'A' + 10;
		else{
			printf( "Invalid hex digit found!\n" );
			return -1;
		}

		/* Add digit as least significant 4 bits of result */
		result = (result << 4) | digit;
	}

	return result;

}


int
hexfile_parse_record(char *read_line, hex_record_t *recp)
{
	unsigned char chksum;
	unsigned int record_pos;
	int read_line_size = 0;

	if(read_line == NULL){
		printf("read line is NULL\n");
		return -1;
	}

	read_line_size = strlen(read_line);

	if(read_line[0] != ':'){
		printf("Wrong HEX file format, does not start with colon! \n");
		return -1;
	}

	recp->length = convert_hex(strndup(read_line+1, 2), 2);
	recp->offset = convert_hex(strndup(read_line+3, 4), 4);
	recp->type = convert_hex(strndup(read_line+7, 2), 2);

	if(read_line_size < (11+recp->length*2) ){
		printf("Wrong HEX file format, missing fields!\n");
		return -1;
	}

	chksum = recp->length;
	chksum += (unsigned char) ((recp->offset >> 8) & 0xff);
	chksum += (unsigned char) (recp->offset & 0xff);
	chksum += recp->type;

	if( recp->length )
	{
		recp->data = (unsigned char*)malloc(recp->length);
		for( record_pos = 0; record_pos < recp->length; record_pos++ )
		{
			recp->data[ record_pos ] = convert_hex( strndup( read_line + 9 + record_pos*2, 2 ), 2);
			chksum += recp->data[ record_pos ];
		}
	}

	chksum += convert_hex( strndup( read_line + 9 + recp->length*2, 2 ), 2);
	if( chksum != 0 )
	{
		printf( "Wrong checksum for HEX record! \n");
		return -1;
	}

	return 0;
}


int
hexfile_read_file(hexfile_t *hex, char *filename)
{
	FILE *fp = NULL;
	int base_address, data_pos;
	char read_line[MAX_BUF_SIZ];
	hex_record_t rec;
	int size = HEX_BUF_SIZ;
	int line_number = 0;


	fp = fopen(filename, "r");
	if(!fp){
		perror(filename);
		return -1;
	}

	rec.data = NULL;
	base_address = 0;
	hex->start = size;
	hex->end = 0;

	memset(read_line, 0, MAX_BUF_SIZ);
	while(fgets(read_line, MAX_BUF_SIZ-1, fp)){
		//printf("#");

		printf("%06x : %s\n", line_number++, read_line);

		if( hexfile_parse_record(read_line, &rec) < 0){
			fclose(fp);
			return -1;
		}

		switch(rec.type){
			case 0x00 : // Data record ?
				/* Copy data */
				if( base_address + rec.offset + rec.length > size ){
					printf("HEX file defines data outside buffer limits! \n");
					printf("base:%d, length:%d offset:%d , size:%d\n", base_address, rec.length, rec.offset, size);
					return -1;
				}
				//printf("base:0x%x, length:%d offset:%d type:0x%x:", base_address, rec.length, rec.offset, rec.type);
				for( data_pos = 0; data_pos < rec.length; data_pos++ ){
					hex->data[ base_address + rec.offset + data_pos ] = rec.data[ data_pos ];
				//	printf("%02x ", rec.data[ data_pos ]);
				}
				//printf("\n");
				if(rec.data != NULL)
					free(rec.data);

				/* Update byte usage */
				if( base_address + rec.offset < hex->start )
					hex->start = base_address + rec.offset;

				if( base_address + rec.offset + rec.length - 1 > hex->end )
					hex->end = base_address + rec.offset + rec.length - 1;
				break;

			case 0x02 : // Extended segment address record ?
				base_address = (rec.data[0] << 8) | rec.data[1];
				base_address <<= 4;
				break;

			case 0x03 : // Start segment address record ?
				break; // Ignore it, since we have no influence on execution start address.

			case 0x04 : // Extended linear address record ?
				base_address = (rec.data[0] << 8) | rec.data[1];
				base_address <<= 16;
				break;

			case 0x05 : // Start linear address record ?
				break; // Ignore it, since we have no influence on exectuion start address.

			case 0x01 : // End of file record ?
				fclose(fp);
				//printf("\n");
				//printf("start:0x%x, end:0s%x\n", hex->start, hex->end);
				return 0;
		}
		memset(read_line, 0, MAX_BUF_SIZ);
	}

	return 0;
}
