/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2002 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Log: GCCDefs.hpp,v $
 * Revision 1.6  2002/11/18 20:38:11  tng
 * [Bug 14612] GCCDefs clashes with cygwin's string.h for stricmp and strnicmp.
 *
 * Revision 1.5  2002/11/04 14:45:20  tng
 * C++ Namespace Support.
 *
 * Revision 1.4  2002/05/28 12:57:17  tng
 * Fix typo.
 *
 * Revision 1.3  2002/05/27 18:02:40  tng
 * define XMLSize_t, XMLSSize_t and their associate MAX
 *
 * Revision 1.2  2002/05/21 19:45:53  tng
 * Define DOMSize_t and XMLSize_t
 *
 * Revision 1.1.1.1  2002/02/01 22:22:18  peiyongz
 * sane_include
 *
 * Revision 1.14  2001/07/06 17:11:58  tng
 * Automatic build of single-threaded library.  By Martin Kalen.
 *
 * Revision 1.13  2001/03/02 20:52:58  knoaman
 * Schema: Regular expression - misc. updates for error messages,
 * and additions of new functions to XMLString class.
 *
 * Revision 1.12  2001/02/08 14:15:33  tng
 * enable COMPAQ Tru64 UNIX machines to build xerces-c with gcc (tested using COMPAQ gcc version2.95.2 19991024 (release) and Tru64 V5.0 1094).  Added by Martin Kalen.
 *
 * Revision 1.11  2000/10/17 00:52:00  andyh
 * Change XMLCh back to unsigned short on all platforms.
 *
 * Revision 1.10  2000/03/09 01:22:29  abagchi
 * Added header guards to include inlines only once
 *
 * Revision 1.9  2000/03/02 19:55:08  roddey
 * This checkin includes many changes done while waiting for the
 * 1.1.0 code to be finished. I can't list them all here, but a list is
 * available elsewhere.
 *
 * Revision 1.8  2000/02/22 01:00:48  aruna1
 * GNUGDefs references removed. Now only GCCDefs is used instead
 *
 * Revision 1.7  2000/02/06 07:48:17  rahulj
 * Year 2K copyright swat.
 *
 * Revision 1.6  2000/01/14 02:08:29  abagchi
 * Sorry GCC does not support long strings
 *
 * Revision 1.5  2000/01/14 01:55:09  abagchi
 * Added XML_LSTRSUPPORT
 *
 * Revision 1.4  2000/01/13 01:39:11  abagchi
 * Changed unsigned short to wchar_t for XMLCh
 *
 * Revision 1.3  1999/11/12 20:36:54  rahulj
 * Changed library name to xerces-c.lib.
 *
 * Revision 1.1.1.1  1999/11/09 01:07:32  twl
 * Initial checkin
 *
 * Revision 1.2  1999/11/08 20:45:23  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */

#if !defined(GCCDEFS_HPP)
#define GCCDEFS_HPP

// ---------------------------------------------------------------------------
//  Include some runtime files that will be needed product wide
// ---------------------------------------------------------------------------
#include <sys/types.h>  // for size_t and ssize_t
#include <limits.h>  // for MAX of size_t and ssize_t

// ---------------------------------------------------------------------------
//  A define in the build for each project is also used to control whether
//  the export keyword is from the project's viewpoint or the client's.
//  These defines provide the platform specific keywords that they need
//  to do this.
// ---------------------------------------------------------------------------
#define PLATFORM_EXPORT
#define PLATFORM_IMPORT

// ---------------------------------------------------------------------------
//  Indicate that we do not support native bools
//  If the compiler can handle boolean itself, do not define it
// ---------------------------------------------------------------------------
// #define NO_NATIVE_BOOL

// ---------------------------------------------------------------------------
//  Each compiler might support L"" prefixed constants. There are places
//  where it is advantageous to use the L"" where it supported, to avoid
//  unnecessary transcoding.
//  If your compiler does not support it, don't define this.
// ---------------------------------------------------------------------------
// #define XML_LSTRSUPPORT

// ---------------------------------------------------------------------------
//  Indicate that we support C++ namespace
//  Do not define it if the compile cannot handle C++ namespace
// ---------------------------------------------------------------------------
#define XERCES_HAS_CPP_NAMESPACE

// ---------------------------------------------------------------------------
//  Define our version of the XML character
// ---------------------------------------------------------------------------
typedef unsigned short XMLCh;

// ---------------------------------------------------------------------------
//  Define unsigned 16 and 32 bits integers
// ---------------------------------------------------------------------------
typedef unsigned short  XMLUInt16;
typedef unsigned int    XMLUInt32;

// ---------------------------------------------------------------------------
//  Define signed 32 bits integers
// ---------------------------------------------------------------------------
typedef int             XMLInt32;

// ---------------------------------------------------------------------------
//  XMLSize_t is the unsigned integral type.
// ---------------------------------------------------------------------------
#if defined(_SIZE_T) && defined(SIZE_MAX) && defined(_SSIZE_T) && defined(SSIZE_MAX)
    typedef size_t              XMLSize_t;
    #define XML_SIZE_MAX        SIZE_MAX
    typedef ssize_t             XMLSSize_t;
    #define XML_SSIZE_MAX       SSIZE_MAX
#else
    typedef unsigned long       XMLSize_t;
    #define XML_SIZE_MAX        ULONG_MAX
    typedef long                XMLSSize_t;
    #define XML_SSIZE_MAX       LONG_MAX
#endif

// ---------------------------------------------------------------------------
//  Force on the Xerces debug token if it was on in the build environment
// ---------------------------------------------------------------------------
#if defined(_DEBUG)
#define XERCES_DEBUG
#endif


// ---------------------------------------------------------------------------
//  Provide some common string ops that are different/notavail on GCC
// ---------------------------------------------------------------------------
#if !defined(XML_TRU64) && !defined(XML_GCC)
inline char toupper(const char toUpper)
{
    if ((toUpper >= 'a') && (toUpper <= 'z'))
        return char(toUpper - 0x20);
    return toUpper;
}

inline char tolower(const char toLower)
{
    if ((toLower >= 'A') && (toLower <= 'Z'))
        return char(toLower + 0x20);
    return toLower;
}
#endif

#ifndef __CYGWIN__

int stricmp(const char* const str1, const char* const  str2);
int strnicmp(const char* const str1, const char* const  str2, const unsigned int count);

#endif // ! __CYGWIN__


// ---------------------------------------------------------------------------
//  The name of the DLL that is built by the GCC version of the system.
// ---------------------------------------------------------------------------
const char* const Xerces_DLLName = "libxerces-c";

#endif  // GCCDEFS_HPP

