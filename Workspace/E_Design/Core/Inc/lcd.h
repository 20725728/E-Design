/*
 * lcd.h
 *
 *  Created on: 18 Apr 2022
 *      Author: 20725728
 *
 *  Derived from Arduino library LiquidCrystal.ccp
 *  URL: https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// GPIO Pins and Pin Ports
#define DB4_Pin GPIO_PIN_12
#define DB4_GPIO_Port GPIOA
#define DB5_Pin GPIO_PIN_5
#define DB5_GPIO_Port GPIOC
#define DB6_Pin GPIO_PIN_6
#define DB6_GPIO_Port GPIOC
#define DB7_Pin GPIO_PIN_8
#define DB7_GPIO_Port GPIOC
#define RS_Pin GPIO_PIN_1
#define RS_GPIO_Port GPIOB
#define RNW_Pin GPIO_PIN_2
#define RNW_GPIO_Port GPIOB
#define E_Pin GPIO_PIN_11
#define E_GPIO_Port GPIOA

#include "stm32f3xx_hal.h"


//  LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
	//	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);


  void init(uint8_t rs, uint8_t rw, uint8_t enable,
	    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

  void begin(uint8_t cols, uint8_t rows);
  //void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void setRowOffsets(int row1, int row2, int row3, int row4);
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);
  void command(uint8_t);

  uint8_t write(uint8_t value);
  void command(uint8_t value);
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void pulseEnable();

  uint8_t _rs_pin; // LOW: command.  HIGH: character.
  uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  uint8_t _enable_pin; // activated by a HIGH pulse.
  uint8_t _data_pins[8];

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines;
  uint8_t _row_offsets[4];

#endif /* INC_LCD_H_ */
