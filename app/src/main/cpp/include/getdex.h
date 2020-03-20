
#ifndef __MIVIK_GETDEX_H_
#define __MIVIK_GETDEX_H_

#include <android/log.h>
#include <dlopen.h>
#include <stdint.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <atomic>
#include <memory>
#include <string>
#include <jni.h>

#include "substrate.h"

#define LOG_TAG "GetDex-Native"

#define GETDEX_EXPORT extern "C" JNIEXPORT JNICALL

#define ARG_STATIC JNIEnv *env, jclass
#define ARG_NORMAL JNIEnv *env, jobject

#define HOOK_FUNCTION_DYNAMIC(name) MSHookFunction(name, (void*) &name##_fake, (void**) &name##_old)
#define HOOK_FUNCTION_STATIC(name) MSHookFunction((void*) &name, (void*) &name##_fake, (void**) &name##_old)

#define DEFINE_HOOK(ret,name,args) \
static ret (*name##_old) args;\
static ret name##_fake args 

#define LOGV(...)	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...)	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...)	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)	__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGF(...)	__android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)

#define PACKED(x) __attribute__ ((__aligned__(x), __packed__))
#define LIKELY( exp )			 (__builtin_expect( (exp) != 0, true	))
#define UNLIKELY( exp )		 (__builtin_expect( (exp) != 0, false ))
#define MANAGED PACKED(4)

// #define OFFSET_OF(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
#define OFFSET_OF_HELPER(TYPE, MEMBER) (reinterpret_cast<uintptr_t>(&reinterpret_cast<TYPE*>(0)->MEMBER))
#define OFFSET_OF(TYPE, MEMBER) (__builtin_constant_p(OFFSET_OF_HELPER(TYPE, MEMBER)) ? OFFSET_OF_HELPER(TYPE, MEMBER) : OFFSET_OF_HELPER(TYPE, MEMBER))
#define LOAD_FUNCTION(func, name) do { *((void**) &(func)) = ndk_dlsym(image, name); } while (false)

template<typename T>
constexpr T RoundDown(T x, T n) { return (x & -n); }
template<typename T>
constexpr T RoundUp(T x, typename std::remove_reference<T>::type n) { return RoundDown(x+n-1, n); }

static inline uint32_t DecodeUnsignedLeb128(const uint8_t** data) {
	const uint8_t* ptr = *data;
	int result = *(ptr++);
	if (UNLIKELY(result > 0x7f)) {
		int cur = *(ptr++);
		result = (result & 0x7f) | ((cur & 0x7f) << 7);
		if (cur > 0x7f) {
			cur = *(ptr++);
			result |= (cur & 0x7f) << 14;
			if (cur > 0x7f) {
				cur = *(ptr++);
				result |= (cur & 0x7f) << 21;
				if (cur > 0x7f) {
					cur = *(ptr++);
					result |= cur << 28;
				}
			}
		}
	}
	*data = ptr;
	return static_cast<uint32_t>(result);
}

template<typename To, typename From>
inline To down_cast(From& f) {
	static_assert(std::is_base_of<From, typename std::remove_reference<To>::type>::value, "down_cast unsafe as To is not a subtype of From");
	return static_cast<To>(f);
}

template<typename To, typename From>
inline To down_cast(From* f) {
	static_assert(std::is_base_of<From, typename std::remove_pointer<To>::type>::value, "down_cast unsafe as To is not a subtype of From");
	return static_cast<To>(f);
}

static constexpr uint32_t kAccObsoleteMethod = 0x00040000;

typedef size_t MemberOffset;

namespace dex {
typedef void TypeId, FieldId, ProtoId, MethodHandleItem;
typedef void CallSiteIdItem, HiddenapiClassData;
typedef uint16_t TypeIndex, ProtoIndex;
typedef uint32_t StringIndex;
struct MethodId {
	dex::TypeIndex class_idx_;
	dex::ProtoIndex proto_idx_;
	dex::StringIndex name_idx_;
};
struct StringId {
	uint32_t string_data_off_;
};
struct ClassDef {
	TypeIndex class_idx_;
	uint16_t pad1_;
	uint32_t access_flags_;
	TypeIndex superclass_idx_;
	uint16_t pad2_;
	uint32_t interfaces_off_;
	StringIndex source_file_idx_;
	uint32_t annotations_off_;
	uint32_t class_data_off_;
	uint32_t static_values_off_;
};
}; // namespace dex

template<class MirrorType>
struct ObjPtr {
	uintptr_t reference_;

	inline ObjPtr(MirrorType* ptr): reference_(reinterpret_cast<uintptr_t>(static_cast<MirrorType*>(ptr))) {}
	inline MirrorType* Ptr() const { return reinterpret_cast<MirrorType*>(reference_); }
	inline bool IsNull() const { return reference_==0; }
	inline MirrorType* operator->() const { return Ptr(); }
};

struct DexFile;

namespace mirror {
struct Object;

template<class MirrorType>
struct PtrCompression {
	static inline uint32_t Compress(MirrorType* mirror_ptr) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(mirror_ptr)); }
	static inline MirrorType* Decompress(uint32_t ref) { return reinterpret_cast<MirrorType*>(ref); }
	static inline uint32_t Compress(ObjPtr<MirrorType> ptr) { return Compress(ptr.Ptr()); }
};

