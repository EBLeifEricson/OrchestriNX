#pragma once
#include <switch.h>
#include "language.h"

void textInit(void);
int textGetLang(void);
const char* textGetString(StrId id);
u64 textGetLanguageCode(void);