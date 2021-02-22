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
 * @brief  Copy a string to another
 * @param  Dest: Destination buffer
 * @param  Src: Source string
 * @param  DestSize: Size of Destination buffer
 * @retval TDMS_Result_t
 *         - TDMS_OK: Operation was successful
 *         - TDMS_WRONG_ARG: Destination length is low
 */
static TDMS_Result_t
TDMS_StrnCpy(char *Dest, const char *Src, uint16_t DestSize)
{
  uint16_t SrcSize = strlen(Src);
  if(SrcSize < DestSize)
  {
    memcpy(Dest, Src, SrcSize);
    Dest[SrcSize] = '\0';
    return TDMS_OK;
  }
  else
  {
    return TDMS_WRONG_ARG;
  }
}


/**
 * @brief  Generation Channel Group path in TDMS file ==> /'Channel Group Name' 
 * @param  Group: Pointer to the Channel Group structure
 * @param  path: Pointer to a string that Channel Group path save into
 * @retval If successful, String size of Channel Group path excluding the
 *         null-character appended at the end of the string. (return value >= 0)
 *         Else return value < 0
 */
static int16_t
TDMS_GenerateGroupPath(TDMS_Group_t *Group, char *path)
{
  int16_t Retval = -1;

  if (path)
    Retval = sprintf(path, "/'%s'", Group->GroupName);

  return Retval;
}


/**
 * @brief  Generation Channel path in TDMS file
 *         ==> /'Channel Group Name'/'Channel Name'
 * @param  Group: Pointer to The Channel Group structure address where the
 *                Channel object assign into
 * @param  Channel: Pointer to the Channel structure
 * @param  path: Pointer to a string that Channel path save into.
 * @retval If successful, String size of Channel Group path excluding the
 *         null-character appended at the end of the string. (return value >= 0)
 *         Else return value < 0
 */
