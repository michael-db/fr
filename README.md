# A Factored Radix Number Filter

`fr` transforms numbers to and from quasi-decimal versions of
higher base numbers.

## What is a factored radix number?

A standard number system like base 20 is defined by a radix.
A factored radix number system FR(P,S) is defined by two radices,
for example, FR(2,10).

FR(2,10) is similar to base 20:
- It uses an alphabet of 20 digits (P*S=20),
  e.g., `0123456789abcdefghij`.
- A number in base 20 can be represented in the same number of digits
  in FR(2,10).

However, FR(2,10) is also similar to decimal:
- Any number that looks like decimal retains its decimal value.
  For example, `426` in FR(2,10) has the decimal value 426
  (whereas `426` in base 20 is decimal 1646).
- Non-decimal digits are optional and used only to reduce the
  number of digits.

Suppose a number is to be represented using at most 2 digits.
Using decimal, 100 values are possible.
Like base 20, FR(2,10) extends this range to 400 values.
However, unlike base 20, the sequence runs
`0`,..,`99`, as for decimal, and then `0a`,..,`jj`
(leading zeros are significant).

Where a number in FR(2,10) has a non-decimal digit,
the decimal value is found as in this example:

```
 d7   FR(2,10)
 37   "stem": non-decimal digits replaced with mod 10 value
 10   "prefix", base 2: a..j -> 1, 0..9 -> 0, so b7 -> 10
  2   prefix converted to base 10
237   prefix and stem concatenated: decimal equivalent of b7
```

For a comprehensive account, see <https://mbreen.com/fr>


## To install and run

The filter is written in C (C99) to keep things easy.
On a typical installation of Linux this should work:
```
make                        # create ./fr
./fr                        # get a usage message
```

To see the full sequence of 2-digit FR(2,10) numbers mentioned above
and their decimal equivalents (using bash):
```
paste <(seq 0 399 |FRDIGITS=DCA fr 2) <(seq 0 399) |less
```
