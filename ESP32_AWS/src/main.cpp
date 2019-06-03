#include <Arduino.h>
#include <AWS_IOT.h>
#include <WiFi.h>
#include "Wire.h"
#include "SSD1306.h"

#include "freertos/FreeRTOS.h"

#include "images.h"

AWS_IOT hornbill;

#define LEDPIN 2

#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15

SSD1306 display(OLED_I2C_ADDR, OLED_SDA, OLED_SCL);

char WIFI_SSID[] = "WLAN-774290";
char WIFI_PASSWORD[] = "1694523466957402";
char HOST_ADDRESS[] = "afk4iec4fycqq-ats.iot.us-east-2.amazonaws.com"; //XXOO.eu-central-1.amazonaws.com
char CLIENT_ID[] = "Esp32Loggers";                                     //"Client Name";
char TOPIC_NAME[] = "$aws/things/Esp32Loggers/io/esp32-1";             //"Topic Name"

int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];

unsigned long lastPublishTime;
unsigned int triggerCounter;

void mySubCallBackHandler(char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload, payLoad, payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    pinMode(OLED_RESET, OUTPUT);
    digitalWrite(OLED_RESET, LOW);
    delay(50);
    digitalWrite(OLED_RESET, HIGH);

    display.init();
    delay(100);
    //display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    display.drawXbm(0, 0, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);

    display.display();
    delay(3000);

    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(WIFI_SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        // wait 5 seconds for connection:
        delay(5000);
    }

    Serial.println("Connected to wifi");

    if (hornbill.connect(HOST_ADDRESS, CLIENT_ID) == 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);

        if (0 == hornbill.subscribe(TOPIC_NAME, mySubCallBackHandler))
        {
            Serial.println("Subscribe Successfull");
        }
        else
        {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while (1)
                ;
        }
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while (1)
            ;
    }

    delay(2000);
}

void loop()
{

    if (msgReceived == 1)
    {
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
    }

    if (Serial.available())
    {
        char i = Serial.read(); //clear out buffer
        sprintf(payload, "Hello from Hornbill Esp32 : %lu ", (millis() - lastPublishTime));
        for (int j = 0; j < 5; j++) //will try to publish for five times
        {
            short errCounter = 0;
            if (hornbill.publish(TOPIC_NAME, payload) == 0)
            {
                Serial.print("Publish Message:");
                Serial.println(payload);
                lastPublishTime = millis();
                break;
            }
            else
            {
                Serial.printf("Publish failed %d times. Retrying after 1 second...\n", ++errCounter);
                vTaskDelay(1000 / portTICK_RATE_MS);
            }
        }
    }
}