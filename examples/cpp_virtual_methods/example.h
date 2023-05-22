/*
 * Example for using virtual methods in C++
 * 05/21/2023 A. Mandera
 */

#pragma once

class Print {
public:
	void doPrint(int);
	virtual void doNewLine(void) = 0;
	virtual void printValue(int) = 0;
};

class ExampleClass : public Print {
public:
	void begin();
	int value();
	void doNewLine(void) override;
	void printValue(int) override;

private:
	int _value;
};

extern ExampleClass Example;