/*
  GUI timer
    by VittoRose

*/

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#define XWIN     240
#define YWIN     320
#define MINPRESSURE 200
#define MAXPRESSURE 1000
#define DDELAY  500
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define BKG     0x0000        //black
#define TXT     0xFFFF        //white

#define BUZZER  11
#define FREQ    1000
#define BZZTIME 200
#define MOTOR   12

// the following value might depends on the specific display
const int XP = 8, XM = A2,YP = A3, YM = 9; //240x320 ID=0x9341
const int TS_LEFT = 954, TS_RT = 90, TS_TOP = 65, TS_BOT = 913;

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button b[3][2], conf;

unsigned long timer = 0;
unsigned long motor_timer = 0;
char s[30];

int counter[3] = {0, 0, 1};
int buff[3] = {0, 0, 0};

bool flag[4] = {0, 0, 0, 0};
bool stato[3] = {0, 0, 0};

int time = 5;                           // get ready time

int pixel_x, pixel_y;     //Touch_getXY() updates global vars

bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);      //because TFT control pins
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}

void setup() {
  int i = 0;

  Serial.begin(9600);

  pinMode(BUZZER, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  digitalWrite(MOTOR, LOW);

  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);            
  tft.fillScreen(BKG);

  b[0][0].initButton(&tft, 28, 30, 50, 50, BLACK, BLACK, GREEN, "+", 3);
  b[0][1].initButton(&tft, 210, 30, 50, 50, BLACK, BLACK, RED, "-", 3);
  b[1][0].initButton(&tft, 28, 100, 50, 50, BLACK, BLACK, GREEN, "+", 3);
  b[1][1].initButton(&tft, 210, 100, 50, 50, BLACK, BLACK, RED, "-", 3);
  b[2][0].initButton(&tft, 28, 170, 50, 50, BLACK, BLACK, GREEN, "+",3);
  b[2][1].initButton(&tft, 210, 170, 50, 50, BLACK, BLACK, RED, "-",3);

  conf.initButton(&tft, XWIN/2, YWIN - 30, XWIN - 20, 50, BLACK, BLACK, GREEN, "CONFIRM", 3 );

  for(i = 0; i < 3; i++ ) {
    b[i][0].drawButton(true);
    b[i][1].drawButton(true);
  }
  
  for(i = 0; i < 3; i++)  draw_settings(counter[i],i);

  conf.drawButton(true);
}

void loop() {

  //User define the training parm
  while(!flag[0]){               
    
    bool down = Touch_getXY();
    
    conf.press(down && conf.contains(pixel_x, pixel_y));
    
    set_training();
    
    if(conf.justReleased()) {
      flag[0] = !flag[0];
      timer = millis();
    }
  }

  while(!flag[1]){
    get_ready();          
  }

  while(!flag[2]){
    run_work();
  }

  while(!flag[3]){
    run_rest();
  }
  
}

void set_training(){
  bool down = Touch_getXY();
  int i = 0;
  for(i=0; i<3; i++){ 
    b[i][0].press(down && b[i][0].contains(pixel_x, pixel_y));
    b[i][1].press(down && b[i][1].contains(pixel_x, pixel_y));
  }

  for(i = 0; i < 3; i++){

    if(b[i][0].justReleased()) {
      counter[i]++;
      delay(100);
      draw_settings(counter[i],i);
    }
    
    if(b[i][1].justReleased()){
      counter[i]--;
      if(counter[i] <= 0) counter[i] = 0;
      delay(100);
      draw_settings(counter[i],i);
    }
  } 
}

void draw_settings(int counter, int index){
  
  int min, sec; 
  min = counter/60;
  sec = counter % 60;


  if(index == 0){

    tft.setTextSize(2);
    tft.setCursor(65, 10);
    tft.setTextColor(TXT);
    
    tft.print("WORK TIME");
    sprintf(s,"%d m - %d s", min, sec);
    tft.fillRect(55, 30, 125, 15, BKG);
    
    tft.drawRect(55, 30, 125, 15, WHITE);
    tft.setTextSize(1);
    tft.setCursor(85, 33);
    
    tft.print(s);
  } 
  else if(index == 1){
    
    tft.setTextSize(2);
    tft.setCursor(65, 80);
    tft.setTextColor(TXT);
    
    tft.print("REST TIME");
    sprintf(s,"%d m - %d s", min, sec);
    tft.fillRect(55, 105, 125, 15, BKG);
    
    tft.drawRect(55, 105, 125, 15, WHITE);
    tft.setTextSize(1);
    tft.setCursor(85, 108);
    
    tft.print(s);
  }
  else if (index == 2){
    
    tft.setTextSize(2);
    tft.setCursor(60, 150);
    tft.setTextColor(TXT);
    
    tft.print("SET NUMBER");
    sprintf(s,"%d", counter);
    tft.fillRect(55, 175, 125, 15, BKG);
    
    tft.drawRect(55, 175, 125, 15, WHITE);
    tft.setTextSize(1);
    tft.setCursor(117, 178);
    
    tft.print(s);
  }
}

