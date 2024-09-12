#include <ctype.h>
#include <stdbool.h>

#include "stringutils.h"


int string_natural_comparer(const char* x, const char* y) {
    int indexX = 0;
    int indexY = 0;

    size_t x_length = strlen(x);
    size_t y_length = strlen(y);

    while (true) {
        // Handle the case when one string has ended.
        if (indexX == x_length) {
            return indexY == y_length ? 0 : -1;
        }
        if (indexY == y_length) {
            return 1;
        }

        char charX = x[indexX];
        char charY = y[indexY];
        if (isdigit(charX) && isdigit(charY)) {
            // Skip leading zeroes in numbers.
            while (indexX < x_length && x[indexX] == '0') {
                indexX++;
            }
            while (indexY < y_length && y[indexY] == '0') {
                indexY++;
            }

            // Find the end of numbers
            int endNumberX = indexX;
            int endNumberY = indexY;
            while (endNumberX < x_length && isdigit((int)x[endNumberX])) {
                endNumberX++;
            }
            while (endNumberY < y_length && isdigit((int)y[endNumberY])) {
                endNumberY++;
            }

            int digitsLengthX = endNumberX - indexX;
            int digitsLengthY = endNumberY - indexY;

            // If the lengths are different, then the longer number is bigger
            if (digitsLengthX != digitsLengthY) {
                return digitsLengthX - digitsLengthY;
            }
            // Compare numbers digit by digit
            while (indexX < endNumberX) {
                if (x[indexX] != y[indexY])
                    return x[indexX] - y[indexY];
                indexX++;
                indexY++;
            }
        } else {
            // Plain characters comparison
            int compareResult = toupper(charX) - toupper(charY);
            if (compareResult != 0) {
                return compareResult;
            }
            indexX++;
            indexY++;
        }
    }
}
