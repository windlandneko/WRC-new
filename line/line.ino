#include "JMDlib.h"
#include "line.h"
#define RED 0
#define BLUE 1
// 扫描程序，先把机器放到场地上，按着按钮开机，听到提示音进入扫描程序，扫描过程会有短促提示音
// 这时让机器5个光电都经过一次黑线和白色区域，然后再按一次按钮跳出扫描程序
int team, halftime; // 红蓝方信息数值为1时为红方出发
int color, box_id[2] = {3, 4};
// 通过手动转动左或者右马达后，启动机器，完成机器选择红蓝方启动的过程
void team_select()
{
  if (getMotor4Code() > 100)
    team = BLUE;
  if (getMotor3Code() > 100)
    team = RED;
  EEPROM.write(10, team); // 记录下当前选择
  newtone((team == RED ? TONE_CH1 : TONE_CH3), 120);
  resetPid();
}
// 左转
void turn_left() { Turn(-45, 45, 2); }
// 右转
void turn_right() { Turn(45, -45, 4); }
// 等待检测货物状态
bool wait_goods()
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
      box_id[team]++ %= 4; // 编号从 1 ~ 4 递增
      EEPROM.write(15, box_id[0]);
      EEPROM.write(16, box_id[1]);
      newtone((team == RED ? TONE_CH1 : TONE_CH3), 120);
      delay(50);
      switch (box_id[team])
      {
      case 0:
        newtone(TONE_C1, 50);
        break;
      case 1:
        newtone(TONE_C2, 50);
        break;
      case 2:
        newtone(TONE_C3, 50);
        break;
      case 3:
        newtone(TONE_C4, 50);
        break;
      }
    }
  }
}

void send(int number, int team, int line = 3)
{
  gocode((line == 1 ? 300 : 800), 50, 50); // 给速度让机器先越过彩色区域
  goline(line - 1);
  if (number != 0)
  {
    if (team == RED)
      turn_right();
    else
      turn_left();
    goline(2 * number);
    if (team == BLUE)
      turn_right();
    else
      turn_left();
    goline(5 - line);
    // goline(1, 20, true);
    golinecode(650);
    gotime(300, 20, 20);
  }
  else // number == 0
  {
    if (line == 1)
      gocode(500, 40, 40);
    goline(4);
    goline(1, 20, true);
  }
  // golinecode(650);     // 编码巡线走一段距离
  // gotime(300, 20, 20); // 再用20速度前进一点时间，避免用编码前进卡住堵死，速度不太快

  setservo(4, 110); // 卸货
  delay(500);
  setservo(4, 70); // 恢复接货

  Turn(-50, 50, 2); // 原地转180°掉头
  if (line != 4)
  {
    golinecode(700);  // 用编码前进一点距离，越过第一个路口
    goline(4 - line); // (line == 4 ? 40 : 50)
  }
  else
    gocode(50, 40, 40);
  if (number != 0)
  {
    if (team == RED)
      turn_right();
    else
      turn_left();
    goline(2 * number);
    if (team == BLUE)
      turn_right();
    else
      turn_left();
    goline(line - 1);
  }
  else
    goline(3);

  // Turn(35, -35, 2);
  // Turn(35, -35, 2);
  // if (line == 1)
  //   gotime(halftime / 2, 50 * (team == BLUE ? 1 : -1), 50 * (team == RED ? 1 : -1));
  // gotime(halftime, 50, -50);
  if (line != 1)
    gocode(550, 45, 45); // 这里处理需要仔细调试，先向前走550距离
  gocode(1100, 40, -40); // 让机器右转一定编码值，正常这里应该让机器转180°，后退等待，但由于巡线角度以及马达等差异，无法保证180°准确值
  gocode(300, -40, -40); // 然后让机器后退一定距离
  gotime(200, -30, -30); // 再后退600ms，由于机器角度向外，这里需要通过后退时间，利用墙壁将机器调直,进入等待下一个货物循环，例程后续补充
}
// 程序开始
void setup()
{
  setMusic(0); // 播放美妙的音乐

  // 读取信息
  halftime = EEPROM.read(20);
  box_id[RED] = EEPROM.read(15);
  box_id[BLUE] = EEPROM.read(16);
  team = EEPROM.read(10);

  init_light_sensor();
  team_select();             // 选择红蓝方程序

  delay(200);
  setMusic(1);     // 播放美妙的音乐

  setservo(4, 70); // 进入等待获取货物状态

  unsigned long clock = micros();
  for (int i = 0; i < 15; i++)
  {
    wait_goods();
    send(box_id[color], team, box_id[BLUE] + 1);
  }
}

void loop() {}
