#include "Util.h"

void FormatNumber(char *buf, int size, int number) {
	int c = sprintf_s(buf, size, "%d", number);

}