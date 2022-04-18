/*
 * lcd.c
 *
 *  Created on: 18 April 2022
 *      Author: 20725728
 *
 *  Derived from Arduino library LiquidCrystal.ccp
 *  URL: https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp
 */

#include "lcd.h"

/*LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}*/


void init(uint8_t rs, uint8_t rw, uint8_t enable,
			 uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;

  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7;

  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

  begin(16, 1);
}

void begin(uint8_t cols, uint8_t lines) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 15 ms after power rises above 2.7 V
  // before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
  HAL_Delay(50);
  // Now we pull both RS and R/W low to begin commands
  HAL_GPIO_WritePin(RS_GPIO_Port,RS_Pin,_rs_pin);
  HAL_GPIO_WritePin(RNW_GPIO_Port, RNW_Pin,_rw_pin);
  HAL_GPIO_WritePin(E_GPIO_Port,E_Pin,_enable_pin);

  //put the LCD into 4 bit or 8 bit mode

	// this is according to the Hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03);
	HAL_Delay(45); // wait min 4.1ms

	// second try
	write4bits(0x03);
	HAL_Delay(45); // wait min 4.1ms

	// third go!
	write4bits(0x03);
	HAL_Delay(0.15);

	// finally, set to 4-bit interface
	write4bits(0x02);

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

void setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  HAL_Delay(2000);  // this command takes a long time!
}

void home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  HAL_Delay(2000);  // this command takes a long time!
}

void setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) {
    row = max_lines - 1;    // we count rows starting w/ 0
  }
  if ( row >= _numlines ) {
    row = _numlines - 1;    // we count rows starting w/ 0
  }

  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}


void scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

void command(uint8_t value) {
  send(value, 0);
}

uint8_t write(uint8_t value) {
  send(value, 1);
  return 1; // assume success
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void send(uint8_t value, uint8_t mode) {
  HAL_GPIO_WritePin(RS_GPIO_Port,RS_Pin,mode);

  //Set RNW low to Write
  HAL_GPIO_WritePin(RNW_GPIO_Port,RNW_Pin,0);
  write4bits(value>>4);
  write4bits(value);
}

void pulseEnable(void) {
  HAL_GPIO_WritePin(E_GPIO_Port,E_Pin,0);
  HAL_Delay(1);
  HAL_GPIO_WritePin(E_GPIO_Port,E_Pin,0);
  HAL_Delay(1);    // enable pulse must be >450 ns
  HAL_GPIO_WritePin(E_GPIO_Port,E_Pin,0);
  HAL_Delay(100);   // commands need >37 us to settle
}

void write4bits(uint8_t value) {
  HAL_GPIO_WritePin(DB4_GPIO_Port,DB4_Pin, (value >> 0) & 0x01);
  HAL_GPIO_WritePin(DB5_GPIO_Port,DB5_Pin, (value >> 1) & 0x01);
  HAL_GPIO_WritePin(DB6_GPIO_Port,DB6_Pin, (value >> 2) & 0x01);
  HAL_GPIO_WritePin(DB7_GPIO_Port,DB7_Pin, (value >> 3) & 0x01);

  pulseEnable();
}
