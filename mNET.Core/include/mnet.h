#pragma once

#ifndef _H_MNET_
#define _H_MNET_

#ifdef COMPILE_DLL_API
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif // COMPILE_DLL_API

#define INTERFACE class DLL_API __declspec(novtable) 

namespace mnet
{
    DLL_API int InitializeLibrary();
    DLL_API int DisposeLibrary();
}

#endif // !_H_MNET_
