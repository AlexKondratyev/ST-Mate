// N25Q032.cpp
#include "FlashDriver.h"
#include <stdio.h>
#include <memory.h>

// Команды
#define CMD_WRITE_ENABLE     0x06
#define CMD_WRITE_DISABLE    0x04
#define CMD_READ_STATUS      0x05
#define CMD_READ_DATA        0x03
#define CMD_PAGE_PROGRAM     0x02
#define CMD_SECTOR_ERASE     0x20
#define CMD_BLOCK_ERASE      0xD8
#define CMD_CHIP_ERASE       0xC7
#define CMD_READ_ID          0x9F

// Биты регистра состояния
#define STATUS_WIP           0x01   ///< Write In Progress
#define STATUS_WEL           0x02   ///< Write Enable Latch

FlashDriver::FlashDriver(SPI_HandleTypeDef* spi, GPIO_TypeDef* csPort, uint16_t csPin)
    : spi_(spi), csPort_(csPort), csPin_(csPin)
{
	init();
	// Проверка ID. Не должно быть -1
	id = readID();
}

FlashDriver::~FlashDriver() {
}

void FlashDriver::csLow() {
    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_RESET);
}

void FlashDriver::csHigh() {
    HAL_GPIO_WritePin(csPort_, csPin_, GPIO_PIN_SET);
}

bool FlashDriver::writeEnable() {
    csLow();
    uint8_t cmd = CMD_WRITE_ENABLE;
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi_, &cmd, 1, HAL_MAX_DELAY);
    csHigh();
    if (status != HAL_OK) return false;

    // Проверяем, что бит WEL установлен
    uint8_t sr = readStatusRegister();
    return (sr & STATUS_WEL) != 0;
}

bool FlashDriver::writeDisable() {
    csLow();
    uint8_t cmd = CMD_WRITE_DISABLE;
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi_, &cmd, 1, HAL_MAX_DELAY);
    csHigh();
    return status == HAL_OK;
}

uint8_t FlashDriver::readStatusRegister() {
    uint8_t cmd = CMD_READ_STATUS;
    uint8_t status;
    csLow();
    HAL_SPI_Transmit(spi_, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(spi_, &status, 1, HAL_MAX_DELAY);
    csHigh();
    return status;
}

void FlashDriver::waitForReady() {
    while ((readStatusRegister() & STATUS_WIP) != 0) {
        // Можно добавить задержку или переключение задач в RTOS
    }
}

bool FlashDriver::init() {
    // Настройка пина CS как выхода
    GPIO_InitTypeDef gpioInit;
    gpioInit.Pin = csPin_;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(csPort_, &gpioInit);
    csHigh();

    // Проверка ID (опционально)
    uint32_t id = readID();
    // Для N25Q032 ожидаемый ID: 0x20 0xBA 0x18 (может отличаться)
    // Если ID некорректен, можно вернуть false.
    // В данном примере считаем успехом любое значение, отличное от 0xFFFFFF.
    return (id != 0xFFFFFFFF);
}

uint32_t FlashDriver::readID() {
    uint8_t cmd = CMD_READ_ID;
    uint8_t id[3] = {0};
    csLow();
    HAL_SPI_Transmit(spi_, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(spi_, id, 3, HAL_MAX_DELAY);
    csHigh();
    return (id[0] << 16) | (id[1] << 8) | id[2];
}

bool FlashDriver::writePage(uint32_t pageAddr, const uint8_t* data, uint32_t len) {
    if (len == 0 || len > 256) return false;
    if ((pageAddr & 0xFF) != 0) return false; // Адрес должен быть кратен 256

    if (!writeEnable()) return false;

    uint8_t cmd[4];
    cmd[0] = CMD_PAGE_PROGRAM;
    cmd[1] = (pageAddr >> 16) & 0xFF;
    cmd[2] = (pageAddr >> 8) & 0xFF;
    cmd[3] = pageAddr & 0xFF;

    csLow();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi_, cmd, 4, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        csHigh();
        return false;
    }
    status = HAL_SPI_Transmit(spi_, (uint8_t*)data, len, HAL_MAX_DELAY);
    csHigh();
    if (status != HAL_OK) return false;

    waitForReady();
    return true;
}

bool FlashDriver::eraseSector(uint32_t sectorAddr) {
    if ((sectorAddr & 0xFFF) != 0) return false; // Адрес должен быть кратен 4KB
    if (!writeEnable()) return false;

    uint8_t cmd[4];
    cmd[0] = CMD_SECTOR_ERASE;
    cmd[1] = (sectorAddr >> 16) & 0xFF;
    cmd[2] = (sectorAddr >> 8) & 0xFF;
    cmd[3] = sectorAddr & 0xFF;

    csLow();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi_, cmd, 4, HAL_MAX_DELAY);
    csHigh();
    if (status != HAL_OK) return false;

    waitForReady();
    return true;
}

