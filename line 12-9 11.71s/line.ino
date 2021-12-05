#include "JMDlib.h"
#include "line.h"
int team, color, iter = 0;

// ================ 设置 ================
int box_id[] = {near, far}; // 液体 红色 ; 食物 蓝色
bool force_send = true,     // 是否强制预定送货位置
    test_enable = true;    // 是否自动进行时间测试
int order_list[] = {0,      // 强制预定列表 (运行中按键取消)
                    RED, RED, RED, BLUE, BLUE, END};
// =====================================

// =========== 等待货物 ===========
bool get_color()
{
  int Rc = 0, Rb = 0, R, G, B;
  while (true)
  {
    R = getColorSensorPin(0, 6);
    G = getColorSensorPin(0, 7);
    B = getColorSensorPin(0, 8);
    if (R > 15 && G > 15 && B > 15)
    {
      if (R > B)
        Rc++;
      else
        Rb++;
    }
    if (Rc >= 10)
      return RED;
    if (Rb >= 10)
      return BLUE;
    if (!getKey()) // 运行中按下按键 取消预定
    {
      if (force_send)
        newtone(TONE_AH3, 80);
      force_send = false;
    }
  }
}

// =============== 转弯 ===============
// 0 LEFT 左转        1 RIGHT 右转
inline void turn(bool direction)
{
  Turn((direction ? 35 : -17),
       (direction ? -17 : 35),
       (direction ? 4 : 2));
  delay(20);
}

// 送完货返回
inline void weird_turn(bool direction)
{
  Turn((direction ? 45 : -15),
       (direction ? -15 : 45),
       (direction ? 5 : 1));
}

void quick_send(int number, int team)
{
  gocode(690, 60, 60);
  goline(1, 50);
  goline(3, 60);
  turn(team == RED);
  goline(1, 55);
  goline(2 * number - (number == 4 ? 3 : 1), 70);
  goline(2, 50);
  turn(team == BLUE);
  goline(1, 25);
  setservo(4, 140);
  delay(500);
  setservo(4, 75);
  gotime(600, -35, -35);
  if (number == 2)
    weird_turn(team == BLUE);
  else
    turn(team == BLUE);
  goline(1, 55);
  goline(2 * number - (number == 4 ? 3 : 1), 70);
  goline(2, 45);
  turn(team == BLUE);
  goline(1, 50);
  goline(3, 70);
  gocode(500, 40, 40);
  gocode(1180, 32, -32);
  gocode(800, -25, -25);
  gotime(200, -10, -10);
}

// =============== 时间测试 ===============
void time_test()
{
  unsigned long clock;
  int numbertone[] = {TONE_CL7, TONE_C1, TONE_C2, TONE_C3, TONE_C4, TONE_C5, TONE_C6, TONE_C7, TONE_CH1, TONE_CH2};
  delay(1000);
  clock = millis();
  quick_send(2, team);
  delay(1000);
  quick_send(3, team);
  clock = (millis() - clock - 1000) / 2;
  delay(500);
  memwrite(clock, 0);
  newtone(numbertone[(clock / 10000) % 10], 800);
  delay(300);
  newtone(numbertone[(clock / 1000) % 10], 800);
  delay(300);
  newtone(numbertone[(clock / 100) % 10], 800);
  delay(300);
  newtone(numbertone[(clock / 10) % 10], 800);
}

// ================ 程序开始 ================
void setup()
{
  setMusic(0); // 播放美妙的音乐
  analysis();  // 输出调试信息
  sets();

  // 设置队伍
  team = EEPROM.read(10);
  if (abs(getMotor4Code()) > 100)
    team = BLUE;
  if (abs(getMotor3Code()) > 100)
    team = RED;
  EEPROM.write(10, team); // 记录下当前选择
  newtone((team == RED ? TONE_CH1 : TONE_CH3), 120);

  resetPid();
  delay(200);
  setMusic(1); // 播放美妙的音乐

  setservo(4, 75); // 进入等待获取货物状态

  if (test_enable)
    time_test();
}

void loop()
{
  color = get_color();
  if (order_list[iter + 1] == -1)
    force_send = false;
  quick_send(box_id[(force_send ? order_list[++iter] : color)], team);
}
