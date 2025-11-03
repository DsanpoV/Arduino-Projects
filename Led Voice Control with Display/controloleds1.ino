#include <Adafruit_NeoPixel.h>
#include <Keypad.h>

#define PIN_LED 6
#define NUM_LEDS 8  // ajuste se sua fita tiver mais ou menos LEDs

Adafruit_NeoPixel strip(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'R','G','B','Y'},
  {'O','P','C','M'},
  {'1','2','3','4'},
  {'5','6','7','8'}
};
byte rowPins[ROWS] = {2,3,4,5};
byte colPins[COLS] = {7,8,9,10};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  strip.begin();
  strip.show(); // Inicializa todos os LEDs desligados
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    switch(key) {
      case 'R': setColor(255,0,0); break;      // vermelho
      case 'G': setColor(0,255,0); break;      // verde
      case 'B': setColor(0,0,255); break;      // azul
      case 'Y': setColor(255,255,0); break;    // amarelo
      case 'O': setColor(255,165,0); break;    // laranja
      case 'P': setColor(128,0,128); break;    // roxo
      case 'C': setColor(0,255,255); break;    // ciano
      case 'M': setColor(255,0,255); break;    // magenta
    }
  }
}

void setColor(int r, int g, int b) {
  for(int i=0;i<NUM_LEDS;i++){
    strip.setPixelColor(i, strip.Color(r,g,b));
  }
  strip.show();
}