bool FlashDriver::eraseBlock(uint32_t blockAddr) {
    if ((blockAddr & 0xFFFF) != 0) return false; // Адрес должен быть кратен 64KB
    if (!writeEnable()) return false;

    uint8_t cmd[4];
    cmd[0] = CMD_BLOCK_ERASE;
    cmd[1] = (blockAddr >> 16) & 0xFF;
    cmd[2] = (blockAddr >> 8) & 0xFF;
    cmd[3] = blockAddr & 0xFF;

    csLow();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi_, cmd, 4, HAL_MAX_DELAY);
    csHigh();
    if (status != HAL_OK) return false;

    waitForReady();
    return true;
}

bool FlashDriver::eraseChip() {
    if (!writeEnable()) return false;

    uint8_t cmd = CMD_CHIP_ERASE;
    csLow();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi_, &cmd, 1, HAL_MAX_DELAY);
    csHigh();
    if (status != HAL_OK) return false;

    waitForReady();
    return true;
}

// N25Q032.cpp (добавления)

bool FlashDriver::eraseSectorContaining(uint32_t addr) {
    uint32_t sectorAddr = addr & ~(SECTOR_SIZE - 1);
    return eraseSector(sectorAddr);
}

bool FlashDriver::eraseRange(uint32_t startAddr, uint32_t endAddr) {
    if (endAddr >= CHIP_SIZE) endAddr = CHIP_SIZE - 1;
    uint32_t firstSector = startAddr & ~(SECTOR_SIZE - 1);
    uint32_t lastSector  = endAddr & ~(SECTOR_SIZE - 1);
    for (uint32_t sector = firstSector; sector <= lastSector; sector += SECTOR_SIZE) {
        if (!eraseSector(sector)) return false;
    }
    return true;
}

bool FlashDriver::isInit()
{
	return (id != 0xFFFFFF);
}

bool FlashDriver::read(uint32_t addr, uint8_t* data, uint32_t len) {
    if (len == 0) return true;

    uint8_t cmd[4];
    cmd[0] = CMD_READ_DATA;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    csLow();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi_, cmd, 4, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        csHigh();
        return false;
    }
    status = HAL_SPI_Receive(spi_, data, len, HAL_MAX_DELAY);
    csHigh();
    return status == HAL_OK;
}

bool FlashDriver::write(uint32_t addr, const uint8_t* data, uint32_t len) {
    if (addr + len > CHIP_SIZE) return false;
    if (len == 0) return true;

    uint32_t firstSector = addr / SECTOR_SIZE;
    uint32_t lastSector  = (addr + len - 1) / SECTOR_SIZE;

    // Буфер для одного сектора (4 КБ)
    uint8_t sectorBuffer[SECTOR_SIZE];

    for (uint32_t sectorIdx = firstSector; sectorIdx <= lastSector; ++sectorIdx) {
        uint32_t sectorStart = sectorIdx * SECTOR_SIZE;
        uint32_t sectorEnd   = sectorStart + SECTOR_SIZE - 1;

        // Определяем диапазон внутри сектора, который нужно обновить
        uint32_t updateStart = (addr > sectorStart) ? addr : sectorStart;
        uint32_t updateEnd   = (addr + len - 1 < sectorEnd) ? (addr + len - 1) : sectorEnd;
        uint32_t updateOffset = updateStart - sectorStart; // смещение в секторе
        uint32_t updateLen    = updateEnd - updateStart + 1;

        // Читаем весь сектор
        if (!read(sectorStart, sectorBuffer, SECTOR_SIZE)) return false;

        // Заменяем нужную часть
        const uint8_t* srcData = data + (updateStart - addr);
        memcpy(sectorBuffer + updateOffset, srcData, updateLen);

        // Стираем сектор
        if (!eraseSector(sectorStart)) return false;

        // Записываем сектор обратно (постранично)
        for (uint32_t page = 0; page < SECTOR_SIZE; page += PAGE_SIZE) {
            if (!writePage(sectorStart + page, sectorBuffer + page, PAGE_SIZE)) return false;
        }
    }

    return true;
}