void get_ready(void){
  
  if(stato[0] == 0){
            
    tft.fillScreen(YELLOW);
    tft.setTextSize(3);
    tft.setTextColor(BLACK);
      
    tft.setCursor(40,50);
    tft.print("GET READY");

    stato[0] = 1;
  }

  if(millis() - timer >= 1000){

    tft.setTextSize(8);
    tft.setTextColor(BLACK);
    
    if(time >= 10) tft.setCursor(75, 140);
    else tft.setCursor(100,140);

    tft.fillRect(20,130, 200, 100, YELLOW);
    sprintf(s, "%d", time);
    tft.print(s);

    if(time <= 3 && time != 0) tone(BUZZER, FREQ, BZZTIME);;

    time--;
      
    if(time < 0) {
      time = 0;
      flag[1] = !flag[1];
    }
      timer = millis();
    }
}

void run_work(){
  if(stato[1] == 0){

    buff[0] = counter[0];
    buff[2]++;
            
    tft.fillScreen(GREEN);
    tft.setTextSize(3);
    tft.setTextColor(BLACK);
      
    tft.setCursor(40,50);
    tft.print("WORK TIME");
    
    tft.setTextSize(2);
    tft.setCursor(40,280);
    sprintf(s, "SET %d OUT OF %d", buff[2], counter[2]);
    tft.print(s);

    stato[1] = 1;
    stato[2] = 0;

  }

  if(millis() - timer >= 1000){

    tft.setTextSize(8);
    tft.setTextColor(BLACK);
    tft.fillRect(20,130, 210, 100, GREEN);
    
    if(buff[0] >= 60){
      tft.setCursor(30, 140);

      if(buff[0]%60 < 10){
        sprintf(s, "%d:0%d", buff[0]/60, buff[0]%60);
      }else sprintf(s,"%d:%d", buff[0]/60, buff[0]%60);

      tft.print(s);
    }
    else if(buff[0] >= 10){
      tft.setCursor(75, 140);
      sprintf(s, "%d", buff[0]);
      tft.print(s);
    
    }
    else {
      tft.setCursor(100,140);
      sprintf(s, "%d", buff[0]);
      tft.print(s);
    }


    if(buff[0] <= 3 && buff[0] != 0) tone(BUZZER, FREQ, BZZTIME);
    
    if(buff[0] <= 1) {
      digitalWrite(MOTOR, HIGH);
      motor_timer = millis();
    }

    buff[0]--;
      
    if(buff[0] < 0) {

      flag[2] = 1;
      flag[3] = 0;
      
      if(counter[2] - buff[2] <= 0){
        delay(1000);
        asm volatile (" jmp 0 ");
      }
    }
    timer = millis();
  }
}

void run_rest(){
  
  if(stato[2] == 0){

    stato[1] = 0;

    buff[1] = counter[1];
            
    tft.fillScreen(RED);
    tft.setTextSize(3);
    tft.setTextColor(BLACK);
      
    tft.setCursor(40,50);
    tft.print("REST TIME");

    tft.setTextSize(2);
    tft.setCursor(40,280);
    sprintf(s, "SET %d OUT OF %d", buff[2], counter[2]);
    tft.print(s);

    stato[2] = 1;

  }

  if(millis() - motor_timer >= 1500) digitalWrite(MOTOR, LOW);

  if(millis() - timer >= 1000){

    tft.setTextSize(8);
    tft.setTextColor(BLACK);

    if(buff[1] >= 10) tft.setCursor(75, 140);
    else tft.setCursor(100,140);

    tft.fillRect(20,130, 200, 100, RED);
    sprintf(s, "%d", buff[1]);
    tft.print(s);

    if(buff[1] <= 3 && buff[1] != 0) tone(BUZZER, FREQ, BZZTIME);

    buff[1]--;
      
    if(buff[1] < 0) {
      flag[3] = 1;
      flag[2] = 0;
    }

    timer = millis();
  }
}