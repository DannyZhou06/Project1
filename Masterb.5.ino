//Game logic for the first esp, this will act as the master
#include <esp_now.h>
#include <WiFi.h>

//Communication address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct message_struct {
  char text[60]; 
} message_struct;

message_struct message;
esp_now_peer_info_t esp2info;

//Pins (placeholder)
const int GLED    = 22; 
const int RLED    = 21;
const int Button1 = 34; // Button A
const int Button2 = 35; // Button B

//States
enum GameState {
  DIFFICULTY_SELECTION,
  QUESTION,
  ANSWER,
  POST_ANSWER
};

GameState currentState = DIFFICULTY_SELECTION;

//Variables
int difficulty = 0;
int selectionmenu = 1;
int score = 0;
int qcount = 0;

int num1, num2, answer;
char op;
int options[2];
char correctbutton;

//Timers
unsigned long questionstarttimer;
unsigned long timerleft;
int easytimer = 15000;
int mediumtimer = 10000;
int hardtimer = 5000;

//Detection for button press
bool b1waspressed = false;
bool b2waspressed = false;

//Communication with slave(ESP for LED matrix screen)
void sendtomatrix(const char* format, ...){
  char buffer[60];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Serial.println(buffer);

  strcpy(message.text, buffer);
  esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
}

void setup(){
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("ESP-now failed");
    return;
  }
  memcpy(esp2info.peer_addr, broadcastAddress, 6);
  esp2info.channel = 0;
  esp2info.encrypt = false;
  if(esp_now_add_peer(&esp2info) != ESP_OK){
    Serial.println("Communication failed to establish");
    return;
  }

  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);
  pinMode(Button1, INPUT);
  pinMode(Button2, INPUT);

  randomSeed(analogRead(0)); 

  // Initial Prompt
  Serial.println("Game Started");
  sendtomatrix("A: Cycle B: Select"); 
}

void loop(){
  bool b1pressed = (digitalRead(Button1) == HIGH);
  bool b2pressed = (digitalRead(Button2) == HIGH);

  switch(currentState){
    case DIFFICULTY_SELECTION:
    //Cycle through difficulty selection
      if(b1pressed && !b1waspressed){
        selectionmenu++;
        if(selectionmenu > 3){
          selectionmenu = 1;
        }

        if(selectionmenu == 1)      sendtomatrix("Mode: Easy");
        else if(selectionmenu == 2) sendtomatrix("Mode: Medium");
        else if(selectionmenu == 3) sendtomatrix("Mode: Hard");
      }
    //Confirm diffculty selection
      else if(b2pressed && !b2waspressed) {
      difficulty = selectionmenu;
      sendtomatrix("Game starting, get ready");
      delay(500);
      startgame();
      }
      break;

    case QUESTION:
      generatequestion();
      printquestion();

      // Set timer based on difficulty
      if(difficulty == 1) timerleft = easytimer;
      else if(difficulty == 2) timerleft = mediumtimer;
      else if(difficulty == 3) timerleft = hardtimer;

      questionstarttimer = millis();
      currentState = ANSWER;
      break;

    case ANSWER:
      if(b1pressed && !b1waspressed){
        handleanswer('A');
      }else if(b2pressed && !b2waspressed){
        handleanswer('B');
      }else if (!b1pressed && !b2pressed) { 
        checktimer();
      }
      break;

    case POST_ANSWER:
      if(b1pressed && !b1waspressed){
        sendtomatrix("Next Question...");
        currentState = QUESTION;
      }else if(b2pressed && !b2waspressed){
        currentState = DIFFICULTY_SELECTION;
        selectionmenu = 1;//Reset menu
        sendtomatrix("A:Cycle B:OK");
      }
      break; 
  }
  
  b1waspressed = b1pressed;
  b2waspressed = b2pressed;
  delay(50);
}

void startgame(){
  score = 0;
  qcount = 0;
  currentState = QUESTION;
}

void generatequestion() {
  qcount++;
  int opType = 0;
  
  if (difficulty == 1) {
    opType = random(0, 2); 
    num1 = random(1, 11); num2 = random(1, 11);
    if (opType == 0) { op = '+'; answer = num1 + num2; }
    else { 
      op = '-';
      if (num2 > num1) { int t = num1; num1 = num2; num2 = t; } 
      answer = num1 - num2; 
    }
  } else if (difficulty == 2) {
    opType = random(0, 3);
    if (opType == 0) { op = '+'; num1 = random(10, 51); num2 = random(10, 51); answer = num1 + num2; }
    else if (opType == 1) { 
      op = '-'; num1 = random(10, 51); num2 = random(10, 51); 
      if (num2 > num1) { int t = num1; num1 = num2; num2 = t; } 
      answer = num1 - num2; 
    }
    else { op = '*'; num1 = random(2, 13); num2 = random(2, 13); answer = num1 * num2; }
  } else {
    //Hard Mode
    opType = random(0, 4);
    if (opType == 0) { op = '+'; num1 = random(50, 201); num2 = random(50, 201); answer = num1 + num2; }
    else if (opType == 1) { 
       op = '-'; num1 = random(50, 201); num2 = random(50, 201); 
       if (num2 > num1) { int t = num1; num1 = num2; num2 = t; } 
       answer = num1 - num2; 
    }
    else if (opType == 2) { op = '*'; num1 = random(10, 31); num2 = random(5, 16); answer = num1 * num2; }
    else { 
      op = '/'; int result = random(2, 16); num2 = random(2, 11);
      num1 = result * num2; answer = result; 
    }
  }

  int distractor;
  if (answer > 5) distractor = answer + random(1, 5);
  else { distractor = answer - random(1, 3); if (distractor < 0) distractor = 0; }
  if (distractor == answer) distractor++;

  options[0] = answer;
  options[1] = distractor;
  int correctslot = 0;
  
  if (random(0, 2) == 1) {
    int temp = options[0]; options[0] = options[1]; options[1] = temp;
    correctslot = 1;
  }
  correctbutton = 'A' + correctslot;
}

void printquestion(){
  sendtomatrix("%d%c%d=? A:%d B:%d", num1, op, num2, options[0], options[1]);
}

void handleanswer(char chosenletter){
  if (chosenletter == correctbutton){
    flashled(GLED, 1000);
    score++;
    sendtomatrix("Correct answer");
  } else {
    flashled(RLED, 1000);
    sendtomatrix("Incorrect answer Answer:%c", correctbutton);
  }
  delay(1500); 
  printscorenav();
}

void checktimer() {
  unsigned long currenttime = millis();
  unsigned long elapsedtime = currenttime - questionstarttimer;
  if (elapsedtime > timerleft) {
    flashled(RLED, 1000);
    sendtomatrix("Time's up! Answer:%c", correctbutton);
    delay(1500);
    printscorenav();
  }
}

void printscorenav() {
  sendtomatrix("Score:%d/%d A:Next B:Menu", score, qcount);
  currentState = POST_ANSWER;
}

void flashled(int pin, int duration) {
  digitalWrite(pin, HIGH);
  delay(duration);
  digitalWrite(pin, LOW);
}