static int16_t
TDMS_GenerateChannelPath(TDMS_Group_t *Group,
                         TDMS_Channel_t *Channel,
                         char *path)
{
  int16_t Retval = -1;

  if (path)
    Retval = sprintf(path, "/'%s'/'%s'",
                     Group->GroupName,
                     Channel->ChannelName);

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
#if (TDMS_SYSTEM_ENDIANNESS == 0)
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
#if (TDMS_SYSTEM_ENDIANNESS == 0)
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
 * @brief  Calculates number of bytes taken by each data type
 * @param  TdsDataType: TDMS Raw Data type
 * @retval Size in Bytes
 */
static uint8_t
TDMS_DataBytesOfEachType(TDMS_Data_t TdsDataType)
{
  uint8_t DataLen = 0; // Bytes
  switch(TdsDataType)
  {
    case tdsTypeVoid:
      DataLen = 1;
      break;
    
    case tdsTypeI8:
      DataLen = sizeof(int8_t);
      break;
    
    case tdsTypeI16:
      DataLen = sizeof(int16_t);
      break;
    
    case tdsTypeI32:
      DataLen = sizeof(int32_t);
      break;
    
    case tdsTypeI64:
      DataLen = sizeof(int64_t);
      break;
    
    case tdsTypeU8:
      DataLen = sizeof(uint8_t);
      break;
    
    case tdsTypeU16:
      DataLen = sizeof(uint16_t);
      break;
    
    case tdsTypeU32:
      DataLen = sizeof(uint32_t);
      break;
    
    case tdsTypeU64:
      DataLen = sizeof(uint64_t);
      break;
    
    case tdsTypeSingleFloat:
      DataLen = sizeof(float);
      break;
    
    case tdsTypeDoubleFloat:
      DataLen = sizeof(double);
      break;
    
    case tdsTypeString:
      DataLen = sizeof(char); // indeterminate
      break;
      
    case tdsTypeBoolean:
      DataLen = sizeof(uint8_t);
      break;

    case tdsTypeTimeStamp:
      DataLen = sizeof(TDMS_Timestamp_t);
      break;
  }
  
  return DataLen;
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
 ==================================================================================
                            ##### Public Functions #####                           
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
                TDMS_File_t *File)
{
  TDMS_Result_t Retval;

  // file description
  Retval = TDMS_StrnCpy(File->FileDescription,
                        Description,
                        TDMS_FILE_DESCRIPTION_LEN);
  if(Retval != TDMS_OK)
    return Retval;

  // file title
  Retval = TDMS_StrnCpy(File->FileTitle,
                        Title,
                        TDMS_FILE_TITLE_LEN);
  if(Retval != TDMS_OK)
    return Retval;

  // file author
  Retval = TDMS_StrnCpy(File->FileAuthor,
                        Author,
                        TDMS_FILE_AUTHOR_LEN);
  if(Retval != TDMS_OK)
    return Retval;

  File->NumOfGroups = 0;
  
  return TDMS_OK;
}


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
              TDMS_Group_t *Group)
{
  TDMS_Result_t Retval;
  
  // Group name
  Retval = TDMS_StrnCpy(Group->GroupName,
                        Name,
                        TDMS_GROUP_NAME_LEN);
  if(Retval != TDMS_OK)
    return Retval;

  // Group description
  Retval = TDMS_StrnCpy(Group->GroupDescription,
                        Description,
                        TDMS_GROUP_DESCRIPTION_LEN);
  if(Retval != TDMS_OK)
    return Retval;

  // Group path
  TDMS_GenerateGroupPath(Group, Group->GroupPath);
  
  // set File pointer
  Group->FileOfGroup = (void *) File;
  
  // add group to file obj
  if (File->NumOfGroups >= TDMS_MAX_GROUP_OF_FILE)
    return TDMS_OUT_OF_CAP;
  File->GroupArray[File->NumOfGroups] = Group;
  File->NumOfGroups++;
  
  Group->NumOfChannels = 0;
  
  return TDMS_OK;
}


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
                TDMS_Channel_t *Channel)
{
  TDMS_Result_t Retval;
  
  if(TDMS_DataBytesOfEachType(DataType)==0)
    return TDMS_WRONG_ARG;
  
  Channel->ChannelDataType = DataType;
  
  // channel name
  Retval = TDMS_StrnCpy(Channel->ChannelName,
                        Name,
                        TDMS_CHANNEL_NAME_LEN);
  if(Retval != TDMS_OK)
    return Retval;

  // channel description
  Retval = TDMS_StrnCpy(Channel->ChannelDescription,
                        Description,
                        TDMS_CHANNEL_DESCRIPTION_LEN);
  if (Retval != TDMS_OK)
    return Retval;

  // channel unitString
  Retval = TDMS_StrnCpy(Channel->ChannelUitString,
                        UnitString,
                        TDMS_CHANNEL_UNIT_STRING_LEN);
  if (Retval != TDMS_OK)
    return Retval;
  
  // channel path
  TDMS_GenerateChannelPath(Group, Channel, Channel->ChannelPath);
  
  // set Group pointer
  Channel->GroupOfChannel = (void *) Group;
  
  // add channel to group obj
  if (Group->NumOfChannels >= TDMS_MAX_CHANNEL_OF_GROUP)
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
  uint32_t noc[TDMS_MAX_CHANNEL_OF_GROUP] = {0}; // Number Of Channel of each Group
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

  
  /*** File meta data ***/
  MetaDataLen = 0;
  MetaDataLen += 53;
  MetaDataLen += strlen("Description"); // Property name
  MetaDataLen += strlen(File->FileDescription); // Value of the property
  MetaDataLen += strlen("Title"); // Property name
  MetaDataLen += strlen(File->FileTitle); // Value of the property
  MetaDataLen += strlen("Author"); // Property name
  MetaDataLen += strlen(File->FileAuthor); // Value of the property
  
  /*** Groups meta data ***/
  for(CounterI=0; CounterI<nog; CounterI++)
  {
    MetaDataLen += 24;
    MetaDataLen += strlen(File->GroupArray[CounterI]->GroupPath); // Groups path
    MetaDataLen += strlen("Description"); // Property name
    // Value of the property
    MetaDataLen += strlen(File->GroupArray[CounterI]->GroupDescription);
    
    /*** Channels meta data ***/
    for(CounterJ=0; CounterJ<noc[CounterI]; CounterJ++)
    {
      MetaDataLen += 36;
      MetaDataLen += strlen(File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelPath); //Channels path
      MetaDataLen += strlen("Description"); // Property name
      MetaDataLen += strlen(File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelDescription); // Value of the property
      MetaDataLen += strlen("Unit"); // Property name
      MetaDataLen += strlen(File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelUitString); // Value of the property
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
                                          0x03); // Number of properties

  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         "Description"); // first Property name
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          tdsTypeString); // Data type of the property value
  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         File->FileDescription); // Value of the property

  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         "Title"); // second Property name
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          tdsTypeString); // Data type of the property value
  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         File->FileTitle); // Value of the property

  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         "Author"); // third Property name
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          tdsTypeString); //Data type of the property value
  DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                         File->FileAuthor); // Value of the property

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
                                            0x01); // Number of properties

    DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                           "Description"); // first Property name
    DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                            tdsTypeString); // Data type of the property value
    DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                           File->GroupArray[CounterI]->GroupDescription); // Value of the property

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
                                              0x02); // Number of properties

      DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                             "Description"); // first Property name
      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              tdsTypeString); // Data type of the property value
      DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                             File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelDescription); // Value of the property

      ///////////////////////////////////////
      DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                             "Unit"); // second Property name
      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              tdsTypeString); // Data type of the property value
      DataSize += TDMS_SaveStrToMetaDataPart(&Buffer[DataSize],
                                             File->GroupArray[CounterI]->ChannelArray[CounterJ]->ChannelUitString); // Value of the property

    }//for(CounterJ=0; CounterJ<noc[CounterI]; CounterJ++)
  }  //for(CounterI=0; CounterI<nog; CounterI++)

  *Size = DataSize;
  
  #undef RawDataLen
  
  return TDMS_OK;
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
  RawDataLen = TDMS_DataBytesOfEachType(Channel->ChannelDataType) * NumOfValues;
  if(Channel->ChannelDataType == tdsTypeBoolean)
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
  if(Channel->ChannelDataType == tdsTypeString)
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
                                          (uint32_t)Channel->ChannelDataType); // Data type of the raw data assigned to this object
  DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                          0x01); // Dimension of the raw data array (must be 1)
  DataSize += TDMS_SaveDataLittleEndian64(&Buffer[DataSize],
                                          NumOfValues); // Number of raw data Values
  if (Channel->ChannelDataType == tdsTypeString)
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
  uint32_t RawDataLenCh[TDMS_MAX_CHANNEL_OF_GROUP] = {0}; // Raw Data for each channel
  uint32_t RawDataLen = 0; // Bytes
  uint32_t MetaDataLen = 0;
  uint32_t NumberOfObjects = 0;
  uint16_t CounterI = 0;

  void* Values[TDMS_MAX_CHANNEL_OF_GROUP];
  uint32_t NumOfValues[TDMS_MAX_CHANNEL_OF_GROUP];
  
  va_list valist;

  /* initialize valist for num number of arguments */
  va_start(valist, (Group->NumOfChannels * 2));

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
      RawDataLenCh[CounterI] = TDMS_DataBytesOfEachType(Group->ChannelArray[CounterI]->ChannelDataType) * NumOfValues[CounterI];
      
      // Meta Data
      MetaDataLen += 28;
      if(Group->ChannelArray[CounterI]->ChannelDataType == tdsTypeString)
        MetaDataLen += 8; // Total Size in bytes (only stored for variable length data types, e.g. strings)
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
                                              (uint32_t)Group->ChannelArray[CounterI]->ChannelDataType); // Data type of the raw data assigned to this object
      DataSize += TDMS_SaveDataLittleEndian32(&Buffer[DataSize],
                                              0x01); // Dimension of the raw data array (must be 1)
      DataSize += TDMS_SaveDataLittleEndian64(&Buffer[DataSize],
                                              NumOfValues[CounterI]); // Number of raw data Values
      if (Group->ChannelArray[CounterI]->ChannelDataType == tdsTypeString)
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
