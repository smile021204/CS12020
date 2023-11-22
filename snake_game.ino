#include <AberLED.h>
/**************************************************************************

   The state machine

 **************************************************************************/
// invalid state to initialise the state variable
#define S_INVALID -1
// the starting state
#define S_START 0
// the playing state
#define S_PLAYING 1
// a life has just been lost
#define S_LIFELOST 2
// the player has lost their last life
#define S_END 3


//the state variable - starts our invalid
int state = S_INVALID;
// the time the current state was entered
unsigned long stateStartTime;

//state variable
void gotoState(int s) {
  Serial.print("Going to state "); // for debugging
  Serial.println(s);
  state = s;
  stateStartTime = millis();
}

// get the time the system has been in the current state
unsigned long getStateTime() {
  return millis() - stateStartTime;
}


/**************************************************************************

   The player model and its code

 **************************************************************************/

unsigned long nextMoveTime = 0; // for timer
#define MOVEINTERVAL 300 // time between updates


int playerX[16]; // it is a global array for player X position and max length for snake
int playerY[16]; // it is a global array for player Y position and max length for snake

int playerLives = 3; // number of lives remaining;
int snakeSize; // size of the snake

int obstacleX, obstacle2X, obstacle3X;
int obstacleY, obstacle2Y, obstacle3Y;

int foodX; // variable for the position of food
int foodY; // variable for the position of food

String sDirection; // direction of snake's move


void renderFood() {
  AberLED.set(foodX, foodY, YELLOW);
  delay(50); // short delay for making new food
}

bool foodPosSame(int x, int y) { // for checking when the food is in same position with snake
  for (int i = 0; i < snakeSize - 1; i++) {
    if ((x == playerX[i]) && (y == playerY[i])) {
      return true;
    }
  }
  return false;
}

bool foodObsSame(int x, int y) { // for checking when the food is in same position with obstacles
  if ((x == obstacleX) && (y == obstacleY)) {
    return true;
  }
  if ((x == obstacle2X) && (y == obstacle2Y)) {
    return true;
  }
  if ((x == obstacle3X) && (y == obstacle3Y)) {
    return true;
  }
  return false;
}

bool obsObs1Same(int x, int y) { // for checking when the obstacles are in same position with the other obstacles
  if ((x == obstacleX) && (y == obstacleY)) {
    return true;
  }
  return false;
}

bool obsObs2Same(int x, int y) {// for checking when the obstacles are in same position with the other obstacles
  if ((x == obstacleX) && (y == obstacleY)) {
    return true;
  }
  if ((x == obstacle2X) && (y == obstacle2Y)) {
    return true;
  }
  return false;
}

void createFood() { // allocating random position for food
  int cFoodX = random(0, 7);
  int cFoodY = random(0, 7);
  while (foodPosSame(cFoodX, cFoodY)) {
    cFoodX = random(0, 7);
    cFoodY = random(0, 7);
  }
  while (foodObsSame(cFoodX, cFoodY)) {
    cFoodX = random(0, 7);
    cFoodY = random(0, 7);
  }
  foodX = cFoodX;
  foodY = cFoodY;
}

