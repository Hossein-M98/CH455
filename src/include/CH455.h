/**
 **********************************************************************************
 * @file   CH455.h
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  CH455 chip driver
 *         Functionalities of the this file:
 *          + Display config and control functions
 *          + Keypad scan functions
 **********************************************************************************
 *
 * Copyright (c) 2026 Hossein.M (MIT License)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************************************
 */
  
/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef _CH455_H_
#define _CH455_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ---------------------------------------------------------------------*/
#include <stdint.h>


/* Exported Constants -----------------------------------------------------------*/
#define CH455_DISPLAY_STATE_OFF   0
#define CH455_DISPLAY_STATE_ON    1

#define CH455_DECIMAL_POINT       0x80


/* Exported Data Types ----------------------------------------------------------*/

/**
 * @brief  Data type of library functions result
 */
typedef enum CH455_Result_e
{
  CH455_OK            = 0,
  CH455_FAIL          = -1,
  CH455_INVALID_PARAM = -2,
} CH455_Result_t;

/**
 * @brief  Display type
 * @note   If you choose 4x7 display, SEG7 is used as key-press interrupt output.
 *         You can use this output to detect key press.
 */
typedef enum CH455_DisplayType_e
{
  CH455_DISPLAY_TYPE_7X4 = 0,
  CH455_DISPLAY_TYPE_8X4 = 1,
} CH455_DisplayType_t;

/**
 * @brief  Function type for Initialize/Deinitialize the platform dependent layer.
 * @retval 
 *         -  0: The operation was successful.
 *         - -1: The operation failed. 
 */
typedef int8_t (*CH455_Platform_InitDeinit_t)(void);


/**
 * @brief  Function type for Send/Receive data to/from the slave.
 * @param  Address: Address of slave (0 <= Address <= 127)
 * @param  Data: Pointer to data
 * @param  Len: data len in Bytes
 * @retval 
 *         -  0: The operation was successful.
 *         - -1: Failed to send/receive.
 *         - -2: Bus is busy.
 *         - -3: Slave doesn't ACK the transfer.
 */
typedef int8_t (*CH455_Platform_SendReceive_t)(uint8_t Address,
                                               uint8_t *Data, uint8_t Len);


/**
 * @brief  Platform dependent layer data type
 * @note   It is optional to initialize this functions:
 *         - Init
 *         - DeInit
 * @note   If success the functions must return 0 
 */
typedef struct CH455_Platform_s
{
  // Initialize platform dependent layer
  CH455_Platform_InitDeinit_t Init;
  // De-initialize platform dependent layer
  CH455_Platform_InitDeinit_t DeInit;
  // Send Data to the slave
  CH455_Platform_SendReceive_t Send;
  // Receive Data from the slave
  CH455_Platform_SendReceive_t Receive;
} CH455_Platform_t;


/**
 * @brief  Handler data type
 * @note   User must initialize platform dependent layer functions
 */
typedef struct CH455_Handler_s
{
  CH455_DisplayType_t DisplayType;

  // Platform dependent layer
  CH455_Platform_t Platform;
} CH455_Handler_t;


/* Exported Macros --------------------------------------------------------------*/
/**
 * @brief  Link platform dependent layer functions to handler
 * @param  HANDLER: Pointer to handler
 * @param  FUNC: Function name
 */
#define CH455_PLATFORM_LINK_INIT(HANDLER, FUNC) \
  (HANDLER)->Platform.Init = FUNC

/**
 * @brief  Link platform dependent layer functions to handler
 * @param  HANDLER: Pointer to handler
 * @param  FUNC: Function name
 */
#define CH455_PLATFORM_LINK_DEINIT(HANDLER, FUNC) \
  (HANDLER)->Platform.DeInit = FUNC

/**
 * @brief  Link platform dependent layer functions to handler
 * @param  HANDLER: Pointer to handler
 * @param  FUNC: Function name
 */
#define CH455_PLATFORM_LINK_SEND(HANDLER, FUNC) \
  (HANDLER)->Platform.Send = FUNC

/**
 * @brief  Link platform dependent layer functions to handler
 * @param  HANDLER: Pointer to handler
 * @param  FUNC: Function name
 */
#define CH455_PLATFORM_LINK_RECEIVE(HANDLER, FUNC) \
  (HANDLER)->Platform.Receive = FUNC



/**
 ==================================================================================
                           ##### Common Functions #####                           
 ==================================================================================
 */

/**
 * @brief  Initialize CH455.
 * @param  Handler: Pointer to handler
 * @param  DisplayType: Display type
 *         - CH455_DISPLAY_TYPE_7X4: 7-segment display with 4 digits. SEG7 is used
 *                                   as key-press interrupt output.
 *         - CH455_DISPLAY_TYPE_8X4: 8-segment display with 4 digits
 * 
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 *         - CH455_FAIL: Failed to initialize.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_Init(CH455_Handler_t *Handler, CH455_DisplayType_t DisplayType);


/**
 * @brief  De-Initialize CH455.
 * @param  Handler: Pointer to handler
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 */
CH455_Result_t
CH455_DeInit(CH455_Handler_t *Handler);



/**
 ==================================================================================
                           ##### Display Functions #####                           
 ==================================================================================
 */