template<class MirrorType>
struct CompressedReference {
	using Compression = PtrCompression<MirrorType>;

	uint32_t reference_;

	inline CompressedReference(MirrorType* ptr) { Assign(ptr); }

	inline MirrorType* AsMirrorPtr() const { return Compression::Decompress(reference_); }
	inline void Assign(MirrorType* ptr) { reference_ = Compression::Compress(ptr); }
	inline void Assign(ObjPtr<MirrorType> ptr) { Assign(ptr.Ptr()); }
};
};

typedef void ArtField;
struct ArtMethod;
struct GcRootSource;

struct GcRootSource {
	ArtField* const field;
	ArtMethod* const method;
};

template<typename MirrorType>
struct GcRoot {
	mirror::CompressedReference<mirror::Object> root_;
	inline MirrorType* Read(GcRootSource* gc_root_source = nullptr);
};

template<typename T>
struct LengthPrefixedArray {
	uint32_t size;
	uint8_t data[0];

	static size_t OffsetOfElement(size_t index, size_t element_size = sizeof(T), size_t alignment = alignof(T)) { return RoundUp(OFFSET_OF(LengthPrefixedArray<T>, data), alignment) + index * element_size; }

	T& operator[](size_t index) { return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OffsetOfElement(index)); }
	const T& operator[](size_t index) const { return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + OffsetOfElement(index)); }
};

struct Thread;

mirror::Object* (*artReadBarrierMark_)(mirror::Object* ptr);
mirror::Object* (*artReadBarrierForRootSlow_)(GcRoot<mirror::Object>* root);
ObjPtr<mirror::Object> (*Thread_DecodeJObject)(Thread* thiz, jobject obj);

inline mirror::Object* artReadBarrierMark(mirror::Object* ptr) {
	if (artReadBarrierMark_) return artReadBarrierMark_(ptr);
	return ptr;
}

inline mirror::Object* artReadBarrierForRootSlow(GcRoot<mirror::Object>* root) {
	if (artReadBarrierForRootSlow_) return artReadBarrierForRootSlow_(root);
	return root->root_.AsMirrorPtr();
}

// namespace mirror
namespace mirror {

struct Class;

struct ValueObject {
};

template<class MirrorType>
struct PACKED(4) StackReference : public CompressedReference<MirrorType> {
	StackReference(MirrorType* ptr) : CompressedReference<MirrorType>(ptr) {}
};

template<class T>
struct PACKED(sizeof(T)) Atomic : public std::atomic<T> {
	inline T LoadJavaData() const { return this->load(std::memory_order_relaxed); }
};

template<class MirrorType>
struct MANAGED HeapReference {
	using Compression = PtrCompression<MirrorType>;
	Atomic<uint32_t> reference_;
	inline MirrorType* AsMirrorPtr() { return Compression::Decompress(reference_.LoadJavaData()); }
	inline MirrorType* operator->() { return AsMirrorPtr(); }
};

struct MANAGED Object {
	HeapReference<Class> klass_;
	uint32_t monitor_;
	template<typename T>
	inline T GetFieldPrimitive(MemberOffset field_offset) const {
		const uint8_t* raw_addr = reinterpret_cast<const uint8_t*>(this) + static_cast<int32_t>(field_offset);
		const T* addr = reinterpret_cast<const T*>(raw_addr);
		return reinterpret_cast<T>(reinterpret_cast<const Atomic<T>*>(addr)->LoadJavaData());
	}
	template<class T>
	inline T* GetFieldObject(MemberOffset field_offset) const {
		Object* addr = *(Object**)(reinterpret_cast<const uint8_t*>(this) + static_cast<int32_t>(field_offset));
		if (monitor_&(1U<<28)) addr = artReadBarrierMark(addr);
		return reinterpret_cast<T*>(addr);
	}
	template<class T>
	inline T GetFieldPtr(MemberOffset field_offset) const { return reinterpret_cast<T>(GetFieldPrimitive<uintptr_t>(field_offset)); }
};

struct MANAGED Array : public Object {
	static constexpr size_t kFirstElementOffset = 12u;

