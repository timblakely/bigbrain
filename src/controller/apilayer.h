#ifndef __PYX_HAVE__apilayer
#define __PYX_HAVE__apilayer


#ifndef __PYX_HAVE_API__apilayer

#ifndef __PYX_EXTERN_C
  #ifdef __cplusplus
    #define __PYX_EXTERN_C extern "C"
  #else
    #define __PYX_EXTERN_C extern
  #endif
#endif

__PYX_EXTERN_C DL_IMPORT(void) StartWebserver(std::string, std::string, std::string, std::string, bool);
__PYX_EXTERN_C DL_IMPORT(void) StopWebserver(void);
__PYX_EXTERN_C DL_IMPORT(int) WebserverRunning(void);
__PYX_EXTERN_C DL_IMPORT(void) BBProcessAgents(void);
__PYX_EXTERN_C DL_IMPORT(void) SetRPCObject(void *);

#endif /* !__PYX_HAVE_API__apilayer */

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initapilayer(void);
#else
PyMODINIT_FUNC PyInit_apilayer(void);
#endif

#endif /* !__PYX_HAVE__apilayer */
