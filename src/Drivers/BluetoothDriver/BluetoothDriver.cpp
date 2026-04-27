/*
 * BluetoothDriver.cpp
 *
 *  Created on: Mar 23, 2026
 *      Author: alex
 */

#include "BluetoothDriver.h"
#include "usart.h"

BluetoothDriver* btDriver = nullptr;
extern "C"
{
// In UART interrupt handler
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Check that interrupt is from UART connected to HC-06
    if (huart->Instance == USART1) {  // Replace with your USART
        if (btDriver != nullptr) {
        	btDriver->handleUartRxComplete();
        }
    }
}
}

BluetoothDriver::BluetoothDriver()
	: _huart(&huart1)
	, rxInProgress(false)
{
	btDriver = this;
    init();
}

BluetoothDriver::~BluetoothDriver()
{
    // Stop reception
    if (rxInProgress) {
        HAL_UART_AbortReceive(_huart);
    }
}

void BluetoothDriver::init()
{
    // Clear buffers
	rxDataBuffer.resize(RX_BUFFER_SIZE);
    rxDataBuffer.clear();
    rxInProgress = false;

    // Start asynchronous data reception
    if (HAL_UART_Receive_IT(_huart, rxBuffer, 1) != HAL_OK) {
        return;
    }
    rxInProgress = true;
}

bool BluetoothDriver::sendData(const std::string &data)
{
    if (data.empty()) return false;

    return HAL_UART_Transmit(_huart, (const uint8_t*)data.data(), data.size(), 100) == HAL_OK;
}

bool BluetoothDriver::isDataAvailable()
{
	return !rxDataBuffer.empty();
}

std::string BluetoothDriver::receiveData()
{
    return rxDataBuffer;
}

void BluetoothDriver::handleUartRxComplete()
{
    // One byte received
    uint8_t receivedByte = rxBuffer[0];
    rxDataBuffer.push_back(receivedByte);

    // Restart reception
    if (HAL_UART_Receive_IT(_huart, rxBuffer, 1) != HAL_OK) {
        rxInProgress = false;
    }

	if (
			rxDataBuffer.size() > 1 &&
			rxDataBuffer[rxDataBuffer.size()-2] == '\r' &&
			rxDataBuffer[rxDataBuffer.size()-1] == '\n')
	{
		rxDataBuffer=rxDataBuffer.substr(0, rxDataBuffer.size()-2);
	    for (char &c : rxDataBuffer) {
	    	if (c == ':' || c == '\r') break;
	        c = std::toupper(static_cast<unsigned char>(c));
	    }
	    receivedCommand = rxDataBuffer;
		rxDataBuffer.clear();
	}
}

bool BluetoothDriver::sendATCommand(const std::string &command,
		const std::string &expectedResponse)
{
    std::string response;
    if (sendCommandAndGetResponse("AT+" + command + "\r\n", response)) {
        return response.find(expectedResponse) != std::string::npos;
    }
    return false;
}

bool BluetoothDriver::sendCommandAndGetResponse(const std::string &command,
		std::string &response)
{
    // Sending command
    uint8_t* cmdData = reinterpret_cast<uint8_t*>(const_cast<char*>(command.c_str()));
    if (HAL_UART_Transmit(_huart, cmdData, command.length(), 100) != HAL_OK) {
        return false;
    }

    // Waiting for response
    uint32_t startTime = HAL_GetTick();

    while ((HAL_GetTick() - startTime) < AT_TIMEOUT) {
            // Check for end of line
            if (rxDataBuffer.size() > 1
           		&& rxDataBuffer[rxDataBuffer.size()-2] == '\r'
           	    && rxDataBuffer[rxDataBuffer.size()-1] == '\n') { break; }
    }

    if (rxDataBuffer.size() > 0) {
        // Removing carriage return and line feed characters
        response = rxDataBuffer;
    	rxDataBuffer.clear();
        return true;
    }

    return false;
}

void BluetoothDriver::run()
{
	if(receivedCommand.empty())
		return;
	notify(receivedCommand);
	receivedCommand.clear();
}
