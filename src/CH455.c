/**
 **********************************************************************************
 * @file   CH455.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  CH455 chip driver
 *         Functionalities of the this file:
 *          + Display config and control functions
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

/* Includes ---------------------------------------------------------------------*/
#include "CH455.h"


/* Private Constants ------------------------------------------------------------*/
/**
 * @brief  Commands
 * @note   These commands used as address in communication protocol. If you want
 *         to share the same address with other devices, you must pick another
 *         solution. Refer to the datasheet for more information about the
 *         communication protocol.
 */
#define COMMAND_SET_SYSTEM_PARAMETER    0x48 // 0b01001000
#define COMMAND_LOAD_WORD_DATA_0        0x68 // 0b01101000
#define COMMAND_LOAD_WORD_DATA_1        0x6A // 0b01111000
#define COMMAND_LOAD_WORD_DATA_2        0x6C // 0b01110000
#define COMMAND_LOAD_WORD_DATA_3        0x6E // 0b01110100
#define COMMAND_READ_KEY_PRESS_CODE     0x4F // 0b01001111 

/**
 * @brief  Set system parameter command bits
 */
#define COMMAND_SET_SYSTEM_PARAMETER_ENA      0
#define COMMAND_SET_SYSTEM_PARAMETER_SLEEP    2
#define COMMAND_SET_SYSTEM_PARAMETER_7SEG     3
#define COMMAND_SET_SYSTEM_PARAMETER_INTENS   4


/* Private Macros ---------------------------------------------------------------*/
#define CH455_CHECK_PLATFORM_INIT(HANDLER)       ((HANDLER)->Platform.Init)
#define CH455_CHECK_PLATFORM_DEINIT(HANDLER)     ((HANDLER)->Platform.DeInit)
#define CH455_CHECK_PLATFORM_SEND(HANDLER)       ((HANDLER)->Platform.Send)
#define CH455_CHECK_PLATFORM_RECEIVE(HANDLER)    ((HANDLER)->Platform.Receive)

#define CH455_PLATFORM_INIT(HANDLER)                      (HANDLER)->Platform.Init()
#define CH455_PLATFORM_DEINIT(HANDLER)                    (HANDLER)->Platform.DeInit()
#define CH455_PLATFORM_SEND(HANDLER, ADDR, DATA, LEN)     (HANDLER)->Platform.Send(ADDR, DATA, LEN)
#define CH455_PLATFORM_RECEIVE(HANDLER, ADDR, DATA, LEN)  (HANDLER)->Platform.Receive(ADDR, DATA, LEN)

#define CH455_CHECK_RES_PLATFORM(FUNC)  (FUNC >= 0)


/* Private variables ------------------------------------------------------------*/
/**
 * @brief  Convert HEX number to Seven-Segment code
 */
const uint8_t HexTo7Seg[40] =
{
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x77, // A
  0x7c, // b
  0x39, // C
  0x5E, // d
  0x79, // E
  0x71, // F
  0x6F, // g
  0x3D, // G
  0x74, // h
  0x76, // H
  0x05, // i
  0x06, // I
  0x0D, // j
  0x30, // l
  0x38, // L
  0x54, // n
  0x37, // N
  0x5C, // o
  0x3F, // O
  0x73, // P
  0x67, // q
  0x50, // r
  0x6D, // S
  0x78, // t
  0x1C, // u
  0x3E, // U
  0x66, // y
  0x08, // _
  0x40, // -
  0x01  // Overscore
};



/**
 ==================================================================================
                           ##### Private Functions #####                           
 ==================================================================================
 */

static inline int8_t
CH455_CommandWrite(CH455_Handler_t *Handler,
                   uint8_t Command, uint8_t Data)
{
  return CH455_PLATFORM_SEND(Handler, Command >> 1, &Data, 1);
}

static inline int8_t
CH455_CommandRead(CH455_Handler_t *Handler,
                  uint8_t Command, uint8_t *Data)
{
  return CH455_PLATFORM_RECEIVE(Handler, Command >> 1, Data, 1);
}

static void
CH455_HexTo7Seg(const uint8_t *Hex, uint8_t *Data, uint8_t Count)
{
  uint8_t DecimalPoint = 0;
  uint8_t Digit = 0;

  if (!Hex || !Data)
    return;

  for (uint8_t i = 0; i < Count; i++)
  {
    DecimalPoint = Hex[i] & CH455_DECIMAL_POINT;
    Digit = Hex[i] & (~CH455_DECIMAL_POINT);

    if (Digit <= 15)
    {
      Data[i] = HexTo7Seg[Digit] | DecimalPoint;
    }
    else
    {
      switch (Digit)
      {
      case 'A':
      case 'a':
        Data[i] = HexTo7Seg[0x0A] | DecimalPoint;
        break;

      case 'B':
      case 'b':
        Data[i] = HexTo7Seg[0x0B] | DecimalPoint;
        break;

      case 'C':
      case 'c':
        Data[i] = HexTo7Seg[0x0C] | DecimalPoint;
        break;

      case 'D':
      case 'd':
        Data[i] = HexTo7Seg[0x0D] | DecimalPoint;
        break;

      case 'E':
      case 'e':
        Data[i] = HexTo7Seg[0x0E] | DecimalPoint;
        break;

      case 'F':
      case 'f':
        Data[i] = HexTo7Seg[0x0F] | DecimalPoint;
        break;

      default:
        Data[i] = 0;
        break;
      }
    }
  }
}

