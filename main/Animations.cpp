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

#include "Animations.h"

extern "C"
{
	#include "esp_log.h"
}

#define TAG					"Animations"
#define TIMER_DIVIDER		16  //  Hardware timer clock divider
#define TIMER_SCALE			(TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

#define CIRCLE_FRAMES		1000
#define PIXELS				3
#define COUNT				3

namespace _2log
{
	timer_group_t	Animations::timerGroup	= TIMER_GROUP_0;
	timer_idx_t		Animations::timer		= TIMER_0;

	Animations::Animations(int gpio, int ledCount)
		: IDFix::Task("animations_task", 3072, 15), _strip(ledCount, gpio), _ledCount(ledCount)
	{

	}

	bool Animations::init()
	{
		ESP_LOGI(TAG, "Animations::init()");

		_strip.Begin();

		initTimer();

        return true;
    }

    void Animations::start()
    {
        startTask();
    }

	void Animations::setState(Animations::State state)
	{
		_currentState = state;
		stopAnimation();

		switch (state)
		{
			case STATE_IDLE:

				setAll(0,0,0);
				show();
				break;

			case STATE_ERROR:

				setAll(255,0,0);
				show();
				break;

			case STATE_WARNING:

				break;

			case STATE_LOGGED_IN:

				setAll(0,0,255);
				show();
				break;

			case STATE_RUNNING:

				showRunning();

				break;
		}
	}

	void Animations::show()
	{
		_strip.Show();
	}

	void Animations::stopAnimation()
	{
		timer_pause(timerGroup, timer);
		_currentAnimation = ANIM_NONE;
		_currentAnimationFrame = 0;

		setAll(0,0,0);
		show();
	}

	void Animations::setAll(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness)
	{
		setBrightness(brightness);
		_strip.ClearTo( RgbColor(red,green,blue) );
	}

	void Animations::showAccept()
	{
		stopAnimation();
		_currentAnimation = ANIM_ACCEPT;

		timer_set_counter_value(timerGroup, timer, 0x00000000ULL);
		timer_set_alarm_value(timerGroup, timer, 80 * ( TIMER_SCALE / 1000.0) );

		animAcceptFrame();
		timer_start(timerGroup, timer);
	}

	void Animations::showError()
	{
		stopAnimation();
		_currentAnimation = ANIM_ERROR;

		timer_set_counter_value(timerGroup, timer, 0x00000000ULL);
		timer_set_alarm_value(timerGroup, timer, 80 * ( TIMER_SCALE / 1000.0) );

		animErrorFrame();
		timer_start(timerGroup, timer);
	}

	void Animations::showCardRead()
	{
		stopAnimation();
		_currentAnimation = ANIM_CARD_READ;

		timer_set_counter_value(timerGroup, timer, 0x00000000ULL);
		timer_set_alarm_value(timerGroup, timer, 60 * ( TIMER_SCALE / 1000.0) );

		animCardReadFrame();
		timer_start(timerGroup, timer);
	}

	void Animations::run()
	{
		static uint32_t ulNotifiedValue;

		ESP_LOGI(TAG, "Animations::task() started");

		while(true)
		{
			ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

			if ( ulNotifiedValue )
			{
				//ESP_LOGI(TAG, "Animations::tick");

				switch (_currentAnimation)
				{
					case ANIM_NONE:

						// onTimer should actually never occure in ANIM_NONE
						ESP_LOGE(TAG, "Timer triggerd on ANIM_NONE");
						break;

					case ANIM_ACCEPT:

						animAcceptFrame();

						break;

					case ANIM_ERROR:

						animErrorFrame();

						break;

					case ANIM_CARD_READ:

						animCardReadFrame();
						break;

					case ANIM_RUNNING:

						animRunningFrame();
						break;

					case ANIM_WARNING:

						break;
				}

			}
			else
			{
				ESP_LOGE(TAG, "ulTaskNotifyTake timeout!");
			}
		}
	}

