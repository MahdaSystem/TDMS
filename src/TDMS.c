/**
 **********************************************************************************
 * @file   TDMS.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  Generate NI TDMS file (.TDMS)
 * @note   This library cannot read TDMS files.
 *         This library do not support all data types.
 **********************************************************************************
 */

/* Includes ---------------------------------------------------------------------*/
#include "TDMS.h"
#include "TDMS_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


/* Private Constants ------------------------------------------------------------*/
/**
 * @brief  Lead in part constants
 */ 
#define LeadInPartLen         28
#define TAG_TDSm_Num          0x5444536D
#define Version_Number4713    0x69120000	

/**
 * @brief  ToC mask constants
 */ 
#define kTocMetaData          0x00000002
#define kTocRawData           0x00000008
#define kTocNewObjList        0x00000004
#define kTocInterleavedData   0x00000020
#define kTocBigEndian         0x00000040
#define kTocDAQmxRawData      0x00000080

/**
 * @brief  Data type constants
 */
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
 * @brief  LabVIEW Timestamp base
 */
#define BaseYear    1904
#define BaseMonth   1
#define BaseDay     1


/* Private Macro ----------------------------------------------------------------*/
/**
 * @brief  Determine whether or not a year is leap year 
 */
#define leapYear(year) (((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) \
                            ? 1 : 0)


/* Private Variables ------------------------------------------------------------*/
/**
 * @brief  Specify the number of days in the month
 *         daysPerMonth[0][]: non leap year
 *         daysPerMonth[0][]: leap year
 */ 
