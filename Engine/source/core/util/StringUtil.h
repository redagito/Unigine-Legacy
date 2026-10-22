#pragma once

// support '|' as "OR" operation, '?' as ANY symbol and '*' as ANY string
int match(const char* mask, const char* name);