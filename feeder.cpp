#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <DS3231.h>
#include <EEPROM.h>

#define BUFFER_SIZE 32

//COMMANDS
#define SET_TIME 1
#define FEED_TIME 2
#define PORTION 3
#define PUSH_PORTION 4
#define REFRESH 5

//MOTORS
#define PWM 9
#define M1 8
#define M2 7

//BLUETOOTH
#define BRX 5
#define BTX 6

//Tasks
void TaskListenBt(void *pvParameters);
void TaskProcessingCommands(void *pvParameters);

//Variables
QueueHandle_t QueueCommands;
SoftwareSerial bt_serial(BRX, BTX);
DS3231 clock;

//function definition

void setup()
{
    Serial.begin(9600);
    bt_serial.begin(9600);
    clock.begin();

    while (!Serial)
    {
        ;
    }

    QueueCommands = xQueueCreate(1, sizeof(char*));

    xTaskCreate(
        TaskListenBt,
        (const portCHAR *)"ListenBt",
        128,
        NULL,
        1,
        NULL);

    xTaskCreate(
        TaskProcessingCommands,
        (const portCHAR *)"ProccessingCommand",
        128,
        NULL,
        2,
        NULL
    );
}

void loop() {}

void TaskListenBt(void *pvParameters)
{
    (void)pvParameters;
    portBASE_TYPE xStatus;
    char incomingByte = '\0';
    char buffer[BUFFER_SIZE] = "";

    for (;;)
    {
        if (bt_serial.available())
        {
            incomingByte = bt_serial.read();
            if (incomingByte == '#')
            {
                unsigned long last_time = millis();
                unsigned long current_time = millis();
                while (incomingByte != '$' && current_time - 2000 < last_time)
                {
                    if (bt_serial.available())
                    {
                        incomingByte = bt_serial.read();
                        if (incomingByte != '$')
                        {
                            size_t len = strlen(buffer);
                            if (len + 1 == BUFFER_SIZE) {
                                break;
                            }
                            buffer[len] = incomingByte;
                            buffer[len + 1] = '\0';
                            last_time = millis();
                        }
                    }
                    current_time = millis();
                }
                if (incomingByte == '$') {
                    char *temp = buffer;
                    xStatus = xQueueSend(QueueCommands, &temp, 0);
                    if (xStatus != pdPASS) {
                        Serial.println("Can't send to the queue");
                    }
                    buffer[0] = '\0';
                } else {
                    buffer[0] = '\0';
                }
            }
        }
    }
}

void TaskProcessingCommands (void *pvParameters) {
    (void)pvParameters;
    portBASE_TYPE xStatus;
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;
    char *recivedCommand;

    for (;;) {
        if (QueueCommands != 0) {
            xStatus = xQueueReceive(QueueCommands, &recivedCommand, xTicksToWait);
            if (xStatus == pdPASS) {
                Serial.println(recivedCommand);
                char *ptr = strtok(recivedCommand, "%");
                switch (atoi(ptr))
                {
                    case (SET_TIME):
                        {
                            uint16_t tp [6] = {0};
                            for (int i = 0; i < 6; i++) {
                                if (ptr != NULL) {
                                    ptr = strtok(NULL, "%");
                                    tp[i] = atoi(ptr);
                                }
                            }
                            for (int i = 0; i < 6; i++) {
                                Serial.println(tp[i]);
                            }
                            clock.setDateTime(tp[0], tp[1], tp[2], tp[3], tp[4], tp[5]);
                            break;
                        }

                    case (FEED_TIME):
                        {
                            break;
                        }

                    case (PORTION):
                        {
                            break;
                        }

                    case (PUSH_PORTION):
                        {
                            break;
                        }

                    case (REFRESH):
                        {
                            break;
                        }
                        
                    default:
                        {
                            break;
                        }
                }
            }
        }
    }
}