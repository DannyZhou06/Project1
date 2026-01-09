// Sebastiaan Kamphuis
// Project 1v5 Code
// Date: 9/1/2026

int Level = 0;
bool MenuShow = true;
bool TotalShow = true;
int random_int_1 = 0;                   // Had some help with this code from Rory
int random_int_2 = 0;
int operation, answer, faulty1, faulty2, x, y, z;  // Varibles for calculations
int options[3];
int correctIndex;   // Extra “cell” for correct answer

// Set Pin Numbers
const int ledgreen = 22;
const int ledred = 21;
const int button34 = 34;
const int button35 = 35;
bool button34pressed = false;
bool button35pressed = false;
int rounds = 0;
bool hoop = false;
int hoopIndex;
int Score = 0;
int TotalScore = 0;
const int hoopA = 27;
const int hoopB = 26;
const int hoopC = 25;
bool hoopAdetect = false;
bool hoopBdetect = false;
bool hoopCdetect = false;
bool timer = false;

hw_timer_t *My_timer = NULL;

void hoopAdetected()  	                    //Answer C
{
  detachInterrupt(digitalPinToInterrupt(hoopA));
  hoop = true;
  hoopIndex = 0;
  hoopAdetect = true;
  // Serial.println("hoopAdetect is HIGH");
}

void hoopBdetected()                        //Answer B
{
  detachInterrupt(digitalPinToInterrupt(hoopB));
  hoop = true;
  hoopIndex = 1;
  //hoopBdetect = true;
  // Serial.println("hoopBdetect is HIGH");
}

void hoopCdetected()                        //Answer A
{
  detachInterrupt(digitalPinToInterrupt(hoopC));
  hoop = true;
  hoopIndex = 2;
  hoopCdetect = true;
  // Serial.println("hoopCdetect is HIGH");
}

void button34pressedISR()
{
  button34pressed = true;
}

void button35pressedISR()
{
  button35pressed = true;
} 

void interruptTimer()
{
  timer = true;
}

enum State                  // Enum is a variable that is defined by the user
{
  Select_Game,
  Start_Round,
  Score_Tally,
  Final_Score
};           

// State Definitions
State currentState = Select_Game;     // Initial State is Select_Game


