#include "rtl_433.h"

static uint8_t mirror_u8(uint8_t b) {
    uint8_t b_flip_nibbles = ((b & 0x11) << 3) | ((b & 0x22) << 1) |
    		((b & 0x44) >> 1) | ((b & 0x88) >> 3);
    return  ((b_flip_nibbles & 0xf0) >> 4) |
    		((b_flip_nibbles & 0x0f) << 4);
}

/* NOTE:
 * the lower 16bits of what is shown in the OWL display are missing (and zeroed) here.
 * A beer to who finds them in the data received. I could not. */
static float parse_power (uint8_t * msg)
{
	uint32_t energy_f = (mirror_u8(msg[6]) & 0xF0);
	energy_f += mirror_u8(msg[7]) << 8;

	return (float)energy_f;
}

/* NOTE:
 * this is not the OregonScientific like rolling code. This part changes after reset.
 * It might include flags and or other info battery --> %%%% TO BE COMPLETED %%%%. */
static uint32_t parse_rolling_code (uint8_t * msg)
{
	/* this changes after reset. Might be it  --> TO BE COMPLETED */
	uint32_t rc = (mirror_u8(msg[4]) & 0xF0)>>4;
	rc = (rc << 8) + mirror_u8(msg[5]);
	rc = (rc << 4) + (mirror_u8(msg[6]) & 0x0F);

	return rc;
}

static int
owl_parser (uint8_t bb[BITBUF_ROWS][BITBUF_COLS], int16_t bits_per_row[BITBUF_ROWS])
{
	fprintf (stderr, "OWL CM180 sensor: RC=0x%04X, kW=%.1f\n",
			parse_rolling_code(bb[0]),
			parse_power(bb[0])
			);

	return 0;
}

static int owl_callback (uint8_t bb[BITBUF_ROWS][BITBUF_COLS], int16_t bits_per_row[BITBUF_ROWS])
{

	/* NOTE: this misses CHECKSUM --> might give wrong readings */

	/* not sure this is right/enough. This does not change after reset on CM180 sensor
	 * --> I take it as distinctive of CM180*/
	if (bb[0][3] != 0x46)
		return 0;

	/* DEBUG */
/*
	int i;
	fprintf(stderr, "\nAS RECEIVED: ");
	for (i = 0 ; i < BITBUF_COLS ; i++) {
		fprintf (stderr, "%02X ", bb[0][i]);
	}
	fprintf (stderr,"\n");
	fprintf(stderr, "MIRRORED     : ");
	for (i = 0 ; i < BITBUF_COLS ; i++) {
		fprintf (stderr, "%02X ", mirror_u8(bb[0][i]));
	}
	fprintf (stderr,"\n");
*/
	/* parse data */
	return owl_parser (bb, bits_per_row);
}

r_device owl = {
    /* .id             = */ 15,
    /* .name           = */ "OWL Intelligent Wireless Energy Sensor",
    /* .modulation     = */ OOK_MANCHESTER,
    /* .short_limit    = */ 125,
    /* .long_limit     = */ 0, // not used
    /* .reset_limit    = */ 600,
    /* .json_callback  = */ &owl_callback,
};