/**
 * @brief  Config display parameters
 * @param  Handler: Pointer to handler
 * @param  DisplayState: Set display ON or OFF
 *         - CH455_DISPLAY_STATE_OFF: Set display and key-scan state OFF
 *         - CH455_DISPLAY_STATE_ON: Set display and key-scan state ON
 * 
 * @param  Sleep: Set sleep mode
 *         - 0: Do not sleep
 *         - 1: Sleep
 * 
 * @param  Brightness: Set brightness level
 *         - 0: Display pulse width is set as 8/8
 *         - 1: Display pulse width is set as 1/8
 *         - 2: Display pulse width is set as 2/8
 *         - 3: Display pulse width is set as 3/8
 *         - 4: Display pulse width is set as 4/8
 *         - 5: Display pulse width is set as 5/8
 *         - 6: Display pulse width is set as 6/8
 *         - 7: Display pulse width is set as 7/8
 * 
 * CH455_Result_t
 *         - CH455_OK: Operation was successful
 *         - CH455_FAIL: Failed to configure.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_ConfigDisplay(CH455_Handler_t *Handler,
                    uint8_t DisplayState, uint8_t Sleep, uint8_t Brightness);


/**
 * @brief  Set data to single digit in 7-segment format
 * @param  Handler: Pointer to handler
 * @param  DigitData: Digit data
 * @param  DigitPos: Digit position
 *         - 0: Seg1
 *         - 1: Seg2
 *         - .
 *         - .
 *         - .
 * 
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful
 */
CH455_Result_t
CH455_SetSingleDigit(CH455_Handler_t *Handler,
                     uint8_t DigitData, uint8_t DigitPos);


/**
 * @brief  Set data to multiple digits in 7-segment format
 * @param  Handler: Pointer to handler
 * @param  DigitData: Array to Digits data
 * @param  StartAddr: First digit position
 *         - 0: Seg1
 *         - 1: Seg2
 *         - .
 *         - .
 *         - .
 * 
 * @param  Count: Number of segments to write data
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 *         - CH455_FAIL: Failed to write.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_SetMultipleDigit(CH455_Handler_t *Handler, const uint8_t *DigitData,
                       uint8_t StartAddr, uint8_t Count);


/**
 * @brief  Set data to single digit in hexadecimal format
 * @param  Handler: Pointer to handler
 * @param  DigitData: Digit data (0, 1, ... , 15, a, A, b, B, ... , f, F) 
 * @param  DigitPos: Digit position
 *         - 0: Seg1
 *         - 1: Seg2
 *         - .
 *         - .
 *         - .
 * 
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 *         - CH455_FAIL: Failed to write.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_SetSingleDigit_HEX(CH455_Handler_t *Handler,
                         uint8_t DigitData, uint8_t DigitPos);


/**
 * @brief  Set data to multiple digits in hexadecimal format
 * @param  Handler: Pointer to handler
 * @param  DigitData: Array to Digits data. 
 *                    (0, 1, ... , 15, a, A, b, B, ... , f, F)
 * 
 * @param  StartAddr: First digit position
 *         - 0: Seg1
 *         - 1: Seg2
 *         - .
 *         - .
 *         - .
 * 
 * @param  Count: Number of segments to write data
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 *         - CH455_FAIL: Failed to write.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_SetMultipleDigit_HEX(CH455_Handler_t *Handler, const uint8_t *DigitData,
                           uint8_t StartAddr, uint8_t Count);


/**
 * @brief  Set data to single digit in char format
 * @param  Handler: Pointer to handler
 * @param  Char: Character to display
 *               Supported chars 0,1,2,3,4,5,6,7,8,9
 *                               A,b,C,d,E,F,g,G,h,H,i,I,j,l,L,n,N,o,O,P,q,r,S,
 *                               t,u,U,y,_,-,Overscore (use ~ to set)
 * @param  DigitPos: Digit position
 *         - 0: Seg1
 *         - 1: Seg2
 *         - .
 *         - .
 *         - .
 * 
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 *         - CH455_FAIL: Failed to write.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_SetSingleDigit_CHAR(CH455_Handler_t *Handler,
                          char Char, uint8_t DigitPos);


/**
 * @brief  Set data to multiple digits in char format
 * @param  Handler: Pointer to handler
 * @param  Str: String of characters
 *              Supported chars 0,1,2,3,4,5,6,7,8,9
 *                              A,b,C,d,E,F,g,G,h,H,i,I,j,l,L,n,N,o,O,P,q,r,S,
 *                              t,u,U,y,_,-,Overscore (use ~ to set)
 * 
 * @param  StartAddr: First digit position
 *         - 0: Seg1
 *         - 1: Seg2
 *         - .
 *         - .
 *         - .
 * 
 * @param  Count: Number of segments to write data
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 *         - CH455_FAIL: Failed to write.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_SetMultipleDigit_CHAR(CH455_Handler_t *Handler, const char *Str,
                            uint8_t StartAddr, uint8_t Count);



/** 
 ==================================================================================
                           ##### Keypad Functions #####                            
 ==================================================================================
 */

/**
 * @brief  Read key-press code.
 * @param  Handler: Pointer to handler
 * @param  KeyAddress: Pointer to key address. Refer to the datasheet for more information
 * @param  State: Pointer to key state
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 *         - CH455_FAIL: Failed to write.
 *         - CH455_INVALID_PARAM: Invalid parameter.
 */
CH455_Result_t
CH455_ScanKeys(CH455_Handler_t *Handler, uint8_t *KeyAddress, uint8_t *State);



#ifdef __cplusplus
}
#endif

#endif //! _CH455_H_
