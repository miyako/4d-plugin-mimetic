/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.h
 #	source generated by 4D Plugin Wizard
 #	Project : MIME
 #	author : miyako
 #	2017/06/20
 #
 # --------------------------------------------------------------------------------*/

#include "mimetic/mimetic.h"
#include "libjson/libjson.h"
#include "iconv.h"

#include "helpers.h"
#include "rfc822.h"
#include "rfc2047.h"

//need to load windows.h after the above libraries 

#include "4DPluginAPI.h"

#if defined(_WIN32)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

// --- MIME

void MIME_PARSE_MESSAGE(PA_PluginParameters params);
void MIME_Create_message(PA_PluginParameters params);

#define ICONV_BUFFER_SIZE 1024
