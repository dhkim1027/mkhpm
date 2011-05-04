#ifndef __HPM_H__
#define __HPM_H__

#define HPMFWUPG_HEADER_SIGNATURE_LENGTH 8
#define HPMFWUPG_MANUFATURER_ID_LENGTH   3
#define HPMFWUPG_PRODUCT_ID_LENGTH       2
#define HPMFWUPG_TIME_LENGTH             4
#define HPMFWUPG_TIMEOUT_LENGTH          1
#define HPMFWUPG_COMP_REVISION_LENGTH    2
#define HPMFWUPG_FIRM_REVISION_LENGTH    6
#define HPMFWUPG_IMAGE_HEADER_VERSION    0
#define HPMFWUPG_IMAGE_SIGNATURE "PICMGFWU"
#define HPMFWUPG_DESCRIPTION_LENGTH   21
#define HPMFWUPG_FIRMWARE_SIZE_LENGTH 4

typedef enum eHpmfwupgActionType
{
	HPMFWUPG_ACTION_BACKUP_COMPONENTS = 0,
	HPMFWUPG_ACTION_PREPARE_COMPONENTS,
	HPMFWUPG_ACTION_UPLOAD_FIRMWARE,
	HPMFWUPG_ACTION_RESERVED = 0xFF
} tHpmfwupgActionType;



struct HpmfwupgComponentBitMask
{
	union
	{
		unsigned char byte;
		struct
		{
			unsigned char component0 : 1;
			unsigned char component1 : 1;
			unsigned char component2 : 1;
			unsigned char component3 : 1;
			unsigned char component4 : 1;
			unsigned char component5 : 1;
			unsigned char component6 : 1;
			unsigned char component7 : 1;
		}bitField;
	}ComponentBits;
} HpmfwupgComponentBitMask_t;

struct HpmfwupgImageHeader
{
	char           signature[HPMFWUPG_HEADER_SIGNATURE_LENGTH];
	unsigned char  formatVersion;
	unsigned char  deviceId;
	unsigned char  manId[HPMFWUPG_MANUFATURER_ID_LENGTH];
	unsigned char  prodId[HPMFWUPG_PRODUCT_ID_LENGTH];
	unsigned char  time[HPMFWUPG_TIME_LENGTH];
	union
	{
		struct
		{
			unsigned char reserved        : 4;
			unsigned char servAffected    : 1;
			unsigned char manRollback     : 1;
			unsigned char autRollback     : 1;
			unsigned char imageSelfTest   : 1;
		}  bitField;
		unsigned char byte;
	}imageCapabilities;
	struct HpmfwupgComponentBitMask components;
	unsigned char  selfTestTimeout;
	unsigned char  rollbackTimeout;
	unsigned char  inaccessTimeout;
	unsigned char  compRevision[HPMFWUPG_COMP_REVISION_LENGTH];
	unsigned char  firmRevision[HPMFWUPG_FIRM_REVISION_LENGTH];
	unsigned short oemDataLength;
} HpmfwupgImageHeader_t;

struct HpmfwupgActionRecord
{
	unsigned char  actionType;
	struct HpmfwupgComponentBitMask components;
	unsigned char  checksum;
} HpmfwupgActionRecord_t;

struct HpmfwupgFirmwareImage
{
	unsigned char version[HPMFWUPG_FIRM_REVISION_LENGTH];
	char          desc[HPMFWUPG_DESCRIPTION_LENGTH];
	unsigned char length[HPMFWUPG_FIRMWARE_SIZE_LENGTH];
} HpmfwupgFirmwareImage_t;





struct HpmfwupgImageOemHeader{
//	int chksum;
} HpmfwupgImageOemHeader_t;

#endif
