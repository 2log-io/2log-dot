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

extern "C"
{
	#include <stdio.h>
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "esp_wifi.h"
	#include "esp_event.h"
	#include "esp_log.h"

	#include "esp_websocket_client.h"
	#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "soc/soc.h"
#include "soc/cpu.h"
#include "soc/rtc_periph.h"
#include "esp32/rom/ets_sys.h"
#include "esp_private/system_internal.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_cntl.h"
}

#include "Dot.h"

#ifdef CONFIG_BROWNOUT_DET_LVL
#define BROWNOUT_DET_LVL CONFIG_BROWNOUT_DET_LVL
#else
#define BROWNOUT_DET_LVL 0
#endif //CONFIG_BROWNOUT_DET_LVL

#define CONFIG_BROWNOUT_DET_LVL_SEL_5 1


extern "C"
{
	static void my_rtc_brownout_isr_handler(void *arg)
	{
		/* Normally RTC ISR clears the interrupt flag after the application-supplied
		 * handler returns. Since restart is called here, the flag needs to be
		 * cleared manually.
		 */
		REG_WRITE(RTC_CNTL_INT_CLR_REG, RTC_CNTL_BROWN_OUT_INT_CLR);
		/* Stall the other CPU to make sure the code running there doesn't use UART
		 * at the same time as the following ets_printf.
		 */
		esp_cpu_stall(!xPortGetCoreID());
		esp_reset_reason_set_hint(ESP_RST_BROWNOUT);

		ets_printf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\033[1;31m#######################################\r\n# Dot Brownout detector was triggered #\r\n#######################################\r\n\033[0m\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");

		esp_cpu_unstall(!xPortGetCoreID());
		//esp_restart_noos();
	}

	void brownout_init()
	{
		REG_WRITE(RTC_CNTL_BROWN_OUT_REG,
				RTC_CNTL_BROWN_OUT_ENA /* Enable BOD */
				| RTC_CNTL_BROWN_OUT_PD_RF_ENA /* Automatically power down RF */
				/* Reset timeout must be set to >1 even if BOR feature is not used */
				| (2 << RTC_CNTL_BROWN_OUT_RST_WAIT_S)
				| (BROWNOUT_DET_LVL << RTC_CNTL_DBROWN_OUT_THRES_S));

		ESP_ERROR_CHECK( rtc_isr_register(my_rtc_brownout_isr_handler, NULL, RTC_CNTL_BROWN_OUT_INT_ENA_M) );
		printf("Initialized BOD\n");

		REG_SET_BIT(RTC_CNTL_INT_ENA_REG, RTC_CNTL_BROWN_OUT_INT_ENA_M);
	}
}

static _2log::Dot   dotInstance;


extern "C" void app_main(void)
{
	brownout_init();

	dotInstance.startDevice();

	while(1)
	{
        //printf("Hello world!\n");
        vTaskDelay(1000 / portTICK_RATE_MS);
	}

    vTaskDelete(nullptr);
}
