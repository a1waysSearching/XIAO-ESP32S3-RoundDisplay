/**
*
* 食物选择转盘 - 帮助你决定吃什么
*
*/
#include <TFT_eSPI.h>

#define USE_TFT_ESPI_LIBRARY
#include "lv_xiao_round_screen.h"

// TFT display instances
TFT_eSprite sprite = TFT_eSprite(&tft); // Off-screen buffer

#define SCREEN_RADIUS 120 // 圆形屏幕半径
#define CENTER_X 120      // 屏幕中心X坐标
#define CENTER_Y 120      // 屏幕中心Y坐标

// 食物选项
const char* foodOptions[] = {
  "Rice", "Noodle", "Jiaozi", 
  "Bun", "Bread", "Porridge", 
  "Null", "MM", "KFC", 
  "Pizza", "BugerK", "Kebab"
};
const int numOptions = sizeof(foodOptions) / sizeof(foodOptions[0]);

// 转盘颜色 - 交替使用
uint16_t wheelColors[] = {
  TFT_ORANGE, TFT_DARKCYAN
};

// 转盘状态
bool isSpinning = false;
float currentAngle = 0.0;
float spinSpeed = 0.0;
int selectedOption = -1;
unsigned long lastTouchTime = 0;
unsigned long spinStartTime = 0;

// 绘制扇形
// 优化 fillSector 函数，使用 fillTriangle 替代逐像素绘制
void fillSector(TFT_eSprite &spr, int x, int y, int r, float startAngle, float endAngle, uint16_t color) {
  int innerRadius = 30; // 内圆半径
  for (int i = innerRadius; i <= r; i += 10) { // 每次增加 10 像素，减少循环次数
    int x1 = x + i * cos(startAngle);
    int y1 = y + i * sin(startAngle);
    int x2 = x + i * cos(endAngle);
    int y2 = y + i * sin(endAngle);
    spr.fillTriangle(x, y, x1, y1, x2, y2, color); // 使用 fillTriangle 绘制扇形
  }
}

// 旋转阶段
enum SpinPhase {
  ACCELERATING,  // 加速阶段
  CONSTANT,      // 匀速阶段
  DECELERATING   // 减速阶段
};
SpinPhase currentPhase = ACCELERATING;

// 旋转参数
float maxSpeed = 0.0;         // 最大旋转速度
float accelerationRate = 0.0;  // 加速率
float decelerationRate = 0.0;  // 减速率
unsigned long constantDuration = 0; // 匀速阶段持续时间
unsigned long phaseStartTime = 0;   // 当前阶段开始时间

// 修改 startSpin 函数，实现随机旋转参数
// ... existing code ...

// 打乱选项顺序的函数
void shuffleOptions() {
  for (int i = numOptions - 1; i > 0; i--) {
    int j = random(i + 1); // 生成 0 到 i 的随机索引
    const char* temp = foodOptions[i];
    foodOptions[i] = foodOptions[j];
    foodOptions[j] = temp;
  }
}

// 修改 startSpin 函数，实现选项顺序打乱
void startSpin() {
  if (!isSpinning) {
    isSpinning = true;

    // 打乱选项顺序
    shuffleOptions();

    // 随机生成旋转参数
    maxSpeed = 0.4 + random(200) / 1000.0;  // 0.4-0.6 的随机最大速度
    accelerationRate = 0.05 + random(50) / 1000.0;  // 加速率
    decelerationRate = 0.01 + random(30) / 1000.0;  // 减速率
    constantDuration = random(2000) + random(2000);  // 匀速阶段持续 0-4 秒

    // 初始化旋转状态
    spinSpeed = 0.1;  // 起始速度
    currentPhase = ACCELERATING;
    phaseStartTime = millis();
    selectedOption = -1;

    Serial.println("开始旋转!");
    Serial.print("最大速度: "); Serial.println(maxSpeed);
    Serial.print("加速率: "); Serial.println(accelerationRate);
    Serial.print("减速率: "); Serial.println(decelerationRate);
    Serial.print("匀速持续: "); Serial.println(constantDuration);

    // 打印打乱后的选项顺序
    Serial.println("打乱后的选项顺序:");
    for (int i = 0; i < numOptions; i++) {
      Serial.println(foodOptions[i]);
    }
  }
}

