/**
 * Mirf
 *
 * Additional bug fixes and improvements
 *  11/03/2011:
 *   Switched spi library.
 *  07/13/2010:
 *   Added example to read a register
 *  11/12/2009:
 *   Fix dataReady() to work correctly
 *   Renamed keywords to keywords.txt ( for IDE ) and updated keyword list
 *   Fixed client example code to timeout after one second and try again
 *    when no response received from server
 * By: Nathan Isburgh <nathan@mrroot.net>
 * $Id: mirf.cpp 67 2010-07-13 13:25:53Z nisburgh $
 *
 *
 * An Ardunio port of:
 * http://www.tinkerer.eu/AVRLib/nRF24L01
 *
 * Significant changes to remove depencence on interupts and auto ack support.
 *
 * Aaron Shrimpton <aaronds@gmail.com>
 *
 */

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

    $Id: mirf.cpp 67 2010-07-13 13:25:53Z nisburgh $
*/

#include "Mirf.h"

Nrf24l Mirf = Nrf24l();

Nrf24l::Nrf24l() :
	PTX(0),
	cePin(8),
	csnPin(7),
	channel(1),
	payload(16),
	baseConfig(_BV(EN_CRC) & ~_BV(CRCO)),
	spi(NULL)
{
}

void Nrf24l::init()
{
	pinMode(cePin, OUTPUT);
	pinMode(csnPin, OUTPUT);

	ceLow();
	csnHi();

	// Initialize spi module
	spi->begin();
}


void Nrf24l::config()
{
	// Set RF channel
	configRegister(RF_CH,channel);

	// Set length of incoming payload 
	configRegister(RX_PW_P0, payload);
	configRegister(RX_PW_P1, payload);

	// Start receiver 
	powerUpRx();
	flushRx();
}

void Nrf24l::setRADDR(const uint8_t * addr)
{
	ceLow();
	writeRegister(RX_ADDR_P1, addr, mirf_ADDR_LEN);
	ceHi();
}

void Nrf24l::setTADDR(const uint8_t * addr)
{
	/*
	 * RX_ADDR_P0 must be set to the sending addr for auto ack to work.
	 */
	writeRegister(RX_ADDR_P0, addr, mirf_ADDR_LEN);
	writeRegister(TX_ADDR, addr, mirf_ADDR_LEN);
}

bool Nrf24l::dataReady()
{
	// See note in getData() function - just checking RX_DR isn't good enough
	uint8_t status = getStatus();

	// We can short circuit on RX_DR, but if it's not set, we still need
	// to check the FIFO for any pending packets
	if (status & _BV(RX_DR))
		return 1;

	return !rxFifoEmpty();
}

bool Nrf24l::rxFifoEmpty()
{
	uint8_t fifoStatus;

	readRegister(FIFO_STATUS, &fifoStatus, sizeof(fifoStatus));

	return (fifoStatus & _BV(RX_EMPTY));
}

void Nrf24l::getData(uint8_t * data)
{
	nrfSpiWrite(R_RX_PAYLOAD, data, true, payload); // Read payload

	// NVI: per product spec, p 67, note c:
	//  "The RX_DR IRQ is asserted by a new packet arrival event. The procedure
	//  for handling this interrupt should be: 1) read payload through SPI,
	//  2) clear RX_DR IRQ, 3) read FIFO_STATUS to check if there are more 
	//  payloads available in RX FIFO, 4) if there are more data in RX FIFO,
	//  repeat from step 1)."
	// So if we're going to clear RX_DR here, we need to check the RX FIFO
	// in the dataReady() function
	configRegister(STATUS, _BV(RX_DR));   // Reset status register
}

void Nrf24l::configRegister(uint8_t reg, uint8_t value)
{
	writeRegister(reg, &value, 1);
}

void Nrf24l::readRegister(uint8_t reg, uint8_t * value, uint8_t len)
{
    nrfSpiWrite((R_REGISTER | (REGISTER_MASK & reg)), value, true, len);
}

void Nrf24l::writeRegister(uint8_t reg, const uint8_t * value, uint8_t len)
{
	nrfSpiWrite((W_REGISTER | (REGISTER_MASK & reg)), const_cast<uint8_t*>(value), false, len);
}


void Nrf24l::send(const uint8_t * value)
{
	while (PTX) {
		uint8_t status = getStatus();

		if((status & ((1 << TX_DS)  | (1 << MAX_RT)))){
			PTX = 0;
			break;
		}
	}                  // Wait until last paket is send

	ceLow();
	
	powerUpTx();       // Set to transmitter mode , Power up
	flushTx();
		
	nrfSpiWrite(W_TX_PAYLOAD, const_cast<uint8_t*>(value), false, payload);   // Write payload

	PTX = 1;
	ceHi();                     // Start transmission
	ceLow();
}

bool Nrf24l::isSending()
{
	if (PTX) {
		uint8_t status = getStatus();

		/*
		 *  if sending successful (TX_DS) or max retries exceded (MAX_RT).
		 */
		if((status & ((1 << TX_DS)  | (1 << MAX_RT)))){
			powerUpRx();
			return false;
		}
		else {
			return true;
		}
	}

	return false;
}

uint8_t Nrf24l::getStatus()
{
	/* Initialize with NOP so we get the first byte read back. */
	uint8_t rv = NOP;
	readRegister(STATUS, &rv, 1);
	return rv;
}

void Nrf24l::flushTx()
{
	nrfSpiWrite(FLUSH_TX);
}

void Nrf24l::powerUpRx()
{
	PTX = 0;
	ceLow();

	configRegister(CONFIG, baseConfig | _BV(PWR_UP) | _BV(PRIM_RX));
	// Clear interrupt flags
	configRegister(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT)); 

	ceHi();
}

void Nrf24l::flushRx()
{
	nrfSpiWrite(FLUSH_RX);
}

void Nrf24l::powerUpTx()
{
	configRegister(CONFIG, baseConfig | _BV(PWR_UP) & ~_BV(PRIM_RX));
}

void Nrf24l::nrfSpiWrite(uint8_t reg, uint8_t *data, boolean readData, uint8_t len)
{
	csnLow();

	spi->transfer(reg);

	if (data) {
		for(uint8_t i = 0; i < len; ++i) {
			uint8_t readValue = spi->transfer(data[i]);

			if (readData) {
				data[i] = readValue;
			}
		}
	}

	csnHi();
}

void Nrf24l::ceHi()
{
	digitalWrite(cePin,HIGH);
}

void Nrf24l::ceLow()
{
	digitalWrite(cePin,LOW);
}

void Nrf24l::csnHi()
{
	digitalWrite(csnPin,HIGH);
}

void Nrf24l::csnLow()
{
	digitalWrite(csnPin,LOW);
}

void Nrf24l::powerDown()
{
	ceLow();

	configRegister(CONFIG, baseConfig);

	flushRx();
	flushTx();
}
