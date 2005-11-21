// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include "cstdio"
#include "cstdlib"
#include "unistd.h"
using namespace std;
#endif

#include "exception.h"
#include "error_numbers.h"


const char* boinc_base_exception::ErrorType() {
    return "BOINC Exception";
}

const char * boinc_base_exception::ErrorMessage() {
    return "Unknown Exception";
}

long  boinc_base_exception::ErrorValue() {
    return -1;
}

const char * boinc_base_exception::what() {
    m_strErrorBuffer.empty();

    memset(m_szConversionBuffer, '\0', sizeof(m_szConversionBuffer));
    snprintf(m_szConversionBuffer, sizeof(m_szConversionBuffer), "%ld", m_lErrorValue);
    
    m_strErrorBuffer.append(m_strErrorType);
    m_strErrorBuffer.append(" ");
    m_strErrorBuffer.append(m_szConversionBuffer);
    m_strErrorBuffer.append(" ");
    m_strErrorBuffer.append(m_strErrorMessage);

    m_strErrorBuffer.append("\n");

    m_strErrorBuffer.append(m_strErrorData);

    m_strErrorBuffer.append("\n");

    m_strErrorBuffer.append("Filename: '");
    m_strErrorBuffer.append(m_strFilename);
    m_strErrorBuffer.append("'\n");

    memset(m_szConversionBuffer, '\0', sizeof(m_szConversionBuffer));
    snprintf(m_szConversionBuffer, sizeof(m_szConversionBuffer), "%ld", m_lLineNumber);

    m_strErrorBuffer.append("Line: '");
    m_strErrorBuffer.append(m_szConversionBuffer);
    m_strErrorBuffer.append("'\n");

    return m_strErrorBuffer.c_str();
}


// BOINC Base Runtime Exception Class Implementation
//
const char * boinc_runtime_base_exception::ErrorType() {
    return "BOINC Runtime Exception";
}

const char * boinc_runtime_base_exception::ErrorMessage() {
    return "Unknown Runtime Exception";
}


// BOINC Runtime Exceptions
//
const char * boinc_out_of_memory_exception::ErrorMessage() {
    return "Out Of Memory";
}

long         boinc_out_of_memory_exception::ErrorValue() {
    return ERR_MALLOC;
}

const char * boinc_invalid_parameter_exception::ErrorMessage() {
    return "Invalid Parameter";
}

long         boinc_invalid_parameter_exception::ErrorValue() {
    return ERR_INVALID_PARAM;
}


const char * boinc_file_operation_exception::ErrorMessage() {
    return "File Operation Failure";
}

long         boinc_file_operation_exception::ErrorValue() {
    return ERR_FOPEN;
}

const char * boinc_signal_operation_exception::ErrorMessage() {
    return "Signal Operation Failure";
}

long         boinc_signal_operation_exception::ErrorValue() {
    return ERR_SIGNAL_OP;
}

const char *BOINC_RCSID_21138c830b = "$Id$";