const int8_t
    daysPerMonth[2][13] = {{-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                           {-1, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

const uint32_t dataTypeBinary[TDMS_DataType_MAX] =
{
  [TDMS_DataType_Void] = tdsTypeVoid,
  [TDMS_DataType_I8] = tdsTypeI8,
  [TDMS_DataType_I16] = tdsTypeI16,
  [TDMS_DataType_I32] = tdsTypeI32,
  [TDMS_DataType_I64] = tdsTypeI64,
  [TDMS_DataType_U8] = tdsTypeU8,
  [TDMS_DataType_U16] = tdsTypeU16,
  [TDMS_DataType_U32] = tdsTypeU32,
  [TDMS_DataType_U64] = tdsTypeU64,
  [TDMS_DataType_SingleFloat] = tdsTypeSingleFloat,
  [TDMS_DataType_DoubleFloat] = tdsTypeDoubleFloat,
  [TDMS_DataType_ExtendedFloat] = tdsTypeExtendedFloat,
  [TDMS_DataType_SingleFloatWithUnit] = tdsTypeSingleFloatWithUnit,
  [TDMS_DataType_DoubleFloatWithUnit] = tdsTypeDoubleFloatWithUnit,
  [TDMS_DataType_ExtendedFloatWithUnit] = tdsTypeExtendedFloatWithUnit,
  [TDMS_DataType_String] = tdsTypeString,
  [TDMS_DataType_Boolean] = tdsTypeBoolean,
  [TDMS_DataType_TimeStamp] = tdsTypeTimeStamp,
  [TDMS_DataType_FixedPoint] = tdsTypeFixedPoint,
  [TDMS_DataType_ComplexSingleFloat] = tdsTypeComplexSingleFloat,
  [TDMS_DataType_ComplexDoubleFloat] = tdsTypeComplexDoubleFloat
};

const uint8_t dataTypeLength[TDMS_DataType_MAX] =
{
  [TDMS_DataType_Void] = 1,
  [TDMS_DataType_I8] = sizeof(int8_t),
  [TDMS_DataType_I16] = sizeof(int16_t),
  [TDMS_DataType_I32] = sizeof(int32_t),
  [TDMS_DataType_I64] = sizeof(int64_t),
  [TDMS_DataType_U8] = sizeof(uint8_t),
  [TDMS_DataType_U16] = sizeof(uint16_t),
  [TDMS_DataType_U32] = sizeof(uint32_t),
  [TDMS_DataType_U64] = sizeof(uint64_t),
  [TDMS_DataType_SingleFloat] = sizeof(float),
  [TDMS_DataType_DoubleFloat] = sizeof(double),
  [TDMS_DataType_ExtendedFloat] = 0,
  [TDMS_DataType_SingleFloatWithUnit] = 0,
  [TDMS_DataType_DoubleFloatWithUnit] = 0,
  [TDMS_DataType_ExtendedFloatWithUnit] = 0,
  [TDMS_DataType_String] = 0,
  [TDMS_DataType_Boolean] = sizeof(uint8_t),
  [TDMS_DataType_TimeStamp] = sizeof(TDMS_Timestamp_t),
  [TDMS_DataType_FixedPoint] = 0,
  [TDMS_DataType_ComplexSingleFloat] = 0,
  [TDMS_DataType_ComplexDoubleFloat] = 0
};


/**
 ==================================================================================
                           ##### Private Functions #####
 ==================================================================================
 */

/**
 * @brief  Calculate number of days between LabVIEW Timestamp base and inserted
 *         date
 * @param  Day: Day number (0 to 31)
 * @param  Month: Month number (0 to 12)
 * @param  Year: Year number (1904 to ...)
 * @retval Number of days
 */
static uint32_t
TDMS_DateDef(uint8_t Day, uint8_t Month, uint16_t Year)
{
  int16_t y, m;
  uint32_t total_days = 0;

  if (Year < BaseYear)
    return 0;

  for (y = Year; y >= BaseYear; y--)
  {
    if (y == Year)
    {
      for (m = Month; m >= 1; m--)
      {
        if (m == Month)
          total_days += Day;
        else
          total_days += daysPerMonth[leapYear(y)][m];
      }
    }
    else if (y == BaseYear)
    {
      for (m = 12; m >= BaseMonth; m--)
      {
        if (m == BaseMonth)
          total_days += daysPerMonth[leapYear(y)][m] - BaseDay;
        else
          total_days += daysPerMonth[leapYear(y)][m];
      }
    }
    else
    {
      for (m = 12; m >= 1; m--)
      {
        total_days += daysPerMonth[leapYear(y)][m];
      }
    }
  }

  return total_days;
}


/**
 * @brief  Generation Channel Group path in TDMS file ==> /'Channel Group Name' 
 * @param  Path: Pointer to a string that Channel Group path save into
 * @param  Name: Channel Group name
 * @retval If successful, String size of Channel Group path excluding the
 *         null-character appended at the end of the string. (return value >= 0)
 *         Else return value < 0
 */
static inline int16_t
TDMS_GenerateGroupPath(char *Path, char *Name)
{
  int16_t Retval = -1;

  if (Path)
    Retval = sprintf(Path, "/'%s'", Name);

  return Retval;
}


/**
 * @brief  Generation Channel path in TDMS file
 *         ==> /'Channel Group Name'/'Channel Name'
 * @note   Channel Group path must be generated before Channel path
 * @param  Group: Pointer to The Channel Group structure address where the
 *                Channel object assign into
 * @param  path: Pointer to a string that Channel path save into.
 * @param  Name: Channel name
 * @retval If successful, String size of Channel Group path excluding the
 *         null-character appended at the end of the string. (return value >= 0)
 *         Else return value < 0
 */
static inline int16_t
TDMS_GenerateChannelPath(TDMS_Group_t *Group,
                         char *Path, char *Name)
{
  int16_t Retval = -1;

  if (Path)
    Retval = sprintf(Path, "%s/'%s'",
                     Group->GroupPath,
                     Name);

  return Retval;
}


/**
 * @brief  Saves a 32 bit data in Little Endian format into a pointed area
 * @param  data: Pointer to array that value save into
 * @param  value: 32 bit data
 * @retval 4
 */
static uint8_t
TDMS_SaveDataLittleEndian32(uint8_t *data, uint32_t value)
{
  typedef union
  {
    uint32_t  Uint32Value;
    uint8_t   Uint8Value[4];
  } U32toU8_t;

  U32toU8_t Buffer = {.Uint32Value = value,};
  
  for (uint8_t i = 0; i < 4; i++)
  {
#if (TDMS_CONFIG_SYSTEM_ENDIANNESS == 0)
    data[i] = Buffer.Uint8Value[i];
#else
    data[i] = Buffer.Uint8Value[4-i];
#endif
  }
  
  return 4;
}


/**
 * @brief  Saves a 64 bit data in Little Endian format into a pointed area
 * @param  data: Pointer to array that value save into
 * @param  value: 64 bit data
 * @retval 8
 */
static uint8_t
TDMS_SaveDataLittleEndian64(uint8_t *data, uint64_t value)
{
  typedef union
  {
    uint32_t  Uint64Value;
    uint8_t   Uint8Value[8];
  } U64toU8_t;
  
  U64toU8_t Buffer = {.Uint64Value = value,};

  for (uint8_t i = 0; i < 8; i++)
  {
#if (TDMS_CONFIG_SYSTEM_ENDIANNESS == 0)
    data[i] = Buffer.Uint8Value[i];
#else
    data[i] = Buffer.Uint8Value[8-i];
#endif
  }

  return 8;
}


/**
 * @brief  Stores a string and its length in standard TDMS format into pointed area
 * @param  data: Pointer to area that value save into.
 * @param  str: Pointer to the string.
 * @retval Number of bytes written into array.
 */
static uint32_t
TDMS_SaveStrToMetaDataPart(uint8_t *data, const char *str)
{
  uint32_t StringLen = 0;
  
  StringLen = strlen(str);
  // write length of str and calculate total number of bytes written
  StringLen += TDMS_SaveDataLittleEndian32(data, StringLen);
  
  strcpy((char *) &data[4], str); // write str to data location
  
  return StringLen;
}


/**
 * @brief  Generates TDMS segment Lead IN part
 * @param  LeadInSTR: Pointer to area that Lead In part footprint stores into
 * @param  ToC_value: Indicate what kind of data the segment contains
 * @param  NextSegmentOffset_value: describe the length of the remaining segment
 *                                  (overall length of the segment minus length of
 *                                  the lead in)
 * @param  RawDataOffset_value: describe the overall length of the meta information
 *                              in the segment
 * @retval None
 */
static void
TDMS_GenerateLeadInPart(uint8_t *LeadInSTR,
                        uint32_t ToC_value,
                        uint64_t NextSegmentOffset_value,
                        uint64_t RawDataOffset_value)
{
  uint8_t CounterI = 0;
  uint8_t CounterJ = 0;
  
  for(CounterI=0; CounterI<4; CounterI++, CounterJ++)
    LeadInSTR[CounterJ] = (TAG_TDSm_Num)>>(24-CounterI*8); //Big Endian
    
  for(CounterI=0; CounterI<4; CounterI++, CounterJ++)
    LeadInSTR[CounterJ] = (ToC_value&0x000000EE)>>(CounterI*8); // Little Endian
    
  for(CounterI=0; CounterI<4; CounterI++, CounterJ++)
    LeadInSTR[CounterJ] = (Version_Number4713)>>(24-CounterI*8);
    
  for(CounterI=0; CounterI<8; CounterI++, CounterJ++)
    LeadInSTR[CounterJ] = (NextSegmentOffset_value)>>(CounterI*8);
    
  for(CounterI=0; CounterI<8; CounterI++, CounterJ++)
    LeadInSTR[CounterJ] = (RawDataOffset_value)>>(CounterI*8);

}


/**
 * @brief  Add Property to the object
 * @note   To use this function, you must first create and initialize the File and
 *         use TDMS_GenFirstPart
 * 
 * @param  Path: Object path
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
static TDMS_Result_t
TDMS_AddPropertyToObject(char *Path,
                        uint8_t *Buffer, uint32_t *Size,
                        char *Name, TDMS_Data_t DataType, void *Value)
{
  uint32_t DataSize = 0;
  uint32_t MetaDataLen = 0;
  uint32_t PropertyDataLen = 0;

  PropertyDataLen = dataTypeLength[DataType];
  if (PropertyDataLen == 0 && DataType != TDMS_DataType_String)
    return TDMS_WRONG_ARG;
  
  /*** ***/
  /*** Meta Data len calculation ***/
  /*** ***/
  // (4B number of objects) +
  // (4B file path length) +
  // (4B raw data index) + (4B number of properties)
  // (4B property name length) +
  // (4B property data type)
  MetaDataLen += 24;
  MetaDataLen += strlen(Path); // Group path length
  MetaDataLen += strlen(Name); // Property name length
  if(DataType == TDMS_DataType_String)
  {
    // (4B string length)
    MetaDataLen += 4;
    MetaDataLen += strlen((const char *)Value); // String length
  }
  else
    MetaDataLen += PropertyDataLen;
  
  
  /*** ***/
  /*** if Buffer address is NULL, return back. ***/
  /*** ***/
  if(Buffer == NULL)
  {
    DataSize = LeadInPartLen + MetaDataLen;
    *Size = DataSize;
    
    return TDMS_OK;
  }
  
  
  /*** ***/
  /*** generate lead in part ***/
  /*** ***/
  TDMS_GenerateLeadInPart(&Buffer[DataSize],
                          kTocNewObjList | kTocMetaData,
                          MetaDataLen,
                          MetaDataLen);
  DataSize += LeadInPartLen;


  /*** ***/
  /*** generate meta data ***/
  /*** ***/
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize], 1); // Number of objects

  /*** File meta data ***/
  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         Path); // Object path
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0xFFFFFFFF); // Raw data index = 0xFFFFFFFF
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0x01); // Number of properties

  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         Name); // first Property name
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          dataTypeBinary[DataType]); // Data type of the property value
  
  if (DataType == TDMS_DataType_String)
  {
    DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                           (char *)Value); // Value of the property
  }
  else
  {
    memcpy(&Buffer[DataSize], Value, PropertyDataLen);
    DataSize += PropertyDataLen;
  }
  
  *Size = DataSize;
  
  return TDMS_OK;
}



