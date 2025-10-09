#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "DFRobot_BMI160.h"
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



DFRobot_BMI160 bmi160;

const int8_t oled_addr = 0x3c;
const int8_t bmi_addr = 0x69;

struct Point3D { float x, y, z; };
struct Point2D { int x, y; };

Point2D project(Point3D p, float pitch, float roll) {
  // chuyển sang radian
  float pr = pitch * PI / 180.0;
  float rr = roll * PI / 180.0;

  // Xoay quanh trục X (pitch)
  float y1 = p.y * cos(pr) - p.z * sin(pr);
  float z1 = p.y * sin(pr) + p.z * cos(pr);

  // Xoay quanh trục Y (roll)
  float x2 = p.x * cos(rr) + z1 * sin(rr);
  float z2 = -p.x * sin(rr) + z1 * cos(rr);

  // Chiếu phối cảnh (nhỏ lại 2/3)
  float scale = 1.0 / (z2 + 3.0);    // giảm scale tổng thể
  int cx = SCREEN_WIDTH / 2;
  int cy = SCREEN_HEIGHT / 2 + 5;    // hạ xuống chút cho cân

  Point2D pt;
  pt.x = (int)(cx + x2 * scale * 35);   // scale 50 cho vừa OLED
  pt.y = (int)(cy - y1 * scale * 35);
  return pt;
}

void drawCube(float pitch, float roll) {
  display.clearDisplay();

  // Định nghĩa 8 đỉnh của khối lập phương (đơn vị 1)
  Point3D cube[8] = {
    {-1, -1, -1}, {1, -1, -1}, {1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, {1, -1,  1}, {1,  1,  1}, {-1,  1,  1}
  };

  // 12 cạnh
  int edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
  };

  // Chiếu 8 đỉnh
  Point2D pts[8];
  for (int i = 0; i < 8; i++) {
    pts[i] = project(cube[i], pitch, roll);
  }

  // Vẽ 12 cạnh
  for (int i = 0; i < 12; i++) {
    int a = edges[i][0];
    int b = edges[i][1];
    display.drawLine(pts[a].x, pts[a].y, pts[b].x, pts[b].y, SH110X_WHITE);
  }

  // Hiển thị góc
 /* display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("P:"); display.print(pitch, 1);
  display.print(" R:"); display.print(roll, 1);*/

  display.display();
}

void setup() {
  Serial.begin(115200);
delay(2000);
  if (!display.begin(oled_addr, true)) {
    Serial.println("OLED init failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(10, 10);
  display.println(F("Dang khoi tao BMI160..."));
  display.display();

  // BMI160 khởi tạo
  if (bmi160.softReset() != BMI160_OK || bmi160.I2cInit(bmi_addr) != BMI160_OK) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Khong tim thay BMI160!"));
    display.display();
    while (1);
  }

  display.clearDisplay();
  display.setCursor(10, 20);
  display.println(F("BMI160 OK!"));
  display.display();
  delay(1000);
}


void loop() {
  int16_t accelGyro[6]={0}; 
  int rslt = bmi160.getAccelGyroData(accelGyro); 

  if(rslt == 0)
  {
     // Lấy accel (index 3..5)
    float ax = accelGyro[3] / 16384.0f;
    float ay = accelGyro[4] / 16384.0f;
    float az = accelGyro[5] / 16384.0f;

    // Tính pitch & roll từ accel
    float pitch = atan2(ax, sqrt(ay*ay + az*az)) * 180.0f / PI;
    float roll  = atan2(ay, sqrt(ax*ax + az*az)) * 180.0f / PI;

    // Hiển thị
    //drawHorizon(pitch, roll);
    drawCube(pitch, roll);

    // (tùy chọn) in ra serial để debug trên PC
    /*Serial.print("AX:"); Serial.print(ax,3);
    Serial.print("\tAY:"); Serial.print(ay,3);
    Serial.print("\tAZ:"); Serial.print(az,3);
    Serial.print("\tP:"); Serial.print(pitch,2);
    Serial.print("\tR:"); Serial.println(roll,2);*/
  }
  else{
    Serial.println("err");
  }

  
  delay(50);
}