// 更新转盘旋转
void updateSpin() {
  if (isSpinning) {
    unsigned long currentTime = millis();
    unsigned long phaseTime = currentTime - phaseStartTime;

    // 根据当前阶段更新旋转速度
    switch (currentPhase) {
      case ACCELERATING:
        // 加速阶段：线性增加速度直到达到最大速度
        spinSpeed += accelerationRate;
        if (spinSpeed >= maxSpeed) {
          spinSpeed = maxSpeed;
          currentPhase = CONSTANT;
          phaseStartTime = currentTime;
          Serial.println("进入匀速阶段");
        }
        break;

      case CONSTANT:
        // 匀速阶段：保持最大速度，偶尔添加小的随机波动
        if (random(100) < 20) {  // 20%的概率产生波动
          spinSpeed = maxSpeed * (0.95 + random(10) / 100.0);  // 最大速度的95%-105%
        }

        // 检查是否应该进入减速阶段
        if (phaseTime > constantDuration) {
          currentPhase = DECELERATING;
          phaseStartTime = currentTime;
          Serial.println("进入减速阶段");
        }
        break;

      case DECELERATING:
        // 减速阶段：指数减速，模拟真实物理效果
        spinSpeed *= (1.0 - decelerationRate);

        // 当速度足够小时停止
        if (spinSpeed < 0.01) {
          isSpinning = false;
          float stopAngle = currentAngle;
          selectedOption = (int)(numOptions - (stopAngle / (2 * PI) * numOptions)) % numOptions;
          if (selectedOption < 0) selectedOption += numOptions;
          Serial.print("选中了: ");
          Serial.println(foodOptions[selectedOption]);

          // 显示选中结果的动画
          showResultAnimation(selectedOption);
        }
        break;
    }

    // 更新角度
    currentAngle += spinSpeed;
    if (currentAngle >= 2 * PI) currentAngle -= 2 * PI;
  }
}

// 修改 loop 函数，移除 delay，使用非阻塞时间管理
void loop() {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();

  // 控制刷新频率，约 60 FPS
  if (currentTime - lastUpdateTime >= 16) {
    lastUpdateTime = currentTime;

    // 更新转盘旋转
    updateSpin();

    // 绘制转盘
    drawWheel();

    // 检测触摸输入
    if (chsc6x_is_pressed()) {
      if (millis() - lastTouchTime > 500) {
        startSpin();
        lastTouchTime = millis();
      }
    }
  }
}

// 显示选中结果的动画
void showResultAnimation(int option) {
  // 保存当前状态以便稍后恢复
  bool wasSpinning = isSpinning;
  isSpinning = false;
  
  // 渐变到白色背景
  for (int i = 0; i < 10; i++) {
    uint16_t bgColor = sprite.color565(
      i * 25, // R
      i * 25, // G
      i * 25  // B
    );
    sprite.fillScreen(bgColor);
    
    // 继续显示转盘，但逐渐淡出
    float fadeAlpha = 1.0 - (i / 10.0);
    drawWheel();
    
    sprite.pushSprite(0, 0);
    delay(50);
  }
  
  // 白色背景
  sprite.fillScreen(TFT_WHITE);
  
  // 显示选中的选项，字体逐渐变大
  for (int size = 1; size <= 4; size++) {
    sprite.fillScreen(TFT_WHITE);
    sprite.setTextDatum(MC_DATUM);
    sprite.setTextColor(TFT_BLACK);
    sprite.setTextSize(size);
    sprite.drawString(foodOptions[option], CENTER_X, CENTER_Y, 2);
    sprite.pushSprite(0, 0);
    delay(180*size);
  }
  
  // 定格显示一段时间
  delay(2000);
  
  // 恢复转盘显示
  isSpinning = wasSpinning;
  drawWheel();
}

