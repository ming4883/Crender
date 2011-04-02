/*
** Copyright (c) 2002  Hughes Technologies Pty Ltd.  All rights
** reserved.
**
** Terms under which this software may be used or copied are
** provided in the  specific license associated with this product.
**
** Hughes Technologies disclaims all warranties with regard to this
** software, including all implied warranties of merchantability and
** fitness, in no event shall Hughes Technologies be liable for any
** special, indirect or consequential damages or any damages whatsoever
** resulting from loss of use, data or profits, whether in an action of
** contract, negligence or other tortious action, arising out of or in
** connection with the use or performance of this software.
**
**
** $Id: protocol.c,v 1.14 2004/09/24 06:24:50 bambi Exp $
**
*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#if defined(ANDROID) || defined(TARGET_OS_IPHONE)
#define HTTPD_HAS_IO 0
#else
#define HTTPD_HAS_IO 1
#endif

#if HTTPD_HAS_IO
#include <io.h>
#endif

#if defined(_WIN32)
#include <Winsock2.h>
#else
#include <unistd.h>
#include <sys/file.h>
#endif

#include "config.h"
#include "httpd.h"
#include "httpd_priv.h"


int _httpd_net_read(sock, buf, len)
	int	sock;
	char	*buf;
	int	len;
{
#if defined(_WIN32)
	return( recv(sock, buf, len, 0));
#else
	return( read(sock, buf, len));
#endif
}


int _httpd_net_write(sock, buf, len)
	int	sock;
	char	*buf;
	int	len;
{
#if defined(_WIN32)
	return( send(sock, buf, len, 0));
#else
	return( write(sock, buf, len));
#endif
}

int _httpd_readChar(server, cp)
	httpd	*server;
	char	*cp;
{
	if (server->readBufRemain == 0)
	{
		bzero(server->readBuf, HTTP_READ_BUF_LEN + 1);
		server->readBufRemain = _httpd_net_read(server->clientSock,
			server->readBuf, HTTP_READ_BUF_LEN);
		if (server->readBufRemain < 1)
			return(0);
		server->readBuf[server->readBufRemain] = 0;
		server->readBufPtr = server->readBuf;
	}
	*cp = *server->readBufPtr++;
	server->readBufRemain--;
	return(1);
}


int _httpd_readLine(server, destBuf, len)
	httpd	*server;
	char	*destBuf;
	int	len;
{
	char	curChar,
		*dst;
	int	count;


	count = 0;
	dst = destBuf;
	while(count < len)
	{
		if (_httpd_readChar(server, &curChar) < 1)
			return(0);
		if (curChar == '\n')
		{
			*dst = 0;
			return(1);
		}
		if (curChar == '\r')
		{
			continue;
		}
		else
		{
			*dst++ = curChar;
			count++;
		}
	}
	*dst = 0;
	return(1);
}


int _httpd_readBuf(server, destBuf, len)
	httpd	*server;
	char	*destBuf;
	int	len;
{
	char	curChar,
		*dst;
	int	count;


	count = 0;
	dst = destBuf;
	while(count < len)
	{
		if (_httpd_readChar(server, &curChar) < 1)
			return(0);
		*dst++ = curChar;
		count++;
	}
	return(1);
}

int _httpd_sendExpandedText(server, buf, bufLen)
	httpd	*server;
	char	*buf;
	int	bufLen;
{
	char	*textStart,
		*textEnd,
		*varStart,
		*varEnd,
		varName[HTTP_MAX_VAR_NAME_LEN + 1];
	int	length,
		offset;
	httpVar	*var;

	length = offset = 0;
	textStart = buf;
	while(offset < bufLen)
	{
		/*
		** Look for the start of a variable name
		*/
		textEnd = strchr(textStart,'$');
		if (!textEnd)
		{
			/*
			** Nope.  Write the remainder and bail
			*/
			_httpd_net_write(server->clientSock,
				textStart, bufLen - offset);
			length += bufLen - offset;
			offset += bufLen - offset;
			break;
		}

		/*
		** Looks like there could be a variable.  Send the
		** preceeding text and check it out
		*/
		_httpd_net_write(server->clientSock, textStart,
			textEnd - textStart);
		length += textEnd - textStart;
		offset  += textEnd - textStart;
		varEnd = strchr(textEnd, '}');
		if (*(textEnd + 1) != '{' || varEnd == NULL)
		{
			/*
			** Nope, false alarm.
			*/
			_httpd_net_write(server->clientSock, "$",1 );
			length += 1;
			offset += 1;
			textStart = textEnd + 1;
			continue;
		}

		/*
		** OK, looks like an embedded variable
		*/
		varStart = textEnd + 2;
		varEnd = varEnd - 1;
		if (varEnd - varStart > HTTP_MAX_VAR_NAME_LEN)
		{
			/*
			** Variable name is too long
			*/
			_httpd_net_write(server->clientSock, "$", 1);
			length += 1;
			offset += 1;
			textStart = textEnd + 1;
			continue;
		}

		/*
		** Is this a known variable?
		*/
		bzero(varName, HTTP_MAX_VAR_NAME_LEN);
		//strncpy_s(varName, HTTP_MAX_VAR_NAME_LEN, varStart, varEnd - varStart + 1);
		strncpy(varName, varStart, varEnd - varStart + 1);
		offset += strlen(varName) + 3;
		var = httpdGetVariableByName(server, varName);
		if (!var)
		{
			/*
			** Nope.  It's undefined.  Ignore it
			*/
			textStart = varEnd + 2;
			continue;
		}

		/*
		** Write the variables value and continue
		*/
		_httpd_net_write(server->clientSock, var->value,
			strlen(var->value));
		length += strlen(var->value);
		textStart = varEnd + 2;
	}
	return(length);
}


