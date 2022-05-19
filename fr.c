// Copyright (c) 2022 Michael Breen (https://mbreen.com)
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

static const char* program_name;

// Usage message split on the program name.
static const char* const usage[] = {
        "Usage: ", " n [P [S]]\n"
        "Convert numbers to or from factored radix form (see\n"
        "https://mbreen.com/fr).\n"
        "  n      Desired number of digits, 0..99.\n"
        "         0: maximum: output numbers in base S.\n"
        "         1: minimum: output canonical FR(P,S).\n"
        "  P      Prefix radix. Default: 2.\n"
        "  S      Stem radix. Default: 10.\n\n"
        "The environment variable FRDIGITS must be set to one of:\n"
        "  DMA    Decimal Morphology Alphabet\n"
        "  DCA    Decade-Congruent Alphabet\n"
        "  or an explicit alphabet (printable ASCII), e.g.,\n"
        "  0123456789ABCDEFGHIJ\n\n"
        "Examples (POSIX shell):\n"
        "  export FRDIGITS=DMA\n"
        "  # FR(5,10) datestamp:\n"
        "  echo 2021-12-25 | ", " 1 5\n"
        "  # sort FR(2,10) by intermediate conversion to decimal:\n"
        "  printf '0z1\\n99q\\n' | ", " 0 | sort -n | ", " 1\n"
        "  # Display the alphabet (ensure FRDIGITS is set):\n"
        "  2>&1 ", " 0 99 |sed -n 's/^Alphabet: //p' |fold -w10\n\n"
        "Notes:\n"
        "  * Insignificant leading zeros should be stripped from\n"
        "  decimal (or base S) numbers passed to the program.\n"
        "  * Punctuation characters not present in the alphabet\n"
        "  are treated as separators between numbers.\n"
        "  * Thus only whole numbers are supported. Fractional\n"
        "  numbers should be scaled to remove the '.' or ','.\n"
        // VERSION
        };

// Output the program name followed by msg and '\n' to stderr.
static void error_msg(const char *msg) {
    fputs(program_name, stderr);
    fputs(": ", stderr);
    fputs(msg, stderr);
    fputs("\n", stderr);
}

// Exit with Usage, optionally preceded by an error message.
static void exit_usage(const char* msg) {
    if (msg)
        error_msg(msg);
    size_t i = 0;
    for (; i < sizeof(usage)/sizeof(usage[0]) - 1; ++i) {
        fputs(usage[i], stderr);
        fputs(program_name, stderr);
    }
    fputs(usage[i], stderr);
    exit(1);
}

static const char* const alphabets[] = {
        "DMA", "0123456789" "cjzwfsbvxq" "nltmhgdrkp"
            "CJZWFSBVXQ" "NLTMHGDRKP" "uiyeaUIYEA",
        "DCA", "0123456789" "abcdefghij" "klmnopqrst"
            "ABCDEFGHIJ" "KLMNOPQRST" "vwxyzVWXYZ",
        };

static const char* get_alphabet(void) {
    const char *digchars = getenv("FRDIGITS");
    if (!digchars || !digchars[0])
        exit_usage("environment variable FRDIGITS undefined");
    for (size_t i = 1; i < sizeof(alphabets)/sizeof(alphabets[0]);
            i += 2) {
        if (!strcmp(digchars, alphabets[i - 1])) {
            return alphabets[i];
        }
    }
    return digchars;
}

// Report an error in the alphabet and exit.
static void err_digits(const char* msg) {
    error_msg(msg);
    fputs("Alphabet: ", stderr);
    fputs(get_alphabet(), stderr);
    fputs("\n\n", stderr);
    exit_usage(NULL);
}

static void overflow(void) {
    error_msg("overflow: too many digits");
    exit(2);
}

#define isalpha(x) (((c)|0x20) >= 'a' && ((c)|0x20) <= 'z')
#define isdecimal(x) ((unsigned) (x) - '0' < 10)

// Convert an argument of 1 or 2 decimal digits to a number.
static int arg2num(char *arg) {
    if (!isdecimal(arg[0]))
        exit_usage("arguments must be 1 or 2 decimal digits");
    int val = arg[0] - '0';
    if (isdecimal(arg[1]) && !arg[2])
        val = val * 10 + arg[1] - '0';
    else if (arg[1])
        exit_usage("arguments must be 1 or 2 decimal digits");
    return val;
}

