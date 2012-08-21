/*                                                              */
/* File name : xv_types.h                                       */
/* Author : YoungHoon Jung (yhoon.jung@samsung.com)             */
/* Protocol Version : 1.0.1 (Dec 16th 2009)                       */
/* This file is for describing Xv APIs' buffer encoding method. */
/*                                                              */

#define XV_PUTIMAGE_HEADER	0xDEADCD01
#define XV_PUTIMAGE_VERSION	0x00010001

/* Return Values */
#define XV_OK 0
#define XV_HEADER_ERROR -1
#define XV_VERSION_MISMATCH -2

/* Video Mode */
#define DISPLAY_MODE_DEFAULT                                      0
#define DISPLAY_MODE_PRI_VIDEO_ON_AND_SEC_VIDEO_FULL_SCREEN       1
#define DISPLAY_MODE_PRI_VIDEO_OFF_AND_SEC_VIDEO_FULL_SCREEN      2

/* Data structure for XvPutImage / XvShmPutImage */
typedef struct {
	unsigned int _header; /* for internal use only */
	unsigned int _version; /* for internal use only */

	unsigned int YPhyAddr;
	unsigned int CbPhyAddr;
	unsigned int CrPhyAddr;
	unsigned int RotAngle;
	unsigned int VideoMode;
} XV_PUTIMAGE_DATA, * XV_PUTIMAGE_DATA_PTR;

static void XV_PUTIMAGE_INIT_DATA(XV_PUTIMAGE_DATA_PTR data)
{
	data->_header = XV_PUTIMAGE_HEADER;
	data->_version = XV_PUTIMAGE_VERSION;
}
