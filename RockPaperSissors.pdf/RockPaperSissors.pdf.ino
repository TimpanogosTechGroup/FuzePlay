#include <FastLED.h>
#include <Bounce2.h>
enum state {CONN, SUCCESS, WARNING, PLAY};
state status = CONN;
enum RESULT {TIE, WIN, LOSE, NA};
enum BUTTON {TRIANGLE, CIRCLE, SQUARE, NONE};
// Initialize LED's
#define NUMBER_OF_LEDS 6
#define DATA_PIN 16 //Serial Data output (don't change this)
#define CLOCK_PIN 15 //Serial Clock output (don't change this)
CRGB leds[NUMBER_OF_LEDS];
//Buttons, Don't change these
//This sets which pins in your Fuze board are connected to each button
//Since these are constants (they can't be changed), they are written in UPPER CASE
#define BUTTON_ONE_PIN 7
#define BUTTON_TWO_PIN 5
#define BUTTON_THREE_PIN 9
#define BUTTON_TRIANGLE_PIN 10
#define BUTTON_CIRCLE_PIN 2
#define BUTTON_SQUARE_PIN 8
//Buzzer, don't change this
//This sets which pin on your Fuze board are connected to a buzzer
//Since these are constants (they can't be changed), they are written in UPPER CASE
#define BUZZER_PIN 3
//This enables "debouncing" on the buttons
//https://github.com/thomasfredericks/Bounce2
Bounce triangleDebouncer = Bounce();
Bounce circleDebouncer = Bounce();
Bounce squareDebouncer = Bounce();
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
  for (int i = 0; i < NUMBER_OF_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}


void loop() {
  // //Read the latest button states
  triangleDebouncer.update();
  squareDebouncer.update();
  circleDebouncer.update();

  clearAllLEDs();
  if (status == CONN) {
    if (connect())
      status = PLAY;
  }
  for (int i = 0; i < 3; i++) {
    playGame();
  }
  status = CONN;
}


bool connect() {
  double count = 0;
  bool isConnected = false;
  bool isDown = false;
  char str[1];
  clearAllLEDs();
  // char recievData = 'N';
  while (!isConnected) {
    triangleDebouncer.update();
    squareDebouncer.update();
    circleDebouncer.update();
    leds[0].setRGB(0, 0, 100 * abs(sin(count + (3.141592659 / 6))));
    leds[1].setRGB(0, 0, 100 * abs(sin(count + (3.141592659 / 3))));
    leds[2].setRGB(0, 0, 100 * abs(sin(count + (3.141592659 / 2))));
    leds[3].setRGB(0, 0, 100 * abs(sin(count + (2 * 3.141592659 / 3))));
    leds[4].setRGB(0, 0, 100 * abs(sin(count + (5 * 3.141592659 / 6))));
    leds[5].setRGB(0, 0, 100 * abs(sin(count + (3.141592659))));
    count += 0.005;
    FastLED.show();
    if (triangleDebouncer.fell() ||
        squareDebouncer.fell() ||
        circleDebouncer.fell()) {
      Serial.println("Fell");
      Serial1.write('1');
      isDown = true;
    }
    if (triangleDebouncer.rose()) {
      isDown = false;
    }
    int i = 0;
    if (Serial1.available()) {
      int i = 0;
      int recievData;
      Serial.println("Recieving Data");
      while (Serial1.available() && i < 1) {
        str[0] = Serial1.read();
        i++;
      }

      if (i > 0 && isDown && str[0] == '1') {
        isConnected = true;
        Serial.println("Recieved connecting... ");
        Serial.println(str[0]);
        Serial1.write(str, 1);
        i = 0;
      }
    }
  }
  connectSuccess();
  return true;
}




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


void setAllLEDs(int r, int g, int b) {
  for (int i = 0; i < NUMBER_OF_LEDS; i++) {
    leds[i].setRGB(r, g, b);
  }
  FastLED.show();
}

void flashAllLEDs(int r, int g, int b, double times = 15) {
  for (double T = 0; T < times; T += 0.01) {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
      leds[i].setRGB(r * abs(sin(T)), g * abs(sin(T)), b * abs(sin(T)));
    }
    FastLED.show();
  }
  clearAllLEDs();
}








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




















