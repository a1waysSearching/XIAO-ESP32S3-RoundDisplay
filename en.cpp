/**
*
* Food Selection Wheel - Helps you decide what to eat
*
*/
#include <TFT_eSPI.h>

#define USE_TFT_ESPI_LIBRARY
#include "lv_xiao_round_screen.h"

// TFT display instances
TFT_eSprite sprite = TFT_eSprite(&tft); // Off-screen buffer

#define SCREEN_RADIUS 120 // Circular screen radius
#define CENTER_X 120      // Screen center X coordinate
#define CENTER_Y 120      // Screen center Y coordinate

// Food options
const char* foodOptions[] = {
  "Rice", "Noodle", "Jiaozi", 
  "Bun", "Bread", "Porridge", 
  "Null", "MM", "KFC", 
  "Pizza", "BugerK", "Kebab"
};
const int numOptions = sizeof(foodOptions) / sizeof(foodOptions[0]);

// Wheel colors - alternating
uint16_t wheelColors[] = {
  TFT_ORANGE, TFT_DARKCYAN
};

// Wheel state
bool isSpinning = false;
float currentAngle = 0.0;
float spinSpeed = 0.0;
int selectedOption = -1;
unsigned long lastTouchTime = 0;
unsigned long spinStartTime = 0;

// Draw a sector
// Optimized fillSector function, using fillTriangle instead of pixel-by-pixel drawing
void fillSector(TFT_eSprite &spr, int x, int y, int r, float startAngle, float endAngle, uint16_t color) {
  int innerRadius = 30; // Inner circle radius
  for (int i = innerRadius; i <= r; i += 10) { // Increase by 10 pixels each time to reduce loop iterations
    int x1 = x + i * cos(startAngle);
    int y1 = y + i * sin(startAngle);
    int x2 = x + i * cos(endAngle);
    int y2 = y + i * sin(endAngle);
    spr.fillTriangle(x, y, x1, y1, x2, y2, color); // Use fillTriangle to draw the sector
  }
}

// Spin phases
enum SpinPhase {
  ACCELERATING,  // Acceleration phase
  CONSTANT,      // Constant speed phase
  DECELERATING   // Deceleration phase
};
SpinPhase currentPhase = ACCELERATING;

// Spin parameters
float maxSpeed = 0.0;         // Maximum spin speed
float accelerationRate = 0.0;  // Acceleration rate
float decelerationRate = 0.0;  // Deceleration rate
unsigned long constantDuration = 0; // Duration of constant speed phase
unsigned long phaseStartTime = 0;   // Start time of the current phase

// Shuffle the order of options
void shuffleOptions() {
  for (int i = numOptions - 1; i > 0; i--) {
    int j = random(i + 1); // Generate a random index between 0 and i
    const char* temp = foodOptions[i];
    foodOptions[i] = foodOptions[j];
    foodOptions[j] = temp;
  }
}

// Start spinning the wheel, shuffling options and generating random spin parameters
void startSpin() {
  if (!isSpinning) {
    isSpinning = true;

    // Shuffle options
    shuffleOptions();

    // Generate random spin parameters
    maxSpeed = 0.4 + random(200) / 1000.0;  // Random max speed between 0.4-0.6
    accelerationRate = 0.05 + random(50) / 1000.0;  // Acceleration rate
    decelerationRate = 0.01 + random(30) / 1000.0;  // Deceleration rate
    constantDuration = random(2000) + random(2000);  // Constant speed phase duration 0-4 seconds

    // Initialize spin state
    spinSpeed = 0.1;  // Initial speed
    currentPhase = ACCELERATING;
    phaseStartTime = millis();
    selectedOption = -1;

    Serial.println("Start spinning!");
    Serial.print("Max speed: "); Serial.println(maxSpeed);
    Serial.print("Acceleration rate: "); Serial.println(accelerationRate);
    Serial.print("Deceleration rate: "); Serial.println(decelerationRate);
    Serial.print("Constant duration: "); Serial.println(constantDuration);

    // Print shuffled options
    Serial.println("Shuffled options:");
    for (int i = 0; i < numOptions; i++) {
      Serial.println(foodOptions[i]);
    }
  }
}

// Update the spinning wheel
void updateSpin() {
  if (isSpinning) {
    unsigned long currentTime = millis();
    unsigned long phaseTime = currentTime - phaseStartTime;

    // Update spin speed based on the current phase
    switch (currentPhase) {
      case ACCELERATING:
        // Acceleration phase: linearly increase speed until max speed is reached
        spinSpeed += accelerationRate;
        if (spinSpeed >= maxSpeed) {
          spinSpeed = maxSpeed;
          currentPhase = CONSTANT;
          phaseStartTime = currentTime;
          Serial.println("Entering constant speed phase");
        }
        break;

      case CONSTANT:
        // Constant speed phase: maintain max speed with occasional small random fluctuations
        if (random(100) < 20) {  // 20% chance of fluctuation
          spinSpeed = maxSpeed * (0.95 + random(10) / 100.0);  // 95%-105% of max speed
        }

        // Check if it's time to enter the deceleration phase
        if (phaseTime > constantDuration) {
          currentPhase = DECELERATING;
          phaseStartTime = currentTime;
          Serial.println("Entering deceleration phase");
        }
        break;

      case DECELERATING:
        // Deceleration phase: exponential deceleration to simulate real physics
        spinSpeed *= (1.0 - decelerationRate);

        // Stop when the speed is low enough
        if (spinSpeed < 0.01) {
          isSpinning = false;
          float stopAngle = currentAngle;
          selectedOption = (int)(numOptions - (stopAngle / (2 * PI) * numOptions)) % numOptions;
          if (selectedOption < 0) selectedOption += numOptions;
          Serial.print("Selected: ");
          Serial.println(foodOptions[selectedOption]);

          // Show animation for the selected result
          showResultAnimation(selectedOption);
        }
        break;
    }

    // Update angle
    currentAngle += spinSpeed;
    if (currentAngle >= 2 * PI) currentAngle -= 2 * PI;
  }
}

