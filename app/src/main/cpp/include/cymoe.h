
#pragma once

#include <android/log.h>
#include <jni.h>

#include "substrate.h"

#define CYMOE_INLINE inline
#define CYMOE_EXPORT extern "C" JNIEXPORT JNICALL

#define ARG_STATIC JNIEnv *env, jclass
#define ARG_NORMAL JNIEnv *env, jobject

#define LOG_TAG "GetDex-Native"

#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)

#define HOOK_FUNCTION_DYNAMIC(name) MSHookFunction(name, (void*) &name##_fake, (void**) &name##_old)
#define HOOK_FUNCTION_STATIC(name) MSHookFunction((void*) &name, (void*) &name##_fake, (void**) &name##_old)

#define DEFINE_HOOK(ret,name,args) \
static ret (*name##_old) args;\
static ret name##_fake args 

#include <string.h>
#include <stdio.h>

#include <unwind.h>
#include <dlfcn.h>

namespace {

struct BacktraceState {
	void** current;
	void** end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg) {
	BacktraceState* state = static_cast<BacktraceState*>(arg);
	uintptr_t pc = _Unwind_GetIP(context);
	if (pc) {
		if (state->current == state->end) return _URC_END_OF_STACK;
		else *state->current++ = reinterpret_cast<void*>(pc);
	}
	return _URC_NO_REASON;
}

};

const char* dumpBacktrace(size_t maxCount) {
	static char tmp[2097152];
	void** buffer = new void*[maxCount];
	BacktraceState state = {buffer, buffer + maxCount};
	_Unwind_Backtrace(unwindCallback, &state);
	size_t count = state.current - buffer;
	char *cur = tmp;
	Dl_info info;
	for (size_t idx = 0; idx < count; ++idx) {
		const void* addr = buffer[idx];
		const char* fname = "";
		const char* symbol = "";
		if (dladdr(addr, &info)) {
			if (info.dli_sname) symbol = info.dli_sname;
			if (info.dli_fname) fname = info.dli_fname;
		}
		// if (!strcmp(fname, "/data/app/com.ilongyuan.cytus2.ly.TapTap-sW6b5vs8EvpbMV4rBA4O3w==/lib/arm/libunity.so")) continue;
		cur += sprintf(cur, "#%02zu %p  %s (%s)\n", idx, addr, fname, symbol);
		// LOGI("#%02zu %p  %s (%s)", idx, addr, fname, symbol);
	}
	return tmp;
}

inline bool endsWith(const char *str, const char *tar) {
	size_t strLen = strlen(str);
	size_t tarLen = strlen(tar);
	if (strLen<tarLen) return false;
	return !strcmp(str+strLen-tarLen, tar);
}

typedef int gboolean;
typedef uint32_t guint32;