//Pins
const int GLED =    22; //Green LED
const int RLED =    21; //Red LED
const int Button1 = 34; //Button A
const int Button2 = 35; //Button B

//Game states
enum GameState {
  DIFFICULTY_SELECTION,
  QUESTION,
  ANSWER,
  POST_ANSWER
};

GameState currentState = DIFFICULTY_SELECTION;

int difficulty = 0;
int score = 0;
int qcount = 0; //Question count

//Variables for questions
int num1, num2, answer;
char op;
int options[2];//2 options for answer
char correctbutton;

//Timer variables
unsigned long questionstarttimer;
unsigned long timerleft; //How much time is left

bool b1waspressed = false;
bool b2waspressed = false;

void setup(){
  Serial.begin(115200);
  while(!Serial){ ; }

  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);

  pinMode(Button1, INPUT);
  pinMode(Button2, INPUT);

  randomSeed(analogRead(A0));
  printstart();
}

void loop(){
  bool b1pressed = (digitalRead(Button1) == HIGH);
  bool b2pressed = (digitalRead(Button2) == HIGH);

  switch(currentState){
    case DIFFICULTY_SELECTION:
      promptdifficulty();
      break;

    case QUESTION:
      generatequestion();
      printquestion();

      if(difficulty == 1) timerleft = 15000;
      else if(difficulty == 2) timerleft = 10000;
      else if(difficulty == 3) timerleft = 5000;

      questionstarttimer = millis(); //Timer start

      currentState = ANSWER;
      break;
    case ANSWER:
      if(b1pressed && !b1waspressed){
        handleanswer('A');
      }else if(b2pressed && !b2waspressed){
        handleanswer('B');
      }else if (!b1pressed && !b2pressed) { // Only check timer if no buttons are pressed
        checktimer();
      }
      break;
    case POST_ANSWER:
      if(b1pressed && !b1waspressed){
        Serial.println("Next question:");
        currentState = QUESTION;
      }else if(b2pressed && !b2waspressed){
        currentState = DIFFICULTY_SELECTION;
        printstart();
      }
      break; 
  }
  b1waspressed = b1pressed;
  b2waspressed = b2pressed;
}

//Logic functions
void printstart(){
  Serial.println("Welcome to the game");
  Serial.println("-------------------");
  Serial.println("Choose a difficulty level");
  Serial.println("1: Easy");
  Serial.println("2: Medium");
  Serial.println("3: Hard");
  Serial.println("Type in the serial monitor");
}

void promptdifficulty(){
  if(Serial.available() > 0){
    char input = Serial.read();
    clearSerialBuffer();
    if(input == '1' || input == '2' || input == '3'){
      difficulty = input - '0';
      Serial.print("Difficulty: ");
      Serial.println(difficulty);
      startgame();
    }else{
      Serial.println("Invalid choice");
    }
  }
}

void startgame(){
  score = 0;
  qcount = 0;
  Serial.println("First question:");
  currentState = QUESTION;
}

void generatequestion() {
  qcount++;
  int opType = 0;
  if (difficulty == 1) {
    opType = random(0, 2); 
    num1 = random(1, 11); 
    num2 = random(1, 11);
    if (opType == 0) { 
      op = '+'; 
      answer = num1 + num2; }
    else { 
      op = '-'; 
    if (num2 > num1) { int t = num1; num1 = num2; num2 = t; } answer = num1 - num2; }
  } else if (difficulty == 2) {
    opType = random(0, 3);
    if (opType == 0) { op = '+'; num1 = random(10, 51); num2 = random(10, 51); answer = num1 + num2; }
    else if (opType == 1) { op = '-'; num1 = random(10, 51); num2 = random(10, 51); if (num2 > num1) { int t = num1; num1 = num2; num2 = t; } answer = num1 - num2; }
    else { op = '*'; num1 = random(2, 13); num2 = random(2, 13); answer = num1 * num2; }
  } else {
    opType = random(0, 4);
    if (opType == 0) { op = '+'; num1 = random(50, 201); num2 = random(50, 201); answer = num1 + num2; }
    else if (opType == 1) { op = '-'; num1 = random(50, 201); num2 = random(50, 201); if (num2 > num1) { int t = num1; num1 = num2; num2 = t; } answer = num1 - num2; }
    else if (opType == 2) { op = '*'; num1 = random(10, 31); num2 = random(5, 16); answer = num1 * num2; }
    else { op = '/'; int result = random(2, 16); num2 = random(2, 11); num1 = result * num2; answer = result; }
  }
  int distractor;
  if (answer > 5) { distractor = answer + random(1, 5); }
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
  Serial.println();
  Serial.print("Question: ");
  Serial.println(qcount);
  Serial.print("What is: ");
  Serial.print(num1);
  Serial.print(op);
  Serial.print(num2);
  Serial.println("?");
  Serial.println();
  Serial.print("A: ");
  Serial.println(options[0]);
  Serial.print("B: ");
  Serial.println(options[1]);
  Serial.println("Press the button for the answer");
}

//Logic for answers
void handleanswer(char chosenLetter){
  if (chosenLetter == correctbutton){
    Serial.println("Correct");
    flashled(GLED, 1000);
    score++;
  } else {
    Serial.print("Incorrect. The answer is: ");
    Serial.print(correctbutton);
    Serial.print(" (");
    Serial.print(answer);
    Serial.println(").");
    flashled(RLED, 1000);
  }

  printscorenav();
}

//Logic for timer
void checktimer() {
  if (currentState != ANSWER) {
    return;
  }

  unsigned long currenttime = millis();
  unsigned long elapsedtime = currenttime - questionstarttimer;

  if (elapsedtime > timerleft) {
    Serial.println("Out of time");
    Serial.println("-------------------------");
    Serial.print("The correct answer was: ");
    Serial.print(correctbutton);
    Serial.print(" (");
    Serial.print(answer);
    Serial.println(").");
    flashled(RLED, 1000);

    printscorenav();
  }
}

//Helper function for printing scores and navigation (POST_ANSWER)
void printscorenav() {
  Serial.print("Score: ");
  Serial.print(score);
  Serial.print(" / ");
  Serial.println(qcount);
  Serial.println();
  Serial.println("Button A = Next Question");
  Serial.println("Button B = Change Difficulty");

  currentState = POST_ANSWER;
}

void flashled(int pin, int duration) {
  digitalWrite(pin, HIGH);
  delay(duration);
  digitalWrite(pin, LOW);
}

void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}