/*
  p6towav.c : convert binary data to wav format for PC-6001 series
    AKIKAWA, Hisashi 2021.6.8
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

#define FORMAT_DEFAULT "b2.0 h3.5 d16 h0.5 d h0.05 b0.6"
#define FORMAT_IO "b2.0 h3.5 d17 h0.05 b3.5 h3.5 d h0.05 b0.6"
#define FORMAT_BIN "b2.0 h3.5 d h0.05 b0.6"

void wav_head(int, FILE *);
int option(int, char *[]);
int dataout(char, FILE *);
int header(double, FILE *);
int fsk(double, FILE *);
int blank(double, FILE *);
void fput2(int, FILE *);
void fput4(int, FILE *);

void readihex(FILE *);
int getihex();

/* wav file parameter */
int sampling_rate;
int quantization_bit;
int nchannel;

/* tape parameter */
int baud_rate;
int carrier_low;
char *format;
double time;
int stop_bit;
int intelhex;

int main(int argc, char *argv[])
{
    int i, j;
    int c;
    int size = 0;
    FILE *fp_in, *fp_out;

    /* default parameter */

    sampling_rate = 11025;
    quantization_bit = 8;
    nchannel = 1;
    baud_rate = 600;
    carrier_low = 1200;
    stop_bit = 3;
    format = NULL;
    intelhex = 0;

    /* option analysis */

    if (option(argc, argv) != argc - 2) {
	printf("usage: p6towav [options] input-file output-file\n");
        printf("options:\n");
	printf(" -b baud-rate\n");
	printf(" -c channels\n");
	printf(" -f format-string | io | bin\n");
	printf(" -q quantization-bits\n");
	printf(" -r sampling-rate\n");
	printf(" -s stop-bits\n");
	printf(" -w lower-carrier-wave\n");
	printf("default: -b %d -c %d -f \"%s\" -q %d -r %d -s %d -w %d\n",
	       baud_rate, nchannel, FORMAT_DEFAULT, quantization_bit,
	       sampling_rate, stop_bit, carrier_low);
	exit(1);
    }

    /* input file */

    if (strcmp(argv[argc - 2], "-") == 0) {
	fp_in = stdin;
    }else{
	fp_in = fopen(argv[argc - 2], "rb");  /* b for Windows */
	if (fp_in == NULL) {
	    printf("cannot open %s\n", argv[argc - 2]);
	    exit(1);
	}
        if (intelhex)
            readihex(fp_in);
    }

    /* output file */

    if (strcmp(argv[argc - 1], "-") == 0) {
	fp_out = stdout;
    }else{
	fp_out = fopen(argv[argc - 1], "wb");  /* b for Windows */
	if (fp_out == NULL) {
	    printf("cannot open %s\n", argv[argc - 1]);
	    exit(1);
	}
    }

    /* make wav data */

    for (i = 0; i < 44; i++) {
	fputc(0, fp_out);
    }

    time = 0;
    for (i = 0; i < strlen(format); i++) {
	double length;
	int byte;

	if (strpbrk(&format[i], "bhd")) {
	    i = strpbrk(&format[i], "bhd") - format;
	}else{
	    break;
	}
	if (format[i] == 'd') {
	    int sscanf_result = sscanf(&format[i + 1], "%d", &byte);
	    if (sscanf_result == 0 || sscanf_result == EOF) {
		byte = 0;
	    }
	}else{
	    sscanf(&format[i + 1], "%lf", &length);
	}

	switch (format[i]) {
	  case 'b':
//	    if (size != 0) {
//		size += header(0.05, fp_out);
//	    }
	    size += blank(length, fp_out);
	    break;
	  case 'h':
	    size += header(length, fp_out);
	    break;
	  case 'd':
	    for (j = 0; byte == 0 || j < byte; j++) {
		if (intelhex)
		    c = getihex();
		else
		    c = fgetc(fp_in);
		if (c == EOF) {
		    break;
		}
		size += dataout(c, fp_out);
	    }
	    break;
	  default:
	    break;
	}
    }
    wav_head(size, fp_out);

//    fclose(fp_in);
    fclose(fp_out);
    return 0;
}


void wav_head(int size, FILE *fp) {
    rewind(fp);

    /* RIFF identifier */
    fputs("RIFF", fp);

    /* file size */
    fput4(size + 36, fp);

    /* WAVE identifier */
    fputs("WAVE", fp);

    /* fmt chunk start */
    fputs("fmt ", fp);

    /* fmt chunk size */
    fput4(16, fp);

    /* format ID */
    fput2(1, fp);

    /* monoaural or streo */
    fput2(nchannel, fp);

    /* sampling rate */
    fput4(sampling_rate, fp);

    /* data rate */
    fput4(sampling_rate * nchannel * quantization_bit / 8, fp);

    /* block size */
    fput2(nchannel * quantization_bit / 8, fp);

    /* sampling bit */
    fput2(quantization_bit, fp);

    /* data chunk start */
    fputs("data", fp);

    /* data size */
    fput4(size, fp);
}


