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

#ifndef DOT_H
#define DOT_H

#include "Animations.h"
#include "CardReaderEventHandler.h"
#include "CardReader.h"
#include "BaseDevice.h"
#include "GPIOButton.h"

using namespace IDFix;

namespace _2log
{
    class Dot : public CardReaderEventHandler, public BaseDevice
	{
		public:

							Dot();

			void			init();

		private:

			virtual void	cardRead(const uint8_t uid[7], const uint8_t idLength) override;

			void			showAccept(cJSON* argument);
			void			showError(cJSON* argument);
			void			setState(cJSON* argument);

            virtual void    baseDeviceEventHandler(BaseDeviceEvent event) override;
            virtual void    baseDeviceStateChanged(BaseDeviceState state) override;

            virtual void    initProperties(cJSON *argument) override;
            virtual void    resetDeviceConfigurationAndRestart() override;

		private:

			bool                        _isInitalized = { false };
			Animations                  _animations;
			CardReader                  _cardReader;
			int                         _rssi;
			int                         _dotState;
                        unsigned long               _lastRSSITick;
                        IODevices::GPIOButton       _resetButton;
	};
}

#endif


