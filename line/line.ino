#include "JMDlib.h"
#include "line.h"
#define RED 0
#define BLUE 1
// 扫描程序，先把机器放到场地上，按着按钮开机，听到提示音进入扫描程序，扫描过程会有短促提示音
// 这时让机器5个光电都经过一次黑线和白色区域，然后再按一次按钮跳出扫描程序
int team; // 红蓝方信息数值为1时为红方出发
int color, id[2] = {1, 2};
// 通过手动转动左或者右马达后，启动机器，完成机器选择红蓝方启动的过程
void selectRG()
{
  if (getMotor4Code() > 100)
  {
    team = BLUE;
    EEPROM.write(10, team); // 记录下当前选择
  }
  else if (getMotor3Code() > 100)
  {
    team = RED;
    EEPROM.write(10, team);
  }
  id[RED] = EEPROM.read(15);
  id[BLUE] = EEPROM.read(16);
  team = EEPROM.read(10); // 读取红蓝方信息
  if (team == RED)
  {
    newtone(1100, 80);
  }
  else
  {
    // 当左马达后转，右马达前转各200个脉冲，表示蓝方出发
    newtone(900, 80);
    delay(50);
    newtone(600, 160);
    delay(50);
    newtone(1100, 80);
  }
  resetPid();
}
// 左转
void turn_left() { Turn(-5, 35, 2); }
// 右转
void turn_right() { Turn(35, -5, 4); }
// 等待检测货物状态
void wait_goods()
{
  int countR = 0, countB = 0;
  int R = 0, G = 0, B = 0;
  resetPid();
  while (true)
  {
    R = getColorSensorPin(0, 6); // 获得颜色传感器R红色分量
    G = getColorSensorPin(0, 7); // 获得颜色传感器G
    B = getColorSensorPin(0, 8); // 获得颜色传感器B蓝色分量
    // 当数所有值大于15被认为已装货
    if (R > 15 && G > 15 && B > 15)
    {
      if (R > B) // R大于B，被认为是红色货物
        countR++, color = RED;
      else // B大于R，被认为是蓝色货物
        countB++, color = BLUE;
      break;
    }
    if (!getKey()) // 运行中按下了按键, 设置盒子编号
    {
      id[team] = (id[team] + 1) % 4; // 编号从 1 ~ 4 递增
      EEPROM.write(15, id[0]);
      EEPROM.write(16, id[1]);
      if (team == RED)
        newtone(TONE_CH1, 120);
      else
        newtone(TONE_CH2, 120);
      delay(100);
      switch (id[team])
      {
      case 1:
        newtone(TONE_C1, 80);
        break;
      case 2:
        newtone(TONE_C2, 80);
        break;
      case 3:
        newtone(TONE_C3, 80);
        break;
      case 4:
        newtone(TONE_C4, 80);
        break;
      }
    }
  }
}

void send(int number, int team)
{
  gocode(700, 30, 30); // 给速度让机器先越过彩色区域
  goline(2);           // 巡线 2 条横线
  if (number != 0)
  {
    if (team == RED)
      turn_right();
    else
      turn_left();
    goline(2 * number, 30 + number * 3);
    if (team == BLUE)
      turn_right();
    else
      turn_left();
    goline(1, 35, 5000);
  }
  else
    goline(2, 35, 5000);
  // golinecode(650);     // 编码巡线走一段距离
  // gotime(300, 20, 20); // 再用20速度前进一点时间，避免用编码前进卡住堵死，速度不太快

  setservo(4, 110); // 卸货
  delay(500);
  setservo(4, 75); // 恢复接货

  Turn(-30, 30, 2); // 原地转180°掉头
  golinecode(400);  // 用编码前进一点距离，越过第一个路口
  goline(1);
  if (number != 0)
  {
    if (team == RED)
      turn_right();
    else
      turn_left();
    goline(2 * number, 30 + number * 5);
    if (team == BLUE)
      turn_right();
    else
      turn_left();
    goline(2, 40);
  }
  else
    goline(3, 40);
  gocode(550, 20, 20);   // 这里处理需要仔细调试，先向前走550距离
  gocode(950, 30, -30);  // 让机器右转一定编码值，正常这里应该让机器转180°，后退等待，但由于巡线角度以及马达等差异，无法保证180°准确值
  gocode(600, -20, -20); // 然后让机器后退一定距离
  // gotime(600, -20, -20); // 再后退600ms，由于机器角度向外，这里需要通过后退时间，利用墙壁将机器调直,进入等待下一个货物循环，例程后续补充
}
// 程序开始
int pig1[] = {TONE_CH5, TONE_CH3, TONE_CH1, TONE_CH2, TONE_C5}, pig2[] = {TONE_C5, TONE_C7, TONE_CH2, TONE_CH4, TONE_CH3, TONE_CH1};
float pigdur1[] = {1, 0.5, 0.5, 1, 0.5}, pigdur2[] = {0.5, 0.5, 0.5, 0.5, 1, 1};
void setup()
{
  setMusic(0); // 播放美妙的音乐
  initLine();
  selectRG(); // 选择红蓝方程序
  delay(100);
  setMusic(1); // 播放美妙的音乐
  delay(100);
  setservo(4, 75); // 进入等待获取货物状态
  for (int i = 0; i < 999; i++)
  {
    wait_goods();
    send(id[color], team);
  }
}

void loop() {}
