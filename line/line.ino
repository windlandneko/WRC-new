#include "JMDlib.h"
#include "line.h"
#define RED 0
#define BLUE 1

int team, halftime;
int color, box_id[2] = {2, 3};

// =========== 选择队伍 ===========
// 通过手动转动左或者右马达后，启动机器，完成机器选择红蓝方启动的过程
void team_select()
{
  if (getMotor4Code() > 100)
    team = BLUE;
  if (getMotor3Code() > 100)
    team = RED;
  newtone((team == RED ? TONE_CH1 : TONE_CH3), 120);

  EEPROM.write(10, team); // 记录下当前选择
  resetPid();
}

// =========== 转弯 ===========
// 0 左转   1 右转
inline void turn(bool direction, int speed = 40)
{
  Turn(speed * (direction ? 1 : -1.1),
       speed * (direction ? -1.1 : 1),
       (direction ? 5 : 1));
}

// =========== 等待货物 ===========
bool get_color()
{
  int R, G, B;
  resetPid();
  while (true)
  {
    R = getColorSensorPin(0, 6); // 获得颜色传感器R红色分量
    G = getColorSensorPin(0, 7); // 获得颜色传感器G
    B = getColorSensorPin(0, 8); // 获得颜色传感器B蓝色分量
    // 当数所有值大于15被认为已装货
    if (R > 15 && G > 15 && B > 15)
      return B > R;
    if (!getKey()) // 运行中按下了按键, 设置盒子编号
    {
      box_id[team] = (box_id[team] + 1) % 4; // 编号从 1 ~ 4 递增
      EEPROM.write(15 + team, box_id[team]);
      newtone((team == RED ? TONE_CH1 : TONE_CH3), 120);
      delay(50);
      newtone((box_id[team] == 0 ? TONE_C1 : (box_id[team] == 1 ? TONE_C2 : (box_id[team] == 3 ? TONE_C3 : TONE_C4))), 50);
    }
  }
}

// =========== 送货部分 ===========
void send(int number, int team, int line = 3)
{
  gocode(800, 50, 50); // 给速度让机器先越过彩色区域
  goline(line - 1);
  if (number != 0)
  {
    turn(team == RED);
    goline(2 * number);
    turn(team == BLUE);
    goline(5 - line);
    goline(1, 15, 1000);
  }
  else // number == 0
  {
    if (line == 1)
      gocode(500, 40, 40);
    goline(3);
    goline(1, 5, 1000);
  }

  setservo(4, 110); // 卸货
  delay(500);       // 扔块
  setservo(4, 70);  // 接货

  Turn(-50, 50, 1); // 掉头
  if (line != 4)
  {
    // gocode(800, 60, 60);  // 越过第一个路口
    goline(5 - line, 40); // (line == 4 ? 40 : 50)
  }
  else
    gocode(80, 50, 50);
  if (number != 0)
  {
    turn(team == RED);
    goline(2 * number);
    turn(team == BLUE);
    goline(line - 1);
  }
  else
    goline(3);
  gocode(400, 40, 40);
  gocode(1080, 40, -35); // 右转一点
  gocode(400, -40, -40); // 后退
  gotime(200, -30, -30); // 再后退
}

// =========== 时间测试 ===========
void time_test(int line)
{
  unsigned long clock;
  int numbertone[] = {TONE_CL7, TONE_C1, TONE_C2, TONE_C3, TONE_C4, TONE_C5, TONE_C6, TONE_C7, TONE_CH1, TONE_CH2};
  clock = millis();

  delay(1000);
  send(2, team, line);
  delay(1000);
  send(3, team, line);
  resetPid();

  // delay(1000);
  // send(2, team, line);
  // delay(1000);
  // send(3, team, line);
  // resetPid();

  clock = (millis() - clock - 2000) / 2;
  delay(1000);
  EEPROM.write(44, clock % 256);
  EEPROM.write(45, clock / 256);
  tone(9, numbertone[(clock / 10000) % 10], 800);
  delay(800);
  tone(9, numbertone[(clock / 1000) % 10], 800);
  delay(800);
  tone(9, numbertone[(clock / 100) % 10], 800);
  delay(800);
  tone(9, numbertone[(clock / 10) % 10], 800);
  delay(2000);
}

// =========== 程序开始 ===========
void setup()
{
  setMusic(0); // 播放美妙的音乐
  // 读取信息
  halftime = EEPROM.read(20);
  // box_id[RED] = EEPROM.read(15);
  // box_id[BLUE] = EEPROM.read(16);
  team = EEPROM.read(10);

  init_light_sensor();
  team_select(); // 选择红蓝方程序

  delay(200);
  setMusic(1); // 播放美妙的音乐

  setservo(4, 70); // 进入等待获取货物状态
  if (true)        // 为真进行时间测试
  {
    time_test(4);
    // time_test(3);
    // time_test(4);
  }
  else
    while (true)
      send(box_id[get_color()], team, 3);
}

void loop() {}
