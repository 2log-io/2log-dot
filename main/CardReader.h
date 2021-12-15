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

#ifndef CARDREADER_H
#define CARDREADER_H

#include "IDFixTask.h"
#include "CardReaderEventHandler.h"

extern "C"
{
	#include <pn532.h>
	#include <freertos/timers.h>
	#include <freertos/queue.h>
}

namespace _2log
{
	class CardReader : public IDFix::Task
	{
		public:

									CardReader(uint8_t SCK, uint8_t MISO, uint8_t MOSI, uint8_t SS, uint8_t IRQ, CardReaderEventHandler *eventHandler);
			virtual					~CardReader() override;

			bool					init(void);
			void                    start();

			virtual void			run() override;

		private:

			static void IRAM_ATTR	onInterrupt(void *cardReaderInstance);

			enum CardReaderEventType
			{
				ReadCard
			};

		private:

			pn532_t					_nfcReader;
			uint8_t					_irqPin;
			CardReaderEventHandler*	_eventHandler;
			QueueHandle_t			_eventQueue = { nullptr };
	};
}

#endif
