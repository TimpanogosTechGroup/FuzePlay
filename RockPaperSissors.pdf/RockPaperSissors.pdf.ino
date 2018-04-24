/**
 *  File Rock Paper Scissors game 
 * 
 *  Authors: Isaac Draper, Ben Brenkman
 *  Email:   fuzeplay@timpanogos-tech.com
 * 
 *  Purpose: This is the rock paper scissors implementation on the FuzePlay zubi board
 *  You will need to boards and three wires to connect the two boards together
 *  One wire to connect both grounds together, Another to connect one boards TX pin to the other boards RX pin,
 *  And another to connect the remaining RX pin to the other boards TX pin.
 * 
 **/

#include <FastLED.h>
#include <Bounce2.h>
enum state {CONN, SUCCESS, WARNING, PLAY};
state status = CONN;
enum RESULT {TIE, WIN, LOSE, NA};
enum BUTTON {TRIANGLE, CIRCLE, SQUARE, NONE};
//---------------------Initialize LED's, Don't change these -------------------------
#define NUMBER_OF_LEDS 6
#define DATA_PIN 16 //Serial Data output (don't change this)
#define CLOCK_PIN 15 //Serial Clock output (don't change this)
CRGB leds[NUMBER_OF_LEDS];
//---------------------Buttons, Don't change these-----------------------------------
//This sets which pins in your Fuze board are connected to each button
//Since these are constants (they can't be changed), they are written in UPPER CASE
#define BUTTON_ONE_PIN 7
#define BUTTON_TWO_PIN 5
#define BUTTON_THREE_PIN 9
#define BUTTON_TRIANGLE_PIN 10
#define BUTTON_CIRCLE_PIN 2
#define BUTTON_SQUARE_PIN 8
//-----------------------------------------------------------------------------------
//Buzzer, don't change this
//This sets which pin on your Fuze board are connected to a buzzer
//Since these are constants (they can't be changed), they are written in UPPER CASE
#define BUZZER_PIN 3
//This enables "debouncing" on the buttons
//https://github.com/thomasfredericks/Bounce2
Bounce triangleDebouncer = Bounce();
Bounce circleDebouncer = Bounce();
Bounce squareDebouncer = Bounce();


//******************************************************************************
// User modifiable values
//******************************************************************************
int NUMBER_OF_PLAYS = 3;
int FLASH_CYCLES = 20;
double INTERVAL_INCREASE = 0.005;


// This is the setup function only called once during the program run. 
// It is called before the loop is called
// Put initialization code here
void setup() {
  // put your setup code here, to run once:
  // Initialize FastLED library with our board led's
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUMBER_OF_LEDS);
  clearAllLEDs();
  Serial.begin(9600);
  Serial1.begin(9600);
  //This tells the Fuze board that these pins are buttons, don't change this part of the code
  pinMode(BUTTON_TRIANGLE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_CIRCLE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SQUARE_PIN, INPUT_PULLUP);
  //Initialize the button debounce, don't change this part of the code
#define DEBOUNCE_INTERVAL 40
  triangleDebouncer.attach(BUTTON_TRIANGLE_PIN);
  triangleDebouncer.interval(DEBOUNCE_INTERVAL);
  squareDebouncer.attach(BUTTON_SQUARE_PIN);
  squareDebouncer.interval(DEBOUNCE_INTERVAL);
  circleDebouncer.attach(BUTTON_CIRCLE_PIN);
  circleDebouncer.interval(DEBOUNCE_INTERVAL);
}

