
#include <dlopen.h>
#include <getdex.h>

#include <linux/limits.h>
#include <unordered_map>
#include <sys/file.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;
char output_path[PATH_MAX];

inline bool startsWith(const char* str, const char* sub) {
	while (*sub) if ((*str++)!=(*sub++)) return false;
	return true;
}

extern "C" bool FixClass(mirror::Class* clz) {
	static char path[PATH_MAX];
	static std::unordered_map<uint32_t, uint32_t> map;
	const DexFile* dexFile = clz->GetDexCache()->GetDexFile();
	if (!dexFile->is_compact_dex_) { // compact dex is not supported, but is easy to support
		LengthPrefixedArray<ArtMethod>& methods = *clz->GetMethods();
		if (!clz->GetMethods()) // no method
			return true;
		LOGE("Fixing class %s, method count: %u", clz->ComputeName()->ToModifiedUtf8().c_str(), methods.size);
		const dex::ClassDef* class_def = clz->GetClassDef();

		pthread_mutex_lock(&mutex);
		sprintf(path, "%s/%zu.dex", output_path, dexFile->Size());
		bool exists = access(path, F_OK)==0;
		int fd = open(path, O_WRONLY | O_CREAT, 0600);
		flock(fd, LOCK_EX);
		pthread_mutex_unlock(&mutex);
		if (!exists) write(fd, dexFile->begin_, dexFile->size_);
		{
			const uint8_t* classData = dexFile->DataPointer<uint8_t>(class_def->class_data_off_);
			if (classData==nullptr) {
				LOGE("Class data is null");
				flock(fd, LOCK_UN);
				close(fd);
				return false;
			}
			uint32_t fieldCount = DecodeUnsignedLeb128(&classData)+DecodeUnsignedLeb128(&classData);
			uint32_t directMethodCount = DecodeUnsignedLeb128(&classData);
			uint32_t virtualMethodCount = DecodeUnsignedLeb128(&classData);
			while (fieldCount--) DecodeUnsignedLeb128(&classData), DecodeUnsignedLeb128(&classData);
			uint32_t index = 0;
			while (directMethodCount--) {
				index += DecodeUnsignedLeb128(&classData);
				DecodeUnsignedLeb128(&classData);
				uint32_t off = DecodeUnsignedLeb128(&classData);
				if (off) map[index] = off;
			}
			index = 0;
			while (virtualMethodCount--) {
				index += DecodeUnsignedLeb128(&classData);
				DecodeUnsignedLeb128(&classData);
				uint32_t off = DecodeUnsignedLeb128(&classData);
				if (off) map[index] = off;
			}
		}
		for (uint32_t i=0;i<methods.size;i++) {
			const ArtMethod* method = &methods[i];
			const CodeItem* codeItem = method->GetCodeItem();
			if (codeItem==nullptr) continue; // native or abstract

			auto iter = map.find(method->dex_method_index_);
			if (iter==map.end()) {
				LOGE("Failed to fix %s.%s, dex_method_index is %u", method->GetDeclaringClass()->ComputeName()->ToModifiedUtf8().c_str(), method->GetName(), method->dex_method_index_);
				continue;
			}
			lseek(fd, iter->second, SEEK_SET);
			StandardCodeItem* item = (StandardCodeItem*) codeItem;
			write(fd, item, OFFSET_OF(StandardCodeItem, insns_));
			write(fd, item->insns_, item->insns_size_in_code_units_*sizeof(uint16_t));
		}
		flock(fd, LOCK_UN);
		close(fd);
		map.clear();
		LOGE("Fixed");
		return true;
	}
	return false;
}

/*DEFINE_HOOK(void, ArtInvoke, (ArtMethod* thiz, void* self, uint32_t* args, uint32_t args_size, void* result, const char* shorty)) {
	static char path[PATH_MAX];
	ObjPtr<mirror::Class> klass = thiz->GetDeclaringClass();
	std::string className = klass->ComputeName()->ToModifiedUtf8();
	if (startsWith(className.c_str(), "com.fmp.")) FixClass(klass.Ptr());
	return ArtInvoke_old(thiz, self, args, args_size, result, shorty);
}*/

GETDEX_EXPORT void Java_com_mivik_getdex_GetDex_fixClass(ARG_STATIC, jobject classObject) {
	Thread* self = ((JNIEnvExt*) env)->self;
	FixClass((mirror::Class*) self->DecodeJObject(classObject).Ptr());
}

GETDEX_EXPORT void Java_com_mivik_getdex_GetDex_setOutputPath(ARG_STATIC, jstring path) {
	const char* chars = env->GetStringUTFChars(path, nullptr);
	strncpy(output_path, chars, PATH_MAX);
	env->ReleaseStringUTFChars(path, chars);
}

GETDEX_EXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	LOGE("GetDex native loaded");
	JNIEnv *env;
	if (vm->GetEnv((void**)&env, JNI_VERSION_1_6)!=JNI_OK) return -1;
	if (env==nullptr) return -1;
	pthread_mutex_init(&mutex, nullptr);

	ndk_init(env);
	getdex_init();
	/*void* image = ndk_dlopen("libart.so", RTLD_LAZY);
	void* ArtInvoke = ndk_dlsym(image, "_ZN3art9ArtMethod6InvokeEPNS_6ThreadEPjjPNS_6JValueEPKc");
	LOGE("Symbol: %p %p", image, ArtInvoke); 
	HOOK_FUNCTION_DYNAMIC(ArtInvoke);*/
	return JNI_VERSION_1_6;
}