void createObs() { // allocating positions for obstacles
  int newObstacleX, newObstacle2X, newObstacle3X;
  int newObstacleY, newObstacle2Y, newObstacle3Y;
  int dx = 0, dx2 = 0, dx3 = 0;
  int dy = 0, dy2 = 0, dy3 = 0;

  do {
    newObstacleX = random(0, 7);
    newObstacleY = random(0, 7);
    dx = abs(newObstacleX - playerX[0]);
    dy = abs(newObstacleY - playerY[0]);
  } while (dx < 3 && dy < 3);
  obstacleX = newObstacleX;
  obstacleY = newObstacleY;


  do {
    newObstacle2X = random(0, 7);
    newObstacle2Y = random(0, 7);
    while (obsObs1Same(newObstacle2X, newObstacle2Y)) {
      newObstacle2X = random(0, 7);
      newObstacle2Y = random(0, 7);
    }
    dx2 = abs(newObstacle2X - playerX[0]);
  } while (dx2 < 3 && dy2 < 3);
  obstacle2X = newObstacle2X;
  obstacle2Y = newObstacle2Y;

  do {
    newObstacle3X = random(0, 7);
    newObstacle3Y = random(0, 7);
    while (obsObs2Same(newObstacle3X, newObstacle3Y)) {
      newObstacle3X = random(0, 7);
      newObstacle3Y = random(0, 7);
    }
    dx3 = abs(newObstacle3X - playerX[0]);
    dy3 = abs(newObstacle3Y - playerY[0]);
  } while (dx3 < 3 && dy3 < 3);

  obstacle3X = newObstacle3X;
  obstacle3Y = newObstacle3Y;
}

void renderObs() {
  AberLED.set(obstacleX, obstacleY, RED);
  AberLED.set(obstacle2X, obstacle2Y, RED);
  AberLED.set(obstacle3X, obstacle3Y, RED);
}


void initGame() { // init of game
  for (int i = 0; i < 8; i++) { //setting the snake as the previous position
    playerX[i] = -1;
    playerY[i] = -1;
  }
  playerX[0] = random(1, 7);
  playerY[0] = random(1, 7);
  sDirection = "up";
  snakeSize = 1;
  createObs();
  createFood();
}

//removes a life from the player,
//returns true if the player is dead
bool removeLife() {
  playerLives--;
  return playerLives == 0;
}

void checkFood() { // if snake hits food, snake grows up
  if (playerX[0] == foodX && playerY[0] == foodY) {
    snakeSize++;
    if (snakeSize == 17) {
      snakeSize = snakeSize - 1;
    }
    createFood();
  }
}


void checkHitSelf() { // if snake hits itself, go to lifelost state
  for (int i = 1; i < snakeSize - 1; i++) {
    if (playerX[0] == playerX[i] && playerY[0] == playerY[i]) {
      if (removeLife()) {
        gotoState(S_END);
      }
      else {
        gotoState(S_LIFELOST);
      }
    }
  }
}

void checkObstacle() {
  if (playerX[0] == obstacleX && playerY[0] == obstacleY) {
    if (removeLife()) {
      gotoState(S_END);
    }
    else {
      gotoState(S_LIFELOST);
    }
  }
  if (playerX[0] == obstacle2X && playerY[0] == obstacle2Y) {
    if (removeLife()) {
      gotoState(S_END);
    }
    else {
      gotoState(S_LIFELOST);
    }
  }
  if (playerX[0] == obstacle3X && playerY[0] == obstacle3Y) {
    if (removeLife()) {
      gotoState(S_END);
    }
    else {
      gotoState(S_LIFELOST);
    }
  }
}


void renderPlayer() { // draw the snake
  for (int i = 0; i < snakeSize; i++) {
    AberLED.set(playerX[i], playerY[i], GREEN);
  }
}

void buttonClick() { // for the AberLED.getButtonDown
  if (AberLED.getButtonDown(UP)) { //set the sDirection up
    sDirection = "up";
    Serial.println(sDirection);
  }
  if (AberLED.getButtonDown(DOWN)) {//set the sDirection down
    sDirection = "down";
    Serial.println(sDirection);
  }
  if (AberLED.getButtonDown(LEFT)) {//set the sDirection left
    sDirection = "left";
    Serial.println(sDirection);
  }
  if (AberLED.getButtonDown(RIGHT)) {//set the sDirection up
    sDirection = "right";
    Serial.println(sDirection);
  }
}

void edge() { //move left if snake head touches edge
  if (playerX[0] == 8) {
    playerX[0] = 7;
    sDirection = "up";
    playerY[0]--;
  }
  else if (playerX[0] == -1) {
    playerX[0] = 0;
    sDirection = "down";
    playerY[0]++;
  }
  else if (playerY[0] == 8) {
    playerY[0] = 7;
    sDirection = "right";
    playerX[0]++;
  }
  else if (playerY[0] == -1) {
    playerY[0] = 0;
    sDirection = "left";
    playerX[0]--;
  }
}


