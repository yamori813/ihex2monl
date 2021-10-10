/* Intel .hex file parser (exmaple program)
 * Alex Hirzel <alex@hirzel.us>
 * May 27, 2012
 *
 * http://github.com/alhirzel/intel_hex_files
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "intel_hex.h"

struct intel_hex_record r;

FILE *ihexfp;

int size = 0;
int start;
unsigned char mem[1024*32];

char my_slurp_char(void) {
	return getc(ihexfp);
}

//int main(int argc, char *argv[]) {
int readihex(FILE *fp){
	int offset;
	enum intel_hex_slurp_error err;

	ihexfp = fp;

	do {
		err = slurp_next_intel_hex_record(&my_slurp_char, &r);

		if (SLURP_ERROR_NONE != err) {
			printf("Got error 0x%02X, aborting due to invalid record!", err);
			exit(1);
		}

		switch (r.record_type) {
			case DATA_RECORD:
				if (size == 0) {
					start = r.address;
				}
				offset = r.address - start;
				size = offset + r.byte_count;
				memcpy(mem + offset, r.data, r.byte_count);
				
//				printf("Got data record with length %u\n", r.byte_count);
				break;
#if 0
			case EOF_RECORD:
				puts("Got EOF record");
				break;
			case ESA_RECORD:
				puts("Got ESA record");
				break;
			case SSA_RECORD:
				puts("Got SSA record");
				break;
			case ELA_RECORD:
				puts("Got ELA record");
				break;
			case SLA_RECORD:
				puts("Got SLA record");
				break;
#endif
		}
	} while (r.record_type != EOF_RECORD);

	printf("Start: %04x Size: %d\n", start, size);

	return EXIT_SUCCESS;
}

int pos = 0;
int dpos;
int sam;
int block;
int bsize;
int getihex()
{
unsigned char b;

	if (pos == 0)
		b = 0x3a;
	else if (pos == 1)
		b = start >> 8;
	else if (pos == 2)
		b = start & 0xff;
	else if (pos == 3) {
		b = start >> 8;
		b += start & 0xff;
		b = 0x100 - b;
		block = 0;
		dpos = 0;
		sam = 0;
	}
	else if (pos == 4) {
		if(dpos == 0) {
			b = 0x3a;
		}
		else if (dpos == 1) {
			if (size < (block + 1) * 256)
				b = size - block * 255;
			else
				b = 255;
			bsize = b;
			sam += b;
		}
		else if (dpos == bsize + 2) {
			b = 0x100 - sam;

			if (block * 255 + bsize == size)
				pos = 5;
			else {
				++block;
				dpos = -1;
				sam = 0;
			}
		}
		else {
			b = mem[block * 255 + dpos - 2];
			sam += b;
		}
		
		++dpos;
	}
	// pos 5 is end of blcok
	else if (pos == 6)
		b = 0x3a;
	else if (pos == 7)
		b = 0x00;
	else if (pos == 8)
		b = 0x00;
	else
		 return -1;
	
	if (pos != 4) ++pos;
//printf("%02x ", b);
	return b;
}
