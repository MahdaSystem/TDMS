# TDMS Library
Library for generating TDMS file format.

## Library Features
-   Support for Little-Endian systems
-   Support for integer, float, double, Boolean and string raw data 
-   Converting normal time and date to NI LabVIEW format

## How To Use
 1. Add library files to your project.
 2. Config `TDMS_config.h`.
 3. Define File, Channel Group and Channel object structures. For example:
```C
TDMS_File_t  FileTDMS;
TDMS_Group_t  Group1;
TDMS_Channel_t  Channel1Group1;
```
 4. Create File, Channel Group and Channel objects. For example:
```C
if (TDMS_CreateFile("File Description",
                    "File Title",
                    "File Author",
                    &FileTDMS) != TDMS_OK)
    printf("Create file failed!");

if (TDMS_AddGroup(&FileTDMS,
                  "Group 1 name",
                  "Group 1 Description",
                  &Group1) != TDMS_OK)
    printf("Add Group failed!");

if (TDMS_AddChannel(&Group1,
                    tdsTypeU8,
                    "Channel 1 name",
                    "Channel 1 Description",
                    "Channel 1 unit",
                    &Channel1Group1) != TDMS_OK)
    printf("Add Channel 1 failed!");
```
 5. Generate first part of file with `TDMS_GenFirstPart` and save the out buffer on disk.
 6. Add data to Channels with `TDMS_SetChannelDataValues` or add data to all Channels of a Channel Group with `TDMS_SetGroupDataValues` then save the out buffer on disk.

## Example
```C
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
  printf("Hello World!\n");

  FILE *MyFile;
  TDMS_File_t FileTDMS;
  TDMS_Group_t Group1;
  TDMS_Group_t Group2;
  TDMS_Channel_t Channel1Group1;
  TDMS_Channel_t Channel2Group1;
  TDMS_Channel_t Channel1Group2;

  uint8_t *Buffer;
  uint32_t Size = 0;

  MyFile = fopen("./output/Test.tdms", "wb");

  if (TDMS_CreateFile("File Description",
                      "File Title",
                      "File Author",
                      &FileTDMS) != TDMS_OK)
    printf("Create file failed!");

  if (TDMS_AddGroup(&FileTDMS,
                    "Group 1 name",
                    "Group 1 Description",
                    &Group1) != TDMS_OK)
    printf("Add Group failed!");
  
  if (TDMS_AddGroup(&FileTDMS,
                    "Group 2 name",
                    "Group 2 Description",
                    &Group2) != TDMS_OK)
    printf("Add Group failed!");

  if (TDMS_AddChannel(&Group1,
                      tdsTypeU8,
                      "Channel 1 name",
                      "Channel 1 Description",
                      "Channel 1 unit",
                      &Channel1Group1) != TDMS_OK)
    printf("Add Channel 1 failed!");

  if (TDMS_AddChannel(&Group1,
                      tdsTypeSingleFloat,
                      "Channel 2 name",
                      "Channel 2 Description",
                      "Channel 2 unit",
                      &Channel2Group1) != TDMS_OK)
    printf("Add Channel 2 failed!");

  if (TDMS_AddChannel(&Group2,
                      tdsTypeTimeStamp,
                      "Channel 1 name",
                      "Channel 1 Description",
                      "Channel 1 unit",
                      &Channel1Group2) != TDMS_OK)
    printf("Add Channel 4 failed!");

  TDMS_GenFirstPart(&FileTDMS, NULL, &Size);
  Buffer = MEMALLOC(Size + 1);
  TDMS_GenFirstPart(&FileTDMS, Buffer, &Size);
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

  uint64_t second = TDMS_TimeSecond(2021, 2, 22, 10, 0, 0);
  TDMS_Timestamp_t Data4[] = {{.Fraction = 0, .Second = second}};
  TDMS_SetChannelDataValues(&Channel1Group2, NULL, &Size,
                            Data4, sizeof(Data4) / sizeof(TDMS_Timestamp_t));
  Buffer = MEMALLOC(Size + 1);
  TDMS_SetChannelDataValues(&Channel1Group2, Buffer, &Size,
                            Data4, sizeof(Data4) / sizeof(TDMS_Timestamp_t));
  fwrite(Buffer, 1, Size, MyFile);
  free(Buffer);


  fclose(MyFile);
  return (0);
}
```
