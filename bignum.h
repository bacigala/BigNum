#pragma once

#include <string>
#include <stdint.h>
#include <vector>
#include <locale>

#define SUPPORT_DIVISION 0 // tbd
#define SUPPORT_IFSTREAM 0 // tbd

class BigNum final {
public:
    BigNum() : _value{ 0 }, isNegative(false) {};

    BigNum(int64_t n) : _value(), isNegative(n < 0) {
        if (n == 0) {
            _value.push_back(0);
            return;
        }
        if (isNegative) n *= -1;
        while (n > 0) {
            _value.push_back(n % 10);
            n /= 10;
        }
    }

    explicit BigNum(const std::string& str) : _value{}, isNegative(false) {
        if (str.empty()) throw std::invalid_argument("Empty string cannot be converted to BigNum.");

        // handle sign
        bool hasSign = false;
        if (str.at(0) == '+') hasSign = true;
        if (str.at(0) == '-') {
            hasSign = true;
            isNegative = true;
        }
        if (hasSign && (str.size() == 1)) throw std::invalid_argument("Wrong string format. String cannot be converted to BigNum.1 " + str);

        // parse digits
        size_t pos = str.size() - 1;
        while (pos >= static_cast<uint8_t>(hasSign ? 1 : 0)) {
            if (!std::isdigit(str.at(pos))) throw std::invalid_argument("Wrong string format. String cannot be converted to BigNum.2 " + str);
            _value.push_back(str.at(pos) - '0');
            if (pos > 0) pos--; else break;
        }

        deleteZeroPrefix(_value);

        // do not accept negative zero
        if (isNegative && _value.size() == 1 && _value.at(0) == 0) throw std::invalid_argument("Wrong string format. String cannot be converted to BigNum.3 " + str);
    }

    // copy
    BigNum(const BigNum& other) = default;
    BigNum& operator=(const BigNum& rhs) = default;

    // unary operators
    const BigNum& operator+() const {
        return *this;
    }

    BigNum operator-() const {
        return -1 * (*this);
    }

    // binary arithmetics operators
    BigNum& operator+=(const BigNum& rhs) {
        BigNum result = *this + rhs;
        _value = result._value;
        isNegative = result.isNegative;
        return *this;
    }
    BigNum& operator-=(const BigNum& rhs) {
        BigNum result = *this - rhs;
        _value = result._value;
        isNegative = result.isNegative;
        return *this;
    }
    BigNum& operator*=(const BigNum& rhs) {
        BigNum result = *this * rhs;
        _value = result._value;
        isNegative = result.isNegative;
        return *this;
    }

#if SUPPORT_DIVISION == 1
    BigNum& operator/=(const BigNum& rhs);
    BigNum& operator%=(const BigNum& rhs);
#endif

private:
    std::vector<uint8_t> _value;
    bool isNegative;

    BigNum(std::vector<uint8_t> value, bool isNeg) : _value(deleteZeroPrefix(value)), isNegative(isNeg) {};

    std::vector<uint8_t>& deleteZeroPrefix(std::vector<uint8_t>& vector) const {
        while (!vector.empty() && vector.at(vector.size() - 1) == 0) vector.pop_back();
        if (vector.empty()) vector.push_back(0);
        return vector;
    }

    // friends
    friend BigNum operator+(BigNum lhs, const BigNum& rhs);
    friend BigNum operator-(BigNum lhs, const BigNum& rhs);
    friend BigNum operator*(BigNum lhs, const BigNum& rhs);
    friend bool operator==(const BigNum& lhs, const BigNum& rhs);
    friend bool operator<(const BigNum& lhs, const BigNum& rhs);
    friend std::ostream& operator<<(std::ostream&, const BigNum&);
};

BigNum operator+(BigNum lhs, const BigNum& rhs) {
    if (lhs.isNegative && !rhs.isNegative) {
        lhs.isNegative = false;
        return rhs - lhs;
    }
    if (!lhs.isNegative && rhs.isNegative) {
        BigNum newRhs = BigNum(rhs);
        newRhs.isNegative = false;
        return lhs - newRhs;
    }
    //sum the values
    uint8_t carry = 0;
    size_t pos = 0;
    while (carry > 0 || pos < lhs._value.size() || pos < rhs._value.size()) {
        uint8_t lhsDigit = pos < lhs._value.size() ? lhs._value.at(pos) : 0;
        uint8_t rhsDigit = pos < rhs._value.size() ? rhs._value.at(pos) : 0;
        carry += lhsDigit + rhsDigit;
        if (pos < lhs._value.size()) lhs._value[pos] = carry % 10; else lhs._value.push_back(carry % 10);
        carry /= 10;
        pos++;
    }

    return lhs;
};

