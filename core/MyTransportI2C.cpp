#include "MyTransport.h"
#include "MyConfig.h"
#include <Wire.h>

#define DEFAULT_ADDRESS ((uint8_t) 127)

typedef struct _transportQueuedMessage
{
	uint8_t m_len;                        // Length of the data
	uint8_t m_data[MAX_MESSAGE_LENGTH];   // The raw data
} transportQueuedMessage;

static transportQueuedMessage transportRxQueue[3];
static uint8_t transportRxQueueLength = 0;
static uint8_t transportRxQueuePosition = 0;

uint8_t I2C_readMessage(void* data) {
	uint8_t* current = (uint8_t*) data;
	uint8_t len = 0;
	while (Wire.available()) {
		current[len++] = Wire.read();
	}
	
	return len;
}

static void transportRxCallback(int howMany) {
  if (transportRxQueueLength < 3) {
	uint8_t next = transportRxQueuePosition + transportRxQueueLength;
	
	if (next >= 3) {
		next -= 3;
	}
	
    transportQueuedMessage* msg = &transportRxQueue[next];
    msg->m_len = I2C_readMessage(msg->m_data);
	transportRxQueueLength++;
  }
}



/**
* @brief Initialize transport HW
* @return true if initialization successful
*/
bool transportInit() {
  Wire.begin(DEFAULT_ADDRESS);
  TWAR = (DEFAULT_ADDRESS << 1) | 1;
  Wire.onReceive(transportRxCallback);  
  
  return true;
}
/**
* @brief Set node address
*/

int _address;
void transportSetAddress(uint8_t address) {
  _address = address;
  
  Wire.begin(_address);
  TWAR = (_address << 1) | 1;
  Wire.onReceive(transportRxCallback);
}

/**
* @brief Retrieve node address
*/
uint8_t transortGetAddress() {
  return _address;
}
/**
* @brief Send message
* @param to recipient
* @param data message to be sent
* @param len length of message (header + payload)
* @return true if message sent successfully
*/
bool transportSend(uint8_t to, const void* data, uint8_t len) {
  
  if (to == 255) {
	  to = 0;
  }
  
  Wire.beginTransmission(to);
  uint8_t* current = (uint8_t*) data;
  
  while (len--) {
    Wire.write(*current++);
  }
  
  Wire.endTransmission();
  
  return true;
}

/**
* @brief Verify if RX FIFO has pending messages
* @return true if message available in RX FIFO
*/
bool transportAvailable() {
  return transportRxQueueLength > 0;
}

/**
* @brief Sanity check for transport: is transport still responsive?
* @return true transport ok
*/
bool transportSanityCheck() {
  return true;
}

/**
* @brief Receive message from FIFO
* @return length of recevied message (header + payload)
*/
uint8_t transportReceive(void* data) {
  uint8_t len = 0;
  
  transportQueuedMessage* msg = &transportRxQueue[transportRxQueuePosition++];
  
  len = msg->m_len;
  (void)memcpy(data, msg->m_data, len);
	
  transportRxQueueLength--;
	
  if (transportRxQueuePosition >= 3) {
    transportRxQueuePosition -= 3;
  }
}

/**
* @brief Power down transport HW
*/
void transportPowerDown() {
  // do nothing
}

