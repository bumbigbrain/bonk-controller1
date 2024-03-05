#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PUSH_BUTTON 19
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//define display
// SDA -> GPIO 21
// SCL -> GPIO 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Controller2Address's peer
uint8_t Controller2Address[] = {0x3C, 0x61, 0x05, 0x03, 0xD4, 0xB0};
esp_now_peer_info_t peerInfoController2;

// moleController1Address's peer
uint8_t MoleController1Address[] = {0x3C, 0x61, 0x05, 0x03, 0xC3, 0x78};
esp_now_peer_info_t peerInfoMoleController1;


uint8_t MoleController2Address[] = {0xE8, 0xDB, 0x84, 0x00, 0xDC, 0xF0};
esp_now_peer_info_t peerInfoMoleController2;



typedef struct ControllerMessage {
  int gameState;
  int ScorePlayer1;
  int ScorePlayer2;

} ControllerMessage;


typedef struct MoleMessage {
  int mole;
  //bool isBonked;
} MoleMessage;

MoleMessage MoleInfo;
ControllerMessage ControllerInfo;
int Controller1Score = 0;
int Controller2Score = 0;
bool canSendMole1 = true;
bool canSendMole2 = true;




void displayScore() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.printf("Score Player 1 : %d\n", Controller1Score);
  display.setCursor(0, 20);
  display.printf("Score Player 2 : %d\n", ControllerInfo.ScorePlayer2);
  if (Controller1Score > ControllerInfo.ScorePlayer2) {      
    display.setCursor(0, 30); 
    display.printf("Player 1 WIN!!!");
  } else {
    display.setCursor(0, 40); 
    display.printf("Player 2 WIN!!!");
  }
  display.display();
}


void displayMatched() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.printf("Press Start !!");
  display.display();
}

void displayNotMatch() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.printf("Not match ");
  display.display();
}


void displayCountdown() {
  
  // Countdown for 5 sec
  int msec = 0, trigger = 5;
  clock_t before = clock();

  do {
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Get Ready");
    display.setCursor(11, 20);
    display.printf("%d\n", trigger - msec);
    display.display();
    
    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;
    
  } while (msec < trigger);


}



void displayPlaying(int time_left) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("PLAYING");
  display.setCursor(11,20);
  display.printf("TIME LEFT : %d\n", time_left);
  display.display();
}


bool isController2(const uint8_t *sentmac) {
  uint8_t address[6] = {0x3C, 0x61, 0x05, 0x03, 0xD4, 0xB0};
  for (int i=0; i<6; i++) {
    if (address[i] != sentmac[i]) {
      return false;
    }
  }
  return true;
}


bool isMole1(const uint8_t *sentmac) {
  uint8_t address[6] = {0x3C, 0x61, 0x05, 0x03, 0xC3, 0x78};
  for (int i=0; i<6; i++) {
    if (address[i] != sentmac[i]) {
      return false;
    }
  }
  return true;
}




bool isMole2(const uint8_t *sentmac) {
  uint8_t address[6] = {0xE8, 0xDB, 0x84, 0x00, 0xDC, 0xF0}; 
  for (int i=0; i<6; i++) {
    if (address[i] != sentmac[i]) {
      return false;
    }
  }
  return true;
}




void sendGameStateToController2() {
  esp_err_t result = esp_now_send(Controller2Address, (uint8_t *) &ControllerInfo, sizeof(ControllerInfo));
  if (result == ESP_OK) {
    Serial.println("Send state to Controller2 : SUCCESS");
  } else {
    Serial.println("Sent state to Controller2 : FAILED");
  }
}


void ResetMole() {
  MoleInfo.mole = -1;
   
  esp_err_t result = esp_now_send(MoleController1Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController1 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController1 : FAILED");
  }
  
   
  result = esp_now_send(MoleController2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController2 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController2 : FAILED");
  }
  
}

void UpMole1() {

  int mode = rand();    
  mode = mode % 4; // 0 1 2 3 4

  if (mode == 0) {
    MoleInfo.mole = 0;   
  } 

  if (mode == 1) {
    MoleInfo.mole = 1; 
  } 

  if (mode == 2) {
    MoleInfo.mole = 2;
  }

  if (mode == 3) {
    MoleInfo.mole = 3;
  }
   
  esp_err_t result = esp_now_send(MoleController1Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController1 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController1 : FAILED");
  }
  
}


void UpMole2() {


  int mode = rand();    
  mode = mode % 4; // 0 1 2 3 4

  if (mode == 0) {
    MoleInfo.mole = 0;   
  } 

  if (mode == 1) {
    MoleInfo.mole = 1; 
  } 

  if (mode == 2) {
    MoleInfo.mole = 2;
  }

  if (mode == 3) {
    MoleInfo.mole = 3;
  }
   
  esp_err_t result = esp_now_send(MoleController2Address, (uint8_t *) &MoleInfo, sizeof(MoleInfo)); 
  if (result == ESP_OK) {
    Serial.println("Send state to MoleController2 : SUCCESS");
  } else {
    Serial.println("Sent state to MoleController2 : FAILED");
  }
  
}