BigNum operator-(BigNum lhs, const BigNum& rhs) {
    if (lhs.isNegative && !rhs.isNegative) {
        BigNum newRhs = BigNum(rhs);
        newRhs.isNegative = true;
        return lhs + newRhs;
    }
    if (!lhs.isNegative && rhs.isNegative) {
        BigNum newRhs = BigNum(rhs);
        newRhs.isNegative = false;
        return lhs + newRhs;
    }
    bool resultIsNegative = false;
    std::vector<uint8_t> resultValues;
    bool reverse = false;
    if (!lhs.isNegative && lhs < rhs) {
        resultIsNegative = true;
        reverse = true;
    }
    if (lhs.isNegative) {
        if (lhs < rhs) {
            resultIsNegative = true;
        }
        else {
            reverse = true;
        }
    }

    const BigNum* minuend = reverse ? &rhs : &lhs;
    const BigNum* subtrahend = reverse ? &lhs : &rhs;

    size_t pos = 0;
    uint8_t borrowed = 0;
    while (borrowed > 0 || pos < (*minuend)._value.size() || pos < (*subtrahend)._value.size()) {
        uint8_t minuendDigit = pos < (*minuend)._value.size() ? (*minuend)._value.at(pos) : 0;
        uint8_t subtrahendDigit = pos < (*subtrahend)._value.size() ? (*subtrahend)._value.at(pos) : 0;

        subtrahendDigit += borrowed;
        borrowed = 0;

        if (minuendDigit < subtrahendDigit) {
            minuendDigit += 10;
            borrowed += 1;
        }

        resultValues.push_back(minuendDigit - subtrahendDigit);
        pos++;
    }

    return BigNum{ resultValues, resultIsNegative };
};

BigNum operator*(BigNum lhs, const BigNum& rhs) {
    std::vector<uint8_t> resultValues;
    size_t carry = 0;
    std::vector<uint8_t> carryInLine;
    for (size_t i = 0; i < rhs._value.size(); i++) carryInLine.push_back(0);

    for (size_t column = 0; column < lhs._value.size() + rhs._value.size(); column++) {
        for (size_t rhsPos = 0; rhsPos < rhs._value.size(); rhsPos++) {
            if (!(rhsPos > column || (column >= lhs._value.size() && column - lhs._value.size() >= rhsPos))) {
                size_t lhsPos = (column - rhsPos) % lhs._value.size();
                carryInLine.at(rhsPos) += lhs._value.at(lhsPos) * rhs._value.at(rhsPos);
            }
            carry += carryInLine.at(rhsPos) % 10;
            carryInLine[rhsPos] /= 10;
        }
        resultValues.push_back(carry % 10);
        carry /= 10;
    }
    return BigNum{ resultValues, (rhs.deleteZeroPrefix(resultValues).size() == 1 && resultValues.at(0) == 0 ? false : lhs.isNegative != rhs.isNegative) };
};

#if SUPPORT_DIVISION == 1
BigNum operator/(BigNum lhs, const BigNum& rhs);
BigNum operator%(BigNum lhs, const BigNum& rhs);
#endif

bool operator==(const BigNum& lhs, const BigNum& rhs) {
    if (lhs.isNegative != rhs.isNegative) return false;
    if (lhs._value.size() != rhs._value.size()) return false;
    for (size_t position = 0; position < lhs._value.size(); ++position) {
        if (lhs._value.at(position) != rhs._value.at(position)) return false;
    }
    return true;
};
bool operator!=(const BigNum& lhs, const BigNum& rhs) {
    return !(lhs == rhs);
};
bool operator<(const BigNum& lhs, const BigNum& rhs) {
    if (lhs.isNegative && !rhs.isNegative) return true;
    if (!lhs.isNegative && rhs.isNegative) return false;
    if (lhs._value.size() != rhs._value.size()) {
        if (lhs.isNegative) return lhs._value.size() > rhs._value.size();
        else return lhs._value.size() < rhs._value.size();
    }
    for (size_t i = lhs._value.size(); i > 0; --i) {
        size_t position = i - 1;
        if (lhs._value.at(position) == rhs._value.at(position)) continue;
        if (lhs.isNegative) return lhs._value.at(position) > rhs._value.at(position);
        else return lhs._value.at(position) < rhs._value.at(position);
    }
    return false;
};
bool operator>(const BigNum& lhs, const BigNum& rhs) {
    return !(lhs == rhs) && !(lhs < rhs);
};
bool operator<=(const BigNum& lhs, const BigNum& rhs) {
    return !(lhs > rhs);
};
bool operator>=(const BigNum& lhs, const BigNum& rhs) {
    return !(lhs < rhs);
};


std::ostream& operator<<(std::ostream& lhs, const BigNum& rhs) {
    if (rhs.isNegative) lhs << '-';
    for (auto it = rhs._value.rbegin(); it != rhs._value.rend(); ++it) lhs << unsigned(*it);
    return lhs;
}

#if SUPPORT_IFSTREAM == 1
std::istream& operator>>(std::istream& lhs, BigNum& rhs);
#endif
