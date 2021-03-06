/*-------------------------------------------------------------------------
NeoPixelBus library wrapper template class that provides overall brightness control

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/

#ifndef NEOPIXELBRIGHTNESSBUS_H
#define NEOPIXELBRIGHTNESSBUS_H

#include "NeoPixelBus.h"

class NeoPixelBrightnessBus : public NeoPixelBus
{
private:
    void ConvertColor(typename NeoGrbFeature::ColorObject* color)
    {
        if (_brightness)
        {
            uint8_t* ptr = (uint8_t*) color;
            uint8_t* ptrEnd = ptr + NeoGrbFeature::PixelSize;

            while (ptr != ptrEnd)
            {
                uint16_t value = *ptr;
                *ptr++ = (value * _brightness) >> 8;
            }
        }
    }

    void RecoverColor(typename NeoGrbFeature::ColorObject* color) const
    {
        if (_brightness)
        {
            uint8_t* ptr = (uint8_t*) color;
            uint8_t* ptrEnd = ptr + NeoGrbFeature::PixelSize;

            while (ptr != ptrEnd)
            {
                uint16_t value = *ptr;
                *ptr++ = (value << 8) / _brightness;
            }
        }
    }

public:
    NeoPixelBrightnessBus(uint16_t countPixels, uint8_t pin) :
        NeoPixelBus(countPixels, pin),
        _brightness(0)
    {
    }

    void SetBrightness(uint8_t brightness)
    {
        // Due to using fixed point math, we modifiy the brightness
        // before storing making the math faster
        uint8_t newBrightness = brightness + 1;

        // Only update if there is a change
        if (newBrightness != _brightness) 
        { 
            // calculate a scale to modify from old brightness to new brightness
            //
            uint8_t oldBrightness = _brightness - 1; // unmodify brightness value
            uint16_t scale;

            if (oldBrightness == 0)
            {
                scale = 0; // Avoid divide by 0
            }
            else if (brightness == 255)
            {
                scale = 65535 / oldBrightness;
            }
            else
            {
                scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
            }

            // re-scale existing pixels
            //
            uint8_t* ptr = this->Pixels();
            uint8_t* ptrEnd = ptr + this->PixelsSize();
            while (ptr != ptrEnd)
            {
                uint16_t value = *ptr;
                *ptr++ = (value * scale) >> 8;
            }

            _brightness = newBrightness;
            this->Dirty();
        }
    }

    uint8_t GetBrightness() const
    {
        return _brightness - 1;
    }

    void SetPixelColor(uint16_t indexPixel, typename NeoGrbFeature::ColorObject color)
    {
        ConvertColor(&color);
        NeoPixelBus::SetPixelColor(indexPixel, color);
    }

    typename NeoGrbFeature::ColorObject GetPixelColor(uint16_t indexPixel) const
    {
        typename NeoGrbFeature::ColorObject color = NeoPixelBus::GetPixelColor(indexPixel);
        RecoverColor(&color);
        return color;
    }

    void ClearTo(typename NeoGrbFeature::ColorObject color)
    {
        ConvertColor(&color);
        NeoPixelBus::ClearTo(color);
    };

    void ClearTo(typename NeoGrbFeature::ColorObject color, uint16_t first, uint16_t last)
    {
        ConvertColor(&color);
        NeoPixelBus::ClearTo(color, first, last);
    }


protected:
    uint8_t _brightness;
};

#endif
