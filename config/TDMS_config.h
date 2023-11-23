/**
 **********************************************************************************
 * @file   TDMS_config.h
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  Configuration part of TDMS library
 **********************************************************************************
 */

/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef _TDMS_CONFIG_H_
#define _TDMS_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Functionality Options --------------------------------------------------------*/
/**
 * @brief  Max NULL terminated strings length of File, Group and Channel data
 *         types
 */
#define TDMS_GROUP_NAME_LEN           30
#define TDMS_CHANNEL_NAME_LEN         30

/**
 * @brief  Determines max Channels of Group and max Groups of TDMS file
 */
#define TDMS_MAX_GROUP_OF_FILE        4
#define TDMS_MAX_CHANNEL_OF_GROUP     8

/**
 * @brief  Determines system Endianness
 *         - 0: little-endian
 *         - 1: big-endian
 */
#define TDMS_SYSTEM_ENDIANNESS        0



#ifdef __cplusplus
}
#endif

#endif //! _TDMS_CONFIG_H_