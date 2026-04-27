/*
 * BluetoothDriver.h
 *
 *  Created on: Mar 23, 2026
 *      Author: alex
 */

#ifndef BLUETOOTHDRIVER_BLUETOOTHDRIVER_H_
#define BLUETOOTHDRIVER_BLUETOOTHDRIVER_H_

#include "main.h"
#include <string>
#include "Publisher.h"

class BluetoothDriver : protected Publisher{
public:
	BluetoothDriver();
	virtual ~BluetoothDriver();


	void run();
    bool sendData(const std::string& data);
    bool isDataAvailable();
    void handleUartRxComplete();

private:
    UART_HandleTypeDef* _huart;	// UART for HC-06
    uint8_t rxBuffer[1];       	// Receive buffer
    std::string rxDataBuffer;	// Accumulated data buffer
    bool rxInProgress;         	// Reception in progress flag
    std::string receivedCommand;// Received command

    // Buffers for AT command work
    static constexpr uint16_t AT_TIMEOUT = 500;  // AT command timeout (ms)
    static constexpr uint16_t RX_BUFFER_SIZE = 64; // Receive buffer size

    void init();
    std::string receiveData();

    bool sendATCommand(const std::string& command, const std::string& expectedResponse);
    bool sendCommandAndGetResponse(const std::string& command, std::string& response);

	// Publisher methods
	virtual void notify(const std::string &message) override
	{
		std::list<Listener *>::iterator iterator = listListeners.begin();
		while (iterator != listListeners.end()) {
		  (*iterator)->messege(message);
		  ++iterator;
		}
	}
public:
	virtual void attach(Listener *listener) override
	{
		listListeners.push_back(listener);
	}

};

#endif /* BLUETOOTHDRIVER_BLUETOOTHDRIVER_H_ */
