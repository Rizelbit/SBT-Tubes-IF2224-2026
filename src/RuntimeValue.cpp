#include "RuntimeValue.hpp"

#include <cmath>
#include <sstream>

bool isInteger(const Value& v) { return std::holds_alternative<int>(v); }
bool isReal(const Value& v)    { return std::holds_alternative<double>(v); }
bool isChar(const Value& v)    { return std::holds_alternative<char>(v); }
bool isBoolean(const Value& v) { return std::holds_alternative<bool>(v); }
bool isString(const Value& v)  { return std::holds_alternative<std::string>(v); }

int toInt(const Value& v) {
    if (auto p = std::get_if<int>(&v)) return *p;
    if (auto p = std::get_if<double>(&v)) return static_cast<int>(*p);
    if (auto p = std::get_if<char>(&v)) return static_cast<int>(*p);
    if (auto p = std::get_if<bool>(&v)) return *p ? 1 : 0;
    return 0;
}

double toReal(const Value& v) {
    if (auto p = std::get_if<double>(&v)) return *p;
    if (auto p = std::get_if<int>(&v)) return static_cast<double>(*p);
    if (auto p = std::get_if<char>(&v)) return static_cast<double>(*p);
    return 0.0;
}

bool toBool(const Value& v) {
    if (auto p = std::get_if<bool>(&v)) return *p;
    if (auto p = std::get_if<int>(&v)) return *p != 0;
    if (auto p = std::get_if<double>(&v)) return *p != 0.0;
    if (auto p = std::get_if<std::string>(&v)) return !p->empty();
    return false;
}

std::string toString(const Value& v) {
    if (auto p = std::get_if<int>(&v)) return std::to_string(*p);
    if (auto p = std::get_if<double>(&v)) return std::to_string(*p);
    if (auto p = std::get_if<char>(&v)) return std::string(1, *p);
    if (auto p = std::get_if<bool>(&v)) return *p ? "true" : "false";
    if (auto p = std::get_if<std::string>(&v)) return *p;
    return "";
}

bool isTruthy(const Value& v) {
    return toBool(v);
}

Value addValues(const Value& a, const Value& b) {
    if (isString(a) || isString(b)) {
        return toString(a) + toString(b);
    }
    if (isReal(a) || isReal(b)) return toReal(a) + toReal(b);
    return toInt(a) + toInt(b);
}

Value subValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) - toReal(b);
    return toInt(a) - toInt(b);
}

Value mulValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) * toReal(b);
    return toInt(a) * toInt(b);
}

Value divValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) {
        double br = toReal(b);
        if (br == 0.0) throw std::runtime_error("Division by zero");
        return toReal(a) / br;
    }
    int bi = toInt(b);
    if (bi == 0) throw std::runtime_error("Division by zero");
    return toInt(a) / bi;
}

Value modValues(const Value& a, const Value& b) {
    int bi = toInt(b);
    if (bi == 0) throw std::runtime_error("Division by zero");
    return toInt(a) % bi;
}

Value negValue(const Value& a) {
    if (isReal(a)) return -toReal(a);
    return -toInt(a);
}

Value eqValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) == toReal(b);
    if (isString(a) || isString(b)) return toString(a) == toString(b);
    return toInt(a) == toInt(b);
}

Value neqValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) != toReal(b);
    if (isString(a) || isString(b)) return toString(a) != toString(b);
    return toInt(a) != toInt(b);
}

Value lssValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) < toReal(b);
    if (isString(a) || isString(b)) return toString(a) < toString(b);
    return toInt(a) < toInt(b);
}

Value leqValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) <= toReal(b);
    if (isString(a) || isString(b)) return toString(a) <= toString(b);
    return toInt(a) <= toInt(b);
}

Value gtrValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) > toReal(b);
    if (isString(a) || isString(b)) return toString(a) > toString(b);
    return toInt(a) > toInt(b);
}

Value geqValues(const Value& a, const Value& b) {
    if (isReal(a) || isReal(b)) return toReal(a) >= toReal(b);
    if (isString(a) || isString(b)) return toString(a) >= toString(b);
    return toInt(a) >= toInt(b);
}

Value andValues(const Value& a, const Value& b) {
    return toBool(a) && toBool(b);
}

Value orValues(const Value& a, const Value& b) {
    return toBool(a) || toBool(b);
}

Value notValue(const Value& a) {
    return !toBool(a);
}
