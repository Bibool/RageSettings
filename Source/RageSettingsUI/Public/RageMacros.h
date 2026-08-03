// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#define RAGE_LOC(Key) FText::FromStringTable(TEXT("/RageSettings/ST_Settings.ST_Settings"), TEXT(Key))
#define RAGE_LOC_Str(Str) FText::FromStringTable(TEXT("/RageSettings/ST_Settings.ST_Settings"), *Str)