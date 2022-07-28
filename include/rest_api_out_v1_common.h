/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 Inesonic, LLC.
* All rights reserved.
********************************************************************************************************************//**
* \file
*
* This header provides macros shared across the usage data API.
***********************************************************************************************************************/

/* .. sphinx-project inerest_api_out_v1 */

#ifndef REST_API_OUT_V1_COMMON_H
#define REST_API_OUT_V1_COMMON_H

#include <QtGlobal>

/** \def REST_API_OUT_V1_PUBLIC_API
 *
 * Macro used to define the interface to the container library.  Resolves to __declspec(dllexport) or
 * __declspec(dllimport) on Windows.
 */
#if (defined(REST_API_OUT_V1_DYNAMIC_BUILD))

    #if (defined(REST_API_OUT_V1_BUILD))

        #define REST_API_OUT_V1_PUBLIC_API Q_DECL_EXPORT

    #else

        #define REST_API_OUT_V1_PUBLIC_API Q_DECL_IMPORT

    #endif

#else

#define REST_API_OUT_V1_PUBLIC_API

#endif

#endif
