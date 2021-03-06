/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#ifndef _MIRF_H_
#define _MIRF_H_

#include <Arduino.h>

#include "nRF24L01.h"
#include "MirfSpiDriver.h"

// Nrf24l settings

#define mirf_ADDR_LEN	5

class Nrf24l {
	public:
		Nrf24l();

		/**
		 * Initializes pins to communicate with the MiRF module.
		 * Should be called in the early initializing phase at startup.
		 */
		void init();

		/**
		 * Sets the important registers in the MiRF module and powers the module
		 * in receiving mode. Channel and payload must be set now.
		 */
		void config();

		/**
		 * Sends a data package to the default address.
		 * Be sure to send the correct amount of bytes as configured as payload
		 * on the receiver.
		 */
		void send(const uint8_t *value);

		/**
		 * @brief Sets the receiving address.
		 */
		void setRADDR(const uint8_t * addr);

		/**
		 * @brief Sets the transmitting address.
		 */
		void setTADDR(const uint8_t * addr);

		/**
		 * @brief Checks if data is available for reading.
		 */
		bool dataReady();

		/**
		 * @brief Checks if the chip is currently sending data.
		 */
		bool isSending();

		bool rxFifoEmpty();

		/**
		 * Reads payload bytes (if available - see dataReady()).
		 *
		 * @param data A byte-array in which the read data will be stored.
		 */
		void getData(uint8_t * data);

		uint8_t getStatus();

		/**
		 * Clocks only one byte into the given MiRF register
		 *
		 * @param reg The register which is written.
		 * @param value The value which is written to the register.
		 */
		void configRegister(uint8_t reg, uint8_t value);

		/**
		 * @brief Reads an array of bytes from the given start position in the MiRF registers.
		 */
		void readRegister(uint8_t reg, uint8_t * value, uint8_t len);

		/**
		 * @brief Writes an array of bytes into inte the MiRF registers.
		 */
		void writeRegister(uint8_t reg, const uint8_t * value, uint8_t len);

		void flushTx();
		void powerUpRx();
		void powerUpTx();
		void powerDown();
		
		void nrfSpiWrite(uint8_t reg, uint8_t *data = 0, boolean readData = false, uint8_t len = 0);

		void csnHi();
		void csnLow();

		void ceHi();
		void ceLow();
		void flushRx();

		/**
		 * In sending mode.
		 */
		uint8_t PTX;

		/**
		 * CE Pin controls RX / TX, default 8.
		 */
		uint8_t cePin;

		/**
		 * CSN Pin Chip Select Not, default 7.
		 */
		uint8_t csnPin;

		/**
		 * Channel 0 - 127 or 0 - 84 in the US.
		 */
		uint8_t channel;

		/**
		 * Payload width in bytes default 16 max 32.
		 */
		uint8_t payload;

		/**
		 * The base config register.
		 * When required PWR_UP and/or PRIM_RX will be OR'ed with this.
		 * 
		 * NOTE: Use "_BV(EN_CRC) | _BV(CRCO)" here if you want to
		 *       connect to a device using the RF24 library.
		 */
		uint8_t baseConfig;

		/**
		 * Spi interface (must extend spi).
		 */
		MirfSpiDriver *spi;
};

extern Nrf24l Mirf;

#endif /* _MIRF_H_ */
