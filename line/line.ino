#include "JMDlib.h"
#include "line.h"
#define RED 0
#define BLUE 1
#define END -1
#define near 2
#define far 3
int team, color, iter = -1;

// ================ 设置 ================
int box_id[] = {near, far}; // 液体 红色 ; 食物 蓝色
bool force_send = true,     // 是否强制预定送货位置
    test_enable = true;     // 是否自动进行时间测试
int order_list[] = {        // 强制预定列表 (运行中按键取消)
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
      {
        newtone(TONE_DH5, 80);
        newtone(TONE_DH3, 80);
      }
      force_send = false;
    }
  }
}

// =============== 转弯 ===============
// 0 LEFT 左转        1 RIGHT 右转
inline void turn(bool direction, bool weird = false)
{
  if (weird)
    Turn((direction ? 35 : -17),
         (direction ? -17 : 35),
         (direction ? 4 : 2));
  else
    Turn((direction ? 45 : -15),
         (direction ? -15 : 45),
         (direction ? 5 : 1));
  delay(20);
}

void quick_send(int number, int team)
{
  gocode(690, 60, 60);
  goline(1, 50, true);
  goline(2, 60);
  turn(team == RED);
  goline(1, 55, true);
  goline(2 * number, 70, true);
  goline(1, 50);
  turn(team == BLUE);
  goline(1, 25);
  setservo(4, 140);
  delay(500);
  setservo(4, 75);
  gotime(600, -35, -35);
  turn(team == BLUE, number == 2);
  goline(1, 55, true);
  goline(2 * number, 70, true);
  goline(1, 45);
  turn(team == BLUE);
  goline(1, 50, true);
  goline(2, 70);
  gocode(500, 40, 40);
  gocode(1150, 35, -35);
  gocode(800, -25, -25);
  gotime(200, -10, -10);
}

// =============== 时间测试 ===============
void time_test()
{
  unsigned long clock;
  int numbertone[] = {TONE_CL1, TONE_C1, TONE_C2, TONE_C3, TONE_C4, TONE_C5, TONE_C6, TONE_C7, TONE_CH1, TONE_CH2};
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
  int rel3 = getMotor3Code(), rel4 = getMotor4Code(), T = millis();
  bool jump_setting = false;
  team = EEPROM.read(10);
  test_enable = EEPROM.read(11);
  box_id[RED] = EEPROM.read(12);
  box_id[BLUE] = EEPROM.read(13);
  // 输出调试信息
  Serial.begin(115200);
  Serial.println("Ver 10");
  Serial.println("这辆车是 Charlie 的 ~ 请不要玩坏了哦 ~");
  if (memread(0) < memread(1))
    memwrite(memread(0), 1);
  Serial.print("啊这... 这个车上次送货需要 ");
  Serial.print(memread(0) / 1000);
  Serial.print(".");
  Serial.print(memread(0) % 1000);
  Serial.println(" 秒.");
  Serial.print("而历史送货记录为 ");
  Serial.print(memread(1) / 1000);
  Serial.print(".");
  Serial.print(memread(1) % 1000);
  Serial.println(" 秒, 真是太慢了! 就不能再快点吗??");
  memwrite(memread(2) + 1, 2);
  Serial.print("这是自 2021/12/02 16:14 以来第 ");
  Serial.print(memread(2));
  Serial.println(" 次启动这辆车了.");
  Serial.println((test_enable ? "[x] 时间测试" : "[ ] 时间测试"));
  Serial.println("======== 设置信息 ========");
  int temp[4] = {-1, -1, -1, -1};
  temp[(team == BLUE ? 3 - box_id[RED] : box_id[RED])] = RED;
  temp[(team == BLUE ? 3 - box_id[BLUE] : box_id[BLUE])] = BLUE;
  for (int i = 0; i < 4; i++)
    Serial.print((temp[i] == -1 ? "[  ] " : (temp[i] == RED ? "[🟥] " : "[🟦] ")));
  if (team == RED)  
    Serial.println("\n🔴----+----+----+");
  else          
    Serial.println("\n +----+----+----🔵");
  if (!getKey())
  {
    newtone(TONE_CH6, 100);
    delay(70);
    newtone(TONE_CH6, 100);
    while (!getKey())
    {
      if (millis() - T >= 1000)
      {
        init_light_sensor();
        jump_setting = true;
        break;
      }
    }
    if (!jump_setting)
    {
      // 1 选择队伍
      newtone(TONE_CH1, 300);
      while (getKey())
      {
        setRGB(team);
        if (abs(getMotor3Code() - rel3) > 100)
        {
          rel3 = getMotor3Code();
          rel4 = getMotor4Code();
          if (team != RED)
            newtone(TONE_CH3, 120);
          team = RED;
        }
        if (abs(getMotor4Code() - rel4) > 100)
        {
          rel3 = getMotor3Code();
          rel4 = getMotor4Code();
          if (team != BLUE)
            newtone(TONE_CH1, 120);
          team = BLUE;
        }
      }

      // 2 是否时间测试
      while (!getKey())
        ;
      newtone(TONE_CH2, 300);
      while (getKey())
      {
        setRGB(7 - !test_enable);
        if (abs(getMotor3Code() - rel3) > 100)
        {
          rel3 = getMotor3Code();
          rel4 = getMotor4Code();
          if (!test_enable)
            newtone(TONE_CH3, 120);
          test_enable = true;
        }
        if (abs(getMotor4Code() - rel4) > 100)
        {
          rel3 = getMotor3Code();
          rel4 = getMotor4Code();
          if (test_enable)
            newtone(TONE_CH1, 120);
          test_enable = false;
        }
      }

      // 3 选择盒子编号
      while (!getKey())
        ;
      newtone(TONE_CH3, 300);
      setRGB(RED);
      delay(300);
      newtone((box_id[RED] == 2 ? TONE_CH3 : TONE_CH4), 120);
      delay(200);
      setRGB(BLUE);
      newtone((box_id[BLUE] == 2 ? TONE_CH3 : TONE_CH4), 120);
      delay(300);
      setRGB(6);
      while (getKey())
      {
        if (abs(getMotor3Code() - rel3) > 100)
        {
          box_id[RED] = 2 + (getMotor3Code() > rel3);
          setRGB(RED);
          rel3 = getMotor3Code();
          newtone((box_id[RED] == 2 ? TONE_CH3 : TONE_CH4), 120);
        }
        if (abs(getMotor4Code() - rel4) > 100)
        {
          box_id[BLUE] = 2 + (getMotor4Code() > rel4);
          setRGB(BLUE);
          rel4 = getMotor4Code();
          newtone((box_id[BLUE] == 2 ? TONE_CH3 : TONE_CH4), 120);
        }
      }

      // TODO 4 设置强制送达列表
    }
  }
  EEPROM.write(10, team);
  EEPROM.write(11, test_enable);
  EEPROM.write(12, box_id[RED]);
  EEPROM.write(13, box_id[BLUE]);
  sets();
  resetPid();
  setRGB(team);
  delay(200);
  setMusic(1); // 播放美妙的音乐

  setservo(4, 75); // 进入等待获取货物状态

  if (test_enable)
  {
    while (getKey())
      ;
    time_test();
  }
}

void loop()
{
  color = get_color();
  if (order_list[iter + 1] == -1)
    force_send = false;
  quick_send(box_id[(force_send ? order_list[++iter] : color)], team);
}
