// N25Q032.h
#ifndef N25Q032_H
#define N25Q032_H

#include "stm32f1xx_hal.h"
#include "FlashDriverInterface.h"

/**
 * @brief Драйвер для SPI NOR Flash N25Q032 (32 Mbit).
 */
class FlashDriver : public FlashDriverInterface
{
public:
    /**
     * @brief Конструктор.
     * @param spi Указатель на инициализированный SPI_HandleTypeDef.
     * @param csPort Порт для вывода CS.
     * @param csPin Пин для вывода CS.
     */
    FlashDriver(SPI_HandleTypeDef* spi, GPIO_TypeDef* csPort, uint16_t csPin);
    ~FlashDriver();

    /**
     * @brief Инициализирована ли память.
     * @return true если успешно, false в противном случае.
     */
    bool isInit() override;

private:

    /**
     * @brief Чтение данных из флеш-памяти.
     * @param addr Начальный адрес чтения.
     * @param data Буфер для приёма данных.
     * @param len Количество байт для чтения.
     * @return true если успешно, false в противном случае.
     */
    bool read(uint32_t addr, uint8_t* data, uint32_t len) override;

    /**
     * @brief Запись данных произвольного размера (стирает затронутые сектора).
     * @param addr Начальный адрес записи.
     * @param data Буфер данных.
     * @param len Количество байт для записи.
     * @return true если успешно, false в противном случае.
     */
    bool write(uint32_t addr, const uint8_t* data, uint32_t len) override;

    /**
     * @brief Запись страницы (256 байт). Адрес должен быть кратен 256.
     * @param pageAddr Адрес страницы (выровненный на 256 байт).
     * @param data Данные для записи.
     * @param len Количество байт (не более 256).
     * @return true если успешно, false в противном случае.
     */
    bool writePage(uint32_t pageAddr, const uint8_t* data, uint32_t len);

    /**
     * @brief Стирание диапазона (по секторам 4 КБ).
     * @param startAddr Начальный адрес.
     * @param endAddr Конечный адрес (включительно).
     * @return true если успешно, false в противном случае.
     */
    bool eraseRange(uint32_t startAddr, uint32_t endAddr);

    uint32_t id = 0;					// id памяти
    SPI_HandleTypeDef* spi_;   ///< Указатель на SPI периферию.
    GPIO_TypeDef* csPort_;     ///< Порт для CS.
    uint16_t csPin_;           ///< Пин для CS.
    static const uint32_t PAGE_SIZE  = 256;   ///< Размер страницы.
    static const uint32_t SECTOR_SIZE = 4096; ///< Размер сектора.
    static const uint32_t CHIP_SIZE   = 4 * 1024 * 1024; ///< 32 Mbit = 4 MB.

    /**
     * @brief Инициализация драйвера.
     * @return true если успешно, false в противном случае.
     */
    bool init();

    /**
     * @brief Чтение идентификатора чипа (3 байта).
     * @return 24-битный идентификатор (manufacturer, memory type, capacity).
     */
    uint32_t readID();

    /**
     * @brief Стирание сектора (4 КБ). Адрес должен быть кратен 4096.
     * @param sectorAddr Адрес сектора.
     * @return true если успешно, false в противном случае.
     */
    bool eraseSector(uint32_t sectorAddr);

    /**
     * @brief Стирание блока (64 КБ). Адрес должен быть кратен 65536.
     * @param blockAddr Адрес блока.
     * @return true если успешно, false в противном случае.
     */
    bool eraseBlock(uint32_t blockAddr);

    /**
     * @brief Стирание сектора по любому адресу (автоматическое выравнивание).
     * @param addr Любой адрес внутри сектора.
     * @return true если успешно.
     */
    bool eraseSectorContaining(uint32_t addr);

    /**
     * @brief Стирание всего чипа.
     * @return true если успешно, false в противном случае.
     */
    bool eraseChip();

    void csLow();              ///< Установка CS в 0.
    void csHigh();             ///< Установка CS в 1.
    void waitForReady();       ///< Ожидание готовности чипа (сброс WIP).
    bool writeEnable();        ///< Разрешение записи.
    bool writeDisable();       ///< Запрет записи.
    uint8_t readStatusRegister(); ///< Чтение регистра состояния.
};

#endif // N25Q032_H
