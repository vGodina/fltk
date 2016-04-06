//
// "$Id$"
//
// Definition of Apple Darwin system driver.
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//


#include "../../config_lib.h"
#include "Fl_Darwin_System_Driver.H"
#include <FL/Fl.H>
#include <string.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
#include <xlocale.h>
#endif
#include <locale.h>
#include <stdio.h>
#include <dlfcn.h>

extern int fl_mac_os_version;	// the version number of the running Mac OS X

//const char* fl_local_alt   = "\xe2\x8c\xa5\\"; // U+2325 (option key)
const char* fl_local_alt   = "⌥\\"; // U+2325 (option key)
//const char* fl_local_ctrl  = "\xe2\x8c\x83\\"; // U+2303 (up arrowhead)
const char* fl_local_ctrl  = "⌃\\"; // U+2303 (up arrowhead)
//const char* fl_local_meta  = "\xe2\x8c\x98\\"; // U+2318 (place of interest sign)
const char* fl_local_meta  = "⌘\\"; // U+2318 (place of interest sign)
//const char* fl_local_shift = "\xe2\x87\xa7\\"; // U+21E7 (upwards white arrow)
const char* fl_local_shift = "⇧\\"; // U+21E7 (upwards white arrow)


/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_Darwin_System_Driver();
}

Fl_Darwin_System_Driver::Fl_Darwin_System_Driver() {
  if (fl_mac_os_version == 0) fl_mac_os_version = calc_mac_os_version();
}

int Fl_Darwin_System_Driver::single_arg(const char *arg) {
  // The Finder application in MacOS X passes the "-psn_N_NNNNN" option to all apps.
  return (strncmp(arg, "psn_", 4) == 0);
}

int Fl_Darwin_System_Driver::arg_and_value(const char *name, const char *value) {
  // Xcode in MacOS X may pass "-NSDocumentRevisionsDebugMode YES"
  return strcmp(name, "NSDocumentRevisionsDebugMode") == 0;
}

int Fl_Darwin_System_Driver::clocale_printf(FILE *output, const char *format, va_list args) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) {
    static locale_t postscript_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
    return vfprintf_l(output, postscript_locale, format, args);
  }
#endif
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}

/* Returns the address of a Carbon function after dynamically loading the Carbon library if needed.
 Supports old Mac OS X versions that may use a couple of Carbon calls:
 GetKeys used by OS X 10.3 or before (in Fl::get_key())
 PMSessionPageSetupDialog and PMSessionPrintDialog used by 10.4 or before (in Fl_Printer::start_job())
 */
void *Fl_Darwin_System_Driver::get_carbon_function(const char *function_name) {
  static void *carbon = dlopen("/System/Library/Frameworks/Carbon.framework/Carbon", RTLD_LAZY);
  return (carbon ? dlsym(carbon, function_name) : NULL);
}

//
// End of "$Id$".
//