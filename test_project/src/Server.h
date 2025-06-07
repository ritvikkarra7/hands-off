#ifndef SERVER_H
#define SERVER_H

void setupWebServer();
void startWebServices();
void broadcastFrequency(float frequency);

#define SEL GPIO_NUM_16 // GPIO for the multiplexer control 

#endif
