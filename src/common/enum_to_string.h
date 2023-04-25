//==============================================================================
//
//   enum_to_string.h - exported_h
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================


#ifndef __ENUM_TO_STRING__
#define __ENUM_TO_STRING__

#undef DECL_ENUM_ELEMENT
#undef BEGIN_ENUM
#undef END_ENUM

#ifndef GENERATE_ENUM_STRINGS
    #define DECL_ENUM_ELEMENT( element ) element
	#define DECL_ENUM_ELEMENT_VALUE( element, value ) element = value
    #define BEGIN_ENUM( ENUM_NAME ) typedef enum tag##ENUM_NAME
    #define END_ENUM( ENUM_NAME ) ENUM_NAME; char* GetString##ENUM_NAME(enum tag##ENUM_NAME index);
#else
    #define DECL_ENUM_ELEMENT( element ) #element
    #define BEGIN_ENUM( ENUM_NAME ) char* gs_##ENUM_NAME [] =
    #define END_ENUM( ENUM_NAME ) ; char* GetString##ENUM_NAME(enum tag##ENUM_NAME index){ return gs_##ENUM_NAME [index]; }
#endif

#endif //__ENUM_TO_STRING__

