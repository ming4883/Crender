/* src/config.h.in.  Generated automatically from configure.in by autoheader.  */

#define HAVE_STDARG_H

#define HAVE_STRING_H

/* Define if you have the <strings.h> header file.  */
#undef HAVE_STRINGS_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

/* Define if you have the nsl library (-lnsl).  */
#undef HAVE_LIBNSL

/* Define if you have the socket library (-lsocket).  */
#undef HAVE_LIBSOCKET

/* Define if you have the socket library (-lsocket).  */
#undef HAVE_EMBER

#if defined(_WIN32)
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define snprintf sprintf_s
#endif