void smove(String dir) { //snake's move function and get direction from the button
  for (int i = snakeSize - 1; i > 0; i--) { // to 
    playerX[i] = playerX[i - 1];
    playerY[i] = playerY[i - 1];
  }
  if (dir == "up") {
    playerY[0]--;
  }
  else if ( dir == "down") {
    playerY[0]++;
  }
  else if (dir == "left") {
    playerX[0]--;
  }
  else if (dir == "right") {
    playerX[0]++;
  }
}

//render lives in the LiveLost state as 3 lights (from worksheet code)
void renderLives() {
  AberLED.set(2, 4, GREEN); // left dot always green
  if (playerLives > 1) { // middle dot green if lives>1
    AberLED.set(3, 4, GREEN);
  }
  else {
    AberLED.set(3, 4, RED);
  }
  if (playerLives > 2) { // right dot green if lives>2
    AberLED.set(4, 4, GREEN);
  }
  else {
    AberLED.set(4, 4, RED);
  }
}


/************************************************************************/
/************************ main code *************************************/
/***********************************************************************/


void setup() {
  AberLED.begin();
  Serial.begin(9600);
  unsigned long seed = 0L; // for random position of player
  for (int i = 0; i < 100; i++) {
    delay(1);
    seed += analogRead(A1);
  }
  randomSeed(seed);
  nextMoveTime = millis() + MOVEINTERVAL; // timer
  initGame(); // initialse the game
  gotoState(S_START);
}


void handleInput() {
  switch (state) {
    case S_START:
      // on FIRE, going into the player state
      if (AberLED.getButtonDown(FIRE)) {
        initGame();
        playerLives = 3;
        gotoState(S_PLAYING);
      }
      break;
    case S_PLAYING:
      buttonClick();
      if (AberLED.getButtonDown(FIRE)) {
        gotoState(S_END);
      }
      break;
    case S_LIFELOST:
      break;
    case S_END:
      // fire button returns to start state
      if (AberLED.getButtonDown(FIRE)) {
        gotoState(S_START);
        break;
      }
  }
}

void updateModel() {
  switch (state) {
    case S_START:
      break;
    case S_PLAYING:
      smove(sDirection);
      edge();
      checkFood();
      checkHitSelf();
      checkObstacle();
      break;
    case S_LIFELOST:
      // if we're in this state for 2 seconds,
      // go back to playing, and reset the game
      if (getStateTime() > 2000) {
        initGame();
        gotoState(S_PLAYING);
      }
      break;
    case S_END:
      break;
    default:
      Serial.println("Bad state in update!");
  }

}


// draw a frame with given colour
void renderFrame(int colour) {
  for (int i = 0; i < 8; i++) {
    AberLED.set(0, i, colour); // left side
    AberLED.set(7, i, colour); // right side
    AberLED.set(i, 0, colour); // top side
    AberLED.set(i, 7, colour); // bottom side
  }
}


void render() {
  switch (state) {
    case S_START:
      // draw a green frame
      renderFrame(GREEN);
      break;
    case S_PLAYING:
      // draw the snake, food and obstacles
      renderFood();
      renderPlayer();
      renderObs();
      break;
    case S_LIFELOST:
      // draw a yellow frame and remaining lives
      renderFrame(YELLOW);
      renderLives();
      break;
    case S_END:
      // draw a red frame
      renderFrame(RED);
      break;
    default:
      Serial.println("Bad state in RENDER!");
      break;
  }
}


void loop() {
  handleInput();
  if (millis() >= nextMoveTime) {
    nextMoveTime = millis() + MOVEINTERVAL;
    updateModel();
  }
  AberLED.clear();
  render();
  AberLED.swap();
}
