# BigNum
C++ header for arbitrary-precision integer representation.

## Features
* construction from
  * no parameters - BigNum is initialized to 0
  * int64_t
  * const std::string& (e.g. -150, +0, 00, -00012, ...)
* coppying
* unary operators `+` and `-`
* binary arithmetic operators `-`, `+`, `*` and `+=`, `*=`, `-=`
* relational and conditional operators `<`, `>`, `<=`, `>=`, `==`, `!=`
* output to stream - overload of opertor `<<`

## Not supported (yet)
* operators `/`, `/=`, `%` and `%=`
* operator `>>` - input from stream
