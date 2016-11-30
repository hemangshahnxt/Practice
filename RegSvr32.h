#ifndef REGSVR32_H
#define REGSVR32_H

#pragma once



#define REG_FAIL_ARGS   1
#define REG_FAIL_OLE    2
#define REG_FAIL_LOAD   3
#define REG_FAIL_ENTRY  4
#define REG_FAIL_REG    5

int RegSvr32(LPCSTR pszDllName);

#endif