int option(int argc, char *argv[])
{
    int i;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-' || strcmp(argv[i], "-") == 0) {
	    break;
	}

	if (strcmp(argv[i], "--") == 0) {
	    i++;
	    break;
	}

	if (strcmp(argv[i], "-b") == 0) {
	    if (++i >= argc) {
		break;
	    }
	    baud_rate = atoi(argv[i]);
	    if (baud_rate <= 0) {
		printf("illegal baud rate\n");
		exit(1);
	    }
	}

	if (strcmp(argv[i], "-c") == 0) {
	    if (++i >= argc) {
		break;
	    }
	    nchannel = atoi(argv[i]);
	    if (nchannel != 1 && nchannel != 2) {
		printf("the number of channels must be 1 or 2\n");
		exit(1);
	    }
	}

	if (strcmp(argv[i], "-f") == 0) {
	    if (++i >= argc) {
		break;
	    }
	    if (strcmp(argv[i], "io") == 0) {
		format = malloc(strlen(FORMAT_IO) + 1);
		if (format == NULL) {
		    printf("cannot allocate memory\n");
		    exit(1);
		}
		strcpy(format, FORMAT_IO);
	    } else if (strcmp(argv[i], "bin") == 0) {
		format = malloc(strlen(FORMAT_BIN) + 1);
		if (format == NULL) {
		    printf("cannot allocate memory\n");
		    exit(1);
		}
		strcpy(format, FORMAT_BIN);
	    }else{
		format = argv[i];
	    }
	}

	if (strcmp(argv[i], "-q") == 0) {
	    if (++i >= argc) {
		break;
	    }
	    quantization_bit = atoi(argv[i]);
	    if (quantization_bit != 8 && quantization_bit != 16) {
		printf("sampling bit must be 8 or 16\n");
		exit(1);
	    }
	}

	if (strcmp(argv[i], "-r") == 0) {
	    if (++i >= argc) {
		break;
	    }
	    sampling_rate = atoi(argv[i]);
	    if (sampling_rate < 1) {
		printf("illegal sampling rate\n");
		exit(1);
	    }
	}

	if (strcmp(argv[i], "-s") == 0) {
	    if (++i >= argc) {
		break;
	    }
	    stop_bit = atoi(argv[i]);
	    if (stop_bit < 0) {
		printf("illegal stop bit\n");
		exit(1);
	    }
	}

	if (strcmp(argv[i], "-w") == 0) {
	    if (++i >= argc) {
		break;
	    }
	    carrier_low = atoi(argv[i]);
	    if (carrier_low < 1) {
		printf("illegal carrier frequency\n");
		exit(1);
	    }
	}

	if (strcmp(argv[i], "-i") == 0) {
             intelhex = 1;
        }
    }

    if (format == NULL) {
	format = malloc(strlen(FORMAT_DEFAULT) + 1);
	if (format == NULL) {
	    printf("cannot allocate memory\n");
	    exit(1);
	}
	strcpy(format, FORMAT_DEFAULT);
    }

    if (carrier_low / baud_rate * baud_rate != carrier_low) {
	printf("illegal carrier frequency\n");
	exit(1);
    }
    if (sampling_rate < carrier_low * 8) {
	printf("too low sampling rate\n");
	exit(1);
    }


    return i;
}


int dataout(char data, FILE *fp)
{
    int i, size;

    /* start bit  */
    size = fsk(carrier_low, fp);

    /* data */
    for (i = 0; i < 8; i++) {
	if (data & (1 << i)) {
	    size += fsk(carrier_low * 2, fp);
	}else{
	    size += fsk(carrier_low, fp);
	}
    }

    /* stop bit  */
    for (i = 0; i < stop_bit; i++) {
	size += fsk(carrier_low * 2, fp);
    }

    return size;
}


int header(double length, FILE *fp)
{
    int size = 0;
    double start = (int)(time * carrier_low) / (double)carrier_low;
    while (time < start + length) {
	size += fsk(carrier_low * 2, fp);
    }
    return size;
}


int fsk(double freq, FILE *fp)
{
    int i, j;
    int size;
    double start = (int)(time * carrier_low) / (double)carrier_low;
    char *data;

    data = malloc ((sampling_rate / baud_rate + 2) * nchannel * quantization_bit / 8);
    if (data == NULL) {
	printf("cannot allocate memory\n");
	exit(1);
    }

    for (i = 0; time < start + 1. / baud_rate; i++, time += 1. / sampling_rate) {
	for (j = 0; j < nchannel; j++) {
	    if (quantization_bit == 8) {
//		fputc(128 - 127 * sin(2 * M_PI * freq * (time - start)), fp);
		data[i * nchannel + j]
		    = 128 - 127 * sin(2 * M_PI * freq * (time - start));

	    }else{
//		fput2(-32767 * sin(2 * M_PI * freq * (time - start)), fp);
		data[(i * nchannel + j) * 2]
		    =  (int)(-32767 * sin(2 * M_PI * freq * (time - start))) & 0xff;
		data[(i * nchannel + j) * 2 + 1]
		    =  ((int)(-32767 * sin(2 * M_PI * freq * (time - start))) >> 8) & 0xff;
	    }
	}
    }
    size = i * j * quantization_bit / 8;
    fwrite(data, 1, size, fp);
    free(data);
    return size;
}


int blank(double length, FILE *fp)
{
    int i, j;
    for (i = 0; i < length * sampling_rate; i++) {
	for (j = 0; j < nchannel; j++) {
	    if (quantization_bit == 8) {
		fputc(128, fp);
	    }else{
		fput2(0, fp);
	    }
	}
    }
    time += (int) (length * sampling_rate) / sampling_rate;
    return i * j * quantization_bit / 8;
}


void fput2(int data, FILE *fp)
{
    fputc(data & 0xff, fp);
    fputc((data >> 8) & 0xff, fp);
}


void fput4(int data, FILE *fp)
{
    fputc(data & 0xff, fp);
    fputc((data >> 8) & 0xff, fp);
    fputc((data >> 16) & 0xff, fp);
    fputc((data >> 24) & 0xff, fp);
}
