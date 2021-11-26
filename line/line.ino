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
      {
        newtone(1100, 80);
      }
      else
      {
        // 当左马达后转，右马达 前转各200个脉冲，表示蓝方出发
        newtone(900, 80);
        delay(50);
        newtone(600, 160);
        delay(50);
        newtone(1100, 80);
      }
      delay(200);
      for (int i = 0; i < id[team] + 1; i++)
      {
        newtone(400 + i * 30, 120);
        delay(30);
      }
    }
  }
}

void send(int number, int team)
{
  gocode(700, 30, 30); // 根据上边后退的状态，给相应速度让机器先越过蓝色区域,接上循环
  goline(2);           // 巡线1条横线
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
    goline(1);
  }
  else
    goline(2);
  golinecode(650);     // 编码巡线走450的距离
  gotime(300, 20, 20); // 再用20速度前进一点时间，避免用编码前进卡住堵死，速度不太快

  setservo(4, 110); // 卸货
  delay(500);
  setservo(4, 75); // 恢复接货状态

  Turn(-30, 30, 2); // 原地转180°掉头
  golinecode(400);  // 用编码前进一点距离，越过第一个路口
  goline(1);
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
    goline(2);
  }
  else
    goline(3);
  gocode(550, 20, 20); // 这里处理需要仔细调试，先向前走550距离
  delay(100);
  gocode(950, 30, -30);  // 让机器右转一定编码值，正常这里应该让机器转180°，后退等待，但由于巡线角度以及马达等差异，无法保证180°准确值
  gocode(600, -20, -20); // 然后让机器后退一定距离
  gotime(600, -20, -20); // 再后退600ms，由于机器角度向外，这里需要通过后退时间，利用墙壁将机器调直,进入等待下一个货物循环，例程后续补充
}
// 程序开始
void setup()
{
  initLine();
  selectRG();      // 选择红蓝方程序
  setservo(4, 75); // 进入等待获取货物状态
  for (int i = 0; i < 999; i++)
  {
    wait_goods();
    send(id[color], team);
  }
}

void loop() {}
