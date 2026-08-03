// Copyright (c) 2026 Abdallah Boutrif
// SPDX-License-Identifier: MIT

#pragma once

#include "Logging/StructuredLog.h" // UE_LOGFMT

RAGESETTINGSSHARED_API DECLARE_LOG_CATEGORY_EXTERN(LogRageSettings, Log, All);

#define S_LOG(Verbosity, Format, ...) \
UE_LOGFMT(LogRageSettings, Verbosity, Format, ##__VA_ARGS__)