// Report printable/DEL/non-ASCII character as invalid and exit.
static void invalid_input_digit(int c) {
    fputs(program_name, stderr);
    fputs(": ", stderr);
    fputs("input character ",stderr);
    if (c < 0x7f) {
        fputs("'", stderr);
        fputc(c, stderr);
        fputs("' not in alphabet\n", stderr);
    } else {
        fputs("outside ASCII range\n", stderr);
    }
    exit(1);
}

// Numeric value of digit or radix. For a digit, -1 means "none".
typedef signed char digval;

#define MAX_LEN 100
typedef struct {
    digval radix;
    int len;
    digval digits[MAX_LEN]; // least significant first
} numbr;

// Make *num = *num * factor + addend.
static void mult_add(numbr *num, digval factor, digval addend) {
    int j, carry = addend;
    for (j = 0; carry || (factor && j < num->len); ++j) {
        if (j < num->len)
            carry += num->digits[j] * factor;
        else if (j >= MAX_LEN)
            overflow();
        num->digits[j] = (digval) (carry % num->radix);
        carry /= num->radix;
    }
    num->len = j;
}

// Make *num = *num / divisor and return the remainder.
static digval div_mod(numbr *num, digval divisor) {
    int remainder = 0;
    for (int j = num->len; j;) {
        remainder = remainder * num->radix + num->digits[--j];
        num->digits[j] = (digval) (remainder / divisor);
        num->len -= !num->digits[j] && j + 1 == num->len;
        remainder %= divisor;
    }
    return (digval) remainder;
}

int main(int argc, char **argv) {
    program_name = argv[0];
    for (const char *ip = program_name; *ip; ++ip)
        if (*ip == '/')
            program_name = ip + 1;
    if (argc < 2 || argc > 4)
        exit_usage(NULL);
    int width = arg2num(argv[1]);
    numbr prefix = {.len = 0, .radix = 2};
    if (argc > 2) {
        prefix.radix = (digval) arg2num(argv[2]);
    }
    numbr stem = {.len = 0, .radix = 10};
    if (argc > 3) {
        stem.radix = (digval) arg2num(argv[3]);
    }
    if (prefix.radix < 2 || stem.radix < 2)
        exit_usage("minimum radix 2 (P and S)");
    const char *digit_chars = get_alphabet();
    digval digit_vals[0x80];
    for (int j = 0; j < 0x80; ++j)
        digit_vals[j] = -1;
    for (digval v = 0; (size_t) v < strlen(digit_chars); ++v) {
        int i = digit_chars[v];
        if (i < ' ' || i > '~')
            err_digits("alphabet must be printable ASCII");
        if (digit_vals[i] != -1)
            err_digits("digit repeated in alphabet");
        digit_vals[i] = v;
    }
    if ((int) strlen(digit_chars) < prefix.radix * stem.radix)
        err_digits("alphabet must have at least P*S characters");
    int digit_count = 0;
    for (;;) {
        int c = getchar();
        digval val = c & 0x80 ? -1 : digit_vals[c];
        if (val >= 0 && val < prefix.radix * stem.radix) {
            // Accumulate number.
            mult_add(&stem, stem.radix,
                    (digval) (val % stem.radix));
            mult_add(&prefix, prefix.radix,
                    (digval) (val / stem.radix));
            if (++digit_count > MAX_LEN)
                overflow();
        } else {
            if (isalpha(c) || isdecimal(c) || c >= 0x7f)
                invalid_input_digit(c);
            // Output accumulated number, if any.
            if (digit_count) {
                // Preserve any leading '0's. Add '0's if necessary.
                while (digit_count > stem.len || !stem.len)
                    stem.digits[stem.len++] = 0;
                // Reduce stem as desired. This can overshoot,
                // making the prefix longer than the stem.
                if (width && (stem.digits[stem.len-1] || prefix.len))
                    while (stem.len > prefix.len && stem.len > width)
                        mult_add(&prefix, stem.radix,
                                stem.digits[--stem.len]);
                // Increase stem as desired or correct an overshoot.
                while (prefix.len && (!width || prefix.len > stem.len
                        || stem.len < width)) {
                    if (stem.len >= MAX_LEN)
                        overflow();
                    stem.digits[stem.len++] =
                            div_mod(&prefix, stem.radix);
                }
                for (int j = stem.len - 1; j >= 0; --j) {
                    int v = stem.digits[j];
                    if (j < prefix.len)
                        v += prefix.digits[j] * stem.radix;
                    putchar(digit_chars[v]);
                }
                digit_count = stem.len = prefix.len = 0;
            }
            if (c == EOF)
                return 0;
            putchar(c);
        }
    }
}
