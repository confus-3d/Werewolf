/*
    Werewolf
    Lead development by FJ Rios
    original game Werewolf (1986)
    6 to 12 Blinks (18 Recomended)          
    6-12 players

    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018
    @madewithblinks
    www.move38.com
    --------------------
*/

enum gameState {SETUP, ASIGN, GAME, RESET};
byte gameState = SETUP;
int resetPressed;
#define RESET_TIME 2000
Timer resetTimer;
Timer waitingTimer;
int numNeighbors;
int Count = 0;
int flag;
int asigned;
int character;
int brightness = 0;
int showcharacter = 0; // Turn 1 for debug and see the character asignation
int startgame = 0;

int WOLFS = 2; //Up to 5 wolfs
enum wolf {WY, WN};
byte wolf = WN;
enum seer {SY, SN};
byte seer = SN;
enum villager {VY, VN};
byte villager = VY;


#define MAX_PLAYERS 12 //Games reset if more Blinks detected
#define MIN_PLAYERS 6 //Games reset if less Blinks detected
int sequence[MAX_PLAYERS - 1];

void setup() {  // put your setup code here, to run once:
    randomize();
}

void loop() {  // put your main code here, to run repeatedly:
  switch (gameState){
    case SETUP:
      setupLoop();
      setupDisplayLoop();
      break;
    case ASIGN:
      asignLoop();
      asignDisplayLoop();
      break;
    case GAME:
      gameLoop();
      gameDisplayLoop();
      break;
    case RESET:
      resetLoop();
      resetDisplayLoop();
      break;
  }
  
//Tile comunication
  byte sendData;
  switch (gameState){
    case RESET: 
      sendData = (gameState);
      break;
    case SETUP: 
      sendData = (gameState) + (Count << 2);
      break;
    case GAME: 
      sendData = (wolf << 3) + (seer << 5);
      break;
  }
  setValueSentOnAllFaces(sendData);
}

void resetFunction(){
//RESET FUNCTION
  if (resetPressed == 0) {
    if (buttonLongPressed()) {
      resetPressed = 1;
      WOLFS = 2;
      wolf = WN;
      seer = SN;
      villager = VY;
      Count = 0;
      startgame = 0;
      gameState = RESET;
      resetTimer.set(RESET_TIME);
      
    }
  
    FOREACH_FACE(f) {
      if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor
        byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
        if (neighborGameState == RESET) {
          resetPressed = 1;
          WOLFS = 2;
          wolf = WN;
          seer = SN;
          villager = VY;
          Count = 0;
          startgame = 0;
          gameState = RESET;
          resetTimer.set(RESET_TIME);
        }
      }
    }
  } 
}

void setupLoop() {
  resetFunction();
  if (resetTimer.isExpired()){
      resetPressed = 0;
  } 
  numNeighbors = 0;  
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor
      numNeighbors++;
    }
  }
  if (buttonDoubleClicked() && numNeighbors == 1){
    Count = 1;
  }
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor
      byte neighborCount = getCount(getLastValueReceivedOnFace(f));
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborCount != 0 && Count == 0){
        Count = neighborCount + 1;
      }
      if (neighborGameState == ASIGN) {
        gameState = ASIGN;
        setValueSentOnAllFaces(gameState);
      }
    }
  }
  if (Count > MAX_PLAYERS){
    gameState = RESET;
  }
  if (Count > 1 && numNeighbors == 1 && MIN_PLAYERS){
    gameState = RESET;
  }
  if (Count > 1 && numNeighbors == 1){
    if (buttonDoubleClicked()){
          randomizeArray();
          gameState = ASIGN;
          setValueSentOnAllFaces(gameState);
    }
    if (buttonMultiClicked() && 2 < buttonClickCount() < 6){
          WOLFS = buttonClickCount();
          randomizeArray();
          gameState = ASIGN;
          setValueSentOnAllFaces(gameState);
    }
  }
}

void asignLoop() { 
    if (Count > 1 && numNeighbors == 1){ //Master blink
      for (int i = 0; i < (WOLFS + 1); i++){
        setValueSentOnAllFaces(sequence[i] + 1);
              if (sequence[i] + 1 == Count){
                switch (i){
                  case 0: 
                    seer = SY;
                    villager = VN;
                    break;
                  case 1: 
                  case 2: 
                  case 3:
                  case 4:
                  case 5: 
                    wolf = WY;
                    villager = VN;
                    break;
                } 
              }
        flag = 0;
        waitingTimer.set(200);
        while(flag == 0){
          if (waitingTimer.isExpired()) {
            flag = 1;
            i++;
          }
        }
      }
    gameState = GAME; //Finish sending characters and start game mode
    setValueSentOnAllFaces(15);
    }
    if (Count == 1 || numNeighbors == 2){ //Slave blink
      asigned = 0;
      while (asigned < 6){ 
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte character = getLastValueReceivedOnFace(f);
            setValueSentOnAllFaces(character);
            if (character == Count){
              switch (asigned){
                case 0: 
                  seer = SY;
                  villager = VN;
                  break;
                case 1: 
                case 2: 
                case 3:
                case 4:
                case 5: 
                  wolf = WY;
                  villager = VN;
                  break;
              } 
            }
            if (character == 15){
              gameState = GAME;
              setValueSentOnAllFaces(15);
              asigned = 9;
            }
            flag = 0;
            waitingTimer.set(200);
            while(flag == 0){
                if (waitingTimer.isExpired()) {
                  flag = 1;
                  asigned++;
                }
            }
          }
        }
    }   
  }
}