	int32_t length_;
	uint32_t first_element_[0];

	static constexpr MemberOffset DataOffset(int firstElementOffset, int componentSize) { return RoundUp(firstElementOffset, componentSize); }
	static constexpr MemberOffset DataOffset(int componentSize) { return RoundUp(OFFSET_OF(Array, first_element_), componentSize); } 
	template<size_t component_size>
	const void* GetRawData(int32_t index) const { return reinterpret_cast<void*>(reinterpret_cast<intptr_t>(this) + static_cast<int32_t>(DataOffset(component_size)) + (index * component_size)); }
	inline int32_t GetLength() { return GetFieldPrimitive<int32_t>(OFFSET_OF(Array, length_)); }
};

template<typename T>
struct MANAGED PrimitiveArray : public Array {
	inline const T* GetData() const { return reinterpret_cast<const T*>(GetRawData<sizeof(T)>(0)); }
	inline T Get(uint32_t i) const { return GetData()[i]; }
};

typedef PrimitiveArray<int32_t> IntArray;
typedef PrimitiveArray<int64_t> LongArray;

struct PointerArray : public Array {
	template<typename T, size_t kPointerSize>
	inline T GetElementPtrSize(uint32_t idx) {
		if (kPointerSize == sizeof(uint64_t)) return (T)static_cast<uintptr_t>(reinterpret_cast<LongArray*>(this)->Get(idx));
		return (T)static_cast<uintptr_t>(reinterpret_cast<IntArray*>(this)->Get(idx));
	}
};

template<class T>
struct MANAGED ObjectArray: public Array {
	static constexpr size_t kHeapReferenceSize = sizeof(uint32_t);
	inline MemberOffset OffsetOfElement(int32_t i) { return MemberOffset(static_cast<int32_t>(DataOffset(kHeapReferenceSize)) + (i * kHeapReferenceSize)); }
	inline ObjPtr<T> Get(int32_t i) { return GetFieldObject<T>(OffsetOfElement(i)); }
};

template<class T>
struct Handle : public ValueObject {
	StackReference<mirror::Object>* reference_;
	inline explicit Handle(StackReference<T>* reference) : reference_(reinterpret_cast<StackReference<Object>*>(reference)) {}
	inline void Assign(T* ptr) {
		reference_->Assign(ptr);
	}
};

typedef void ClassLoader;

struct String;

std::string (*String_ToModifiedUtf8)(String* thiz);
ObjPtr<String> (*Class_ComputeName)(Handle<Class> thiz);

struct String final : public Object {
	int32_t count_;
	uint32_t hash_code_;
	union {
		uint16_t value_[0];
		uint8_t value_compressed_[0];
	};

	std::string ToModifiedUtf8() { return String_ToModifiedUtf8(this); }
};

struct MANAGED DexCache : public Object {
	HeapReference<ClassLoader> class_loader_;
	HeapReference<String> location_;
	uint64_t dex_file_;
	// not complete

	const DexFile* GetDexFile() { return GetFieldPtr<const DexFile*>(OFFSET_OF(DexCache, dex_file_)); }
};

struct MANAGED ClassExt : public Object {
	HeapReference<PointerArray> instance_jfield_ids_;
	HeapReference<PointerArray> jmethod_ids_;
	HeapReference<Class> obsolete_class_;
	HeapReference<ObjectArray<DexCache>> obsolete_dex_caches_;
	HeapReference<PointerArray> obsolete_methods_;
	HeapReference<Object> original_dex_file_;
	HeapReference<PointerArray> static_jfield_ids_;
	HeapReference<Object> verify_error_;
	int64_t pre_redefine_dex_file_ptr_;
	int32_t pre_redefine_class_def_index_;

	inline ObjPtr<PointerArray> GetObsoleteMethods() const { return GetFieldObject<PointerArray>(OFFSET_OF(ClassExt, obsolete_methods_)); }
	inline ObjPtr<ObjectArray<DexCache>> GetObsoleteDexCaches() const { return GetFieldObject<ObjectArray<DexCache>>(OFFSET_OF(ClassExt, obsolete_dex_caches_)); }
};

typedef void IfTable;

struct MANAGED Class final : public Object {
	HeapReference<ClassLoader> class_loader_;
	HeapReference<Class> component_type_;
	HeapReference<DexCache> dex_cache_;
	HeapReference<ClassExt> ext_data_;
	HeapReference<IfTable> iftable_;
	HeapReference<String> name_;
	HeapReference<Class> super_class_;
	HeapReference<PointerArray> vtable_;
	uint64_t ifields_;
	uint64_t methods_;
	uint64_t sfields_;
	uint32_t access_flags_;
	uint32_t class_flags_;
	uint32_t class_size_;
	pid_t clinit_thread_id_;
	int32_t dex_class_def_idx_;
	// not complete

