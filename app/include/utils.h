#pragma once

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res) ((res) >= 0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)    ((res) < 0)

extern char g_err_string[64];

int getSystemParamDateTimeFormat(void);
