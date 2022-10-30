#pragma once

void printf(char*, ...);
void debug_printf(char*, ...);
void panic(char*) __attribute__((noreturn));
void printfinit(void);