void _httpd_writeAccessLog(server)
	httpd	*server;
{
	char	dateBuf[30];
	struct 	tm *timePtr;
	time_t	clock;
	int	responseCode;


	if (server->accessLog == NULL)
		return;
	clock = time(NULL);
	timePtr = localtime(&clock);
	strftime(dateBuf, 30, "%d/%b/%Y:%X %Z",  timePtr);
	responseCode = atoi(server->response.response);
	fprintf(server->accessLog, "%s - - [%s] %s \"%s\" %d %d\n",
		server->clientAddr, dateBuf, httpdRequestMethodName(server),
		httpdRequestPath(server), responseCode,
		server->response.responseLength);
}

void _httpd_writeErrorLog(server, level, message)
	httpd	*server;
	char	*level,
		*message;
{
	char	dateBuf[30];
	struct 	tm *timePtr;
	time_t	clock;


	if (server->errorLog == NULL)
		return;
	clock = time(NULL);
	timePtr = localtime(&clock);
	strftime(dateBuf, 30, "%a %b %d %X %Y",  timePtr);
	if (*server->clientAddr != 0)
	{
		fprintf(server->errorLog, "[%s] [%s] [client %s] %s\n",
			dateBuf, level, server->clientAddr, message);
	}
	else
	{
		fprintf(server->errorLog, "[%s] [%s] %s\n",
			dateBuf, level, message);
	}
}



int _httpd_decode (bufcoded, bufplain, outbufsize)
	char *		bufcoded;
	char *		bufplain;
	int		outbufsize;
{
	static char six2pr[64] = {
    		'A','B','C','D','E','F','G','H','I','J','K','L','M',
    		'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    		'a','b','c','d','e','f','g','h','i','j','k','l','m',
    		'n','o','p','q','r','s','t','u','v','w','x','y','z',
    		'0','1','2','3','4','5','6','7','8','9','+','/'
	};

	static unsigned char pr2six[256];

	/* single character decode */
#	define DEC(c) pr2six[(int)c]
#	define _DECODE_MAXVAL 63

	static int first = 1;

	int nbytesdecoded, j;
	register char *bufin = bufcoded;
	register unsigned char *bufout = bufplain;
	register int nprbytes;

	/*
	** If this is the first call, initialize the mapping table.
	** This code should work even on non-ASCII machines.
	*/
	if(first)
	{
		first = 0;
		for(j=0; j<256; j++) pr2six[j] = _DECODE_MAXVAL+1;
		for(j=0; j<64; j++) pr2six[(int)six2pr[j]] = (unsigned char)j;
	}

   	/* Strip leading whitespace. */

   	while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

	/*
	** Figure out how many characters are in the input buffer.
	** If this would decode into more bytes than would fit into
	** the output buffer, adjust the number of input bytes downwards.
	*/
	bufin = bufcoded;
	while(pr2six[(int)*(bufin++)] <= _DECODE_MAXVAL);
	nprbytes = bufin - bufcoded - 1;
	nbytesdecoded = ((nprbytes+3)/4) * 3;
	if(nbytesdecoded > outbufsize)
	{
		nprbytes = (outbufsize*4)/3;
	}
	bufin = bufcoded;

	while (nprbytes > 0)
	{
		*(bufout++)=(unsigned char)(DEC(*bufin)<<2|DEC(bufin[1])>>4);
		*(bufout++)=(unsigned char)(DEC(bufin[1])<<4|DEC(bufin[2])>>2);
		*(bufout++)=(unsigned char)(DEC(bufin[2])<<6|DEC(bufin[3]));
		bufin += 4;
		nprbytes -= 4;
	}
	if(nprbytes & 03)
	{
		if(pr2six[(int)bufin[-2]] > _DECODE_MAXVAL)
		{
			nbytesdecoded -= 2;
		}
		else
		{
			nbytesdecoded -= 1;
		}
	}
	bufplain[nbytesdecoded] = 0;
	return(nbytesdecoded);
}



