#include <nrf.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/sys/printk.h>
#include <zephyr/console/console.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/byteorder.h>

#define PDU_CRCINIT 0xdeadbeef
#define PDU_CRC_POLYNOMIAL (0x5b | (0x06 << 8) | (0x00UL << 16))

// NOTE: parts of the code are copied from: https://github.com/zephyrproject-rtos/zephyr
// Func hex2int copied from: https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int

uint32_t hex2int(char *hex)
{
	uint32_t val = 0;
	while (*hex) {
		uint8_t byte = *hex++;
		if (byte >= '0' && byte <= '9') {
			byte = byte - '0';
		} else if (byte >= 'a' && byte <= 'f') {
			byte = byte - 'a' + 10;
		} else if (byte >= 'A' && byte <= 'F') {
			byte = byte - 'A' + 10;
		}
		val = (val << 4) | (byte & 0xF);
	}
	return val;
}

int main(void)
{
	console_getline_init();

	printk("Select PHY: [1M, 2M]\n");
	char *input_phy = console_getline();
	if (strcmp(input_phy, "1M") == 0) {
		NRF_RADIO->MODE =
			(RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos) & RADIO_MODE_MODE_Msk;
	} else if (strcmp(input_phy, "2M") == 0) {
		NRF_RADIO->MODE =
			(RADIO_MODE_MODE_Ble_2Mbit << RADIO_MODE_MODE_Pos) & RADIO_MODE_MODE_Msk;
	} else {
		printk("ERROR: invalid PHY\n");
		return 0;
	}

	printk("Enter RF Channel: (between 0-39)\n");
	uint8_t input_chan = (uint8_t)atoi(console_getline());
	if (input_chan < 0 || input_chan > 39) {
		printk("ERROR: invalid RF channel\n");
		return 0;
	}
	switch (input_chan) {
	case 37:
		NRF_RADIO->FREQUENCY = 2;
		break;
	case 38:
		NRF_RADIO->FREQUENCY = 26;
		break;
	case 39:
		NRF_RADIO->FREQUENCY = 80;
		break;
	default:
		if (input_chan < 11) {
			NRF_RADIO->FREQUENCY = 4 + (input_chan * 2U);
		} else if (input_chan < 40) {
			NRF_RADIO->FREQUENCY = 28 + ((input_chan - 11) * 2U);
		}
		break;
	}
	NRF_RADIO->DATAWHITEIV = input_chan;

	printk("Enter Access Address: (e.g., 0xdeadbeef)\n");
	uint32_t input_aa = hex2int(console_getline());
	uint8_t aa[4];
	aa[3] = (input_aa & 0xFF000000) >> 24;
	aa[2] = (input_aa & 0x00FF0000) >> 16;
	aa[1] = (input_aa & 0x0000FF00) >> 8;
	aa[0] = input_aa & 0x000000FF;
	NRF_RADIO->PREFIX0 = aa[3];
	NRF_RADIO->BASE0 = (aa[2] << 24) | (aa[1] << 16) | (aa[0] << 8);

	printk("Starting Sniffer...\n");

	NRF_CLOCK->TASKS_HFCLKSTART = 1;
	while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {}

	NRF_RADIO->PCNF0 =
		((1 << RADIO_PCNF0_S0LEN_Pos) & RADIO_PCNF0_S0LEN_Msk) |
		((8 << RADIO_PCNF0_LFLEN_Pos) & RADIO_PCNF0_LFLEN_Msk) |
		((0 << RADIO_PCNF0_S1LEN_Pos) & RADIO_PCNF0_S1LEN_Msk) |
		((RADIO_PCNF0_PLEN_16bit << RADIO_PCNF0_PLEN_Pos) & RADIO_PCNF0_PLEN_Msk);

	NRF_RADIO->PCNF1 = 0x00000000UL;
	NRF_RADIO->PCNF1 &= ~(RADIO_PCNF1_MAXLEN_Msk | RADIO_PCNF1_STATLEN_Msk |
			      RADIO_PCNF1_BALEN_Msk | RADIO_PCNF1_ENDIAN_Msk);
	NRF_RADIO->PCNF1 |=
		((255 << RADIO_PCNF1_MAXLEN_Pos) & RADIO_PCNF1_MAXLEN_Msk) |
		((0 << RADIO_PCNF1_STATLEN_Pos) & RADIO_PCNF1_STATLEN_Msk) |
		((3 << RADIO_PCNF1_BALEN_Pos) & RADIO_PCNF1_BALEN_Msk) |
		((RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos) & RADIO_PCNF1_ENDIAN_Msk);

	NRF_RADIO->RXADDRESSES = (RADIO_RXADDRESSES_ADDR0_Enabled << RADIO_RXADDRESSES_ADDR0_Pos);

	NRF_RADIO->CRCCNF =
		((RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos) & RADIO_CRCCNF_SKIPADDR_Msk) |
		((RADIO_CRCCNF_LEN_Disabled << RADIO_CRCCNF_LEN_Pos) & RADIO_CRCCNF_LEN_Msk);
	NRF_RADIO->CRCPOLY = PDU_CRC_POLYNOMIAL;
	NRF_RADIO->CRCINIT = PDU_CRCINIT;

	volatile uint8_t packet[255];
	NRF_RADIO->PACKETPTR = (uint32_t)&packet[0];

	NRF_RADIO->PCNF1 &= ~RADIO_PCNF1_WHITEEN_Msk;
	NRF_RADIO->PCNF1 |= (1 << RADIO_PCNF1_WHITEEN_Pos) & RADIO_PCNF1_WHITEEN_Msk;

	NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
			    (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);

	// TODO: implement support for device address matching via getline()
	uint8_t bdaddr[6];
	bdaddr[5] = 0x2a; // 2a:2f:c1:f2:a4:e3 (example)
	bdaddr[4] = 0x2f;
	bdaddr[3] = 0xc1;
	bdaddr[2] = 0xf2;
	bdaddr[1] = 0xa4;
	bdaddr[0] = 0xe3;

	uint8_t index = 0;
	NRF_RADIO->DAB[index] = ((uint32_t)bdaddr[3] << 24) | ((uint32_t)bdaddr[2] << 16) |
				((uint32_t)bdaddr[1] << 8) | bdaddr[0];
	NRF_RADIO->DAP[index] = ((uint32_t)bdaddr[5] << 8) | bdaddr[4];
	NRF_RADIO->DACNF = 1 << RADIO_DACNF_ENA0_Pos;
	NRF_RADIO->DACNF = 1 << RADIO_DACNF_TXADD0_Pos;

	while (1) {
		NRF_RADIO->TASKS_RXEN = 1;
		while (NRF_RADIO->EVENTS_DISABLED == 0) {}
		NRF_RADIO->EVENTS_DISABLED = 0;

		uint8_t devmatch = NRF_RADIO->EVENTS_DEVMATCH;
		uint8_t devmiss = NRF_RADIO->EVENTS_DEVMISS;
		uint8_t rxmatch = NRF_RADIO->RXMATCH;

		printk("DEVMATCH: %u | DEVMISS: %u | RXMATCH: %x | CRCOK: %u | Packet: ", devmatch,
		       devmiss, rxmatch, NRF_RADIO->EVENTS_CRCOK);
		for (uint8_t i = 0; i < 255; i++) {
			printk("%02x ", packet[i]);
		}
		printk("\n");
	}
}