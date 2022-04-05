#include <SPI.h>
#include <TFT_eSPI.h>
#include "back.h"
#include "player.h"
#include "enemy.h"
#include "ani.h"
#include "gameover.h"

//螢幕設定
TFT_eSPI tft = TFT_eSPI(); 

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;

//色碼
#define TFT_GREY 0x5AEB

//int player
float pX = random(30, 100);
float pY = 200;
int pW = 25;
int pH = 31;
float pXs = 0.2;
float pYs = 0.2;
int pos[4] = {2, 35, 68, 101};

//bulet 子彈
float bx[10];
float by[10];
int bw = 2; //player bullet size
float bs = 0.35; //player bulett speed

//enemies 敵人
float ex[8];
float ey[8];
int ef[8];
int ew = 32;
int eh = 32;
float esy = 0.11;
float esx = 0;
int eFrame[8];
int el[8];

int counter = 0;
int fire = 0;
int score = 0;
int timeAlive = 0;
int currentTime = 0;
int newLevelTime = 15;

//生命值
int lifePh = 3;

int visableTime = 800;
int visableCount = visableTime ;
int invincible = 0 ;


//外部按鍵
int pinLeft = 37;
int pinRight = 38;

//生命值LED
int pinLed1 = 2 ;
int pinLed2 = 15 ;
int pinLed3 = 13 ;

//初始 function
void setup() {
  //Serial.begin(2000000);
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BLACK, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, 100);
  //pinMode(0, INPUT_PULLUP); // pinLeft
  //pinMode(35, INPUT_PULLUP);//pinRight

  pinMode(pinLeft, INPUT);
  pinMode(pinRight, INPUT);

  pinMode(pinLed1, OUTPUT);
  pinMode(pinLed2, OUTPUT);
  pinMode(pinLed3, OUTPUT);

  digitalWrite(pinLed1, HIGH);
  digitalWrite(pinLed2, HIGH);
  digitalWrite(pinLed3, HIGH);

  for (int i = 0; i < 8; i++) {
    ex[i] = pos[random(0, 4)];
    ey[i] = -50;
    eFrame[i] = random(0, 4);
    el[i] = 3;
  }

  for (int i = 0; i < 10; i++) {
    by[i] = 250;
  }

}


int count = 0;
int startCounter = 0;
int endCounter = 0;
int aniFrame = 0;

//狀態參數
int fase = 0; //fase 0=start screen (開始) 1=play(正常遊戲中) 2=gameover()

//主程式(主迴圈)
void loop() {
//等待開始 fase:0
  if (fase == 0) {
    delay(500);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.pushImage(0, 0, 135, 240, back);
    //Serial.println("-->" + digitalRead(pinLeft) );
    while (digitalRead(pinLeft) == 1) {
      //Serial.print("L." + digitalRead(pinLeft) );
      //Serial.print("R." + digitalRead(pinLeft) );
      tft.pushImage(34, 0, 102, 84, ani[aniFrame]);
      delay(40);
      aniFrame++;
      if (aniFrame == 25)
        aniFrame = 0;
    }
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, 0x7820);
    tft.fillRect(0, 0, 135, 17, 0x7820);
    tft.drawString("SCORE:", 2, 1, 2);
    tft.drawString("TIME:", 70, 1, 2);
    tft.drawLine(0, 17, 135, 17, 0x6577);
    tft.drawLine(0, 18, 0, 240, 0x6577);
    tft.drawLine(134, 18, 134, 240, 0x6577);
    fase = 1;
  }

  //遊戲進行中 fase:1
  if (fase == 1) {
    if (millis() - currentTime > 1000) {
      timeAlive++;
      tft.drawString(String(timeAlive), 105, 1, 2);

      currentTime = millis();
      if (timeAlive == newLevelTime) {
        esy = esy + 0.02;
        newLevelTime = timeAlive * 2;
      }
    }
    if (startCounter == 0) {
      if (count < 8) {
        startCounter = 1;
        ey[count] = 19;
        count++;
        if (count == 8)
          count = 0;
        endCounter = random(180, 500);
      }
    }

    //物件繪圖
    draw();

    //按鍵檢查
    checkButton();

    for (int i = 0; i < 10; i++) {
      if (by[i] > 23 && by[i] < 240)
        by[i] = by[i] - bs;
    }

    for (int i = 0; i < 8; i++) {
      if (ey[i] != -50.00) {
        ey[i] = ey[i] + esy;
        ex[i] = ex[i] - esx;
      }

      if (ey[i] > 242) {
        ex[i] = pos[random(0, 4)];
        ey[i] = -50;
        eFrame[i] = random(0, 4);
        esy = esy + 0.0025;
        el[i] = 3;
      }
    }

    if (fire > 0) {
      fire++;
    }
    if (fire > 60) {
      fire = 0;
    }

    if (startCounter > 0) {
      startCounter++;
    }
    if (startCounter >= endCounter) {
      startCounter = 0;
    }
    
    collision();

  }

  if (fase == 2) {
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(0, 0, 135, 240, gameover);
    tft.drawString("SCORE: " + String(score), 25, 120, 2);
    tft.drawString("TIME: " + String(timeAlive), 25, 140, 2);
    delay(500);
    while (digitalRead(pinLeft) == 1);
    restart();
    fase = 0;
  }

}//loop

