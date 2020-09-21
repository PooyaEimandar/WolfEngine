/************************************************************************************//**
// Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
****************************************************************************************/
#ifndef _JML_H_
#define _JML_H_

#ifdef __WIN32
    // helpful alias for 16-byte alignment
    #define ALIGN16 __declspec(align(16))
#else
    #define ALIGN16
#endif

#include "JMLVec2.h"
#include "JMLVec3.h"
#include "JMLMatrix.h"
#include "JMLFuncs.h"



#endif
