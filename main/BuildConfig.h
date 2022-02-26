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

#ifndef BUILDCONFIG_H
#define BUILDCONFIG_H

extern "C"
{
    #include <esp_log.h>
}

#pragma GCC diagnostic ignored "-Wunused-variable"

#ifndef DEVICE_FIRMWARE_VERSION
    #define DEVICE_FIRMWARE_VERSION     "-"
#endif

#ifndef DEVICE_FIRMWARE_BUILD
    #define DEVICE_FIRMWARE_BUILD       "-"
#endif

#define DISABLE_TLS_CA_VALIDATION       1
#define ALLOW_3RD_PARTY_FIRMWARE        0

#define DEVICE_LOG_LEVEL                ESP_LOG_INFO
#define SYSTEM_MONITORING               0

#define	DEVICE_DEBUGGING                0
#define DEBUGGING_DEVICE_ID             "DUDE"
#define DUMP_TASK_STATS                 0
#define OVERRIDE_CONFIG                 0
#define OVERRIDE_WIFI                   0
#define DISABLE_BROWNOUT_DETECTION      0

#if OVERRIDE_CONFIG == 1
    #define SERVER_URL                  "wss://switchtesting.dev.2log.io"
#endif

#define MEMORY_DEBUGGING                0
#define MAX_DEBUGGING_TASKS_NUMBER      15

#define DOT_HARDWARE_REVISION           320 

namespace
{
    const int   PING_TIMEOUT_TIMER                              = 15000; // ms
    const int   PING_TIMEOUT                                    = 60000; // ms

    const int   CONNECTION_RETRY_LIMIT_UNTIL_DELAY              = 6;
    const int   CONNECTION_RETRY_LIMIT_UNTIL_WIFI_RECONNECT     = 24;
    const int   CONNECTION_RETRY_DELAY_TIME                     = 60000; // ms

    const int   CONFIGURATION_MAX_RETRY                         = 4;
    const int   CONFIGURATION_RETRY_DELAY                       = 5000;	// ms

    const char* CONFIGURATION_WIFI_SSID                         = "I'm a Dot";
    const char* CONFIGURATION_WIFI_PWD                          = "";
    const char* FIRMWARE_MAGIC_BYTES                            = "2LOG_DT";

    const char*	DEVICE_TYPE                                     = "2log Dot";
}

// Fallback values. Do not edit and use configuration above.

#if defined __has_include
    #if __has_include("confidential-password.h")
        #include "confidential-password.h"
    #endif
#endif

#if DOT_HARDWARE_REVISION == 311

#define DOT_RESETBUTTON_GPIO            35
#define DOT_LED_RING_GPIO               32

#define PN532_SCK                       18
#define PN532_MOSI                      23
#define PN532_SS                        5
#define PN532_MISO                      19
#define PN532_IRQ                       27

#endif


#if DOT_HARDWARE_REVISION == 312
#define DOT_RESETBUTTON_GPIO            13
#define DOT_LED_RING_GPIO               2

#define PN532_SCK                       18
#define PN532_MOSI                      23
#define PN532_SS                        5
#define PN532_MISO                      19
#define PN532_IRQ                       4

#endif

#if DOT_HARDWARE_REVISION == 211

#define DOT_RESETBUTTON_GPIO            13
#define DOT_LED_RING_GPIO               32

#define PN532_SCK                       18
#define PN532_MOSI                      23
#define PN532_SS                        5
#define PN532_MISO                      19
#define PN532_IRQ                       16

#endif

#if DOT_HARDWARE_REVISION == 320

#define DOT_RESETBUTTON_GPIO            13
#define DOT_LED_RING_GPIO               32

// i2c
#define PN532_SDA                       21
#define PN532_SCL                       22
#define PN532_RST                       25
#define PN532_IRQ                       16

#endif

#if OVERRIDE_WIFI == 1

    #ifndef WIFI_SSID
        #error "Define WIFI_SSID in confidential-password.h"
    #endif

    #ifndef WIFI_PASSWORD
        #error "Define WIFI_PASSWORD in confidential-password.h"
    #endif

#endif

#ifndef DEVICE_LOG_LEVEL
    #define DEVICE_LOG_LEVEL    ESP_LOG_NONE
#endif

#ifndef SYSTEM_MONITORING
    #define SYSTEM_MONITORING   0
#endif

#endif
