#ifndef __UTIL_H__
#define __UTIL_H__

#define _CRT_SECURE_NO_DEPRECATE	// remove warning C4996, 

#include "ostype.h"
#include "Lock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#endif

#include <sys/stat.h>
#include <assert.h>


#ifdef _WIN32
#define	snprintf	sprintf_s
#else
#include <stdarg.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#endif

#define NOTUSED_ARG(v) ((void)v)		// used this to remove warning C4100, unreferenced parameter

#define READ_BUF_SIZE	2048
// 实现自定义log输出
void log_custom(...);

#define LOG__(...) log_custom(__VA_ARGS__)
// not thread safe
class CRefObject
{
public:
	CRefObject();
	virtual ~CRefObject();

	void AddRef();
	void ReleaseRef();
private:
	int				m_refCount;
};

uint64_t get_tick_count();
void util_sleep(uint32_t millisecond);

class CStrExplode
{
public:
	CStrExplode(char* str, char seperator);
	virtual ~CStrExplode();

	uint32_t GetItemCnt() { return m_item_cnt; }
	char* GetItem(uint32_t idx) { return m_item_list[idx]; }
private:
	uint32_t	m_item_cnt;
	char** 		m_item_list;
};

char* replaceStr(char* pSrc, char oldChar, char newChar);
std::string int2string(uint32_t user_id);
uint32_t string2int(const std::string& value);
void replace_mark(std::string& str, std::string& new_value, uint32_t& begin_pos);
void replace_mark(std::string& str, uint32_t new_value, uint32_t& begin_pos);

void writePid();
inline unsigned char toHex(const unsigned char &x);
inline unsigned char fromHex(const unsigned char &x);
std::string URLEncode(const std::string &sIn);
std::string URLDecode(const std::string &sIn);


int64_t get_file_size(const char *path);
const char*  memfind(const char *src_str,size_t src_len, const char *sub_str, size_t sub_len, bool flag = true);


#endif
