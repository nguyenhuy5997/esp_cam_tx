/*
// CAPTURE AND SEND IMAGE OVER ESP NOW
// Code by: Tal Ofer
// https://github.com/talofer99/ESP32CAM-Capture-and-send-image-over-esp-now
//
// This is the screen portion of the code.
//
// for more information
// https://www.youtube.com/watch?v=0s4Bm9Ar42U
*/



#include <esp_now.h>
#include <WiFi.h>
#define CHANNEL 1

#include <SPIFFS.h>
#include <FS.h>

int currentTransmitCurrentPosition = 0;
int currentTransmitTotalPackages = 0;
byte showImage = 0;

void configDeviceAP();
void InitESPNow();
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

void setup() {
  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");

  if (!SPIFFS.begin())
  {
    Serial.println(F("ERROR: File System Mount Failed!"));
  }
  else
  {
    Serial.println(F("success init spifss"));
  }



  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
}


void loop() {
  // if show image flag
  if (showImage)
  {
    showImage = 0;
    unsigned long start = millis();
  }
}



// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {

  switch (*data++)
  {
    case 0x01:
      Serial.println("Start of new file transmit");
      currentTransmitCurrentPosition = 0;
      currentTransmitTotalPackages = (*data++) << 8 | *data;
      Serial.println("currentTransmitTotalPackages = " + String(currentTransmitTotalPackages));
      SPIFFS.remove("/moon.jpg");
      break;
    case 0x02:
      //Serial.println("chunk of file transmit");
      currentTransmitCurrentPosition = (*data++) << 8 | *data++;
      //Serial.println("chunk NUMBER = " + String(currentTransmitCurrentPosition));
      File file = SPIFFS.open("/moon.jpg",FILE_APPEND);
      if (!file)
        Serial.println("Error opening file ...");
        
      for (int i=0; i < (data_len-3); i++)
      {
        //byte dat = *data++;
        //Serial.println(dat);
        file.write(*data++);
      }
      file.close();

      if (currentTransmitCurrentPosition == currentTransmitTotalPackages)
      {
        showImage = 1;
        Serial.println("done file transfer");
        File file = SPIFFS.open("/moon.jpg");
        Serial.println(file.size());
        while(file.available())
        {
          Serial.write(file.read());
        }
        file.close();
      }
      break;
  } //end case 
} //end 

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}