void UpMoleMaster() {
  int rand_mole = rand();
  rand_mole = rand_mole % 2;
  if (rand_mole == 0 && canSendMole1) {
    UpMole1();
    canSendMole1 = false;
  } else if (rand_mole == 1 && canSendMole2) {
    UpMole2(); 
    canSendMole2 = false;
  }
  delay(50);
}

int rand(void);


void Playing() { 
  //UpMole1();
 
  // Playing for 30 sec
  int msec = 0, trigger = 30;
  clock_t before = clock();

  do {

    displayPlaying(trigger - msec);
    UpMoleMaster(); 

    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;
    
  } while (msec < trigger);

   
}

bool Debounce() {
  if (digitalRead(PUSH_BUTTON) == LOW) {
    delay(50);
    if (digitalRead(PUSH_BUTTON) == HIGH) {
      return true;
    }
  }
  return false;
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println("SEND TO CONTROLLER2 : ");
  delay(300);
}


void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {

  if (isController2(mac)) {
    memcpy(&ControllerInfo, incomingData, sizeof(ControllerInfo));
    
  } else if (isMole1(mac) && ControllerInfo.gameState == 3) {
    canSendMole1 = true;
    Controller1Score++;
  } else if (isMole2(mac) && ControllerInfo.gameState == 3) {
    canSendMole2 = true;
    Controller1Score++;
  }
  
}

void setup() {
  ControllerInfo.gameState = 0;
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  //setup BUTTON
  pinMode(PUSH_BUTTON, INPUT_PULLUP);

  //setup DISPLAY
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Do OnDataSent when send
  esp_now_register_send_cb(OnDataSent);

  //Do OnDataRecv when recieve
  esp_now_register_recv_cb(OnDataRecv);

  //register peer Controller2
  memcpy(peerInfoController2.peer_addr, Controller2Address, 6);
  peerInfoController2.channel = 0;
  peerInfoController2.encrypt = false;
  esp_now_add_peer(&peerInfoController2);
  if (esp_now_add_peer(&peerInfoController2) != ESP_OK) {
    Serial.println("Failed to add peer Controller2");
  }


  // register peer MoleController1
  memcpy(peerInfoMoleController1.peer_addr, MoleController1Address, 6);
  peerInfoMoleController1.channel = 0;
  peerInfoMoleController1.encrypt = false;
  esp_now_add_peer(&peerInfoMoleController1);
  if (esp_now_add_peer(&peerInfoMoleController1) != ESP_OK) {
    Serial.println("Failed to add peer MoleController1");
  }

  // register peer MoleController2
  memcpy(peerInfoMoleController2.peer_addr, MoleController2Address, 6);
  peerInfoMoleController2.channel = 0;
  peerInfoMoleController2.encrypt = false;
  esp_now_add_peer(&peerInfoMoleController2);
  if (esp_now_add_peer(&peerInfoMoleController2) != ESP_OK) {
    Serial.println("Failed to add peer MoleController2");
  }
  

  
  // request matching to Controller2
  ControllerInfo.gameState = 1;
  esp_err_t resultMatchingController2 = esp_now_send(Controller2Address, (uint8_t *) &ControllerInfo, sizeof(ControllerInfo));
  if (resultMatchingController2 == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending thte data");
  }
  
  

}

void loop() {





  // if (digitalRead(PUSH_BUTTON) == LOW) { // }
  if (ControllerInfo.gameState == 0) { // No matching
    displayNotMatch();
  }
  if (ControllerInfo.gameState == 1) { // Entry
    Controller1Score = 0;
    displayMatched();
    if (Debounce()) {
      ControllerInfo.gameState = 2;
      
      sendGameStateToController2();
      
    }  
  }
  
  if (ControllerInfo.gameState == 2) { // Countdown
    displayCountdown();        
    ControllerInfo.gameState = 3; // Change state to Playing
    sendGameStateToController2(); 
    
  }
  
  if (ControllerInfo.gameState == 3) { // Playing
    Playing();
    ControllerInfo.gameState = 4;
    sendGameStateToController2();
    
  }
  
  if (ControllerInfo.gameState == 4) { // Exchange Score
    ControllerInfo.ScorePlayer1 = Controller1Score;
    if (ControllerInfo.ScorePlayer2 > 0) {
      ControllerInfo.gameState = 5;
      sendGameStateToController2();
      
    } 
  }
  
  if (ControllerInfo.gameState == 5) {
    displayScore();
    if (Debounce()) {
      //ControllerInfo.gameState = 1; 
      //Add
      ControllerInfo.gameState = 6;
      sendGameStateToController2();
    }
    
    
  }
  
  if (ControllerInfo.gameState == 6) {
    ResetMole();  
    ControllerInfo.gameState = 1;
  }
  
  

  
}