void setup() 
{
  Serial.begin(115200);
  pinMode(button34, INPUT);
  pinMode(button35, INPUT);
  attachInterrupt(digitalPinToInterrupt(button34), button34pressedISR, RISING);
  attachInterrupt(digitalPinToInterrupt(button35), button35pressedISR, RISING);
  pinMode(hoopA, INPUT);
  pinMode(hoopB, INPUT);
  pinMode(hoopC, INPUT);
  attachInterrupt(digitalPinToInterrupt(hoopA), hoopAdetected, RISING);
  attachInterrupt(digitalPinToInterrupt(hoopB), hoopBdetected, FALLING);
  attachInterrupt(digitalPinToInterrupt(hoopC), hoopCdetected, RISING);
  pinMode(ledgreen, OUTPUT);
  pinMode(ledred, OUTPUT);
  digitalWrite(ledgreen, LOW);
  digitalWrite(ledred, LOW);
  My_timer = timerBegin(10'000);                          
  timerAlarm(My_timer , 50'000, HIGH, 0);                  
  timerAttachInterrupt(My_timer, &interruptTimer);      
  timerStop(My_timer);
}

void loop() 
{
  switch (currentState)
  {
    case Select_Game:
      if(MenuShow == true)
      {
        Serial.println("Select Game");
        Serial.println("      Menu");
        Serial.println("1. Easy Mode");
        Serial.println("2. Medium Mode");
        Serial.println("3. Hard Mode");
        MenuShow = false;
      }
      if (Serial.available() > 0)
        Level = Serial.parseInt();
      if(Level >= 1 && Level <= 3)
      {
        Serial.print("Level: ");     
        Serial.println(Level);          // Serial.parseInt means that we investigate in the input buffer
        currentState = Start_Round;
        MenuShow = true;
      }
      while(Serial.available() == true) // Empty input buffer
        Serial.read();

    break;
    case Start_Round:
      Serial.println("Start Round");
      
      if (Level == 1)
      {
      operation = random(1,3);
      x = random(1, 11);
      y = random(1, 11);
      if(operation == 1)
        {
          answer = x + y;
          Serial.print(x); Serial.print(" + "); Serial.print(y); Serial.print(" = "); Serial.println("?");
        }
      else
        {
          answer = x - y;
          Serial.print(x); Serial.print(" - "); Serial.print(y); Serial.print(" = "); Serial.println("?");
        }
      }
      else if (Level == 2)
      {
        x = random(1, 11);
        y = random(1, 11);
        answer = x * y;
        Serial.print(x); Serial.print(" * "); Serial.print(y); Serial.print(" = "); Serial.println("?");
      }
      else 
      {
        answer = random(1,11);
        y = random(1,11);
        z = answer * y;
        Serial.print(z); Serial.print(" / "); Serial.print(y); Serial.print(" = "); Serial.println("?");
      }
      random_int_1 = random (1,5);  
      random_int_2 = random (1,5);  
      faulty1 = answer - random_int_1;
      faulty2 = answer + random_int_2;

      options[0] = answer;
      options[1] = faulty1;
      options[2] = faulty2;

      // Shuffle (Fisher–Yates)
      for (int i = 2; i > 0; i--) 
        {
          int j = random(0, i + 1);
          int temp = options[i];
          options[i] = options[j];
          options[j] = temp;
        }

      // Find correct answer
      for (int i = 0; i < 3; i++) 
      {
        if (options[i] == answer) 
          {
            correctIndex = i;
            break;
          }
      }

      // Output
 
      Serial.print("Answer A = "); Serial.println(options[2]);
      Serial.print("Answer B = "); Serial.println(options[1]);
      Serial.print("Answer C = "); Serial.println(options[0]);

      timerStart(My_timer);
      currentState = Score_Tally;
    break;
    
    case Score_Tally:
      if (hoop == true)
        {
          if (hoopIndex == correctIndex)          // Answer is correct
            {
              Score = Score + 1;
              digitalWrite(ledgreen, HIGH);       
            }
            else
            { 
              digitalWrite (ledred, HIGH);
            } 
        }
      if (timer == true)
        {
          digitalWrite (ledred, HIGH);
          // timerStart(My_timer);
          // timerStop(My_timer);
          Serial.println("Too Late!!");
        }  
      if((hoop == true) || (timer == true)) 
        {
          Serial.print("Score = "); Serial.println(Score);
          rounds = rounds + 1;
          Serial.print("rounds = "); Serial.println(rounds);
          delay(50);                                                // Testing delays
          hoop = false;
          timer = false;
          timerRestart(My_timer);
          timerStop(My_timer);
          if (rounds == 10)    
            {
              currentState = Final_Score;
            }
        }

      else if (button35pressed == true)                             // next round
            {
              currentState = Start_Round;
              Serial.println("State: Select Round");
              digitalWrite(ledgreen, LOW);
              digitalWrite(ledred, LOW);
              delay(100);
              button35pressed = false;
              attachInterrupt(digitalPinToInterrupt(hoopA), hoopAdetected, RISING);
              attachInterrupt(digitalPinToInterrupt(hoopB), hoopBdetected, FALLING);
              attachInterrupt(digitalPinToInterrupt(hoopC), hoopCdetected, RISING);
              //hoopBdetect = false;
              timerStart(My_timer);
            }
    break;

    case Final_Score:
      if(TotalShow == true)
      {
        Serial.println("State: Final_Score");
        Serial.print("Total Score = "); Serial.println(Score);
        TotalShow = false;
      }
      if (button34pressed == true)                                   // next game
      { 
        currentState = Select_Game;
        Serial.println("State Select Game");
        button34pressed = false;
        Score = 0;
        rounds = 0;
        Level = 0;
        TotalShow = true;
        button35pressed = false;
        Serial.print("Score = "); Serial.println(Score);
        Serial.print("rounds = "); Serial.println(rounds);
      }
    break;   
  }
  delay (100);
}