static void
CH455_StringTo7Seg(const char *Str, uint8_t *Data, uint8_t Count)
{
  uint8_t DecimalPoint = 0;
  char Ch = 0;

  if (!Str|| !Data)
    return;

  for (uint8_t i = 0; i < Count; i++)
  {
    if (Str[i] == '.')
      DecimalPoint = CH455_DECIMAL_POINT;
    else
      DecimalPoint = Str[i] & CH455_DECIMAL_POINT;
    
    Ch = Str[i] & (~CH455_DECIMAL_POINT);

    // numbers 0 - 9
    if (Ch >= '0' && Ch <= '9')
    {
      Data[i] = HexTo7Seg[Ch - '0'] | DecimalPoint;
    }
    else
    {
      switch (Ch)
      {
      case 'A':
      case 'a':
        Data[i] = HexTo7Seg[0x0A] | DecimalPoint;
        break;

      case 'B':
      case 'b':
        Data[i] = HexTo7Seg[0x0B] | DecimalPoint;
        break;

      case 'C':
      case 'c':
        Data[i] = HexTo7Seg[0x0C] | DecimalPoint;
        break;

      case 'D':
      case 'd':
        Data[i] = HexTo7Seg[0x0D] | DecimalPoint;
        break;

      case 'E':
      case 'e':
        Data[i] = HexTo7Seg[0x0E] | DecimalPoint;
        break;

      case 'F':
      case 'f':
        Data[i] = HexTo7Seg[0x0F] | DecimalPoint;
        break;

      case 'g':
        Data[i] = HexTo7Seg[0x10] | DecimalPoint;
        break;

      case 'G':
        Data[i] = HexTo7Seg[0x11] | DecimalPoint;
        break;

      case 'h':
        Data[i] = HexTo7Seg[0x12] | DecimalPoint;
        break;

      case 'H':
        Data[i] = HexTo7Seg[0x13] | DecimalPoint;
        break;

      case 'i':
        Data[i] = HexTo7Seg[0x14] | DecimalPoint;
        break;

      case 'I':
        Data[i] = HexTo7Seg[0x15] | DecimalPoint;
        break;

      case 'j':
      case 'J':
        Data[i] = HexTo7Seg[0x16] | DecimalPoint;
        break;

      case 'l':
        Data[i] = HexTo7Seg[0x17] | DecimalPoint;
        break;

      case 'L':
        Data[i] = HexTo7Seg[0x18] | DecimalPoint;
        break;

      case 'n':
        Data[i] = HexTo7Seg[0x19] | DecimalPoint;
        break;

      case 'N':
        Data[i] = HexTo7Seg[0x1A] | DecimalPoint;
        break;

      case 'o':
        Data[i] = HexTo7Seg[0x1B] | DecimalPoint;
        break;

      case 'O':
        Data[i] = HexTo7Seg[0x1C] | DecimalPoint;
        break;

      case 'p':
      case 'P':
        Data[i] = HexTo7Seg[0x1D] | DecimalPoint;
        break;

      case 'q':
      case 'Q':
        Data[i] = HexTo7Seg[0x1E] | DecimalPoint;
        break;

      case 'r':
      case 'R':
        Data[i] = HexTo7Seg[0x1F] | DecimalPoint;
        break;

      case 's':
      case 'S':
        Data[i] = HexTo7Seg[0x20] | DecimalPoint;
        break;

      case 't':
      case 'T':
        Data[i] = HexTo7Seg[0x21] | DecimalPoint;
        break;

      case 'u':
        Data[i] = HexTo7Seg[0x22] | DecimalPoint;
        break;

      case 'U':
        Data[i] = HexTo7Seg[0x23] | DecimalPoint;
        break;

      case 'y':
      case 'Y':
        Data[i] = HexTo7Seg[0x24] | DecimalPoint;
        break;

      case '_':
        Data[i] = HexTo7Seg[0x25] | DecimalPoint;
        break;

      case '-':
        Data[i] = HexTo7Seg[0x26] | DecimalPoint;
        break;

      case '~':
        Data[i] = HexTo7Seg[0x27] | DecimalPoint;
        break;

      default:
        Data[i] = 0;
        break;
      }
    }
  }
}



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
CH455_Init(CH455_Handler_t *Handler, CH455_DisplayType_t DisplayType)
{
  if (!CH455_CHECK_PLATFORM_SEND(Handler) ||
      !CH455_CHECK_PLATFORM_RECEIVE(Handler))
      return CH455_INVALID_PARAM;

  if (DisplayType != CH455_DISPLAY_TYPE_7X4 &&
      DisplayType != CH455_DISPLAY_TYPE_8X4)
    return CH455_INVALID_PARAM;

  Handler->DisplayType = DisplayType;

  if (CH455_CHECK_PLATFORM_INIT(Handler))
    if (!CH455_CHECK_RES_PLATFORM(CH455_PLATFORM_INIT(Handler)))
      return CH455_FAIL;

  return CH455_SetMultipleDigit(Handler, (void *) 0, 0, 4);
}

