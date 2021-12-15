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

#ifndef ANIMATIONSTIMER_H
#define ANIMATIONSTIMER_H

#include "IDFixTask.h"

extern "C"
{
	#include "driver/timer.h"
}

#include <NeoPixelBrightnessBus.h>

namespace _2log
{
	class Animations : public IDFix::Task
	{
		public:

			enum Animation
			{
				ANIM_NONE		= 0,
				ANIM_ACCEPT		= 1,
				ANIM_ERROR		= 2,
				ANIM_CARD_READ	= 3,
				ANIM_RUNNING	= 4,
				ANIM_WARNING	= 5
			};

			enum State
			{
				STATE_IDLE = 0,
				STATE_ERROR = -2,
				STATE_WARNING = -1,
				STATE_LOGGED_IN = 1,
				STATE_RUNNING = 2
			};

			Animations(int gpio, int ledCount = 24);

			bool					init(void);

			void                    start();

			void					setState(State state);
			void					show(void);
			void					stopAnimation(void);
			void					setAll(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness = 255);

			void					showAccept(void);
			void					showError(void);
			void					showCardRead(void);

		private:

			virtual void			run() override;

			void					initTimer(void);
			static void IRAM_ATTR	onTimer(void *instance);

			void					animAcceptFrame(void);
			void					animErrorFrame(void);
			void					animCardReadFrame(void);

			void					showRunning(void);
			void					animRunningFrame(void);

			void					animateStrobe(uint8_t red, uint8_t green, uint8_t blue, int strobeCount);
			void					animateCircle(uint8_t red, uint8_t green, uint8_t blue);

			void					setPixelColor(uint8_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
			void					setPixelColor(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
			void					setBrightness(uint8_t brightness);

		private:

			static timer_group_t	timerGroup;
			static timer_idx_t		timer;

			NeoPixelBrightnessBus	_strip;
			int						_ledCount;
			State					_currentState = STATE_IDLE;
			Animation				_currentAnimation = ANIM_NONE;
			int						_currentAnimationFrame = 0;

	};
}

#endif
