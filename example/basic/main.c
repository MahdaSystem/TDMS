/**
 **********************************************************************************
 * @file   main.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  Basic example for TDMS library
 **********************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "TDMS.h"

#define MEMALLOC(size) calloc(4096, sizeof(uint8_t)); \
                       if(!Buffer) \
                       { \
                         printf("malloc failed!\n"); \
                         char ch; \
                         scanf("%c", &ch); \
                         return (1); \
                       }


int main()
{
  printf("TDMS Library test\n");

  FILE *MyFile;
  TDMS_File_t FileTDMS;
  TDMS_Group_t Group1;
  TDMS_Group_t Group2;
  TDMS_Channel_t Channel1Group1;
  TDMS_Channel_t Channel2Group1;
  TDMS_Channel_t Channel1Group2;

  uint8_t *Buffer;
  uint32_t Size = 0;

  MyFile = fopen("./build/Test.tdms", "wb");
  if (!MyFile)
  {
    printf("File open failed!\n");
    char ch;
    scanf("%c", &ch);
    return (1);
  }

  if (TDMS_InitFile(&FileTDMS) != TDMS_OK)
    printf("Init file failed!");


  if (TDMS_AddGroupToFile(&Group1,
                          &FileTDMS,
                          "Group 1 name") != TDMS_OK)
    printf("Add Group 1 failed!");

  if (TDMS_AddGroupToFile(&Group2,
                          &FileTDMS,
                          "Group 2 name") != TDMS_OK)
    printf("Add Group 2 failed!");


  if (TDMS_AddChannelToGroup(&Channel1Group1,
                             &Group1,
                             "Channel 1 name",
                             TDMS_DataType_U8) != TDMS_OK)
    printf("Add Channel 1 failed!");

  if (TDMS_AddChannelToGroup(&Channel2Group1,
                             &Group1,
                             "Channel 2 name",
                             TDMS_DataType_SingleFloat) != TDMS_OK)
    printf("Add Channel 2 failed!");

  if (TDMS_AddChannelToGroup(&Channel1Group2,
                             &Group2,
                             "Channel 3 name",
                             TDMS_DataType_TimeStamp) != TDMS_OK)
    printf("Add Channel 3 failed!");


  TDMS_GenFirstPart(&FileTDMS, NULL, &Size);
  Buffer = MEMALLOC(Size + 1);
  TDMS_GenFirstPart(&FileTDMS, Buffer, &Size);
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);


  TDMS_AddPropertyToFile(NULL,
                         &Size,
                         "Description",
                         TDMS_DataType_String,
                         "A file generated by TDMS library");
  Buffer = MEMALLOC(Size + 1);
  TDMS_AddPropertyToFile(Buffer,
                         &Size,
                         "Description",
                         TDMS_DataType_String,
                         "A file generated by TDMS library");
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);

  TDMS_AddPropertyToFile(NULL,
                         &Size,
                         "Author",
                         TDMS_DataType_String,
                         "Hossein-M98");
  Buffer = MEMALLOC(Size + 1);
  TDMS_AddPropertyToFile(Buffer,
                         &Size,
                         "Author",
                         TDMS_DataType_String,
                         "Hossein-M98");
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);


  TDMS_AddPropertyToGroup(&Group1,
                          NULL,
                          &Size,
                          "Description",
                          TDMS_DataType_String,
                          "This is Group 1");
  Buffer = MEMALLOC(Size + 1);
  TDMS_AddPropertyToGroup(&Group1,
                          Buffer,
                          &Size,
                          "Description",
                          TDMS_DataType_String, "This is Group 1");
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);


  TDMS_AddPropertyToChannel(&Channel1Group2,
                            NULL,
                            &Size,
                            "Description",
                            TDMS_DataType_String,
                            "This is a Date and Time channel");
  Buffer = MEMALLOC(Size + 1);
  TDMS_AddPropertyToChannel(&Channel1Group2,
                            Buffer,
                            &Size,
                            "Description",
                            TDMS_DataType_String,
                            "This is a Date and Time channel");
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);


  uint8_t Data[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};
  float Data2[] = {100.25, 101.5, 102.75, 103.25, 104.5, 105.75};
  TDMS_SetGroupDataValues(&Group1, NULL, &Size,
                          Data, sizeof(Data) / sizeof(uint8_t),
                          Data2, sizeof(Data2) / sizeof(float));
  Buffer = MEMALLOC(Size + 1);
  TDMS_SetGroupDataValues(&Group1, Buffer, &Size,
                          Data, sizeof(Data) / sizeof(uint8_t),
                          Data2, sizeof(Data2) / sizeof(float));
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);

  uint64_t second = TDMS_TimeSecond(2023, 5, 17, 12, 14, 10);
  TDMS_Timestamp_t Data4[] = {{.Fraction = 0, .Second = second}};
  TDMS_SetChannelDataValues(&Channel1Group2, NULL, &Size,
                            Data4, sizeof(Data4) / sizeof(TDMS_Timestamp_t));
  Buffer = MEMALLOC(Size + 1);
  TDMS_SetChannelDataValues(&Channel1Group2, Buffer, &Size,
                            Data4, sizeof(Data4) / sizeof(TDMS_Timestamp_t));
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);

  printf("Process finished successfully!\n");
  fclose(MyFile);
  return (0);
}
