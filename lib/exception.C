// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifdef _WIN32
#include "stdafx.h"
#endif

#include "exception.h"
#include "error_numbers.h"


// BOINC Base Exception Class Implementation
//

const char * boinc_base_exception::ErrorType()
    { return "BOINC Exception"; }

const char * boinc_base_exception::ErrorMessage()
    { return "Unknown Exception"; }

long         boinc_base_exception::ErrorValue()
    { return -1; }

const char * boinc_base_exception::what()
{
	m_strErrorBuffer.empty();

	memset(m_szConversionBuffer, '\0', sizeof(m_szConversionBuffer));
	ltoa(ErrorValue(), m_szConversionBuffer, 10);

	m_strErrorBuffer.append(ErrorType());
	m_strErrorBuffer.append(" ");
	m_strErrorBuffer.append(m_szConversionBuffer);
	m_strErrorBuffer.append(" ");
	m_strErrorBuffer.append(ErrorMessage());

	m_strErrorBuffer.append("\n");

	m_strErrorBuffer.append(m_strErrorData);

	m_strErrorBuffer.append("\n");

	m_strErrorBuffer.append("Filename: '");
	m_strErrorBuffer.append(m_strFilename);
	m_strErrorBuffer.append("'\n");

	memset(m_szConversionBuffer, '\0', sizeof(m_szConversionBuffer));
	ltoa(m_lLineNumber, m_szConversionBuffer, 10);

	m_strErrorBuffer.append("Line: '");
	m_strErrorBuffer.append(m_szConversionBuffer);
	m_strErrorBuffer.append("'\n");

	return m_strErrorBuffer.c_str();
}


// BOINC Base Runtime Exception Class Implementation
//
const char * boinc_runtime_base_exception::ErrorType()
    { return "BOINC Runtime Exception"; }

const char * boinc_runtime_base_exception::ErrorMessage()
    { return "Unknown Runtime Exception"; }


// BOINC Runtime Exceptions
//
const char * boinc_out_of_memory_exception::ErrorMessage()
    { return "Out Of Memory"; }
long         boinc_out_of_memory_exception::ErrorValue()
    { return ERR_MALLOC; }

const char * boinc_invalid_parameter_exception::ErrorMessage()
    { return "Invalid Parameter"; }
long         boinc_invalid_parameter_exception::ErrorValue()
    { return -1001; }


const char * boinc_file_operation_exception::ErrorMessage()
    { return "File Operation Failure"; }
long         boinc_file_operation_exception::ErrorValue()
    { return -1100; }

const char * boinc_signal_operation_exception::ErrorMessage()
    { return "Signal Operation Failure"; }
long         boinc_signal_operation_exception::ErrorValue()
    { return -1101; }