	inline ObjPtr<DexCache> GetDexCache() const { return GetFieldObject<DexCache>(OFFSET_OF(Class, dex_cache_)); }
	inline ObjPtr<String> ComputeName() {
		Handle<Class> handle(new StackReference<Class>(this));
		return Class_ComputeName(handle);
	}
	inline const dex::ClassDef* GetClassDef() const;
	inline LengthPrefixedArray<ArtMethod>* GetMethods() const { return reinterpret_cast<LengthPrefixedArray<ArtMethod>*>(static_cast<uintptr_t>(GetFieldPrimitive<int64_t>(OFFSET_OF(Class, methods_)))); }
	inline const ObjPtr<ClassExt> GetExtData() const { return GetFieldObject<ClassExt>(OFFSET_OF(Class, ext_data_)); }
};
}; // namespace mirror

struct ReadBarrier {
	template<typename MirrorType>
	static MirrorType* BarrierForRoot(mirror::CompressedReference<MirrorType>* root, GcRootSource* gc_root_source) { return root->AsMirrorPtr(); }
};

template<class MirrorType>
inline MirrorType* GcRoot<MirrorType>::Read(GcRootSource* gc_root_source)	{
	return reinterpret_cast<MirrorType*>(ReadBarrier::BarrierForRoot<mirror::Object>(&root_, gc_root_source));
}

struct Item {
 	static constexpr uint32_t kOffsetUnassigned = 0u;
	Item(uint32_t offset, uint32_t size) : offset_(offset), size_(size) { }
	uint32_t offset_ = kOffsetUnassigned;
	uint32_t size_ = 0;
};

struct CodeItem {
};

struct StandardCodeItem : public CodeItem {
	static constexpr size_t kAlignment = 4;

	StandardCodeItem() = default;

	uint16_t registers_size_;
	uint16_t ins_size_;
	uint16_t outs_size_;
	uint16_t tries_size_;
	uint32_t debug_info_off_;
	uint32_t insns_size_in_code_units_;
	uint16_t insns_[1];
};

struct IrCodeItem : public Item {
	typedef void DebugInfoItem;
	typedef void TryItemVector;
	typedef void CatchHandlerVector;
	typedef void CodeFixups;
	uint16_t registers_size_;
	uint16_t ins_size_;
	uint16_t outs_size_;
	DebugInfoItem* debug_info_;	// This can be nullptr.
	uint32_t insns_size_;
	std::unique_ptr<uint16_t[]> insns_;
	std::unique_ptr<TryItemVector> tries_;	// This can be nullptr.
	std::unique_ptr<CatchHandlerVector> handlers_;	// This can be nullptr.
	std::unique_ptr<CodeFixups> fixups_;	// This can be nullptr.
};


struct DexFile {
protected:
	virtual void nothing() {} // make this class virtual
public:
	typedef void OatDexFile, DexFileContainer;
	static constexpr uint32_t MAGIC_NUMBER = 0x0A786564;
	static constexpr size_t kSha1DigestSize = 20;
	struct Header {
		uint8_t magic_[8] = {};
		uint32_t checksum_ = 0;
		uint8_t signature_[kSha1DigestSize] = {};
		uint32_t file_size_ = 0;
		uint32_t header_size_ = 0;
		uint32_t endian_tag_ = 0;
		uint32_t link_size_ = 0;
		uint32_t link_off_ = 0;
		uint32_t map_off_ = 0;
		uint32_t string_ids_size_ = 0;
		uint32_t string_ids_off_ = 0;
		uint32_t type_ids_size_ = 0;
		uint32_t type_ids_off_ = 0;
		uint32_t proto_ids_size_ = 0;
		uint32_t proto_ids_off_ = 0;
		uint32_t field_ids_size_ = 0;
		uint32_t field_ids_off_ = 0;
		uint32_t method_ids_size_ = 0;
		uint32_t method_ids_off_ = 0;
		uint32_t class_defs_size_ = 0;
		uint32_t class_defs_off_ = 0;
		uint32_t data_size_ = 0;
		uint32_t data_off_ = 0;
	};
	uint8_t* begin_;
	size_t size_;
	uint8_t* data_begin_;
	size_t data_size_;
	std::string location_;
	uint32_t location_checksum_;
	Header* header_;
	dex::StringId* string_ids_;
	dex::TypeId* type_ids_;
	dex::FieldId* field_ids_;
	dex::MethodId* method_ids_;
	dex::ProtoId* proto_ids_;
	dex::ClassDef* class_defs_;
	dex::MethodHandleItem* method_handles_;
	size_t num_method_handles_;
	dex::CallSiteIdItem* call_site_ids_;
	size_t num_call_site_ids_;
	dex::HiddenapiClassData* hiddenapi_class_data_;
	mutable OatDexFile* oat_dex_file_;
	std::unique_ptr<DexFileContainer> container_;
	bool is_compact_dex_;
	// not complete

