//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000-2005  W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Character code conversion
//---------------------------------------------------------------------------

#ifndef __CharacterSet_H__
#define __CharacterSet_H__

// various character conding conversion.
// currently only utf-8 related functions are implemented.
#include "tjsTypes.h"


extern tjs_int TVPWideCharToUtf8String(const tjs_char *in, char * out);
extern tjs_int TVPUtf8ToWideCharString(const char * in, tjs_char *out);



#endif