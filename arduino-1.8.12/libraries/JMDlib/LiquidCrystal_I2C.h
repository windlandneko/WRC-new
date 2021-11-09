/*** Translated by Charlie ***/
// YWROBOT
#ifndef LiquidCrystal_I2C_h
#define LiquidCrystal_I2C_h

#include "Print.h"
#include <Wire.h>
#include <inttypes.h>

// 命令
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// 显示屏输入模式
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// 显示屏开关切换
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// 显示位置/光标移动
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// 函数集
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// 背光控制
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100 // 启用位
#define Rw B00000010 // 读写位
#define Rs B00000001 // 寄存器选择位

class LiquidCrystal_I2C : public Print
{
public:
  LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
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
  void printLeft();
  void printRight();
  void leftToRight();
  void rightToLeft();
  void shiftIncrement();
  void shiftDecrement();
  void noBacklight();
  void backlight();
  void autoscroll();
  void noAutoscroll();
  void createChar(uint8_t, uint8_t[]);
  void createChar(uint8_t location, const char *charmap);
  // 例: 	const char bell[8] PROGMEM = {B00100,B01110,B01110,B01110,B11111,B00000,B00100,B00000};

  void setCursor(uint8_t, uint8_t);
#if defined(ARDUINO) && ARDUINO >= 100
  virtual size_t write(uint8_t);
#else
  virtual void write(uint8_t);
#endif
  void command(uint8_t);
  void init();

  // 兼容的 API
  void blink_on();                                             // 别名：blink()
  void blink_off();                                            // 别名：noBlink()
  void cursor_on();                                            // 别名：cursor()
  void cursor_off();                                           // 别名：noCursor()
  void setBacklight(uint8_t new_val);                          // 别名：backlight() and nobacklight()
  void load_custom_character(uint8_t char_num, uint8_t *rows); // 别名：createChar()
  void printstr(const char[]);

  // 不兼容的 API（这个库里没有实现）
  uint8_t status();
  void setContrast(uint8_t new_val);
  uint8_t keypad();
  void setDelay(int, int);
  void on();
  void off();
  uint8_t init_bargraph(uint8_t graphtype);
  void draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end);
  void draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end);

private:
  void init_priv();
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void expanderWrite(uint8_t);
  void pulseEnable(uint8_t);
  uint8_t _Addr;
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _numlines;
  uint8_t _cols;
  uint8_t _rows;
  uint8_t _backlightval;
};

#endif