/**
 ==================================================================================
                            ##### Public Functions #####                           
 ==================================================================================
 */

/**
 * @brief  Initialize File object structure
 * @param  File: Pointer to File object structure
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 */
TDMS_Result_t
TDMS_InitFile(TDMS_File_t *File)
{
  File->NumOfGroups = 0;
  
  return TDMS_OK;
}


/**
 * @brief  Initialize Channel Group object structure
 * @param  Group: Pointer to TDMS Channel Group object structure
 * @param  File: Pointer to the File object structure that Channel Group assign
 *               into
 * @param  Name: Channel Group name
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_OUT_OF_CAP: The file object capacity is full
 */
TDMS_Result_t
TDMS_AddGroupToFile(TDMS_Group_t *Group, TDMS_File_t *File, char *Name)
{
  // Group path
  TDMS_GenerateGroupPath(Group->GroupPath, Name);
  
  // set File pointer
  Group->FileOfGroup = (void *) File;
  
  // add group to file obj
  if (File->NumOfGroups >= TDMS_CONFIG_MAX_GROUP_OF_FILE)
    return TDMS_OUT_OF_CAP;
  File->GroupArray[File->NumOfGroups] = Group;
  File->NumOfGroups++;
  
  Group->NumOfChannels = 0;
  
  return TDMS_OK;
}


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
                       char *Name, TDMS_Data_t DataType)
{
  if(dataTypeLength[DataType] == 0)
    return TDMS_WRONG_ARG;
  
  Channel->ChannelDataType = DataType;
  
  // channel path
  TDMS_GenerateChannelPath(Group, Channel->ChannelPath, Name);
  
  // set Group pointer
  Channel->GroupOfChannel = (void *) Group;
  
  // add channel to group obj
  if (Group->NumOfChannels >= TDMS_CONFIG_MAX_CHANNEL_OF_GROUP)
    return TDMS_OUT_OF_CAP;
  Group->ChannelArray[Group->NumOfChannels] = Channel;
  Group->NumOfChannels++;

  
  return TDMS_OK;
}


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
                  uint32_t *Size)
{
  uint32_t CounterI = 0;
  uint32_t CounterJ = 0;
  uint32_t CounterK = 0;
  uint32_t nog = 0; // Number Of Groups
  uint32_t noc[TDMS_CONFIG_MAX_CHANNEL_OF_GROUP] = {0}; // Number Of Channel of each Group
  uint32_t tnoc = 0; //  Total Number Of Channel
  uint32_t noo = 0; //  Number Of Objects
  uint32_t MetaDataLen = 0;
  uint32_t DataSize = 0;
  
  #define RawDataLen	0
  
  /*** ***/
  /*** calculate number of objects ***/
  /*** ***/
  nog = File->NumOfGroups;
  for(CounterI=0; CounterI<nog; CounterI++)
  {
    noc[CounterI] = File->GroupArray[CounterI]->NumOfChannels;
    tnoc += noc[CounterI];
  }
  noo = nog + tnoc + 1; // Groups number + total number of Channels + File object

  
  MetaDataLen = 0;

  /*** File meta data ***/
  // (4B number of objects) +
  // (4B file path length) + (1B file path) +
  // (4B raw data index) + (4B number of properties)
  MetaDataLen += 17;
  
  /*** Groups meta data ***/
  for(CounterI=0; CounterI<nog; CounterI++)
  {
    // (4B gorup path length) + (4B raw data index) + (4B number of properties)
    MetaDataLen += 12;
    MetaDataLen += strlen(File->GroupArray[CounterI]->GroupPath); // Groups path length
    
    /*** Channels meta data ***/
    for(CounterJ=0; CounterJ<noc[CounterI]; CounterJ++)
    {
      // (4B channel path length) + (4B raw data index) + (4B number of properties)
      MetaDataLen += 12;
      MetaDataLen += strlen(File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelPath); //Channels path
    }//for(CounterJ=0; CounterJ<noc[CounterI]; CounterJ++)
  }//for(CounterI=0; CounterI<nog; CounterI++)
  

  /*** ***/
  /*** if Buffer address is NULL, return back. ***/
  /*** ***/
  if(Buffer == NULL)
  {
    DataSize = LeadInPartLen + MetaDataLen + RawDataLen;
    *Size = DataSize;
    
    return TDMS_OK;
  }
  
  /*** ***/
  /*** generate lead in part ***/
  /*** ***/
  TDMS_GenerateLeadInPart(&Buffer[DataSize],
                          kTocMetaData | kTocNewObjList,
                          MetaDataLen + RawDataLen,
                          MetaDataLen);
  DataSize += LeadInPartLen;
  
  
  /*** ***/
  /*** generate meta data ***/
  /*** ***/
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          noo); // Number of objects

  /*** File meta data ***/
  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         "/"); // File path
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0xFFFFFFFF); // Raw data index = 0xFFFFFFFF
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0x00); // Number of properties

  /*** Goups meta data ***/
  for (CounterI = 0; CounterI < nog; CounterI++)
  {
    CounterK = strlen(File->GroupArray[CounterI]->GroupPath);
    DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                            CounterK); // Length of Groups path
    strcpy((char *)&Buffer[DataSize],
           (const char *)File->GroupArray[CounterI]->GroupPath); // Groups path
    DataSize += CounterK;
    DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                            0xFFFFFFFF); // Raw data index = 0xFFFFFFFF
    DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                            0x00); // Number of properties

    /*** Channels meta data ***/
    for(CounterJ=0; CounterJ<noc[CounterI]; CounterJ++)
    {
      CounterK = strlen(File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelPath);
      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              CounterK); // Length of Channels path
      strcpy((char *)&Buffer[DataSize],
             (const char *)File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelPath); // Channels path
      DataSize += CounterK;

      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              0xFFFFFFFF); // Raw data index = 0xFFFFFFFF

      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              0x00); // Number of properties
    }//for(CounterJ=0; CounterJ<noc[CounterI]; CounterJ++)
  }  //for(CounterI=0; CounterI<nog; CounterI++)

  *Size = DataSize;
  
  #undef RawDataLen
  
  return TDMS_OK;
}


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
                       char *Name, TDMS_Data_t DataType, void *Value)
{
  return TDMS_AddPropertyToObject("/", Buffer, Size, Name, DataType, Value);
}


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
                        char *Name, TDMS_Data_t DataType, void *Value)
{
  return TDMS_AddPropertyToObject(Group->GroupPath, Buffer, Size, Name, DataType, Value);
}


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
                          char *Name, TDMS_Data_t DataType, void *Value)
{
  return TDMS_AddPropertyToObject(Channel->ChannelPath, Buffer, Size, Name, DataType, Value);
}

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
                          uint32_t NumOfValues)
{
  uint32_t DataSize = 0;
  uint32_t RawDataLen = 0; // Bytes
  uint32_t MetaDataLen = 0;	
  uint16_t CounterI = 0;
  
  /*** ***/
  /*** Raw Data len calculation ***/
  /*** ***/
  RawDataLen = dataTypeLength[Channel->ChannelDataType] * NumOfValues;
  if (RawDataLen == 0)
    return TDMS_WRONG_ARG;

  if(Channel->ChannelDataType == TDMS_DataType_Boolean)
  {
    uint8_t *ValuesBoolean = (uint8_t *) Values;
    for(CounterI=0; CounterI<NumOfValues; CounterI++)
      ValuesBoolean[CounterI] = (ValuesBoolean[CounterI]) ? 1:0; 
  }

  
  /*** ***/
  /*** Meta Data len calculation ***/
  /*** ***/
  MetaDataLen = 0;
  MetaDataLen += 32;
  if(Channel->ChannelDataType == TDMS_DataType_String)
    MetaDataLen += 8; // Total Size in bytes (only stored for variable length data types, e.g. strings)
  MetaDataLen += strlen(Channel->ChannelPath); //Channel path
  
  
  
  /*** ***/
  /*** if Buffer address is NULL, return back. ***/
  /*** ***/
  if(Buffer == NULL)
  {
    DataSize = LeadInPartLen + MetaDataLen + RawDataLen;
    *Size = DataSize;
    
    return TDMS_OK;
  }
  
  
  /*** ***/
  /*** generate lead in part ***/
  /*** ***/
  TDMS_GenerateLeadInPart(&Buffer[DataSize],
                          kTocRawData | kTocNewObjList | kTocMetaData,
                          MetaDataLen + RawDataLen,
                          MetaDataLen);
  DataSize += LeadInPartLen;

  /*** ***/
  /*** generate meta data ***/
  /*** ***/
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          1); // Number of objects = 1 -> the object is the Channel

  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         Channel->ChannelPath); // Channel path

  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0x14); // Length of index information
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          dataTypeBinary[Channel->ChannelDataType]); // Data type of the raw data assigned to this object
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0x01); // Dimension of the raw data array (must be 1)
  DataSize += TDMS_SaveDataLittleEndian64(&Buffer[DataSize],
                                          NumOfValues); // Number of raw data Values
  if (Channel->ChannelDataType == TDMS_DataType_String)
    DataSize += TDMS_SaveDataLittleEndian64(&Buffer[DataSize],
                                            strlen(Values)); // Total Size in bytes

  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0); // Number of properties

  /*** ***/
  /*** write RAW data ***/
  /*** ***/
  memcpy(&Buffer[DataSize], Values, RawDataLen);
  DataSize += RawDataLen;
  
  *Size = DataSize;
  
  return TDMS_OK;
}


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
                        )
{
  uint32_t DataSize = 0;
  uint32_t RawDataLenCh[TDMS_CONFIG_MAX_CHANNEL_OF_GROUP] = {0}; // Raw Data for each channel
  uint32_t RawDataLen = 0; // Bytes
  uint32_t MetaDataLen = 0;
  uint32_t NumberOfObjects = 0;
  uint16_t CounterI = 0;

  void* Values[TDMS_CONFIG_MAX_CHANNEL_OF_GROUP];
  uint32_t NumOfValues[TDMS_CONFIG_MAX_CHANNEL_OF_GROUP];
  
  va_list valist;

  /* initialize valist for num number of arguments */
  va_start(valist, Size);

  for(CounterI=0; CounterI<(Group->NumOfChannels); CounterI++)
  {
    Values[CounterI] = va_arg(valist, void*);
    NumOfValues[CounterI] = va_arg(valist, uint32_t);
    if(NumOfValues[CounterI])
      NumberOfObjects++;
  }

  /* clean memory reserved for valist */
  va_end(valist);

  
  if(!NumberOfObjects)
  {
    *Size = 0;
    return TDMS_OK;
  }
  
  
  /*** ***/
  /*** Raw Data & Meta Data len calculation ***/
  /*** ***/
  RawDataLen = 0;
  MetaDataLen = 4; // Number of objects 
  for(CounterI=0; CounterI<(Group->NumOfChannels); CounterI++)
  {
    if(NumOfValues[CounterI])
    {
      // Raw Data for each channel
      RawDataLenCh[CounterI] = dataTypeLength[Group->ChannelArray[CounterI]->ChannelDataType] * NumOfValues[CounterI];
      if (RawDataLenCh[CounterI] == 0)
        return TDMS_WRONG_ARG;
      
      // Meta Data
      MetaDataLen += 28;
      if(Group->ChannelArray[CounterI]->ChannelDataType == TDMS_DataType_String)
        MetaDataLen += 4; // Total Size in bytes (only stored for variable length data types, e.g. strings)
      MetaDataLen += strlen(Group->ChannelArray[CounterI]->ChannelPath); //Channel path
    }
    else
    {
      RawDataLenCh[CounterI] = 0;
    }
    
    RawDataLen += RawDataLenCh[CounterI];
  } //for(CounterI=0; CounterI<noc; CounterI++)
  
  
  /*** ***/
  /*** if Buffer address is NULL, return back. ***/
  /*** ***/
  if(Buffer == NULL)
  {
    *Size = LeadInPartLen + MetaDataLen + RawDataLen;
    
    return TDMS_OK;
  }
  
  DataSize = 0;
  
  
  /*** ***/
  /*** generate lead in part ***/
  /*** ***/
  TDMS_GenerateLeadInPart(&Buffer[DataSize],
                          kTocRawData | kTocNewObjList | kTocMetaData,
                          MetaDataLen + RawDataLen,
                          MetaDataLen);
  DataSize += LeadInPartLen;

  /*** ***/
  /*** generate meta data ***/
  /*** ***/
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          NumberOfObjects); // Number of objects
  for (CounterI = 0; CounterI < (Group->NumOfChannels); CounterI++)
  {
    if (NumOfValues[CounterI])
    {
      DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                             Group->ChannelArray[CounterI]->ChannelPath); // Channel path

      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              0x14); // Length of index information
      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              dataTypeBinary[Group->ChannelArray[CounterI]->ChannelDataType]); // Data type of the raw data assigned to this object
      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              0x01); // Dimension of the raw data array (must be 1)
      DataSize += TDMS_SaveDataLittleEndian64(&Buffer[DataSize],
                                              NumOfValues[CounterI]); // Number of raw data Values
      if (Group->ChannelArray[CounterI]->ChannelDataType == TDMS_DataType_String)
        DataSize += TDMS_SaveDataLittleEndian64(&Buffer[DataSize],
                                                strlen((const char *)*(Values + CounterI))); // Total Size in bytes

      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              0); // Number of properties
    }
  }

  /*** ***/
  /*** generate RAW data ***/
  /*** ***/
  for(CounterI=0; CounterI<(Group->NumOfChannels); CounterI++)
  {
    if(NumOfValues[CounterI])
    {
      memcpy(&Buffer[DataSize],
             Values[CounterI],
             RawDataLenCh[CounterI]);
      DataSize += RawDataLenCh[CounterI];
    }
  }
  
  *Size = DataSize;
  
  return TDMS_OK;
}


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
                uint8_t Hour, uint8_t Minute, uint8_t Second)
{
  uint32_t DiffDay;
  int64_t DiffSecond;

  DiffDay = TDMS_DateDef(Day, Month, Year);
  DiffSecond  = (int64_t) (DiffDay * 86400ul);
  DiffSecond += (int64_t) (Hour * 3600); 
  DiffSecond += (int64_t) (Minute * 60);
  DiffSecond += (int64_t) (Second);

  return DiffSecond;
}
