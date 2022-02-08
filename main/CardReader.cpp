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
	CardReader::CardReader(uint8_t sda, uint8_t scl,uint8_t reset, uint8_t irq, CardReaderEventHandler *eventHandler)
		: IDFix::Task("cardreader_task", 3072, 20)
		, _irqPin(irq)
		, _eventHandler(eventHandler)
	{
	
	}

	CardReader::~CardReader()
	{

	}

	bool CardReader::init()
	{

		init_PN532_I2C(21, 22, 25, 16, 0 );
		uint32_t versionData = getPN532FirmwareVersion();
		if (!versionData)
		{
			ESP_LOGE(LOG_TAG, "Didn't find PN53x board");
			return false;
		}

		// Got ok data, print it out!
		ESP_LOGI(LOG_TAG, "Found chip PN5 %x", (versionData >> 24) & 0xFF);
		ESP_LOGI(LOG_TAG, "Firmware ver. %d.%d", (versionData >> 16) & 0xFF, (versionData >> 8) & 0xFF);

		// configure board to read RFID tags
		SAMConfig();
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
		

		while (true)
		{

			if ( readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000))
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
		}
	}
}