char _httpd_from_hex (c)
        char    c;
{
    return  c >= '0' && c <= '9' ?  c - '0'
            : c >= 'A' && c <= 'F'? c - 'A' + 10
            : c - 'a' + 10;     /* accept small letters just in case */
}

char * _httpd_unescape(str)
        char    *str;
{
    char * p = str;
    char * q = str;
    static char blank[] = "";

    if (!str)
        return(blank);
    while(*p) {
        if (*p == '%') {
            p++;
            if (*p) *q = _httpd_from_hex(*p++) * 16;
            if (*p) *q = (*q + _httpd_from_hex(*p++));
            q++;
        } else {
            if (*p == '+') {
                *q++ = ' ';
                p++;
            } else {
                *q++ = *p++;
              }
        }
    }

    *q++ = 0;
    return str;
}


void _httpd_freeVariables(var)
	httpVar	*var;
{
	httpVar	*curVar, *lastVar;

	if (var == NULL)
		return;
	_httpd_freeVariables(var->nextVariable);
	var->nextVariable = NULL;
	curVar = var;
	while(curVar)
	{
		lastVar = curVar;
		curVar = curVar->nextValue;
		free(lastVar->name);
		free(lastVar->value);
		free(lastVar);
	}
	return;
}

void _httpd_storeData(server, query)
	httpd	*server;
        char    *query;
{
        char    *cp,
		*cp2,
                var[50],
                *val,
		*tmpVal;

        if (!query)
                return;

	cp = query;
	cp2 = var;
        bzero(var,sizeof(var));
	val = NULL;
        while(*cp)
        {
                if (*cp == '=')
                {
                        cp++;
			*cp2 = 0;
                        val = cp;
                        continue;
                }
                if (*cp == '&')
                {
			*cp = 0;
			tmpVal = _httpd_unescape(val);
			httpdAddVariable(server, var, tmpVal);
                        cp++;
                        cp2 = var;
			val = NULL;
                        continue;
                }
		if (val)
		{
			cp++;
		}
		else
		{
                	*cp2 = *cp++;
			if (*cp2 == '.')
			{
				//strcpy_s(cp2, 50,"_dot_");
				strcpy(cp2, "_dot_");
				cp2 += 5;
			}
			else
			{
				cp2++;
			}
		}
        }
	*cp = 0;
	tmpVal = _httpd_unescape(val);
	httpdAddVariable(server, var, tmpVal);
}


void _httpd_formatTimeString(server, ptr, clock)
	httpd	*server;
	char	*ptr;
	int	clock;
{
	struct 	tm *timePtr;
	time_t ptm;
	if (clock == 0){
		ptm = time(NULL);
		timePtr = gmtime(&ptm);
	}
	else {
		ptm = clock;
		timePtr = gmtime(&ptm);
	}
	strftime(ptr, HTTP_TIME_STRING_LEN,"%a, %d %b %Y %X GMT",timePtr);
}