void gameLoop() {
  resetFunction(); 
  if (isAlone() && startgame == 0) {
    startgame = 1;
  }
  if (startgame == 1) {
    if (resetTimer.isExpired() && showcharacter == 1){
      showcharacter = 0;
    }
    if (buttonSingleClicked() && resetTimer.isExpired()){
      showcharacter = 1;
      resetTimer.set(RESET_TIME);
    }
    if (buttonDoubleClicked() && showcharacter != 2){ //DEAD
      showcharacter = 2;
    } 
    if (buttonDoubleClicked() && showcharacter == 2){ //RESURRECT
      resetTimer.set(RESET_TIME);
      showcharacter = 1;
    } 
    FOREACH_FACE(f) { //Detect WOLF, and DEAD
        if ( !isValueReceivedOnFaceExpired( f ) ) {
          byte neighborWolf = getWolf(getLastValueReceivedOnFace(f));
          byte neighborSeer = getSeer(getLastValueReceivedOnFace(f));
          if (neighborWolf == WY && wolf == WN) {
            showcharacter = 2;
          }
          if (neighborSeer == SY){
            showcharacter = 1;
            resetTimer.set(1000);
          }
        }
      }
  }
}

void resetLoop() { //Timer to reset the game
  if (resetTimer.isExpired()){
        gameState = SETUP;  
        resetTimer.set(RESET_TIME);
  }
}

void randomizeArray(){
  for (int i = Count - 1; i > 0; i--){
    int j = random(i);
    int temp = sequence[i];
    sequence[i] = sequence[j];
    sequence[j] = temp;
  }
}

//Visuals
void setupDisplayLoop() { 
  if (numNeighbors == 1){
    setColorOnFace( dim( GREEN,  255  ), 0);
    setColorOnFace( dim( RED,  255  ), 1);
    setColorOnFace( dim( GREEN,  255  ), 2);
    setColorOnFace( dim( YELLOW,  255  ), 3);
    setColorOnFace( dim( GREEN,  255  ), 4);
    setColorOnFace( dim( RED,  255  ), 5); 
  }
  if (isAlone()){
    setColor(OFF);
  }
  if (numNeighbors > 1 && Count > 0){
    setColor(WHITE);
  }
  if (numNeighbors > 1 && Count == 0){
    setColor(OFF);
  }
}

void asignDisplayLoop() { 
  setColor(OFF);
}

void gameDisplayLoop() { 
  if (showcharacter == 0){
    setColor(OFF);
  }
  if (showcharacter == 1){
    if (villager == VY){
      setColor(GREEN);
    }
    if (wolf == WY){
      setColor(RED);
    }
    if (seer == SY){
      setColor(YELLOW);
    }
  }
  if (showcharacter == 2){
    if (villager == VY){
      setColorOnFace( dim( RED,  255  ), 0);
      setColorOnFace( dim( GREEN,  255  ), 1);
      setColorOnFace( dim( GREEN,  255  ), 2);
      setColorOnFace( dim( RED,  255  ), 3);
      setColorOnFace( dim( GREEN,  255  ), 4);
      setColorOnFace( dim( GREEN,  255  ), 5); 
    }
    if (wolf == WY){
      setColorOnFace( dim( RED,  255  ), 0);
      setColorOnFace( dim( RED,  0  ), 1);
      setColorOnFace( dim( RED,  0  ), 2);
      setColorOnFace( dim( RED,  255  ), 3);
      setColorOnFace( dim( RED,  0  ), 4);
      setColorOnFace( dim( RED,  0  ), 5); 
    }
    if (seer == SY){
      setColorOnFace( dim( RED,  255  ), 0);
      setColorOnFace( dim( YELLOW,  255  ), 1);
      setColorOnFace( dim( YELLOW,  255  ), 2);
      setColorOnFace( dim( RED,  255  ), 3);
      setColorOnFace( dim( YELLOW,  255  ), 4);
      setColorOnFace( dim( YELLOW,  255  ), 5); 
    }
  }
}

void resetDisplayLoop() { 
  setColor(OFF);
}

//Comunication
byte getGameState(byte data) { //BYTE EF
  return (data & 3);
}

byte getCount(byte data) { //BYTE ABCD
  return ((data >> 2) & 15);
}

byte getSeer(byte data) { //BYTE AB
  return ((data >> 5) & 3);
}

byte getWolf(byte data) { //BYTE CD
  return ((data >> 3) & 3);
}
