#pragma once

#ifndef UNICODE
	#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdint.h>
#include <windows.h>

#if _DEBUG
	#include <string.h>
	#include <stdarg.h>
	#include <stdio.h>
	#include <stdlib.h>

	#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#else
	#define Assert(cond)
	#define __FILENAME__ '\0'
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;

typedef struct String
{
	u32 length;
	i8 *text;

	String(u32 length, i8 *text)
	{
		this->length = length;
		this->text = (i8 *)VirtualAlloc(nullptr, (length * sizeof(i8)) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	}

	~String()
	{
		VirtualFree(this->text, 0, MEM_RELEASE);
	}

	i8 operator[](u32 index)
	{
		if (index >= 0 && index < length)
		{
			return text[index];
		}
		return (i8)(-1);
	}
} String;
