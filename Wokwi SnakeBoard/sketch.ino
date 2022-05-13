#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH        128 // OLED ความกว้างหน้าจอ  หน่วยพิกเซล
#define SCREEN_HEIGHT        64 // ความสูงของหน้าจอ หน่วยพิกเซล


#define OLED_RESET            4 // Reset pin or ถ้าเป็น -1 ให้บอร์ด Arduino resetpin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte buttonPins[] = {4, 2, 5, 3}; // setปุ่ม 4ซ้าย 2 บน 5 ขวา 3 ล่าง


typedef enum {  // เก็บตัวเเปรที่จะทำเป็น START RUNNING GAMEOVERเป็น Structure
  START,
  RUNNING,
  GAMEOVER
} State;

typedef enum { // เก็บตัวเเปรที่จะทำเป็นLEFT UP RIGHT  DOWN  เป็น Structure ที่ตัวเเปรDirection
    LEFT,
    UP,
    RIGHT,
    DOWN
} Direction;

#define SNAKE_PIECE_SIZE     3 // ขนาดของที่งูต้องกิน
#define MAX_SANKE_LENGTH   165 // set ขนาดของงูให้ยาวที่สุดได้เเค่165 หลังกินของ
#define MAP_SIZE_X          41 // ความกว้างของ map ที่งูจะเลื้อย
#define MAP_SIZE_Y          15  //ความสูงของ map  ที่งูจะเลื้อย
#define STARTING_SNAKE_SIZE  3 // ขนาดเริ่มต้นของงูที่ยังไม่เริ่มกิน
int SNAKE_MOVE_DELAY   =20; // set เวลาdelay หลังกดปุ่มให้งูเดิน

State gameState;

int8_t snake[MAX_SANKE_LENGTH][2];
uint8_t snake_length;
Direction dir;
Direction newDir;

int8_t fruit[2];

