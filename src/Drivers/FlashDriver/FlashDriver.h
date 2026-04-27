// N25Q032.h
#ifndef N25Q032_H
#define N25Q032_H

#include "stm32f1xx_hal.h"
#include "FlashDriverInterface.h"

/**
 * @brief Driver for SPI NOR Flash N25Q032 (32 Mbit).
 */
class FlashDriver : public FlashDriverInterface
{
public:
    /**
     * @brief Constructor.
     * @param spi Pointer to initialized SPI_HandleTypeDef.
     * @param csPort Port for CS output.
     * @param csPin Pin for CS output.
     */
    FlashDriver(SPI_HandleTypeDef* spi, GPIO_TypeDef* csPort, uint16_t csPin);
    ~FlashDriver();

    /**
     * @brief Is memory initialized.
     * @return true if successful, false otherwise.
     */
    bool isInit() override;

private:

    /**
     * @brief Reading data from flash memory.
     * @param addr Starting read address.
     * @param data Buffer for receiving data.
     * @param len Number of bytes to read.
     * @return true if successful, false otherwise.
     */
    bool read(uint32_t addr, uint8_t* data, uint32_t len) override;

    /**
     * @brief Writing data of arbitrary size (erases affected sectors).
     * @param addr Starting write address.
     * @param data Data buffer.
     * @param len Number of bytes to write.
     * @return true if successful, false otherwise.
     */
    bool write(uint32_t addr, const uint8_t* data, uint32_t len) override;

    /**
     * @brief Writing page (256 bytes). Address must be multiple of 256.
     * @param pageAddr Page address (aligned to 256 bytes).
     * @param data Data to write.
     * @param len Number of bytes (no more than 256).
     * @return true if successful, false otherwise.
     */
    bool writePage(uint32_t pageAddr, const uint8_t* data, uint32_t len);

    /**
     * @brief Erasing range (by 4 KB sectors).
     * @param startAddr Starting address.
     * @param endAddr End address (inclusive).
     * @return true if successful, false otherwise.
     */
    bool eraseRange(uint32_t startAddr, uint32_t endAddr);

    uint32_t id = 0;					// memory id
    SPI_HandleTypeDef* spi_;   ///< Pointer to SPI peripheral.
    GPIO_TypeDef* csPort_;     ///< Port for CS.
    uint16_t csPin_;           ///< Pin for CS.
    static const uint32_t PAGE_SIZE  = 256;   ///< Page size.
    static const uint32_t SECTOR_SIZE = 4096; ///< Sector size.
    static const uint32_t CHIP_SIZE   = 4 * 1024 * 1024; ///< 32 Mbit = 4 MB.

    /**
     * @brief Driver initialization.
     * @return true if successful, false otherwise.
     */
    bool init();

    /**
     * @brief Reading chip identifier (3 bytes).
     * @return 24-bit identifier (manufacturer, memory type, capacity).
     */
    uint32_t readID();

    /**
     * @brief Erasing sector (4 KB). Address must be multiple of 4096.
     * @param sectorAddr Sector address.
     * @return true if successful, false otherwise.
     */
    bool eraseSector(uint32_t sectorAddr);

    /**
     * @brief Erasing block (64 KB). Address must be multiple of 65536.
     * @param blockAddr Block address.
     * @return true if successful, false otherwise.
     */
    bool eraseBlock(uint32_t blockAddr);

    /**
     * @brief Erase a sector at any address (automatic alignment).
     * @param addr Any address inside the sector.
     * @return true if successful.
     */
    bool eraseSectorContaining(uint32_t addr);

    /**
     * @brief Erase the entire chip.
     * @return true if successful, false otherwise.
     */
    bool eraseChip();

    void csLow();              ///< Set CS to 0.
    void csHigh();             ///< Set CS to 1.
    void waitForReady();       ///< Wait for chip ready (WIP reset).
    bool writeEnable();        ///< Enable writing.
    bool writeDisable();       ///< Disable writing.
    uint8_t readStatusRegister(); ///< Read status register.
};

#endif // N25Q032_H
