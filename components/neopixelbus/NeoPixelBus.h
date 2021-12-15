/*-------------------------------------------------------------------------
NeoPixel library 

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

#ifndef NEOPIXELBUS_H
#define NEOPIXELBUS_H

// #include <Arduino.h>	// idf-port

// some platforms do not define this standard progmem type for some reason
//
#ifndef PGM_VOID_P
#define PGM_VOID_P const void *
#endif

// '_state' flags for internal state
#define NEO_DIRTY   0x80 // a change was made to pixel data that requires a show

#include "internal/RgbColor.h"
#include "internal/HslColor.h"
#include "internal/HsbColor.h"
#include "internal/HtmlColor.h"
#include "internal/RgbwColor.h"

#include "internal/NeoColorFeatures.h"  // Needed

#include "internal/NeoBufferContext.h"  // Needed

#include "internal/NeoEsp32I2sMethod.h" // Needed

#include "internal/NeoEsp32RmtMethod.h"

class NeoPixelBus
{
public:
    // Constructor: number of LEDs, pin number
    // NOTE:  Pin Number maybe ignored due to hardware limitations of the method.
   
    NeoPixelBus(uint16_t countPixels, uint8_t pin) :
        _countPixels(countPixels),
        _state(0),
        _method(pin, countPixels, NeoGrbFeature::PixelSize)
    {
    }

    ~NeoPixelBus()
    {
    }

    operator NeoBufferContext<NeoGrbFeature>()
    {
        Dirty(); // we assume you are playing with bits
        return NeoBufferContext<NeoGrbFeature>(_method.getPixels(), _method.getPixelsSize());
    }

    void Begin()
    {
        _method.Initialize();
        Dirty();
    }

    void Show(bool maintainBufferConsistency = true)
    {
        if (!IsDirty())
        {
            return;
        }

		_method.Update(maintainBufferConsistency);
		//_method.Update();

        ResetDirty();
    }

    inline bool CanShow() const
    { 
        return _method.IsReadyToUpdate();
    };

    bool IsDirty() const
    {
        return  (_state & NEO_DIRTY);
    };

    void Dirty()
    {
        _state |= NEO_DIRTY;
    };

    void ResetDirty()
    {
        _state &= ~NEO_DIRTY;
    };

    uint8_t* Pixels() 
    {
        return _method.getPixels();
    };

    size_t PixelsSize() const
    {
        return _method.getPixelsSize();
    };

    size_t PixelSize() const
    {
        return NeoGrbFeature::PixelSize;
    };

    uint16_t PixelCount() const
    {
        return _countPixels;
    };

    void SetPixelColor(uint16_t indexPixel, typename NeoGrbFeature::ColorObject color)
    {
        if (indexPixel < _countPixels)
        {
            NeoGrbFeature::applyPixelColor(_method.getPixels(), indexPixel, color);
            Dirty();
        }
    };

    typename NeoGrbFeature::ColorObject GetPixelColor(uint16_t indexPixel) const
    {
        if (indexPixel < _countPixels)
        {
            return NeoGrbFeature::retrievePixelColor(_method.getPixels(), indexPixel);
        }
        else
        {
            // Pixel # is out of bounds, this will get converted to a 
            // color object type initialized to 0 (black)
            return 0;
        }
    };

    void ClearTo(typename NeoGrbFeature::ColorObject color)
    {
        uint8_t temp[NeoGrbFeature::PixelSize]; 
        uint8_t* pixels = _method.getPixels();

        NeoGrbFeature::applyPixelColor(temp, 0, color);

        NeoGrbFeature::replicatePixel(pixels, temp, _countPixels);

        Dirty();
    };

    void ClearTo(typename NeoGrbFeature::ColorObject color, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first <= last)
        {
            uint8_t temp[NeoGrbFeature::PixelSize];
            uint8_t* pixels = _method.getPixels();
            uint8_t* pFront = NeoGrbFeature::getPixelAddress(pixels, first);

            NeoGrbFeature::applyPixelColor(temp, 0, color);

            NeoGrbFeature::replicatePixel(pFront, temp, last - first + 1);

            Dirty();
        }
    }

    void RotateLeft(uint16_t rotationCount)
    {
        if ((_countPixels - 1) >= rotationCount)
        {
            _rotateLeft(rotationCount, 0, _countPixels - 1);
        }
    }

    void RotateLeft(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first < last &&
            (last - first) >= rotationCount)
        {
            _rotateLeft(rotationCount, first, last);
        }
    }

    void ShiftLeft(uint16_t shiftCount)
    {
        if ((_countPixels - 1) >= shiftCount)
        {
            _shiftLeft(shiftCount, 0, _countPixels - 1);
            Dirty();
        }
    }

    void ShiftLeft(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels && 
            last < _countPixels && 
            first < last &&
            (last - first) >= shiftCount)
        {
            _shiftLeft(shiftCount, first, last);
            Dirty();
        }
    }

    void RotateRight(uint16_t rotationCount)
    {
        if ((_countPixels - 1) >= rotationCount)
        {
            _rotateRight(rotationCount, 0, _countPixels - 1);
        }
    }

    void RotateRight(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first < last &&
            (last - first) >= rotationCount)
        {
            _rotateRight(rotationCount, first, last);
        }
    }

    void ShiftRight(uint16_t shiftCount)
    {
        if ((_countPixels - 1) >= shiftCount)
        {
            _shiftRight(shiftCount, 0, _countPixels - 1);
            Dirty();
        }
    }

    void ShiftRight(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first < last &&
            (last - first) >= shiftCount)
        {
            _shiftRight(shiftCount, first, last);
            Dirty();
        }
    }
    
    void SwapPixelColor(uint16_t indexPixelOne, uint16_t indexPixelTwo)
    {
        auto colorOne = GetPixelColor(indexPixelOne);
        auto colorTwo = GetPixelColor(indexPixelTwo);

        SetPixelColor(indexPixelOne, colorTwo);
        SetPixelColor(indexPixelTwo, colorOne);
    };
 
protected:
    const uint16_t _countPixels; // Number of RGB LEDs in strip

    uint8_t _state;     // internal state
	NeoEsp32I2s1800KbpsMethod _method;	// uncomment _method.Update(maintainBufferConsistency);
	//NeoEsp32Rmt1800KbpsMethod _method;		// change _method.Update(maintainBufferConsistency); to _method.Update();

    void _rotateLeft(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        // store in temp
        uint8_t temp[rotationCount * NeoGrbFeature::PixelSize];
        uint8_t* pixels = _method.getPixels();

        uint8_t* pFront = NeoGrbFeature::getPixelAddress(pixels, first);

        NeoGrbFeature::movePixelsInc(temp, pFront, rotationCount);

        // shift data
        _shiftLeft(rotationCount, first, last);

        // move temp back
        pFront = NeoGrbFeature::getPixelAddress(pixels, last - (rotationCount - 1));
        NeoGrbFeature::movePixelsInc(pFront, temp, rotationCount);

        Dirty();
    }

    void _shiftLeft(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        uint16_t front = first + shiftCount;
        uint16_t count = last - front + 1;

        uint8_t* pixels = _method.getPixels();
        uint8_t* pFirst = NeoGrbFeature::getPixelAddress(pixels, first);
        uint8_t* pFront = NeoGrbFeature::getPixelAddress(pixels, front);

        NeoGrbFeature::movePixelsInc(pFirst, pFront, count);

        // intentional no dirty
    }

    void _rotateRight(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        // store in temp
        uint8_t temp[rotationCount * NeoGrbFeature::PixelSize];
        uint8_t* pixels = _method.getPixels();

        uint8_t* pFront = NeoGrbFeature::getPixelAddress(pixels, last - (rotationCount - 1));

        NeoGrbFeature::movePixelsDec(temp, pFront, rotationCount);

        // shift data
        _shiftRight(rotationCount, first, last);

        // move temp back
        pFront = NeoGrbFeature::getPixelAddress(pixels, first);
        NeoGrbFeature::movePixelsDec(pFront, temp, rotationCount);

        Dirty();
    }

    void _shiftRight(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        uint16_t front = first + shiftCount;
        uint16_t count = last - front + 1;

        uint8_t* pixels = _method.getPixels();
        uint8_t* pFirst = NeoGrbFeature::getPixelAddress(pixels, first);
        uint8_t* pFront = NeoGrbFeature::getPixelAddress(pixels, front);

        NeoGrbFeature::movePixelsDec(pFront, pFirst, count);
        // intentional no dirty
    }
};

#endif
