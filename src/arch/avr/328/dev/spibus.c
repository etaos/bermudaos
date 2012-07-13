/*
 *  BermudaOS - SPI bus driver
 *  Copyright (C) 2012   Michel Megens
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(__SPI__) || defined(__DOXYGEN__)

//! \file arch/avr/328/dev/spibus.c SPI hardware bus

#include <bermuda.h>

#include <fs/vfile.h>

#include <sys/thread.h>
#include <sys/mem.h>
#include <sys/events/event.h>

#include <dev/dev.h>
#include <dev/spibus.h>

#include <arch/avr/io.h>
#include <arch/avr/interrupts.h>
#include <arch/avr/328/dev/spireg.h>
#include <arch/avr/328/dev/spibus.h>

#ifdef __EVENTS__
THREAD *BermudaSPI0TransferQueue = SIGNALED;
THREAD *BermudaSPI0Mutex = SIGNALED;
#endif

/**
 * \var BermudaSPI0HardwareIO
 * \brief SPI0 hardware registers.
 * 
 * Hardware I/O registers for the SPI 0 bus.
 */
static HWSPI BermudaSPI0HardwareIO = {
        .spcr = &SPI_CTRL,
        .spsr = &SPI_STATUS,
        .spdr = &SPI_DATA,
};

/**
 * \var BermudaSpiHardwareCtrl
 * \brief Hardware SPI interface.
 * 
 * Interface for hardware SPI on the avr ATmega328.
 */
static SPICTRL BermudaSpiHardwareCtrl = {
        .transfer = &BermudaHardwareSpiTransfer,
        .set_mode = &BermudaSpiSetMode,
        .set_rate = &BermudaSpiSetRate,
        .select   = &BermudaHardwareSpiSelect,
        .deselect = &BermudaHardwareSpiDeselect,
};

/**
 * \var BermudaSpi0HardwareBus
 * \brief SPI0 hardware bus.
 * \see SPI0
 * 
 * Definition of the SPI hardware bus 0.
 */
SPIBUS BermudaSpi0HardwareBus = {
#ifdef __EVENTS__
        .queue = (void*)&BermudaSPI0TransferQueue,
#else
        .queue = NULL,
#endif
        .ctrl  = &BermudaSpiHardwareCtrl,
        .io    = &BermudaSPI0HardwareIO,
        .mode  = (BERMUDA_SPI_MODE0 | BERMUDA_SPI_MODE_UPDATE | BERMUDA_SPI_RATE_UPDATE),
        .rate  = F_CPU/128,
        .cs    = 0,
};

/**
 * \brief Initialize hardware SPI bus 0.
 * \param dev Device to initialize.
 * \return 0 On success, -1 otherwise (allocation of memory failed).
 * \see _device
 * \see SPI0
 * \see _vfile
 * 
 * When this function returns 0, the device is ready to use.
 */
PUBLIC int BermudaSPI0HardwareInit()
{
	int rc = 0;
	HWSPI *hwio = &BermudaSPI0HardwareIO;

	// enable the spi interface
	SPI_DDR |= (SPI_SCK | SPI_MOSI | SPI_SS);
	SPI_PORT &= ~(SPI_SCK | SPI_MOSI);
	SPI_PORT |= SPI_SS;

#ifdef __EVENTS__
	*(hwio->spcr) |= SPI_ENABLE | SPI_MASTER_ENABLE | SPI_IRQ_ENABLE;
#else
	*(hwio->spcr) |= SPI_ENABLE | SPI_MASTER_ENABLE;
#endif
        
	return rc;
}

/**
 * \brief Select a chip.
 * \param bus SPI bus.
 * \todo Update SPI configuration.
 * \note The SPI bus needs to be set for a specific device first.
 * 
 * It will not only select the given pin, it will also update the SPI configuration,
 * if needed.
 */
