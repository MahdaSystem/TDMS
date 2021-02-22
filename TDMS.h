/**
 **********************************************************************************
 * @file   TDMS.h
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  Generate NI TDMS file (.TDMS)
 * @note   This library cannot read TDMS files.
 *         This library do not support all data types.
 **********************************************************************************
 */

/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef _TDMS_H_
#define _TDMS_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ---------------------------------------------------------------------*/
#include "TDMS_config.h"
#include <stdint.h>


/* Exported Data Types ----------------------------------------------------------*/
typedef uint32_t TDMS_Data_t;

/**
 * @brief  Functions result data type
 */
typedef enum
{
  TDMS_OK           = 0,
  TDMS_OUT_OF_CAP   = -1,
  TDMS_WRONG_ARG    = -2
} TDMS_Result_t;

/**
 * @brief  Time data type of TDMS file
 */
typedef struct
{
  // positive fractions (2-64) of a second
  uint64_t  Fraction;
  // number of whole seconds after 12:00 a.m.,
  // Friday, January 1, 1904, Universal Time
  int64_t   Second;
} TDMS_Timestamp_t;

/**
 * @brief  Channel structure
 */
typedef struct 
{
  TDMS_Data_t ChannelDataType;
  char ChannelName[TDMS_CHANNEL_NAME_LEN];
  char ChannelDescription[TDMS_CHANNEL_DESCRIPTION_LEN];
  char ChannelUitString[TDMS_CHANNEL_UNIT_STRING_LEN];
  char ChannelPath[TDMS_GROUP_NAME_LEN+TDMS_CHANNEL_NAME_LEN+6];
  void *GroupOfChannel;
} TDMS_Channel_t;

/**
 * @brief  Group structure
 */
typedef struct 
{
  char GroupName[TDMS_GROUP_NAME_LEN];
  char GroupDescription[TDMS_GROUP_DESCRIPTION_LEN];
  char GroupPath[TDMS_GROUP_DESCRIPTION_LEN+3];
  uint8_t NumOfChannels;
  TDMS_Channel_t *ChannelArray[TDMS_MAX_CHANNEL_OF_GROUP];
  void *FileOfGroup;
} TDMS_Group_t;

/**
 * @brief  File structure
 */
typedef struct 
{
  char FileDescription[TDMS_FILE_DESCRIPTION_LEN];
  char FileTitle[TDMS_FILE_TITLE_LEN];
  char FileAuthor[TDMS_FILE_AUTHOR_LEN];
  uint8_t NumOfGroups;
  TDMS_Group_t *GroupArray[TDMS_MAX_GROUP_OF_FILE];
} TDMS_File_t;


/* Exported Constants -----------------------------------------------------------*/
#define tdsTypeVoid                   0x00000000
#define tdsTypeI8                     0x00000001
#define tdsTypeI16                    0x00000002
#define tdsTypeI32                    0x00000003
#define tdsTypeI64                    0x00000004
#define tdsTypeU8                     0x00000005
#define tdsTypeU16                    0x00000006
#define tdsTypeU32                    0x00000007
#define tdsTypeU64                    0x00000008
#define tdsTypeSingleFloat            0x00000009
#define tdsTypeDoubleFloat            0x0000000A
#define tdsTypeExtendedFloat          0x0000000B
#define tdsTypeSingleFloatWithUnit    0x00000019
#define tdsTypeDoubleFloatWithUnit    0x0000001A
#define tdsTypeExtendedFloatWithUnit  0x0000001B
#define tdsTypeString                 0x00000020
#define tdsTypeBoolean                0x00000021
#define tdsTypeTimeStamp              0x00000044
#define tdsTypeFixedPoint             0x0000004F
#define tdsTypeComplexSingleFloat     0x0008000C
#define tdsTypeComplexDoubleFloat     0x0010000D
#define tdsTypeDAQmxRawData           0xFFFFFFFF



/**
 ==================================================================================
                             ##### Functions #####                                 
 ==================================================================================
 */

/**
 * @brief  Initialize File object structure
 * @param  Description: Pointer to description string
 * @param  Title: Pointer to title string
 * @param  Author: Pointer to author string
 * @param  File: Pointer to File object structure
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 */
TDMS_Result_t
TDMS_CreateFile(const char *Description,
                const char *Title,
                const char *Author,
                TDMS_File_t *File);


/**
 * @brief  Initialize Channel Group object structure
 * @param  File: Pointer to the File object structure that Channel Group assign
 *               into
 * @param  Name: Pointer to the Name of TDMS Channel Group object
 * @param  Description: Pointer to the Description of TDMS Channel Group object
 * @param  Group: Pointer to TDMS Channel Group object structure
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_OUT_OF_CAP: The file object capacity is full
 */
TDMS_Result_t
TDMS_AddGroup(TDMS_File_t *File,
              const char *Name,
              const char *Description,
              TDMS_Group_t *Group);