void _httpd_sendHeaders(server, contentLength, modTime)
	httpd	*server;
	int	contentLength,
		modTime;
{
	char	tmpBuf[80],
		timeBuf[HTTP_TIME_STRING_LEN];
	bzero(timeBuf, HTTP_TIME_STRING_LEN);

	if(server->response.headersSent)
		return;

	server->response.headersSent = 1;
	_httpd_net_write(server->clientSock, "HTTP/1.0 ", 9);
	_httpd_net_write(server->clientSock, server->response.response,
		strlen(server->response.response));
	_httpd_net_write(server->clientSock, server->response.headers,
		strlen(server->response.headers));

	_httpd_formatTimeString(server, timeBuf, 0);
	_httpd_net_write(server->clientSock,"Date: ", 6);
	_httpd_net_write(server->clientSock, timeBuf, strlen(timeBuf));
	_httpd_net_write(server->clientSock, "\n", 1);

	_httpd_net_write(server->clientSock, "Connection: close\n", 18);
	_httpd_net_write(server->clientSock, "Content-Type: ", 14);
	_httpd_net_write(server->clientSock, server->response.contentType,
		strlen(server->response.contentType));
	_httpd_net_write(server->clientSock, "\n", 1);

	if (contentLength > 0)
	{
		_httpd_net_write(server->clientSock, "Content-Length: ", 16);
		snprintf(tmpBuf, sizeof(tmpBuf), "%d", contentLength);
		_httpd_net_write(server->clientSock, tmpBuf, strlen(tmpBuf));
		_httpd_net_write(server->clientSock, "\n", 1);

		_httpd_formatTimeString(server, timeBuf, modTime);
		_httpd_net_write(server->clientSock, "Last-Modified: ", 15);
		_httpd_net_write(server->clientSock, timeBuf, strlen(timeBuf));
		_httpd_net_write(server->clientSock, "\n", 1);
	}
	_httpd_net_write(server->clientSock, "\n", 1);
}

httpDir *_httpd_findContentDir(server, dir, createFlag)
	httpd	*server;
	char	*dir;
	int	createFlag;
{
	char	buffer[HTTP_MAX_URL],
		*curDir;
	httpDir	*curItem,
		*curChild;

	//strncpy_s(buffer, HTTP_MAX_URL, dir, HTTP_MAX_URL);
	strncpy(buffer, dir, HTTP_MAX_URL);
	curItem = server->content;
	curDir = strtok(buffer,"/");
	while(curDir)
	{
		curChild = curItem->children;
		while(curChild)
		{
			if (strcmp(curChild->name, curDir) == 0)
				break;
			curChild = curChild->next;
		}
		if (curChild == NULL)
		{
			if (createFlag == HTTP_TRUE)
			{
				curChild = malloc(sizeof(httpDir));
				bzero(curChild, sizeof(httpDir));
				curChild->name = strdup(curDir);
				curChild->next = curItem->children;
				curItem->children = curChild;
			}
			else
			{
				return(NULL);
			}
		}
		curItem = curChild;
		curDir = strtok(NULL,"/");
	}
	return(curItem);
}


httpContent *_httpd_findContentEntry(server, dir, entryName)
	httpd	*server;
	httpDir	*dir;
	char	*entryName;
{
	httpContent *curEntry;

	curEntry = dir->entries;
	while(curEntry)
	{
		if (curEntry->type == HTTP_WILDCARD ||
		    curEntry->type ==HTTP_C_WILDCARD)
			break;
		if (*entryName == 0 && curEntry->strchrFlag)
			break;
		if (strcmp(curEntry->name, entryName) == 0)
			break;
		curEntry = curEntry->next;
	}
	if (curEntry)
		server->response.content = curEntry;
	return(curEntry);
}


void _httpd_send304(server)
	httpd	*server;
{
	httpdSetResponse(server, "304 Not Modified\n");
	if (server->errorFunction304)
	{
		(server->errorFunction304)(server,304);
	}
	else
	{
		_httpd_sendHeaders(server,0,0);
	}
}


void _httpd_send403(server)
	httpd	*server;
{
	httpdSetResponse(server, "403 Permission Denied\n");
	if (server->errorFunction403)
	{
		(server->errorFunction403)(server,403);
	}
	else
	{
		_httpd_sendHeaders(server,0,0);
		_httpd_sendText(server,
		"<HTML><HEAD><TITLE>403 Permission Denied</TITLE></HEAD>\n");
		_httpd_sendText(server,
		"<BODY><H1>Access to the request URL was denied!</H1>\n");
	}
}


void _httpd_send404(server)
	httpd	*server;
{
	char	msg[HTTP_MAX_URL];


	snprintf(msg, HTTP_MAX_URL,
		"File does not exist: %s", server->request.path);
	_httpd_writeErrorLog(server,LEVEL_ERROR, msg);
	httpdSetResponse(server, "404 Not Found\n");
	if (server->errorFunction404)
	{
		(server->errorFunction404)(server,404);
	}
	else
	{
		_httpd_sendHeaders(server,0,0);
		_httpd_sendText(server,
			"<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n");
		_httpd_sendText(server,
			"<BODY><H1>The request URL was not found!</H1>\n");
		_httpd_sendText(server, "</BODY></HTML>\n");
	}
}


