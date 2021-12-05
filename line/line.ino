#include "JMDlib.h"
#include "line.h"
//扫描程序，先把机器放到场地上，按着按钮开机，听到提示音进入扫描程序，扫描过程会右短促提示音
//这时让机器5个光电都经过一次黑线和白色区域，然后再按一次按钮跳出扫描程序
int isRedTeam=1;   //红蓝方信息数值为1时为红方出发

void selectRG()
{
  Serial.print("M3S:");
  Serial.print(getMotor3Code());
  Serial.print(",M4S:");
  Serial.println(getMotor4Code());
  if(abs(getMotor4Code()<200))
  {
    //当左马达往前转，右马达后转各200个脉冲，表示红方出发，
    newtone(9,1100,80);
    isRedTeam=1;
    EEPROM.write(10,isRedTeam);//记录下当前选择
  }
  else
  if(abs(getMotor3Code())>200)
  {
    //当左马达后转，右马达前转各200个脉冲，表示蓝方出发
    newtone(9,1100,80);
    delay(50);
    newtone(9,1100,80);
    isRedTeam=0;
    EEPROM.write(10,isRedTeam);
  }
  
  isRedTeam=EEPROM.read(10);//读取红蓝方信息
  resetPid();
}
void TurnLeft()
{
  Turn(-5,35,2);
  
}
void TurnRight()
{
  Turn(35,-5,4);
  
}
int waitGoods()//等待检测货物状态
{
  int countR=0,countB=0;
  int R=0,G=0,B=0;
  
  while(1)
  {
    R=getColorSensorPin(0,6);//获得颜色传感器R红色分量
    G=getColorSensorPin(0,7);//获得颜色传感器G
    B=getColorSensorPin(0,8);//获得颜色传感器B蓝色分量
    //当数所有值大于15被认为已装货
    if(R>15 && G>15 && B>15)
    {
      if(R > B)  //R大于B，被认为是红色货物
        countR++;
      else  //B大于R，被认为是蓝色货物
        countB++;
    }
    
    if(countR>20 )
      return 0;
    if(countB>20 )
      return 2;
  }
}
void getGoods()//舵机初始位置(准备获取货物位置)，需要实测更改
{
  setservo(4,80);
  delay(1000);
}
void setGoods()//舵机卸货角度，需要实测
{
  setservo(4,110);
  delay(1000);
}
void H_shiwu()//红色区出发，到红方食物区卸货
{
  gocode(700,30,30);    //先用左右马达各30的速度走700个编码脉冲，离开红色区域
  goline(1);            //巡线1条横线
  TurnRight();          //右转
  goline(4);            //巡线4条线
  TurnLeft();           //左转
  goline(2);            //巡线2条线
  golinecode(650);      //编码巡线走450的距离
  gotime(300,20,20);          //再用20速度前进一点时间，避免用编码前进卡住堵死，速度不太快
  setGoods();           //卸货
  delay(1000);
  getGoods();            //恢复接货状态
//  gocode(350,-20,-20);  //编码后退350距离
  
  Turn(-20,20,2);        //原地转180°掉头
  golinecode(400);       //用编码前进一点距离，越过第一个路口
  goline(1); 
  TurnRight();
  goline(4); 
  TurnLeft();
  goline(2); 
  gocode(550,20,20);     //这里处理需要仔细调试，先向前走550距离
  delay(1000);        
  gocode(1000,15,-15);   //让机器右转一定编码值，正常这里应该让机器转180°，后退等待，但由于巡线角度以及马达等差异，无法保证180°准确值
  delay(2000);           //这时我的处理方式是让机器角度稍微偏向左边向外，
  gocode(600,-20,-20);   //然后让机器后退一定距离
  gotime(600,-20,-20);   //再后退600ms，由于机器角度向外，这里需要通过后退时间，利用墙壁将机器调直,进入等待下一个货物循环，例程后续补充

//  gocode(700,20,20);
}
void yinliao()//红方饮料区卸货
{
  H_shiwu();
}


void setup()
{
  
  initLine();
  
  selectRG();//选择红蓝方程序
  getGoods();//进入等待获取货物状态
  
  if(1)
  {
    if(waitGoods()==0)
    {
      setRGB(0);
      H_shiwu();
    }
    else
    {
      setRGB(2);
      H_shiwu();
    }
  }
  
 
}

void loop() {
  

}
