#include "JMDlib.h"
#include "line.h"
#define RED 0
#define BLUE 1
#define END -1
int team, color, box_id[2], iter = 0;
bool force_send = true;

void selectRG()
{
  if (getMotor4Code() > 100)
    team = BLUE;
  if (getMotor3Code() > 100)
    team = RED;
  EEPROM.write(10, team); // 记录下当前选择
  newtone((team == RED ? TONE_CH1 : TONE_CH3), 120);
  resetPid();
}

// =========== 等待货物 ===========
bool get_color()
{
  int countR = 0, countB = 0;
  int R = 0, G = 0, B = 0;
  while (true)
  {
    // stop_all();
    // resetPid();
    // set_2Motor(0, 0);
    R = getColorSensorPin(0, 6); // 获得颜色传感器R红色分量
    G = getColorSensorPin(0, 7); // 获得颜色传感器G
    B = getColorSensorPin(0, 8); // 获得颜色传感器B蓝色分量
    // 当数所有值大于15被认为已装货
    if (R > 15 && G > 15 && B > 15)
    {
      if (R > B) // R大于B，被认为是红色货物
        countR++;
      else // B大于R，被认为是蓝色货物
        countB++;
    }
    if (countR >= 10)
      return RED;
    if (countB >= 10)
      return BLUE;
    if (!getKey()) // 运行中按下了按键
      force_send = false;
  }
}

// =========== 转弯 ===========
// 0 左转   1 右转
inline void turn(bool direction, int speed = 45)
{
  // set_2Motor(-10, -10);
  Turn((direction ? 30 : -15),
       (direction ? -15 : 30),
       (direction ? 4 : 2));
  delay(20);
}
inline void weird_turn(bool direction)
{
  // set_2Motor(-10, -10);
  Turn((direction ? 45 : -15),
       (direction ? -15 : 45),
       (direction ? 5 : 1));
}

void send(int number, int team, int line = 3)
{
  gocode((line == 1 ? 270 : 700), 60, 60); // 给速度让机器先越过彩色区域
  goline(line - 1, 55);
  if (number != 0)
  {
    turn(team == RED);
    goline(2 * number - (number == 4 ? 1 : 0), 55);
    turn(team == BLUE);
    goline(5 - line, 40);
    goline(1, 20, true);
  }
  else // number == 0
  {
    if (line == 1)
      gocode(500, 40, 40);
    goline(4, 45);
    goline(1, 20, true);
  }
  set_2Motor(0, 0);
  setservo(4, 150); // 卸货
  delay(500);
  setservo(4, 75); // 恢复接货

  if (team == BLUE)
    Turn(-50, 50, 2); // 原地转180°掉头
  else
    Turn(50, -50, 4);
  if (line != 4)
  {
    golinecode(750); // 用编码前进一点距离，越过第一个路口
    goline(4 - line, 50);
  }
  else
    gocode(80, 40, 40);
  if (number != 0)
  {
    turn(team == RED);
    goline(2 * number, 55);
    turn(team == BLUE);
    goline(line - 1, 50);
  }
  else
    goline(3, 40);

  if (line != 1)
    gocode(550, 45, 45); // 这里处理需要仔细调试，先向前走550距离
  gocode(1150, 40, -40); // 让机器右转一定编码值，正常这里应该让机器转180°，后退等待，但由于巡线角度以及马达等差异，无法保证180°准确值
  gocode(200, -40, -40); // 然后让机器后退一定距离
  gotime(200, -30, -30); // 再后退600ms，由于机器角度向外，这里需要通过后退时间，利用墙壁将机器调直,进入等待下一个货物循环，例程后续补充
  resetPid();
  set_2Motor(0, 0);
}

void quick_send(int number, int team)
{
  gocode(690, 60, 60); // 给速度让机器先越过彩色区域
  goline(1, 50);
  goline(3, 60);

  turn(team == RED);
  goline(1, 55);
  goline(2 * number - (number == 4 ? 2 : 1), 65);
  goline(2, 50);
  turn(team == BLUE);
  goline(1, 30);
  // golineTime(10, 20);

  set_2Motor(0, 0);
  setservo(4, 140);
  delay(500);
  setservo(4, 75);

  gotime(600, -40, -40);

  if (number == 2)
    weird_turn(team == BLUE);
  else
    turn(team == BLUE);
  goline(1, 55);
  // golineTime(200, 10);
  goline(2 * number - (number == 4 ? 2 : 1), 70);
  goline(2, 55);
  turn(team == BLUE);
  goline(1, 50);
  goline(3, 70);

  gocode(500, 45, 45);
  gocode(1340 - (number == 2 ? 10 : 0), 40, -40);
  gocode(500, -30, -30);
  gotime(400, -10, -10);

  resetPid();
}

void fast_send(int number, int team)
{
  gocode(690, 60, 60); // 给速度让机器先越过彩色区域
  goline(3, 55);

  turn(team == RED);
  goline(2 * number - (number == 4 ? 1 : 0), 55);
  turn(team == BLUE);
  // goline(1, 40);
  // goline(1, 15, true);

  set_2Motor(0, 0);
  setservo(4, 150); // 卸货
  delay(500);
  setservo(4, 75); // 恢复接货

  gotime(600, -40, -40);
  turn(team == BLUE);
  goline(2 * number, 55);
  turn(team == BLUE);
  goline(3, 50);

  gocode(500, 45, 45);   // 这里处理需要仔细调试，先向前走550距离
  gocode(1350, 40, -40); // 让机器右转一定编码值，正常这里应该让机器转180°，后退等待，但由于巡线角度以及马达等差异，无法保证180°准确值
  gocode(500, -30, -30); // 然后让机器后退一定距离
  gotime(300, -10, -10); // 再后退600ms，由于机器角度向外，这里需要通过后退时间，利用墙壁将机器调直,进入等待下一个货物循环，例程后续补充

  resetPid();
}
// =========== 时间测试 ===========
void time_test(int line)
{
  unsigned long clock;
  int numbertone[] = {TONE_CL7, TONE_C1, TONE_C2, TONE_C3, TONE_C4, TONE_C5, TONE_C6, TONE_C7, TONE_CH1, TONE_CH2};

  delay(1000);
  clock = millis();
  quick_send(2, team);
  delay(1000);
  quick_send(3, team);
  stop_all();
  
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
  delay(2000);
}
// 程序开始
void setup()
{
  setMusic(0); // 播放美妙的音乐
  analysis();
  sets();
  box_id[RED] = 2;        // liquid
  box_id[BLUE] = 3;       // food
  team = EEPROM.read(10); // 读取红蓝方信息
  selectRG();             // 选择红蓝方程序
  delay(200);
  setMusic(1);     // 播放美妙的音乐
  setservo(4, 75); // 进入等待获取货物状态

  time_test(4);
  int order_list[] = {END};
  while (true)
  {
    get_color();
    if (order_list[iter + 1] == -1)
      force_send = false;
    if (force_send)
      quick_send(box_id[order_list[++iter]], team);
    else
      quick_send(box_id[color], team);
  }
}

void loop() {}