// 绘制转盘
void drawWheel() {
  const int radius = SCREEN_RADIUS - 5;
  
  // 清空画布
  sprite.fillScreen(TFT_BLACK);
  
  // 绘制外圆
  sprite.drawCircle(CENTER_X, CENTER_Y, radius, TFT_WHITE);
  
  // 绘制扇形区域
  float anglePerOption = 2 * PI / numOptions;
  
  for (int i = 0; i < numOptions; i++) {
    float startAngle = currentAngle + i * anglePerOption - PI/2;
    float endAngle = startAngle + anglePerOption;
    
    uint16_t sectorColor = wheelColors[i % 2];
    if (!isSpinning && i == selectedOption) {
      sectorColor = TFT_RED;
    }
    
    fillSector(sprite, CENTER_X, CENTER_Y, radius - 10, startAngle, endAngle, sectorColor);
    
    sprite.drawLine(
      CENTER_X, 
      CENTER_Y, 
      CENTER_X + cos(startAngle) * radius, 
      CENTER_Y + sin(startAngle) * radius, 
      TFT_WHITE
    );
    
    // 修复字体绘制问题
    float textAngle = startAngle + anglePerOption / 2;
    int textRadius = radius * 0.7;
    int textX = CENTER_X + textRadius * cos(textAngle);
    int textY = CENTER_Y + textRadius * sin(textAngle);
    
    // 设置字体颜色为黑色，移除背景色
    sprite.setTextColor(TFT_BLACK); 
    sprite.setTextSize(1);
    
    // 使用 drawString 方法并设置文本基准点为中心
    sprite.setTextDatum(MC_DATUM); // 设置文本基准点为中心
    sprite.drawString(foodOptions[i], textX, textY, 1); // 1 是字体大小
  }
  
  sprite.fillCircle(CENTER_X, CENTER_Y, 30, TFT_DARKGREY);
  sprite.drawCircle(CENTER_X, CENTER_Y, 30, TFT_WHITE);
  
  sprite.setTextColor(TFT_WHITE, TFT_DARKGREY);
  if (isSpinning) {
    sprite.setCursor(CENTER_X - 20, CENTER_Y - 4);
    sprite.print("Rotating");
  } else if (selectedOption >= 0) {
    sprite.setCursor(CENTER_X - 20, CENTER_Y - 4);
    sprite.print("Eat it!");
  } else {
    sprite.setCursor(CENTER_X - 20, CENTER_Y - 4);
    sprite.print("Press");
  }
  
  sprite.fillTriangle(
    CENTER_X, 25, // 顶点位置
    CENTER_X - 10, 5, // 左下角
    CENTER_X + 10, 5, // 右下角
    TFT_RED
  );
  
  sprite.pushSprite(0, 0);
}

// 显示启动动画
void showStartupAnimation() {
  sprite.fillScreen(TFT_BLACK);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE);
  
  for (int i = 0; i < 30; i++) {
    sprite.fillScreen(TFT_BLACK);
    sprite.setTextSize(1 + i / 10);
    sprite.drawString("食物选择转盘", CENTER_X, CENTER_Y, 2);
    sprite.pushSprite(0, 0);
    delay(5);
  }
  
  delay(500);
  
  for (int i = 0; i < 20; i++) {
    sprite.fillScreen(TFT_BLACK);
    sprite.setTextSize(3);
    sprite.drawString("食物选择转盘", CENTER_X, CENTER_Y - 20, 2);
    sprite.setTextSize(1 + i / 10);
    sprite.drawString("帮你决定吃什么", CENTER_X, CENTER_Y + 30, 2);
    sprite.pushSprite(0, 0);
    delay(5);
  }
  
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n食物选择转盘启动");

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  sprite.createSprite(240, 240);

  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();

  randomSeed(analogRead(0));
  showStartupAnimation();

  Serial.println("设置完成");
}