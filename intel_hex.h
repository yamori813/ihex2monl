/* Intel .hex file parser
 * Alex Hirzel <alex@hirzel.us>
 * May 27, 2012
 *
 * http://github.com/alhirzel/intel_hex_files
 */

#ifndef _INTEL_HEX_H_
#define _INTEL_HEX_H_

#include <stdint.h>

/* Different possible records for Intel .hex files. */
enum intel_hex_record_type {
	DATA_RECORD = 0x00,
	EOF_RECORD  = 0x01, /* End Of File */
	ESA_RECORD  = 0x02, /* Extended Segment Address */
	SSA_RECORD  = 0x03, /* Start Segment Address */
	ELA_RECORD  = 0x04, /* Extended Linear Address */
	SLA_RECORD  = 0x05  /* Start Linear Address */
};

/* Represents an Intel .hex record. This structure is populated by
 * slurp_next_intel_hex_record and must be manually allocated for use with the
 * aforementioned function. */
struct intel_hex_record {
	uint8_t byte_count;
	uint16_t address;
	uint8_t record_type;
	uint8_t data[UINT8_MAX + 1];
	uint8_t checksum;
};

/* For the purposes of this implementation, an "error" is an exception to state
 * machine execution (including healthy, normal termination). */
enum intel_hex_slurp_error {

	/* A non-error condition used within the state machine. Also the correct
	 * return value for slurp_next_intel_hex_record for successful 'slurps'. */
	SLURP_ERROR_NONE,

	/* A non-error condition used within the state machine to signify end of
	 * parsing. */
	SLURP_ERROR_DONE,

	/* Miscellaneous parsing error. */
	SLURP_ERROR_PARSING,

	/* Invalid hexadecimal character encountered (other than line endings and
	 * record-delimiting colon (':') characters). Unable to extract meaningful
	 * data. */
	SLURP_ERROR_NON_HEX_CHARACTER,

	/* Individual record checksum failure. */
	SLURP_ERROR_INVALID_CHECKSUM,

	/* Invalid Extended Segment Address (ESA) Record encountered with nonzero
	 * address, byte count other than 2 or a data payload that does not have a
	 * zero (0) as the last digit. */
	SLURP_ERROR_ESA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_ESA_BYTE_COUNT_NOT_TWO,
	SLURP_ERROR_ESA_DATA_FORMAT_INVALID,

	/* Invalid Start Segment Address (SSA) Record encountered with nonzero
	 * address or byte count other than 4. */
	SLURP_ERROR_SSA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_SSA_BYTE_COUNT_NOT_FOUR,

	/* Invalid Extended Linear Address (ELA) Record encountered with nonzero
	 * address or byte count other than 2. */
	SLURP_ERROR_ELA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_ELA_BYTE_COUNT_NOT_TWO,

	/* Invalid Start Linear Address (SLA) Record encountered with nonzero
	 * address or byte count other than 4. */
	SLURP_ERROR_SLA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_SLA_BYTE_COUNT_NOT_FOUR
};

/* Returns valid, populated 'intel_hex_record' structure. Fails on any error
 * including invalid checksum.
 *
 * Arguments: slurp_char - function to retrieve next character
 *            r          - allocated 'intel_hex_record' object to overwrite
 *
 * Return:    Either SLURP_ERROR_NONE in the case of success or a different item
 *            of intel_hex_slurp_error in the case of failure.
 */
enum intel_hex_slurp_error slurp_next_intel_hex_record(char (*slurp_char)(void), struct intel_hex_record *r);

#endif /* _INTEL_HEX_H_ */

