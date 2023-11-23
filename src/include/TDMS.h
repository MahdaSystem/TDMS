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

/**
 * @brief  Functions result data type
 */
typedef enum
{
  TDMS_OK           = 0,
  TDMS_OUT_OF_CAP   = -1,
  TDMS_WRONG_ARG    = -2
} TDMS_Result_t;

typedef enum TDMS_Data_e
{
  TDMS_DataType_Void = 0,
  TDMS_DataType_I8,
  TDMS_DataType_I16,
  TDMS_DataType_I32,
  TDMS_DataType_I64,
  TDMS_DataType_U8,
  TDMS_DataType_U16,
  TDMS_DataType_U32,
  TDMS_DataType_U64,
  TDMS_DataType_SingleFloat,
  TDMS_DataType_DoubleFloat,
  TDMS_DataType_ExtendedFloat,
  TDMS_DataType_SingleFloatWithUnit,
  TDMS_DataType_DoubleFloatWithUnit,
  TDMS_DataType_ExtendedFloatWithUnit,
  TDMS_DataType_String,
  TDMS_DataType_Boolean,
  TDMS_DataType_TimeStamp,
  TDMS_DataType_FixedPoint,
  TDMS_DataType_ComplexSingleFloat,
  TDMS_DataType_ComplexDoubleFloat,
  TDMS_DataType_MAX // Do not use this
} TDMS_Data_t;

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
  void *GroupOfChannel;
  TDMS_Data_t ChannelDataType;
  char ChannelPath[TDMS_GROUP_NAME_LEN+TDMS_CHANNEL_NAME_LEN+6];
} TDMS_Channel_t;

/**
 * @brief  Group structure
 */
typedef struct 
{
  void *FileOfGroup;
  uint32_t NumOfChannels;
  TDMS_Channel_t *ChannelArray[TDMS_MAX_CHANNEL_OF_GROUP];
  char GroupPath[TDMS_GROUP_NAME_LEN+3];
} TDMS_Group_t;

/**
 * @brief  File structure
 */
typedef struct 
{
  uint32_t NumOfGroups;
  TDMS_Group_t *GroupArray[TDMS_MAX_GROUP_OF_FILE];
} TDMS_File_t;



/**
 ==================================================================================
                             ##### Functions #####                                 
 ==================================================================================
 */

/**
 * @brief  Initialize File object structure
 * @param  File: Pointer to File object structure
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 */
TDMS_Result_t
TDMS_InitFile(TDMS_File_t *File);


/**
 * @brief  Initialize Channel Group object structure
 * @param  Group: Pointer to TDMS Channel Group object structure
 * @param  File: Pointer to the File object structure that Channel Group assign
 *               into
 * 
 * @param  Name: Channel Group name
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_OUT_OF_CAP: The file object capacity is full
 */
TDMS_Result_t
TDMS_AddGroupToFile(TDMS_Group_t *Group, TDMS_File_t *File, char *Name);


/**
 * @brief  Initialize Channel object structure
 * @param  Channel: Pointer to TDMS Channel object structure
 * @param  Group: Pointer to the Channel Group object structure that Channel assign
 *                into
 * 
 * @param  Name: Pointer to Name of TDMS Channel object
 * @param  DataType: Data type of Channel Raw data
 *         - TDMS_DataType_Void: Data type is unknown
 *         - TDMS_DataType_I8: signed int 8 bit
 *         - TDMS_DataType_I16: signed int 16 bit
 *         - TDMS_DataType_I32: signed int 32 bit
 *         - TDMS_DataType_I64: signed int 64 bit
 *         - TDMS_DataType_U8: unsigned int 8 bit
 *         - TDMS_DataType_U16: unsigned int 16 bit
 *         - TDMS_DataType_U32: unsigned int 32 bit
 *         - TDMS_DataType_U64: unsigned int 64 bit
 *         - TDMS_DataType_SingleFloat: 4 byte floating point number
 *         - TDMS_DataType_DoubleFloat: 8 byte floating point number
 *         - TDMS_DataType_String: string, array of characters
 *         - TDMS_DataType_Boolean: boolean data
 *         - TDMS_DataType_TimeStamp: time stamp data
 * 
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_WRONG_ARG: Wrong argument
 */
TDMS_Result_t
TDMS_AddChannelToGroup(TDMS_Channel_t *Channel, TDMS_Group_t *Group,
                       char *Name, TDMS_Data_t DataType);


/**
 * @brief  Generate First part of TDMS file
 * @note   To use this function, you must first create the File, and add
 *         Groups and Channels.
 * 
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
 * @brief  Add Property to the file object
 * @note   To use this function, you must first create and initialize the File and
 *         use TDMS_GenFirstPart
 * 
 * @param  Buffer: Pointer to the buffer that data save in
 * @note   If the buffer address is Null, then function only calculates needed
 *         buffer size
 * 
 * @param  Size: Size of data in buffer (Byte)
 * @param  Name: Name of Property
 * @param  DataType: Data type of Property
 * @param  Value: Pointer to the value of Property
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_WRONG_ARG: Wrong argument
 */
TDMS_Result_t
TDMS_AddPropertyToFile(uint8_t *Buffer, uint32_t *Size,
                       char *Name, TDMS_Data_t DataType, void *Value);


/**
 * @brief  Add Property to the group object
 * @note   To use this function, you must first create and initialize the File and
 *         use TDMS_GenFirstPart
 * 
 * @param  Group: Pointer to TDMS group object structure
 * @param  Buffer: Pointer to the buffer that data save in
 * @note   If the buffer address is Null, then function only calculates needed
 *         buffer size
 * 
 * @param  Size: Size of data in buffer (Byte)
 * @param  Name: Name of Property
 * @param  DataType: Data type of Property
 * @param  Value: Pointer to the value of Property
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_WRONG_ARG: Wrong argument
 */
TDMS_Result_t
TDMS_AddPropertyToGroup(TDMS_Group_t *Group,
                        uint8_t *Buffer, uint32_t *Size,
                        char *Name, TDMS_Data_t DataType, void *Value);


/**
 * @brief  Add Property to the channel object
 * @note   To use this function, you must first create and initialize the File and
 *         use TDMS_GenFirstPart
 * 
 * @param  Channel: Pointer to TDMS channel object structure
 * @param  Buffer: Pointer to the buffer that data save in
 * @note   If the buffer address is Null, then function only calculates needed
 *         buffer size
 * 
 * @param  Size: Size of data in buffer (Byte)
 * @param  Name: Name of Property
 * @param  DataType: Data type of Property
 * @param  Value: Pointer to the value of Property
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_WRONG_ARG: Wrong argument
 */
TDMS_Result_t
TDMS_AddPropertyToChannel(TDMS_Channel_t *Channel,
                          uint8_t *Buffer, uint32_t *Size,
                          char *Name, TDMS_Data_t DataType, void *Value);


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
int64_t
TDMS_TimeSecond(uint16_t Year, uint8_t Month, uint8_t Day,
                uint8_t Hour, uint8_t Minute, uint8_t Second);



#ifdef __cplusplus
}
#endif


#endif //! _TDMS_H_