// Main loop function, using non-blocking time management
void loop() {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();

  // Control refresh rate, approximately 60 FPS
  if (currentTime - lastUpdateTime >= 16) {
    lastUpdateTime = currentTime;

    // Update spinning wheel
    updateSpin();

    // Draw the wheel
    drawWheel();

    // Detect touch input
    if (chsc6x_is_pressed()) {
      if (millis() - lastTouchTime > 500) {
        startSpin();
        lastTouchTime = millis();
      }
    }
  }
}

// Show animation for the selected result
void showResultAnimation(int option) {
  // Save current state for later restoration
  bool wasSpinning = isSpinning;
  isSpinning = false;
  
  // Gradually transition to a white background
  for (int i = 0; i < 10; i++) {
    uint16_t bgColor = sprite.color565(
      i * 25, // R
      i * 25, // G
      i * 25  // B
    );
    sprite.fillScreen(bgColor);
    
    // Continue displaying the wheel, but gradually fade out
    float fadeAlpha = 1.0 - (i / 10.0);
    drawWheel();
    
    sprite.pushSprite(0, 0);
    delay(50);
  }
  
  // White background
  sprite.fillScreen(TFT_WHITE);
  
  // Display the selected option with gradually increasing font size
  for (int size = 1; size <= 4; size++) {
    sprite.fillScreen(TFT_WHITE);
    sprite.setTextDatum(MC_DATUM);
    sprite.setTextColor(TFT_BLACK);
    sprite.setTextSize(size);
    sprite.drawString(foodOptions[option], CENTER_X, CENTER_Y, 2);
    sprite.pushSprite(0, 0);
    delay(180 * size);
  }
  
  // Hold the display for a while
  delay(2000);
  
  // Restore the wheel display
  isSpinning = wasSpinning;
  drawWheel();
}

// Draw the spinning wheel
void drawWheel() {
  const int radius = SCREEN_RADIUS - 5;
  
  // Clear the canvas
  sprite.fillScreen(TFT_BLACK);
  
  // Draw the outer circle
  sprite.drawCircle(CENTER_X, CENTER_Y, radius, TFT_WHITE);
  
  // Draw sector areas
  float anglePerOption = 2 * PI / numOptions;
  
  for (int i = 0; i < numOptions; i++) {
    float startAngle = currentAngle + i * anglePerOption - PI / 2;
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
    
    // Fix text drawing issues
    float textAngle = startAngle + anglePerOption / 2;
    int textRadius = radius * 0.7;
    int textX = CENTER_X + textRadius * cos(textAngle);
    int textY = CENTER_Y + textRadius * sin(textAngle);
    
    // Set text color to black, remove background color
    sprite.setTextColor(TFT_BLACK); 
    sprite.setTextSize(1);
    
    // Use drawString method and set text baseline to center
    sprite.setTextDatum(MC_DATUM); // Set text baseline to center
    sprite.drawString(foodOptions[i], textX, textY, 1); // 1 is font size
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
    CENTER_X, 25, // Top vertex
    CENTER_X - 10, 5, // Bottom left
    CENTER_X + 10, 5, // Bottom right
    TFT_RED
  );
  
  sprite.pushSprite(0, 0);
}

// Show startup animation
void showStartupAnimation() {
  sprite.fillScreen(TFT_BLACK);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE);
  
  for (int i = 0; i < 30; i++) {
    sprite.fillScreen(TFT_BLACK);
    sprite.setTextSize(1 + i / 10);
    sprite.drawString("Food Selection Wheel", CENTER_X, CENTER_Y, 2);
    sprite.pushSprite(0, 0);
    delay(5);
  }
  
  delay(500);
  
  for (int i = 0; i < 20; i++) {
    sprite.fillScreen(TFT_BLACK);
    sprite.setTextSize(3);
    sprite.drawString("Food Selection Wheel", CENTER_X, CENTER_Y - 20, 2);
    sprite.setTextSize(1 + i / 10);
    sprite.drawString("Helps you decide what to eat", CENTER_X, CENTER_Y + 30, 2);
    sprite.pushSprite(0, 0);
    delay(5);
  }
  
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nFood Selection Wheel Starting");

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  sprite.createSprite(240, 240);

  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();

  randomSeed(analogRead(0));
  showStartupAnimation();

  Serial.println("Setup complete");
}