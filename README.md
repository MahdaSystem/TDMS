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
if (TDMS_InitFile(&FileTDMS) != TDMS_OK)
    printf("Create file failed!");

if (TDMS_AddGroupToFile(&Group1,
                        &FileTDMS,
                        "Group 1 name") != TDMS_OK)
    printf("Add Group failed!");

if (TDMS_AddChannelToGroup(&Channel1Group1,
                           &Group1,
                           "Channel 1 name",
                           TDMS_DataType_U8) != TDMS_OK)
    printf("Add Channel 1 failed!");
```
 5. Generate first part of file with `TDMS_GenFirstPart` and save the out buffer on disk.
 6. Add properties to File, Channel Group or Channel with `TDMS_AddPropertyToFile`, `TDMS_AddPropertyToGroup` or `TDMS_AddPropertyToChannel` then save the out buffer on disk.
 7. Add data to Channels with `TDMS_SetChannelDataValues` or add data to all Channels of a Channel Group with `TDMS_SetGroupDataValues` then save the out buffer on disk.

## Example
To run the basic example, follow these steps:
  1. Clone the repository
```bash
git clone https://github.com/MahdaSystem/TDMS.git
```
  2. Go to the example directory
```bash
cd ./TDMS/example/basic
```
  3. Run the make command
```bash
make all
```
  4. Run the example
```bash
./build/output.elf
```
  5. The `.tdms` file will be generated in the `build` directory