void _httpd_catFile(server, path, mode)
	httpd	*server;
	char	*path;
	int	mode;
{
#if HTTPD_HAS_IO
	int	fd,
		readLen,
		writeLen;
	char	buf[HTTP_MAX_LEN];

	fd = _open(path,_O_RDONLY | _O_BINARY);
	if (fd < 0)
		return;
	readLen = _read(fd, buf, HTTP_MAX_LEN - 1);
	while(readLen > 0)
	{
		if (mode == HTTP_RAW_DATA)
		{
			server->response.responseLength += readLen;
			_httpd_net_write(server->clientSock, buf, readLen);
		}
		else
		{
			buf[readLen] = 0;
			writeLen = _httpd_sendExpandedText(server,buf,readLen);
			server->response.responseLength += writeLen;
		}
		readLen = _read(fd, buf, HTTP_MAX_LEN - 1);
	}
	_close(fd);
#endif
}


void _httpd_sendStatic(server, data)
	httpd	*server;
	char	*data;
{
	if (_httpd_checkLastModified(server,server->startTime) == 0)
	{
		_httpd_send304(server);
	}
	_httpd_sendHeaders(server, server->startTime, strlen(data));
	httpdOutput(server, data);
}



void _httpd_sendFile(server, path)
	httpd	*server;
	char	*path;
{
	char	*suffix;
	struct 	stat sbuf;

	suffix = strrchr(path, '.');
	if (suffix != NULL)
	{
		if (strcasecmp(suffix,".gif") == 0)
			strcpy(server->response.contentType,"image/gif");
		if (strcasecmp(suffix,".bmp") == 0)
			strcpy(server->response.contentType,"image/x-windows-bmp");
		if (strcasecmp(suffix,".jpg") == 0)
			strcpy(server->response.contentType,"image/jpeg");
		if (strcasecmp(suffix,".xbm") == 0)
			strcpy(server->response.contentType,"image/xbm");
		if (strcasecmp(suffix,".png") == 0)
			strcpy(server->response.contentType,"image/png");
		if (strcasecmp(suffix,".css") == 0)
			strcpy(server->response.contentType,"text/css");
		if (strcasecmp(suffix,".rtf") == 0)
			strcpy(server->response.contentType,"text/rtf");
		if (strcasecmp(suffix,".tif") == 0)
			strcpy(server->response.contentType,"image/tiff");
		if (strcasecmp(suffix,".ico") == 0)
			strcpy(server->response.contentType,"image/x-icon");
		if (strcasecmp(suffix,".wbmp") == 0)
			strcpy(server->response.contentType,"image/vnd.wap.wbmp");
		if (strcasecmp(suffix,".pdf") == 0)
			strcpy(server->response.contentType,"application/pdf");
		if (strcasecmp(suffix,".csv") == 0)
			strcpy(server->response.contentType,"application/csv");
		if (strcasecmp(suffix,".swf") == 0)
			strcpy(server->response.contentType,"application/x-shockwave-flash");
		if (strcasecmp(suffix,".js") == 0)
			strcpy(server->response.contentType,"application/x-javascript");
		if (strcasecmp(suffix,".xml") == 0)
			strcpy(server->response.contentType,"application/xml");
		if (strcasecmp(suffix,".wml") == 0)
			strcpy(server->response.contentType,"application/vnd.wap.wml");
		if (strcasecmp(suffix,".xhtml") == 0)
			strcpy(server->response.contentType,"application/xhtml+xml");
		if (strcasecmp(suffix,".doc") == 0)
			strcpy(server->response.contentType,"application/msword");
		if (strcasecmp(suffix,".xls") == 0)
			strcpy(server->response.contentType,"application/vnd.ms-excel");
		if (strcasecmp(suffix,".ppt") == 0)
			strcpy(server->response.contentType,"application/vnd.ms-powerpoint");
		if (strcasecmp(suffix,".zip") == 0)
			strcpy(server->response.contentType,"application/zip");
		if (strcasecmp(suffix,".exe") == 0)
			strcpy(server->response.contentType,"application/octet-stream");
	}
	if (stat(path, &sbuf) < 0)
	{
		_httpd_send404(server);
		return;
	}
	if (_httpd_checkLastModified(server,(int)sbuf.st_mtime) == 0)
	{
		_httpd_send304(server);
	}
	else
	{
		_httpd_sendHeaders(server, sbuf.st_size, (int)sbuf.st_mtime);

		if (strncmp(server->response.contentType,"text/",5) == 0)
			_httpd_catFile(server, path, HTTP_EXPAND_TEXT);
		else
			_httpd_catFile(server, path, HTTP_RAW_DATA);
	}
}


