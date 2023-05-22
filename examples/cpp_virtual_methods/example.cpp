/*
 * Example for using virtual methods in C++
 * 05/21/2023 A. Mandera
 */

#include "example.h"
#include "stdio.h"

ExampleClass Example;

void Print::doPrint(int i) {
	printValue(i);
	doNewLine();
}

void ExampleClass::begin() {
	this->_value = 1;
}

int ExampleClass::value() {
	return this->_value++;
}

void ExampleClass::doNewLine() {
	printf("\n");
}

void ExampleClass::printValue(int i) {
	printf("Value: %d", this->value() + i);
};