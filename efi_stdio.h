#pragma once

void EFIPutChar(wchar_t c);
char EFIGetChar();
void PrintChar(char c);
void PrintString(const char *s);
void PrintStringWithSize(const char *s, int n);
void PrintU8AsHex(uint8_t data);
void PrintU64AsHex(uint64_t data);
void PrintStringAndHex64(const char *s, uint64_t data);
void PrintStringAndHex64(const char *s, const void *data);
void PrintStringAndHex64(const char *s, const volatile void *data);
