#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hpm.h"
#include "md5.h"
#include "hexfile.h"

#define BUF_SIZE	100

void
help(char *prog)
{
	printf("Usage : %s [src filename] [dst filename] [maj_ver] [min_ver]\n", prog);
}

unsigned char 
HpmfwupgCalculateChecksum(unsigned char* pData, unsigned int length)
{
	unsigned char checksum = 0;
	int dataIdx = 0;

	for ( dataIdx = 0; dataIdx < length; dataIdx++ )
	{
		checksum += pData[dataIdx];
	}
	return checksum;
}

int
main2(int argc, char **argv)
{
	hexfile_t hex;
	
	if(argc < 2){
		help(argv[0]);
		return -1;
	}

	hexfile_read_file(&hex, argv[1]);


	return 0;
}

int
main(int argc, char **argv)
{
	char *src_file, *dst_file;
	FILE *dst_fp;
	unsigned char *dst_file_buf = NULL;
	struct HpmfwupgImageHeader image_header;
	struct HpmfwupgActionRecord action_record;
	struct HpmfwupgFirmwareImage fw_image;
	int dst_file_size = 0;
	unsigned int src_file_size = 0;
	md5_state_t ctx;
	unsigned char md[HPMFWUPG_MD5_SIGNATURE_LENGTH];
	int i;
	char chksum=0;
	int header_size = 0;
	int maj_ver, min_ver;
	hexfile_t hex;

	if(argc < 5){
		help(argv[0]);
		return -1;
	}

	src_file = argv[1];
	dst_file = argv[2];
	maj_ver = atoi(argv[3]);
	min_ver = atoi(argv[4]);

	printf("Reading Hex input file.....\n");
	if(hexfile_read_file(&hex, argv[1]) < 0){
		return -1;
	}
	src_file_size = hex.end - hex.start + 1;

	printf("Hex file start:0x%x, end:0x%x, size:%d\n", hex.start, hex.end, src_file_size);

	dst_file_size += src_file_size;
	dst_file_size += sizeof(struct HpmfwupgImageHeader);
	dst_file_size += 1;
	dst_file_size += sizeof(struct HpmfwupgActionRecord);
	dst_file_size += sizeof(struct HpmfwupgActionRecord);
	dst_file_size += sizeof(struct HpmfwupgFirmwareImage);

	dst_file_buf = (unsigned char*)malloc(sizeof(char)*dst_file_size);
	if(!dst_file_buf){
		perror("malloc");
		return -1;
	}

	dst_fp = fopen(dst_file, "w");
	if(!dst_fp){
		perror(dst_file);
		return -1;
	}

//	printf("sizeof (heaer) : %d\n", sizeof(struct HpmfwupgImageHeader))+
//	printf("src file size : %d\n", dst_file_size);

	// HPM Header
	memset(&image_header, 0, sizeof(struct HpmfwupgImageHeader));
	strcpy(image_header.signature, HPMFWUPG_IMAGE_SIGNATURE);
	image_header.formatVersion = HPMFWUPG_IMAGE_HEADER_VERSION;
	image_header.oemDataLength = 0;
	image_header.deviceId = 0;
	image_header.prodId[0] = 0x01;
	image_header.prodId[1] = 0x00;
	image_header.manId[0] = 0xf9;
	image_header.manId[1] = 0x19;
	image_header.manId[2] = 0x00;
	
	memcpy(dst_file_buf, &image_header, sizeof(struct HpmfwupgImageHeader));
	header_size += sizeof(struct HpmfwupgImageHeader);

	// HPM Header Checksum
	chksum = HpmfwupgCalculateChecksum((unsigned char*)dst_file_buf, sizeof(struct HpmfwupgImageHeader)+1);
	chksum = -chksum;
	memcpy(&dst_file_buf[header_size], &chksum, 1);
	header_size += 1;

	chksum = HpmfwupgCalculateChecksum((unsigned char*)dst_file_buf, sizeof(struct HpmfwupgImageHeader)+image_header.oemDataLength+1);
	//printf("chksum:%d\n", chksum);

	// Action Record Prepare Comp
	memset(&action_record, 0, sizeof(struct HpmfwupgActionRecord));
	action_record.actionType = HPMFWUPG_ACTION_BACKUP_COMPONENTS;
	action_record.components.ComponentBits.byte = 0x01;
	chksum = HpmfwupgCalculateChecksum((unsigned char*)&action_record, sizeof(struct HpmfwupgActionRecord)-1);
	chksum = -chksum;
	action_record.checksum = chksum;

	memcpy(&dst_file_buf[header_size], &action_record, sizeof(struct HpmfwupgActionRecord));
	header_size += sizeof(struct HpmfwupgActionRecord);

	// Action Record Upload Firmware
	memset(&action_record, 0, sizeof(struct HpmfwupgActionRecord));
	action_record.actionType = HPMFWUPG_ACTION_UPLOAD_FIRMWARE;
	action_record.components.ComponentBits.byte = 0x01;
	chksum = HpmfwupgCalculateChecksum((unsigned char*)&action_record, sizeof(struct HpmfwupgActionRecord)-1);
	chksum = -chksum;
	action_record.checksum = chksum;

	memcpy(&dst_file_buf[header_size], &action_record, sizeof(struct HpmfwupgActionRecord));
	header_size += sizeof(struct HpmfwupgActionRecord);

	// Firmware Image
	memset(&fw_image, 0, sizeof(struct HpmfwupgFirmwareImage));

	fw_image.length[0] = src_file_size & 0xff;
	fw_image.length[1] = (src_file_size >> 8 ) & 0xff;
	fw_image.length[2] = (src_file_size >> 16) & 0xff;
	fw_image.length[3] = (src_file_size >> 24) & 0xff;

	strcpy(fw_image.desc, "IPMC F/W");
	fw_image.version[0] = maj_ver;
	fw_image.version[1] = min_ver;
	
	memcpy(&dst_file_buf[header_size], &fw_image, sizeof(struct HpmfwupgFirmwareImage));
	header_size += sizeof(struct HpmfwupgFirmwareImage);

	// Copy to Destinateion File
	memcpy(&dst_file_buf[header_size], &hex.data[hex.start], src_file_size);
//	fread(&dst_file_buf[header_size], sizeof(char), src_file_size, src_fp);

	// Write Dest File
	fwrite(dst_file_buf, 1, dst_file_size, dst_fp);

	// MD5
	memset(md, 0, HPMFWUPG_MD5_SIGNATURE_LENGTH);
	memset(&ctx, 0, sizeof(md5_state_t));
	md5_init(&ctx);
	md5_append(&ctx, dst_file_buf, dst_file_size);
	md5_finish(&ctx, md);


	fwrite(md, 1, HPMFWUPG_MD5_SIGNATURE_LENGTH, dst_fp);

#if 0
	for(i=0;i<HPMFWUPG_MD5_SIGNATURE_LENGTH;i++)
		printf("%02x  ", md[i]);
	printf("\n");
#endif

	free(dst_file_buf); 
	fclose(dst_fp); 
	printf("Complete....\n");

	if(argc == 5)
		return 0;

	for(i=0;i<src_file_size;i++){
		if(!(i%16)){
			printf("\n");
			printf("0x%06x :", hex.start+i);
		}
		printf("%02x ", hex.data[hex.start+i]);
	}
	printf("\n");

	return 0;
}
