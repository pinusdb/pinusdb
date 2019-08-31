#pragma once

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Token Token;

struct Token
{
  const char* str_;
  int len_;
};