//Used to clear all the LEDs after a light sequence is complete
void clearAllLEDs() {
    // Goes through every led and then sets them to the color black
  for (int i = 0; i < NUMBER_OF_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show(); // Updates the changes so we can see them
}


// This is the loop function.
// This function is called over and over agian until the end of the program
void loop() {
  // //Read the latest button states
  triangleDebouncer.update();
  squareDebouncer.update();
  circleDebouncer.update();

  clearAllLEDs(); // Clear LED's

  // This section of code keeps track of the status of the game
  // There are two statuses
  // 1. Connecting, this is the waiting peroid, waits until both players are ready
  if (status == CONN) {
    if (connect())
      status = PLAY;
  }
  for (int i = 0; i < NUMBER_OF_PLAYS; i++) {
    playGame(); // This plays the game as many times as specified.
  }
  status = CONN;
}


// This is the connect function. Waits until both players are ready.
bool connect() {
  double count = 0;
  bool isConnected = false;
  bool isDown = false;
  char str[1];
  clearAllLEDs();
  // char recievData = 'N';
  while (!isConnected) {
      // Update the buttons
    triangleDebouncer.update();
    squareDebouncer.update();
    circleDebouncer.update();
    // Makes the cool circling blue ring of LED's
    leds[0].setRGB(0, 0, 100 * abs(sin(count + (3.141592659 / 6))));
    leds[1].setRGB(0, 0, 100 * abs(sin(count + (3.141592659 / 3))));
    leds[2].setRGB(0, 0, 100 * abs(sin(count + (3.141592659 / 2))));
    leds[3].setRGB(0, 0, 100 * abs(sin(count + (2 * 3.141592659 / 3))));
    leds[4].setRGB(0, 0, 100 * abs(sin(count + (5 * 3.141592659 / 6))));
    leds[5].setRGB(0, 0, 100 * abs(sin(count + (3.141592659))));
    count += 0.005;
    FastLED.show();
    // Check to see if the triangle button was pressed
    if (triangleDebouncer.fell() ||
        squareDebouncer.fell() ||
        circleDebouncer.fell()) {
      Serial.println("Fell");
      Serial1.write('1'); // Send the number '1' to the other board.
      isDown = true;
    }
    if (triangleDebouncer.rose()) {
      isDown = false;
    }
    int i = 0;
    // If the other board is trying to send something back see what message it is trying to send
    if (Serial1.available()) {
      int i = 0;
      int recievData;
      Serial.println("Recieving Data");
      while (Serial1.available() && i < 1) {
        str[0] = Serial1.read();
        i++;
      }

      if (i > 0 && isDown && str[0] == '1') { // If this board recieved the message down and the triangle button is pressed, send '1' to the other board
        isConnected = true;
        Serial.println("Recieved connecting... ");
        Serial.println(str[0]);
        Serial1.write(str, 1);
        i = 0;
      }
    }
  }
  connectSuccess(); // Once both players are ready flash green
  return true;
}



// Flashes the LED's green to indicate both players are ready
void connectSuccess() {
  clearAllLEDs();
  for (double i = 0; i < 15; i += 0.01) {
    leds[0].setRGB(0, 100 * abs(sin(i)), 0);
    leds[1].setRGB(0, 100 * abs(sin(i)), 0);
    leds[2].setRGB(0, 100 * abs(sin(i)), 0);
    leds[3].setRGB(0, 100 * abs(sin(i)), 0);
    leds[4].setRGB(0, 100 * abs(sin(i)), 0);
    leds[5].setRGB(0, 100 * abs(sin(i)), 0);
    FastLED.show();
  }
  clearAllLEDs();
}

// Sets all the LED's a specific color
void setAllLEDs(int r, int g, int b) {
  for (int i = 0; i < NUMBER_OF_LEDS; i++) {
    leds[i].setRGB(r, g, b);
  }
  FastLED.show();
}

// Flash the LED's a specific color
void flashAllLEDs(int r, int g, int b, double times = FLASH_CYCLES) {
  for (double T = 0; T < times; T += INTERVAL_INCREASE) {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
      leds[i].setRGB(r * abs(sin(T)), g * abs(sin(T)), b * abs(sin(T)));
    }
    FastLED.show();
  }
  clearAllLEDs();
}