void setup() {  // set จอ oled ssd
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  for (byte i = 0; i < 4; i++) { // set input การกดปุ่ม
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  randomSeed(analogRead(A0));

  setupGame();
}

void setupGame() {
  gameState = START;
  dir = RIGHT;
  newDir = RIGHT;
  resetSnake();
  generateFruit();
  display.clearDisplay();

  drawScore();
  drawPressToStart();
  display.display();
}

void resetSnake() {   
  snake_length = STARTING_SNAKE_SIZE;
  for(int i = 0; i < snake_length; i++) {
    snake[i][0] = MAP_SIZE_X / 2 - i;
    snake[i][1] = MAP_SIZE_Y / 2;
  }
}

int moveTime = 0;
void loop() {
  switch(gameState) {
    case START:
      if(buttonPress()) gameState = RUNNING; //setให้กดปุ่มใดก็ได้เพื่อเริ่มเกม
      break;
    

    case RUNNING:
      moveTime++;
      readDirection();
      if(moveTime >= SNAKE_MOVE_DELAY) {
        dir = newDir;
        display.clearDisplay();
        if(moveSnake()) {
          gameState = GAMEOVER;
          gameover();
          delay(1000);
          
        }
        drawMap();
        drawScore();
        display.display();
        checkFruit();
        moveTime = 0;
      }
      break;
    
    case GAMEOVER:
      if(buttonPress()) {
        delay(500);//delay ข ้อความ GAMEOVER หลังชนกำเเพง
        setupGame();
        gameState = START;//วนกลับไปโดยถ้าจะเล่นต่อให้กดปุ่มใดก็ได้เพื่อเริ่มเกม
      }
      break;
  }
  
  delay(10);
}

bool buttonPress() {
  for (byte i = 0; i < 4; i++) {//วนลูปเพื่อที่จะให้กดปุ่มใดก็ได้เพื่อเริ่มเกมเเละกดปุ่มใดก็เพื่อเริ่มเล่นเกมใหม่
  // 0 ซ้าย 1 บน 2 ขวา 3 ล่าง
    if (digitalRead(buttonPins[0]) == LOW) { //ก่อนที่จะเริ่มเล่นตัวงูต้องหยุด
    SNAKE_MOVE_DELAY   =50;
      return true;
    }
    else if (digitalRead(buttonPins[1]) == LOW) { //ก่อนที่จะเริ่มเล่นตัวงูต้องหยุด
    SNAKE_MOVE_DELAY   =30;
      return true;
    }
    else if (digitalRead(buttonPins[2]) == LOW) { //ก่อนที่จะเริ่มเล่นตัวงูต้องหยุด
    SNAKE_MOVE_DELAY   =20;
      return true;
    }
    else if (digitalRead(buttonPins[3]) == LOW) { //ก่อนที่จะเริ่มเล่นตัวงูต้องหยุด
    SNAKE_MOVE_DELAY   =10;
      return true;
    }
  }
  return false;
}

void readDirection() {
  for (byte i = 0; i < 4; i++) {
    byte buttonPin = buttonPins[i];
    if (digitalRead(buttonPin) == LOW && i != ((int)dir + 2) % 4) {
      newDir = (Direction)i;
      return;
    }
  }
}

bool moveSnake() {
  int8_t x = snake[0][0];
  int8_t y = snake[0][1];

  switch(dir) {
    case LEFT:
      x -= 1;
      break;
    case UP:
      y -= 1;
      break;
    case RIGHT:
      x += 1;
      break;
    case DOWN:
      y += 1;
      break;
  }

  if(collisionCheck(x, y))
    return true;

  for(int i = snake_length - 1; i > 0; i--) {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }

  snake[0][0] = x;
  snake[0][1] = y;
  return false;
}

void checkFruit() {
  if(fruit[0] == snake[0][0] && fruit[1] == snake[0][1])
  {
    if(snake_length + 1 <= MAX_SANKE_LENGTH)
      snake_length++;
    generateFruit();
  }
}

void generateFruit() {
  bool b = false;
  do {
    b = false;
    fruit[0] = random(0, MAP_SIZE_X);
    fruit[1] = random(0, MAP_SIZE_Y);
    for(int i = 0; i < snake_length; i++) {
      if(fruit[0] == snake[i][0] && fruit[1] == snake[i][1]) {
        b = true;
        continue;
      }
    }
  } while(b);
}

bool collisionCheck(int8_t x, int8_t y) {
  for(int i = 1; i < snake_length; i++) {
    if(x == snake[i][0] && y == snake[i][1]) return true;
  }
  if(x < 0 || y < 0 || x >= MAP_SIZE_X || y >= MAP_SIZE_Y) return true;
  return false;
}

void drawMap() {
  int offsetMapX = SCREEN_WIDTH - SNAKE_PIECE_SIZE * MAP_SIZE_X - 2;
  int offsetMapY = 2;

  display.drawRect(fruit[0] * SNAKE_PIECE_SIZE + offsetMapX, fruit[1] * SNAKE_PIECE_SIZE + offsetMapY, SNAKE_PIECE_SIZE, SNAKE_PIECE_SIZE, SSD1306_INVERSE);
  display.drawRect(offsetMapX - 2, 0, SNAKE_PIECE_SIZE * MAP_SIZE_X + 4, SNAKE_PIECE_SIZE * MAP_SIZE_Y + 4, SSD1306_WHITE);
  for(int i = 0; i < snake_length; i++) {
    display.fillRect(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX, snake[i][1] * SNAKE_PIECE_SIZE + offsetMapY, SNAKE_PIECE_SIZE, SNAKE_PIECE_SIZE, SSD1306_WHITE);
  }
}

void drawScore() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2,51);
  
  display.print(F("Score:"));
  display.println(snake_length - STARTING_SNAKE_SIZE);
}

void drawPressToStart() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2,0);
  display.print(F("Press to play!\n Left:easy\n Up:normal\n Right:hard\n Down:very hard"));
}

void drawGameover() {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5,20);
  display.println(F("GAME OVER!"));
}
void gameover(){
   dir = RIGHT;
  newDir = RIGHT;
  display.clearDisplay();

  drawGameover();
}
