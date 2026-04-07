/*
 * timestamp.h
 *
 *  Created on: Apr 7, 2026
 *      Author: alex
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#ifdef STM32F103xB
#include <stm32f1xx.h>
#else
#include <chrono>
#endif

// ChessEngine.cpp
uint64_t getCurrentTimeMs() {
#ifdef STM32F103xB
    return HAL_GetTick();  // миллисекунды от старта
#else
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
#endif
}

#endif /* TIMESTAMP_H_ */