/**
 * @brief  Initialize Channel object structure
 * @param  Group: Pointer to the Channel Group object structure that Channel assign
 *                into
 * @param  DataType: Data type of Channel Raw data
 *         - tdsTypeVoid: Data type is unknown
 *         - tdsTypeI8: signed int 8 bit
 *         - tdsTypeI16: signed int 16 bit
 *         - tdsTypeI32: signed int 32 bit
 *         - tdsTypeI64: signed int 64 bit
 *         - tdsTypeU8: unsigned int 8 bit
 *         - tdsTypeU16: unsigned int 16 bit
 *         - tdsTypeU32: unsigned int 32 bit
 *         - tdsTypeU64: unsigned int 64 bit
 *         - tdsTypeSingleFloat: 4 byte floating point number
 *         - tdsTypeDoubleFloat: 8 byte floating point number
 *         - tdsTypeString: string, array of characters
 *         - tdsTypeBoolean: boolean data
 *         - tdsTypeTimeStamp: time stamp data
 * 
 * @param  Name: Pointer to Name of TDMS Channel object
 * @param  Description: Pointer to Description of TDMS Channel object
 * @param  UnitString: Pointer to UnitString of TDMS Channel object
 * @param  Channel: Pointer to TDMS Channel object structure
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_WRONG_ARG: Wrong argument
 */
TDMS_Result_t
TDMS_AddChannel(TDMS_Group_t *Group,
                TDMS_Data_t DataType,
                const char *Name,
                const char *Description,
                const char *UnitString,
                TDMS_Channel_t *Channel);


/**
 * @brief  Generate First part of TDMS file
 * @param  File: Pointer to TDMS File object structure
 * @param  Buffer: Pointer to the buffer that data save in
 * @param  Size: Size of data in buffer (Byte)
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 */
TDMS_Result_t
TDMS_GenFirstPart(TDMS_File_t *File,
                  uint8_t *Buffer,
                  uint32_t *Size);


/**
 * @brief  Set data to a Channel
 * @note   To use this function, you must first create and initialize the File,
 *         Channel Group, and Channel objects with their associated functions,
 *         then generate and save the first part of TDMS file using the
 *         TDMS_GenFirstPart function, and then use this function to add data to
 *         the Channels.
 * 
 * @param  Channel: Pointer to TDMS Channel object structure
 * @param  Buffer: Pointer to the buffer that data save in
 * @note   If the buffer address is Null, then function only calculates needed
 *         buffer size and returns TDMS_OK.
 * 
 * @param  Size: Size of data in buffer (Byte)
 * @param  Values: Pointer to data values
 * @param  NumOfValues: Number of values
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 */
TDMS_Result_t
TDMS_SetChannelDataValues(TDMS_Channel_t *Channel,
                          uint8_t *Buffer,
                          uint32_t *Size,
                          void *Values,
                          uint32_t NumOfValues);


/**
 * @brief  Set data to Channels of a Group
 * @note   To use this function, you must first create and initialize the File,
 *         Channel Group, and Channel objects with their associated functions,
 *         then generate and save the first part of TDMS file using the
 *         TDMS_GenFirstPart function, and then use this function to add data to
 *         the Channels of a Groupe.
 * 
 * @param  Group: Pointer to TDMS Group object structure
 * @param  Buffer: Pointer to the buffer that data save in
 * @note   If the buffer address is Null, then function only calculates needed
 *         buffer size and returns TDMS_OK.
 * 
 * @param  Size: Size of data in buffer (Byte)
 * @param  ...: Pointer to Channels data values, Number of values.
 * @note   This is a example of calling this function for a Group with 2
 *         Channels:
 *         uint8_t Ch1Values[3] = {0,1,2}; // Ch1 data type is tdsTypeU8
 *         float Ch2Values[2] = {10.0,11.1}; // Ch1 data type is tdsTypeSingleFloat
 *         TDMS_SetGroupDataValues(&Group, Buffer, &Size,
 *                                 Ch1Values, 3, Ch2Values , 2);
 * 
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 */
TDMS_Result_t
TDMS_SetGroupDataValues(TDMS_Group_t *Group,
                        uint8_t *Buffer,
                        uint32_t *Size,
                        ...
                        );


/**
 * @brief  Calculate second part of TDMS Timestamp from normal time and date
 * @param  Year: Normal Year (1904 to ...)
 * @param  Month: Normal Month (1 to 12)
 * @param  Day: Normal Day (1 to 31)
 * @param  Hour: Normal Hour (0 to 23)
 * @param  Minute: Normal Minute (0 to 59)
 * @param  Second: Normal Second (0 to 59)
 * @retval Second part of TDMS Timestamp
 */
uint64_t
TDMS_TimeSecond(uint16_t Year, uint8_t Month, uint8_t Day,
                uint8_t Hour, uint8_t Minute, uint8_t Second);



#ifdef __cplusplus
}
#endif


#endif //! _TDMS_H_
