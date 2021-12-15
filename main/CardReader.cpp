#include "CardReader.h"

extern "C"
{
	#include <esp_log.h>
	#include <driver/gpio.h>
}

namespace
{
	const char* LOG_TAG = "_2log::CardReader";
}

namespace _2log
{
	CardReader::CardReader(uint8_t SCK, uint8_t MISO, uint8_t MOSI, uint8_t SS, uint8_t IRQ, CardReaderEventHandler *eventHandler)
		: IDFix::Task("cardreader_task", 3072, 20), _nfcReader(), _irqPin(IRQ), _eventHandler(eventHandler)
	{
		pn532_spi_init(&_nfcReader, SCK, MISO, MOSI, SS);
	}

	CardReader::~CardReader()
	{

	}

	bool CardReader::init()
	{
		pn532_begin(&_nfcReader);

		uint32_t versionData = pn532_getFirmwareVersion(&_nfcReader);
		if (!versionData)
		{
			ESP_LOGE(LOG_TAG, "Didn't find PN53x board");
			return false;
		}

		// Got ok data, print it out!
		ESP_LOGI(LOG_TAG, "Found chip PN5 %x", (versionData >> 24) & 0xFF);
		ESP_LOGI(LOG_TAG, "Firmware ver. %d.%d", (versionData >> 16) & 0xFF, (versionData >> 8) & 0xFF);

		// configure board to read RFID tags
		pn532_SAMConfig(&_nfcReader);

		gpio_config_t interruptPinConfig;

		interruptPinConfig.intr_type	= GPIO_INTR_NEGEDGE;
		interruptPinConfig.pin_bit_mask	= (BIT(_irqPin));
		interruptPinConfig.mode			= GPIO_MODE_INPUT;
		interruptPinConfig.pull_down_en	= GPIO_PULLDOWN_DISABLE;
		interruptPinConfig.pull_up_en	= GPIO_PULLUP_ENABLE;

		gpio_config(&interruptPinConfig);

		_eventQueue = xQueueCreate( 2, sizeof( CardReaderEventType ) );

        return true;
    }

    void CardReader::start()
    {
        startTask();
    }

	void CardReader::run()
	{
		CardReaderEventType event;
		uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
		uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

		gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
		gpio_isr_handler_add(static_cast<gpio_num_t>(_irqPin), CardReader::onInterrupt, static_cast<void*>(this) );

		pn532_readPassiveTargetID(&_nfcReader, PN532_MIFARE_ISO14443A, uid, &uidLength, 1);

		while (true)
		{
			if( xQueueReceive(_eventQueue, &event, portMAX_DELAY) == pdTRUE )
			{				
				ESP_LOGD(LOG_TAG, "CardReader event");

				if ( pn532_readDetectedPassiveTargetID(&_nfcReader, uid, &uidLength) )
				{
					// Display some basic information about the card

					ESP_LOGV(LOG_TAG, "Found an ISO14443A card running in Task %s", pcTaskGetTaskName(NULL) );

					ESP_LOGD(LOG_TAG, "Found an ISO14443A card");
					ESP_LOGD(LOG_TAG, "UID Length: %d bytes", uidLength);
					ESP_LOGD(LOG_TAG, "UID Value:");
					esp_log_buffer_hexdump_internal(LOG_TAG, uid, uidLength, ESP_LOG_INFO);

					if ( _eventHandler != nullptr )
					{
						_eventHandler->cardRead(uid, uidLength);
					}

					vTaskDelay(1000 / portTICK_RATE_MS);
				}

				pn532_readPassiveTargetID(&_nfcReader, PN532_MIFARE_ISO14443A, uid, &uidLength, 1);

				// pn532_readPassiveTargetID will trigger an interrupt by itself
				// consume and ignore this event here
				xQueueReceive(_eventQueue, &event, 0);

				ESP_LOGD(LOG_TAG, "CardReader enabled");
			}
			else
			{
				ESP_LOGW(LOG_TAG, "xQueueReceive timeout!");
			}

		}
	}

	void CardReader::onInterrupt(void *cardReaderInstance)
	{
		CardReader* objectInstance			= static_cast<CardReader*>(cardReaderInstance);
		BaseType_t xHigherPriorityTaskWoken	= pdFALSE;

		CardReaderEventType event = ReadCard;

		xQueueSendFromISR(objectInstance->_eventQueue, &event, &xHigherPriorityTaskWoken);

		if ( xHigherPriorityTaskWoken )
		{
			portYIELD_FROM_ISR();
		}
	}
}