/**
 * @brief  De-Initialize CH455.
 * @param  Handler: Pointer to handler
 * @retval CH455_Result_t
 *         - CH455_OK: Operation was successful.
 */
CH455_Result_t
CH455_DeInit(CH455_Handler_t *Handler)
{
  if (CH455_CHECK_PLATFORM_DEINIT(Handler))
    if (CH455_CHECK_RES_PLATFORM(CH455_PLATFORM_DEINIT(Handler)))
      return CH455_FAIL;

  return CH455_OK;
}



/**
 ==================================================================================
                        ##### Public Display Functions #####                       
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
                    uint8_t DisplayState, uint8_t Sleep, uint8_t Brightness)
{
  uint8_t CommandData = 0;

  if (DisplayState != CH455_DISPLAY_STATE_OFF)
    CommandData |= (1 << COMMAND_SET_SYSTEM_PARAMETER_ENA);

  if (Sleep)
    CommandData |= (1 << COMMAND_SET_SYSTEM_PARAMETER_SLEEP);

  if (Handler->DisplayType == CH455_DISPLAY_TYPE_7X4)
    CommandData |= (1 << COMMAND_SET_SYSTEM_PARAMETER_7SEG);

  if (Brightness > 7)
    return CH455_INVALID_PARAM;

  CommandData |= (Brightness << COMMAND_SET_SYSTEM_PARAMETER_INTENS);

  if (!CH455_CHECK_RES_PLATFORM(CH455_CommandWrite(Handler,
                                                   COMMAND_SET_SYSTEM_PARAMETER,
                                                   CommandData)))
    return CH455_FAIL;

  return CH455_OK;
}

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
                     uint8_t DigitData, uint8_t DigitPos)
{
  return CH455_SetMultipleDigit(Handler, &DigitData, DigitPos, 1);
}


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
                        uint8_t StartAddr, uint8_t Count)
{
  if (StartAddr > 3 || (StartAddr + Count) > 4)
    return CH455_INVALID_PARAM;

  for (int i = 0; i < Count; i++)
  {
    uint8_t Command = 0;
    switch (StartAddr + i)
    {
    case 0:
      Command = COMMAND_LOAD_WORD_DATA_0;
      break;

    case 1:
      Command = COMMAND_LOAD_WORD_DATA_1;
      break;

    case 2:
      Command = COMMAND_LOAD_WORD_DATA_2;
      break;

    case 3:
      Command = COMMAND_LOAD_WORD_DATA_3;
      break;

    default:
      return CH455_INVALID_PARAM;
      break;
    }

    if (!CH455_CHECK_RES_PLATFORM(CH455_CommandWrite(Handler, Command,
                                                     DigitData ? DigitData[i] : 0)))
      return CH455_FAIL;
  }

  return CH455_OK;
}


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
                          uint8_t DigitData, uint8_t DigitPos)
{
  uint8_t DigitDataOut = 0;
  CH455_HexTo7Seg((const uint8_t *)&DigitData, &DigitDataOut, 1);
  return CH455_SetSingleDigit(Handler, DigitDataOut, DigitPos);
}


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
                            uint8_t StartAddr, uint8_t Count)
{
  uint8_t DigitDataOut[16];

  if (Count > 16)
    Count = 16;

  CH455_HexTo7Seg(DigitData, DigitDataOut, Count);
  return CH455_SetMultipleDigit(Handler,
                                (const uint8_t *)DigitDataOut, StartAddr, Count);
}


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
                           char Char, uint8_t DigitPos)
{
  uint8_t DigitData = 0;
  CH455_StringTo7Seg((const char *)&DigitData, &DigitData, 1);
  return CH455_SetSingleDigit(Handler, DigitData, DigitPos);
}

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
                             uint8_t StartAddr, uint8_t Count)
{
  uint8_t DigitData[16];

  if (Count > 16)
    Count = 16;

  CH455_StringTo7Seg(Str, DigitData, Count);
  return CH455_SetMultipleDigit(Handler,
                                (const uint8_t *)DigitData, StartAddr, Count);
}
