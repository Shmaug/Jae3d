#include "Util.h"

int PrintFormattedf(char *buf, int size, const char* format, float number) {
	int c = sprintf_s(buf, size, format, number);
	int j = 0;
	bool dec = false;
	for (int i = c - 1; i > 0; i--) {
		if (!dec) {
			if (buf[i] == '.') dec = true;
			continue;
		}
		j++;
		if (j >= 3) {
			for (int k = size - 1; k > i; k--)
				buf[k] = buf[k - 1];
			buf[i] = ',';
			c++;
			j = 0;
		}
	}
	return c;
}

int PrintFormattedl(char *buf, int size, unsigned long number) {
	int c = sprintf_s(buf, size, "%d", number);
	int j = 0;
	for (int i = c - 1; i > 0; i--) {
		j++;
		if (j >= 3) {
			for (int k = size - 1; k > i; k--)
				buf[k] = buf[k - 1];
			buf[i] = ',';
			c++;
			j = 0;
		}
	}
	return c;
}