// Plays the game
void playGame() {
  bool inGame = true;
  bool isDown = false;
  BUTTON buttonDown = NONE;
  char str[1];
  int i = 0;

  // red, yellow, green
  clearAllLEDs();
  setAllLEDs(100, 0, 0);
  delay(1000);
  setAllLEDs(100, 100, 0);
  delay(1000);
  setAllLEDs(0, 100, 0);

  while (inGame) {
    triangleDebouncer.update();
    squareDebouncer.update();
    circleDebouncer.update();

    // send button info to other device
    if (triangleDebouncer.fell()) {
      Serial1.write('1');
      isDown = true;
      buttonDown = TRIANGLE;
    }
    else if (squareDebouncer.fell()) {
      Serial1.write('2');
      isDown = true;
      buttonDown = SQUARE;
    }
    else if (circleDebouncer.fell()) {
      Serial1.write('3');
      isDown = true;
      buttonDown = CIRCLE;
    }

    // send zero if button is released
    if (triangleDebouncer.rose() || squareDebouncer.rose() || circleDebouncer.rose()) {
      isDown = false;
      itoa(0, str, 10); //Turn value into a character array
      Serial1.write(str, 1);
      isDown = false;
      buttonDown = NONE;
    }

    // receiving data
    if (Serial1.available()) {
      i = 0;
      int recievData;
      while (Serial1.available() && i < 1) {
        str[0] = Serial1.read();
        if (str[0] == '1' ||
            str[0] == '2' ||
            str[0] == '3') i++;
      }
      Serial.println("Recieving Data:");
      Serial.println(str[0]);
      Serial.println("");
      if (str[0] == '0') i = 0;
    }

    // both users have pressed a button
    if (i > 0 && isDown) {
      // 1 triangle, 2 square, 3

      Serial.println("Both pressed");
      Serial.println(str[0]);
      Serial.println("Button:");
      Serial.println(buttonDown);

      if (str[0] == '1') {
        if (buttonDown == TRIANGLE) {
          showResult(TIE, buttonDown);
        }
        else if (buttonDown == SQUARE) {
          showResult(LOSE, buttonDown);
        }
        else if (buttonDown == CIRCLE) {
          showResult(WIN, buttonDown);
        }
        i = 0;
        isDown = false;
        return;
      }
      else if (str[0] == '2') {
        if (buttonDown == TRIANGLE) {
          showResult(WIN, buttonDown);
        }
        else if (buttonDown == SQUARE) {
          showResult(TIE, buttonDown);
        }
        else if (buttonDown == CIRCLE) {
          showResult(LOSE, buttonDown);
        }
        i = 0;
        isDown = false;
        return;
      }
      else if (str[0] == '3') {
        if (buttonDown == TRIANGLE) {
          showResult(LOSE, buttonDown);
        }
        else if (buttonDown == SQUARE) {
          showResult(WIN, buttonDown);
        }
        else if (buttonDown == CIRCLE) {
          showResult(TIE, buttonDown);
        }
        i = 0;
        isDown = false;
        return;
      }
    }
  }
}




// Displays the result of the game
void showResult(RESULT re, BUTTON btn) {
  char str[1];
  Serial.println("BTN:");
  Serial.println(btn);
  Serial.println("");

  switch (btn) {
    case TRIANGLE:
      //itoa(1, str, 10); //Turn value into a character array
      Serial1.write('1');
      Serial.println("Sending ");
      Serial.println(str[0]);
      Serial.println("");
      break;
    case SQUARE:
      //itoa(2, str, 10); //Turn value into a character array
      Serial1.write('2');
      Serial.println("Sending ");
      Serial.println(str[0]);
      Serial.println("");
      break;
    case CIRCLE:
      //itoa(3, str, 10); //Turn value into a character array
      Serial1.write('3');
      Serial.println("Sending ");
      Serial.println(str[0]);
      Serial.println("");
      break;
    default:
      Serial.println("Error Sending");
  }
  switch (re) {
    case WIN:
      Serial.println("Won");
      flashAllLEDs(0, 100, 0);
      break;
    case LOSE:
      Serial.println("LOST");
      flashAllLEDs(100, 0, 0);
      break;
    case TIE:
      Serial.println("TIE");
      flashAllLEDs(100, 100, 0);
      break;
    default:
      flashAllLEDs(100, 100, 100);
      break;
  }
  delay(1000);
  clearAllLEDs();
}