	inline size_t Size() const { return size_; }
	template <typename T>
	const T* DataPointer(size_t offset) const { return (offset != 0u) ? reinterpret_cast<const T*>(data_begin_ + offset) : nullptr; }
	inline const char* GetString(dex::StringIndex index) const {
		uint32_t unicode_length;
		const dex::StringId& string_id = string_ids_[index];
		const uint8_t* ret = DataPointer<uint8_t>(string_id.string_data_off_);
		DecodeUnsignedLeb128(&ret);
		return (char*) ret;
	}
};

inline const dex::ClassDef* mirror::Class::GetClassDef() const { return &GetDexCache()->GetDexFile()->class_defs_[dex_class_def_idx_]; }

struct ArtMethod {
	GcRoot<mirror::Class> declaring_class_;
	std::atomic<uint32_t> access_flags_;
	uint32_t dex_code_item_offset_;
	uint32_t dex_method_index_;
	uint16_t method_index_;
	union {
		uint16_t hotness_count_;
		uint16_t imt_index_;
	};
	struct PtrSizedFields {
		void* data_;
		void* entry_point_from_quick_compiled_code_;
	} ptr_sized_fields_;

	inline uint32_t GetAccessFlags() const { return access_flags_.load(std::memory_order_relaxed); }
	inline mirror::Class* GetDeclaringClass() const { return down_cast<mirror::Class*>(artReadBarrierForRootSlow((GcRoot<mirror::Object>*) (&declaring_class_))); }
	inline ObjPtr<mirror::DexCache> GetDexCache() const {
		if (LIKELY(!IsObsolete())) {
			ObjPtr<mirror::Class> klass = GetDeclaringClass();
			return klass->GetDexCache();
		} else {
			ObjPtr<mirror::ClassExt> ext(GetDeclaringClass()->GetExtData());
			ObjPtr<mirror::PointerArray> obsolete_methods(ext.IsNull() ? nullptr : ext->GetObsoleteMethods());
			int32_t len = (obsolete_methods.IsNull() ? 0 : obsolete_methods->GetLength());
			for (int32_t i = 0; i < len; i++) {
				if (this == obsolete_methods->GetElementPtrSize<ArtMethod*, sizeof(uintptr_t)>(i)) {
					return ext->GetObsoleteDexCaches()->Get(i);
				}
			}
			LOGE("GetDexCache for obsolete dex reaches here - unexpected");
			return GetDeclaringClass()->GetDexCache();
		}
	}
	inline const char* GetName() const { return GetDexFile()->GetString(GetMethodId()->name_idx_); }
	inline const dex::MethodId* GetMethodId() const { return &GetDexFile()->method_ids_[method_index_]; }
	inline const CodeItem* GetCodeItem() const { return GetDexFile()->DataPointer<CodeItem>(dex_code_item_offset_); }
	inline const DexFile* GetDexFile() const { return GetDexCache()->GetDexFile(); }
	inline bool IsObsolete() const { return (GetAccessFlags()&kAccObsoleteMethod) != 0; }
};

struct Thread {
	// not complete
	inline ObjPtr<mirror::Object> DecodeJObject(jobject obj) { return Thread_DecodeJObject(this, obj); }
};

struct JNIEnvExt {
	const struct JNINativeInterface* functions;
	Thread* const self;
	// not complete
};

inline void getdex_init() {
	void* image = ndk_dlopen("libart.so", RTLD_LAZY);
	LOAD_FUNCTION(artReadBarrierMark_, "artReadBarrierMark");
	LOAD_FUNCTION(artReadBarrierForRootSlow_, "artReadBarrierForRootSlow");
	LOAD_FUNCTION(mirror::String_ToModifiedUtf8, "_ZN3art6mirror6String14ToModifiedUtf8Ev");
	LOAD_FUNCTION(mirror::Class_ComputeName, "_ZN3art6mirror5Class11ComputeNameENS_6HandleIS1_EE");
	LOAD_FUNCTION(Thread_DecodeJObject, "_ZNK3art6Thread13DecodeJObjectEP8_jobject");
}

#endif