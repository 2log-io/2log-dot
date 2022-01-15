/*   2log.io
 *   Copyright (C) 2021 - 2log.io | mail@2log.io,  friedemann@2log.io
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Dot.h"

#include <stdint.h>
#include <stdlib.h>
#include <string_view>

extern "C"
{
    #include "esp_log.h"
    #include "soc/timer_group_struct.h"
    #include "soc/timer_group_reg.h"
    #include "freertos/FreeRTOSConfig.h"
    #include <freertos/task.h>
}

#include "BuildConfig.h"
#include "IDeviceNode.h"
#include "auxiliary.h"
#include "IDFixTask.h"
#include "DeviceProperties.h"

namespace
{
	const char* LOG_TAG = "_2log::Dot";
}

namespace _2log
{
	Dot::Dot() : BaseDevice(),
                _animations(DOT_LED_RING_GPIO),
				_cardReader(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS, PN532_IRQ, this),
                _rssi(0),
                _dotState(0),
                _lastRSSITick(0),
                _resetButton(DOT_RESETBUTTON_GPIO)
	{

		_animations.init();
		_animations.start();
		_animations.setAll(25,0,0);
        _animations.show();

                _resetButton.setPressDuration(3000);
                _resetButton.setButtonPressedCallback(std::bind(static_cast<void(Dot::*)(void)>(&Dot::resetDeviceConfigurationAndRestart), this));
	}

	void Dot::init()
	{
        if ( _isInitalized )
        {
            ESP_LOGW(LOG_TAG, "Already initialized!");
            return;
        }

        bool success;
        int dotState = DeviceProperties::instance().getProperty("dotState", 0).asInt(&success);

        if (success)
        {
            _dotState = dotState;
        }

		_deviceNode->registerRPC("showAccept",      std::bind(&Dot::showAccept, this, std::placeholders::_1) );
		_deviceNode->registerRPC("showError",		std::bind(&Dot::showError, this, std::placeholders::_1) );
		_deviceNode->registerRPC("setState",		std::bind(&Dot::setState, this, std::placeholders::_1) );

		if ( ! _cardReader.init() )
		{
			ESP_LOGE(LOG_TAG, "failed to init card reader");
			return;
		}

		_cardReader.start();

		_isInitalized = true;

		return;
	}

	void Dot::cardRead(const uint8_t uid[], const uint8_t idLength)
	{
		char		hexNumber[3];
		std::string	idString;

		ESP_LOGI("DOT", "cardRead() - UID Length: %d bytes", idLength);

		for (int i = 0; i < idLength; i++)
		{
			sprintf(hexNumber, "%02X", uid[i]);
			idString.append(hexNumber);
		}
        _animations.showCardRead();

		_deviceNode->sendData( idString.c_str() );
	}

	void Dot::showAccept(cJSON* UNUSED(argument) )
	{
		ESP_LOGD("DOT", "showAccept() running in Task %s", pcTaskGetTaskName(NULL));
		_animations.showAccept();
	}

	void Dot::showError(cJSON* UNUSED(argument) )
	{
		_animations.showError();
	}

	void Dot::setState(cJSON* argument)
	{
		cJSON *stateValItem = cJSON_GetObjectItem(argument, "val");

		if ( cJSON_IsNumber(stateValItem) )
		{
			_dotState = stateValItem->valueint;
			_animations.setState( static_cast<Animations::State>(_dotState) );
			_deviceNode->setProperty("state", _dotState);
			DeviceProperties::instance().saveProperty("dotState", _dotState);
		}
		else
		{
			ESP_LOGE("DOT", "cJSON_IsNumber(stateValItem) failed");
        }
    }

    void Dot::baseDeviceEventHandler(BaseDeviceEvent event)
    {
        switch(event)
        {
            case BaseDeviceEvent::NetworkConnected:
            {
                init();
                return;
            }

            default: ESP_LOGD(LOG_TAG, "No handler implemented for this event.");
        }
    }

    void Dot::baseDeviceStateChanged(BaseDeviceState state)
    {
        switch(state)
        {
            case BaseDeviceState::Configuring:

                ESP_LOGW(LOG_TAG, "baseDeviceStateChanged: Configuring");
                _animations.fadeTo(25,0,25);

				return;

            case BaseDeviceState::Connecting:

                ESP_LOGW(LOG_TAG, "baseDeviceStateChanged: Connecting");
                _animations.fadeTo(0,25,0);

                _resetButton.start();
                if( _resetButton.isPressed() )
                {
                    resetDeviceConfigurationAndRestart();
                }

                return;

            case BaseDeviceState::Connected:

                ESP_LOGW(LOG_TAG, "baseDeviceStateChanged: Connected");
                _animations.setState( static_cast<Animations::State>(_dotState) );

				return;

            case BaseDeviceState::UpdatingFirmware:

                ESP_LOGW(LOG_TAG, "baseDeviceStateChanged: UpdatingFirmware");
                _animations.fadeTo(0,0,25);

				return;

            default: return;
        }
    }

    void Dot::initProperties(cJSON *argument)
    {
        BaseDevice::initProperties(argument);

        cJSON_AddStringToObject(argument, ".fwvers",    DEVICE_FIRMWARE_VERSION);
        cJSON_AddStringToObject(argument, ".fwbuild",   DEVICE_FIRMWARE_BUILD);
        cJSON_AddNumberToObject(argument, "state",      _dotState);
    }

    void Dot::resetDeviceConfigurationAndRestart()
    {
        ESP_LOGI(LOG_TAG, "resetDeviceConfigurationAndRestart");

        _animations.setState( static_cast<Animations::State>(0) );
        _deviceNode->setProperty("state", 0);

        IDFix::Task::delay(1000);

        BaseDevice::resetDeviceConfigurationAndRestart();
    }
}
