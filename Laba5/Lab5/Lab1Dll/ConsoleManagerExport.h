#pragma once

#if defined(CALCAPI_EXPORT) // ������ DLL
#   define CALCAPI   __declspec(dllexport)
#else // ��� DLL
#   define CALCAPI   __declspec(dllimport)
#endif 

#include <iostream>
#include <iomanip>

CALCAPI double execute(std::string str);

