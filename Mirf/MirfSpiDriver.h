#ifndef __MIRF_SPI_DRIVER
#define __MIRF_SPI_DRIVER

class MirfSpiDriver {
	public:
		virtual uint8_t transfer(uint8_t data) = 0;

		virtual void begin() = 0;
		virtual void end() = 0;
};

#endif