int _httpd_sendDirectoryEntry(server, entry, entryName)
	httpd		*server;
	httpContent	*entry;
	char		*entryName;
{
	char		path[HTTP_MAX_URL];

	snprintf(path, HTTP_MAX_URL, "%s\\%s", entry->path, entryName);
	_httpd_sendFile(server,path);
	return(0);
}


void _httpd_sendText(server, msg)
	httpd	*server;
	char	*msg;
{
	server->response.responseLength += strlen(msg);
	_httpd_net_write(server->clientSock,msg,strlen(msg));
}


int _httpd_checkLastModified(server, modTime)
	httpd	*server;
	int	modTime;
{
	char 	timeBuf[HTTP_TIME_STRING_LEN];

	_httpd_formatTimeString(server, timeBuf, modTime);
	if (strcmp(timeBuf, server->request.ifModified) == 0)
		return(0);
	return(1);
}


static unsigned char isAcceptable[96] =

/* Overencodes */
#define URL_XALPHAS     (unsigned char) 1
#define URL_XPALPHAS    (unsigned char) 2

/*      Bit 0           xalpha          -- see HTFile.h
**      Bit 1           xpalpha         -- as xalpha but with plus.
**      Bit 2 ...       path            -- as xpalpha but with /
*/
    /*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
    {    7,0,0,0,0,0,0,0,0,0,7,0,0,7,7,7,       /* 2x   !"#$%&'()*+,-./ */
         7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,0,       /* 3x  0123456789:;<=>?  */
         7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       /* 4x  @ABCDEFGHIJKLMNO */
         7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,7,       /* 5X  PQRSTUVWXYZ[\]^_ */
         0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,       /* 6x  `abcdefghijklmno */
         7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0 };     /* 7X  pqrstuvwxyz{\}~ DEL */

#define ACCEPTABLE(a)   ( a>=32 && a<128 && ((isAcceptable[a-32]) & mask))

static char *hex = "0123456789ABCDEF";


char *_httpd_escape(str)
        char *str;
{
    unsigned char mask = URL_XPALPHAS;
    char * p;
    char * q;
    char * result;
    int unacceptable = 0;
    for(p=str; *p; p++)
        if (!ACCEPTABLE((unsigned char)*p))
                unacceptable +=2;
    result = (char *) malloc(p-str + unacceptable + 1);
    bzero(result,(p-str + unacceptable + 1));

    if (result == NULL)
    {
	return(NULL);
    }
    for(q=result, p=str; *p; p++) {
        unsigned char a = *p;
        if (!ACCEPTABLE(a)) {
            *q++ = '%';  /* Means hex commming */
            *q++ = hex[a >> 4];
            *q++ = hex[a & 15];
        }
        else *q++ = *p;
    }
    *q++ = 0;                   /* Terminate */
    return result;
}



void _httpd_sanitiseUrl(url)
	char	*url;
{
	char	*from,
		*to,
		*last;

	/*
	** Remove multiple slashes
	*/
	from = to = url;
	while(*from)
	{
		if (*from == '/' && *(from+1) == '/')
		{
			from++;
			continue;
		}
		*to = *from;
		to++;
		from++;
	}
	*to = 0;


	/*
	** Get rid of ./ sequences
	*/
	from = to = url;
	while(*from)
	{
		if (*from == '/' && *(from+1) == '.' && *(from+2)=='/')
		{
			from += 2;
			continue;
		}
		*to = *from;
		to++;
		from++;
	}
	*to = 0;


	/*
	** Catch use of /../ sequences and remove them.  Must track the
	** path structure and remove the previous path element.
	*/
	from = to = last = url;
	while(*from)
	{
		if (*from == '/' && *(from+1) == '.' &&
			*(from+2)=='.' && *(from+3)=='/')
		{
			to = last;
			from += 3;
			continue;
		}
		if (*from == '/')
		{
			last = to;
		}
		*to = *from;
		to++;
		from++;
	}
	*to = 0;
}
