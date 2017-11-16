/* Wireworld Brain
 * 
 * This is the implementation code for an art collaboration with Emory Arts
 * Underground and HackATL.
 * 
 * Benjamin Bolte, 2017
 */

#include "FastLED.h"

#define NUM_LEDS 110
#define MAX_CONNECTIONS (NUM_LEDS + 40)
#define MAX_QUEUE 20
#define PIN 6
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 16
#define PROP_TIME 30

const CRGB HEAD_COLOR = CRGB::Amethyst,
           TAIL_COLOR = CRGB::Indigo,
           OFF_COLOR = CRGB::Black;

CRGB leds[NUM_LEDS];

// Defines the three connection states.
enum State { OFF, HEAD, TAIL };

struct LED;

// Defines a connection.
typedef struct Connection {
  struct LED *onto;
  Connection *next;
} Connection;

typedef struct LED {
  CRGB *led;
  State state;
  Connection *c;
} LED;

typedef struct Queue {
  int n;
  LED *leds[MAX_QUEUE];
} Queue;

// Parameters controlling the firing threshold.
#define THRESHOLD_DELTA 0.01
#define THRESHOLD_MIN 60
#define THRESHOLD_MAX 99
#define THRESHOLD_RANDOM_MIN 0
#define THRESHOLD_RANDOM_MAX 100
double threshold = THRESHOLD_MIN;
bool incFlag = true;

LED conns[NUM_LEDS];
Queue heads, newHeads, tails;
Connection allConns[MAX_CONNECTIONS];
int usedConnections = 0;

// Some helper functions for dealing with queues.
void resetQueue(Queue *q) { q->n = 0; }
bool addToQueue(Queue *q, LED *c) {
  if (q->n < MAX_QUEUE) {
    q->leds[q->n++] = c;
    return true;
  } else {
    return false;
  }
}

// Some helper functions for dealing with LED transitions.
void flipOn(LED *l) {
  if (addToQueue(&heads, l)) {
    l->state = HEAD;
    *(l->led) = HEAD_COLOR;
  }
}

void makeHead(LED *l) {
  if (l->state == HEAD) return;
  if (addToQueue(&newHeads, l)) {
    l->state = HEAD;
    *(l->led) = HEAD_COLOR;
  }
}

void makeTail(LED *l) {
  if (addToQueue(&tails, l)) {
    l->state = TAIL;
    *(l->led) = TAIL_COLOR;
  }
}

void makeOff(LED *l) {
  l->state = OFF;
  *(l->led) = OFF_COLOR;
}

// Some helper functions for dealing with connections.
Connection* getConnection() {
  if (usedConnections >= MAX_CONNECTIONS) {
    return NULL;
  } else {
    return &allConns[usedConnections++];
  }
}

int numConns = 0;

void addConnection(int from, int to) {  
  Connection *c = getConnection();
  c->next = conns[from].c;
  c->onto = &conns[to];
  conns[from].c = c;
}

void addConnectionRow(int from, int to) {
  if (from < to) {
    for (int i = from + 1; i <= to; i++) {
      addConnection(i - 1, i);
    }
  } else {
    for (int i = from; i > to; i--) {
      addConnection(i, i - 1);
    }
  }
}

void propagateHead(LED *l) {
  Connection *c = l->c;
  while (c != NULL) {
    if (random(0, 30) > 0) {
      makeHead(c->onto);
    }
    c = c->next;
  }
}


void setup() {
  FastLED.addLeds<CHIPSET, PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Initializes the "heads" and "tails" queues.
  resetQueue(&heads);
  resetQueue(&tails);

  // Initializes the LEDs.
  for (int i = 0; i < NUM_LEDS; i++) {
    conns[i].led = &leds[i];
    conns[i].state = OFF;
    conns[i].c = NULL;
  }

  // Initializes some connections.
  addConnectionRow(16, 0);
  addConnectionRow(17, 23);
  addConnectionRow(37, 24);
  addConnectionRow(57, 45);
  addConnectionRow(57, 68);
  addConnectionRow(82, 69);
  addConnectionRow(98, 83);
  addConnectionRow(99, 109);
  addConnectionRow(37, 44);

  addConnection(83, 82);
  addConnection(37, 98);
  addConnection(17, 82);

  addConnection(0, 82);
  addConnection(23, 16);
  addConnection(24, 17);
  addConnection(45, 37);
  addConnection(32, 99);
  addConnection(69, 57);

  // Resets all the queues.
  resetQueue(&tails);
  resetQueue(&heads);
  resetQueue(&newHeads);
}

void loop() {  
  for (int i = 0; i < tails.n; i++) {
    makeOff(tails.leds[i]);
  }
  resetQueue(&tails);
  resetQueue(&newHeads);
  
  for (int i = 0; i < heads.n; i++) {
    makeTail(heads.leds[i]);
    propagateHead(heads.leds[i]);
  }

  if (random(THRESHOLD_RANDOM_MIN, THRESHOLD_RANDOM_MAX) > (int) threshold) {
    makeHead(&conns[random(0, NUM_LEDS)]); // Fire one randomly.
  }

  if (incFlag) {
    threshold += THRESHOLD_DELTA;
    if (threshold > THRESHOLD_MAX) {
      incFlag = false;
    }
  } else {
    threshold -= THRESHOLD_DELTA;
    if (threshold < THRESHOLD_MIN) {
      incFlag = true;
    }
  }
  
  resetQueue(&heads);
  for (int i = 0; i < newHeads.n; i++) {
    addToQueue(&heads, newHeads.leds[i]);
  }
  
  FastLED.show();
  FastLED.delay(PROP_TIME);
}