	void Animations::initTimer()
	{
		/* Select and initialize basic parameters of the timer */
		timer_config_t config;
		config.divider = TIMER_DIVIDER;
		config.counter_dir = TIMER_COUNT_UP;
		config.counter_en = TIMER_PAUSE;
		config.alarm_en = TIMER_ALARM_EN;
		config.intr_type = TIMER_INTR_LEVEL;
		config.auto_reload = TIMER_AUTORELOAD_EN;

		timer_init(timerGroup, timer, &config);

		/* Timer's counter will initially start from value below.
		   Also, if auto_reload is set, this value will be automatically reload on alarm */
		timer_set_counter_value(timerGroup, timer, 0x00000000ULL);

		/* Configure the alarm value and the interrupt on alarm. */
		timer_set_alarm_value(timerGroup, timer, 1000 * ( TIMER_SCALE / 1000.0) );
		timer_enable_intr(timerGroup, timer);
		timer_isr_register(timerGroup, timer, onTimer, static_cast<void*>(this), ESP_INTR_FLAG_IRAM, nullptr);
	}

	void Animations::onTimer(void *instance)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		Animations* objectInstance			= static_cast<Animations*>(instance);

		timer_group_intr_clr_in_isr(timerGroup, timer);

		// After the alarm has been triggered we need enable it again, so it is triggered the next time
		timer_group_enable_alarm_in_isr(timerGroup, timer);

		// Notify the task so it will wake up when the ISR is complete
		vTaskNotifyGiveFromISR(objectInstance->_taskHandle, &xHigherPriorityTaskWoken);

		if ( xHigherPriorityTaskWoken )
		{
			portYIELD_FROM_ISR();
		}
	}

	void Animations::animAcceptFrame(void)
	{
		animateStrobe(0, 0, 255, 2);
	}

	void Animations::animErrorFrame(void)
	{
		animateStrobe(255, 0, 0, 3);
	}

	void Animations::animCardReadFrame()
	{
		animateStrobe(100, 100, 100, 1);
	}

	void Animations::showRunning()
	{
		stopAnimation();
		_currentAnimation = ANIM_RUNNING;

		timer_set_counter_value(timerGroup, timer, 0x00000000ULL);
		timer_set_alarm_value(timerGroup, timer, 3 * ( TIMER_SCALE / 1000.0) );

		animRunningFrame();
		timer_start(timerGroup, timer);
	}

	void Animations::animRunningFrame()
	{
		animateCircle(0, 0, 255);
	}

	void Animations::animateStrobe(uint8_t red, uint8_t green, uint8_t blue, int strobeCount)
	{
		if(_currentAnimationFrame % 2 == 0)
		{
			setAll(red, green, blue);
			show();
		}
		else
		{
			setAll(0,0,0);
			show();
		}
		_currentAnimationFrame++;

		if(_currentAnimationFrame == strobeCount * 2)
		{
			stopAnimation();

			// reset current state, to restart possible previously running animation
			setState(_currentState);
		}
	}

	void Animations::animateCircle(uint8_t red, uint8_t green, uint8_t blue)
	{
		setAll(0, 0, 0);

		float tmp = static_cast<float>(_ledCount) / CIRCLE_FRAMES * _currentAnimationFrame;
		int idx = tmp;
		float rest = tmp - idx;

		for( int x = 0; x < COUNT; x++)
		{
			for(int i = 0; i < PIXELS; i++)
			{
				int tmp = (idx - i + PIXELS + x * _ledCount / COUNT) % _ledCount;
				setPixelColor(tmp , red, green, blue, (( PIXELS -i - rest) * 1/PIXELS - 1)  * 255);
			}
			setPixelColor((idx+PIXELS+1+ x * _ledCount / COUNT) % _ledCount, red, green, blue, rest * 255);
		}

		show();
		_currentAnimationFrame = (_currentAnimationFrame+1) % CIRCLE_FRAMES;
	}

	void Animations::setPixelColor(uint8_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		float factor = (float) a / 255;
		setPixelColor(idx, factor * r, factor * g, factor * b);
	}

	void Animations::setPixelColor(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
	{
		_strip.SetPixelColor(i, RgbColor(r,g,b));
	}

	void Animations::setBrightness(uint8_t brightness)
	{
		_strip.SetBrightness(brightness);
	}
}