void draw() {
  tft.pushImage(pX, pY, pW, pH, player);
  for (int i = 0; i < 10; i++) {
    tft.fillCircle(bx[i], by[i], bw + 2, TFT_BLACK);
    if (by[i] > 23)
      tft.fillCircle(bx[i], by[i], bw, TFT_RED);
  }

  for (int i = 0; i < 8; i++) {
    if (ex[i] < -32)
      tft.fillRect(0, 14, 32, 32, TFT_BLACK);
    else
      tft.pushImage(ex[i], ey[i], eh, ew, enemy[eFrame[i]]);

  }
}

//按鍵偵測
void checkButton()
{
  // Serial.print("R." + digitalRead(pinLeft) , digitalRead(pinLeft) );
  if (digitalRead(pinLeft) == 0 && digitalRead(pinRight) == 1 ) {
    if (pX > 2)
      pX = pX - pXs;
  }

  if (digitalRead(pinRight) == 0 && digitalRead(pinLeft) == 1) {
    if (pX < 135 - pW - 1)
      pX = pX + pXs;
  }

  if (fire == 0) {
    bx[counter] = pX + pW / 2;
    by[counter] = pY - bw / 2;
    counter++;
    fire = 1;
    if (counter >= 10)
      counter = 0;
  }

}

//碰撞偵測
void collision() {
  if (invincible == 1 && visableCount > 0 ){
    //無敵狀態
     visableCount -- ;
  }else{
    //無敵結束
    visableCount = visableTime ;
    invincible = 0 ;
  }
 
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 8; j++) {
      if (invincible == 0 ){
      if (pX + pW / 2 > ex[j] && pX + pW / 2 < ex[j] + ew && ey[j] + 26 > pY && ey[j] < 230) {
        invincible = 1 ;
        lifePh --;
        if (lifePh == 2 ) {
          digitalWrite(pinLed1, LOW);
        } else if (lifePh == 1) {
          digitalWrite(pinLed2, LOW);
        } else if (lifePh == 0) {
          digitalWrite(pinLed3, LOW);
          fase = 2;
        }
        //return ;
        //continue ;
      }
      }
      if (bx[i] > ex[j] + 2 && bx[i] < (ex[j] + ew - 2) && by[i] < ey[j] + 20) {
        score++;
        tft.drawString(String(score), 48, 1, 2);
        by[i] = 250;
        el[j] = el[j] - 1;
        if (el[j] == 0) {
          score = score + 5;
          tft.fillRect(ex[j], ey[j], 32, 32, TFT_BLACK);
          ex[j] = pos[random(0, 4)];
          ey[j] = -50;
          eFrame[j] = random(0, 4);
          el[j] = 3;
        }
      }
    }
  }
}

//重新開始,歸零初始化
void restart() {
  esy = 0.11;
  for (int i = 0; i < 8; i++) {
    ex[i] = pos[random(0, 4)];
    ey[i] = -50;
    eFrame[i] = random(0, 4);
    el[i] = 3;
  }

  for (int i = 0; i < 10; i++) {
    by[i] = 250;
  }

  //pres1 = 0; //debounce left button
  //pres2 = 0; //debounce right button
  counter = 0;
  fire = 0;
  score = 0;
  timeAlive = 0;
  currentTime = 0;
  newLevelTime = 15;
  count = 0;
  startCounter = 0;
  endCounter = 0;
  aniFrame = 0;

  lifePh = 3 ;
  visableCount = visableTime ;
  invincible = 0 ;
  digitalWrite(pinLed1, HIGH);
  digitalWrite(pinLed2, HIGH);
  digitalWrite(pinLed3, HIGH);

}