PRIVATE WEAK void BermudaHardwareSpiSelect(SPIBUS *bus)
{
	HWSPI *hw = (HWSPI*)bus->io;
	if(bus->mode & BERMUDA_SPI_RATE_UPDATE) {
		BermudaSpiRateToHwBits(&bus->rate, (bus->mode & BERMUDA_SPI_RATE2X) >> BERMUDA_SPI_X2_SHIFT);
		bus->mode &= ~(BERMUDA_SPI_RATE_UPDATE | BERMUDA_SPI_RATE2X);

		// config rate to hardware
		*(hw->spcr) = (*(hw->spcr) & (~B11)) | (bus->rate & B11);
		*(hw->spsr) = (*(hw->spsr) & (~B1)) | ((bus->rate & B100) >> 2);
	}
	if(bus->mode & BERMUDA_SPI_MODE_UPDATE) {
		bus->mode &= ~BERMUDA_SPI_MODE_UPDATE;

		// set the mode
		*(hw->spcr) = (*(hw->spcr) & (~B1100)) | ((bus->mode & B11) << SPI_MODE_SHIFT);
	}

	BermudaSetPinMode(bus->cs, OUTPUT);
	BermudaDigitalPinWrite(bus->cs, LOW);
}

/**
 * \brief Deselect a chip.
 * \param bus SPI bus.
 * \note It is save to remove the bus/device mutex after the deselect signal.
 */
PRIVATE WEAK void BermudaHardwareSpiDeselect(SPIBUS *bus)
{
	BermudaDigitalPinWrite(bus->cs, HIGH);
}

/**
 * \brief Write data to the given SPI bus.
 * \param bus SPI bus to write to.
 * \param data Data to write.
 * \note If events are enabled, the system will wait inactive for an interrupt.
 *
 * The given data will be written to the SPI bus.
 */
PRIVATE WEAK int BermudaHardwareSpiTransfer(SPIBUS* bus, const uint8_t *tx, uint8_t *rx,
	uptr len, unsigned int tmo)
{
	uptr idx = 0;
	HWSPI *io = bus->io;
	int rc = 0;
	
	for(; idx < len; idx++) {
		*(io->spdr) = tx[idx];
#ifdef __EVENTS__
		if(BermudaEventWait((volatile THREAD**)bus->queue, tmo) == -1) {
			rc -= 1;
			rx[idx] = 0xFF;
		}
		else {
			rx[idx] = *(io->spdr);
			rc += 1;
		}
#else
		while(!(*(io->spsr) & BIT(SPIF)));
		rx[idx] = *(io->spdr);
		rc += 1;
#endif
	}

	return rc;
}

/**
 * \brief Converts an SPI rate to hardware configure bits.
 * \param rate_select Pointer to the desired rate.
 * \param spi2x When spi2x != 0, the SPI bus will be confiured with a 2X rate.
 */
PRIVATE WEAK void BermudaSpiRateToHwBits(unsigned long *rate_select, unsigned char spi2x)
{
	unsigned long pres = BermudaSpiRateToPrescaler(F_CPU, *rate_select, SPI_MAX_PRES);
	unsigned char hw = 0;

	switch(pres) {
	case 8:
	case 16:
		hw = B1;
		break;
		
	case 32:
		hw = B10;
		break;

	case 64:
		if(spi2x) {
			hw = B11;
		}
		else {
			hw = B10;
		}
		break;

	case 128:
		hw = B11;
		break;

	default:
		hw = 0;
		break;
	}

	hw |= spi2x;
	*rate_select = hw;
}

/**
 * \brief Update the SPI rate.
 * \param bus SPI bus to update.
 * \param rate New SPI rate.
 * \note The rate will not be synchronized with the hardware. The
 *       next SPI select call, with this bus, will sync with the hardware.
 * \warning It is wise to allocate the associated SPI device first.
 */
PRIVATE WEAK void BermudaSpiSetRate( SPIBUS *bus, uint32_t rate )
{
	bus->rate = rate;
	bus->mode |= BERMUDA_SPI_RATE_UPDATE;
}

/**
 * \brief Update the SPI mode settings.
 * \param bus SPI bus to update.
 * \param mode New SPI mode setting.
 * \note The mode will not be synchronized with the hardware. The
 *       next SPI select call, with this bus, will sync with the hardware.
 * \warning It is wise to allocate the associated SPI device first.
 */
PRIVATE WEAK void BermudaSpiSetMode( SPIBUS *bus, unsigned char mode )
{
	bus->mode = ((bus->mode & (~0xFF)) | mode) | BERMUDA_SPI_MODE_UPDATE;
}

#ifdef __EVENTS__
SIGNAL(SPI_STC_vect)
{
	BermudaEventSignalFromISR((volatile THREAD**)(&BermudaSpi0HardwareBus)->queue);
}
#endif /* __EVENTS__ */
#endif /* __SPI__ */