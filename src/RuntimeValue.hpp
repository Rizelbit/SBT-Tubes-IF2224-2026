#pragma once

#include <string>
#include <variant>
#include <stdexcept>

using Value = std::variant<int, double, char, bool, std::string>;

bool isInteger(const Value& v);
bool isReal(const Value& v);
bool isChar(const Value& v);
bool isBoolean(const Value& v);
bool isString(const Value& v);

int toInt(const Value& v);
double toReal(const Value& v);
bool toBool(const Value& v);
std::string toString(const Value& v);

bool isTruthy(const Value& v);

// Operasi aritmatika
Value addValues(const Value& a, const Value& b);
Value subValues(const Value& a, const Value& b);
Value mulValues(const Value& a, const Value& b);
Value divValues(const Value& a, const Value& b);
Value modValues(const Value& a, const Value& b);
Value negValue(const Value& a);

// Operasi relasional
Value eqValues(const Value& a, const Value& b);
Value neqValues(const Value& a, const Value& b);
Value lssValues(const Value& a, const Value& b);
Value leqValues(const Value& a, const Value& b);
Value gtrValues(const Value& a, const Value& b);
Value geqValues(const Value& a, const Value& b);

// Operasi logika
Value andValues(const Value& a, const Value& b);
Value orValues(const Value& a, const Value& b);
Value notValue(const Value& a);
