// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: charlie.proto

#ifndef PROTOBUF_charlie_2eproto__INCLUDED
#define PROTOBUF_charlie_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
// @@protoc_insertion_point(includes)

namespace charlie {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_charlie_2eproto();
void protobuf_AssignDesc_charlie_2eproto();
void protobuf_ShutdownFile_charlie_2eproto();

class CMsgHeader;
class CMsgContainer;
class CSaveContainer;
class CIdentity;
class CModule;

enum EMsgType {
  NO_TYPE = 0,
  PING = 1,
  CLIENT_REGISTER = 2,
  CLIENT_REGISTERED = 3,
  VALIDATION_ERROR = 9000
};
bool EMsgType_IsValid(int value);
const EMsgType EMsgType_MIN = NO_TYPE;
const EMsgType EMsgType_MAX = VALIDATION_ERROR;
const int EMsgType_ARRAYSIZE = EMsgType_MAX + 1;

enum EBodyEncryptionType {
  NO_ENCRYPTION = 0,
  SIGNED = 1,
  XOR_TIMESTAMP = 2
};
bool EBodyEncryptionType_IsValid(int value);
const EBodyEncryptionType EBodyEncryptionType_MIN = NO_ENCRYPTION;
const EBodyEncryptionType EBodyEncryptionType_MAX = XOR_TIMESTAMP;
const int EBodyEncryptionType_ARRAYSIZE = EBodyEncryptionType_MAX + 1;

// ===================================================================

class CMsgHeader : public ::google::protobuf::MessageLite {
 public:
  CMsgHeader();
  virtual ~CMsgHeader();

  CMsgHeader(const CMsgHeader& from);

  inline CMsgHeader& operator=(const CMsgHeader& from) {
    CopyFrom(from);
    return *this;
  }

  static const CMsgHeader& default_instance();

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  // Returns the internal default instance pointer. This function can
  // return NULL thus should not be used by the user. This is intended
  // for Protobuf internal code. Please use default_instance() declared
  // above instead.
  static inline const CMsgHeader* internal_default_instance() {
    return default_instance_;
  }
  #endif

  void Swap(CMsgHeader* other);

  // implements Message ----------------------------------------------

  CMsgHeader* New() const;
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from);
  void CopyFrom(const CMsgHeader& from);
  void MergeFrom(const CMsgHeader& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::std::string GetTypeName() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .charlie.EMsgType type = 1;
  inline bool has_type() const;
  inline void clear_type();
  static const int kTypeFieldNumber = 1;
  inline ::charlie::EMsgType type() const;
  inline void set_type(::charlie::EMsgType value);

  // optional int64 timestamp = 2;
  inline bool has_timestamp() const;
  inline void clear_timestamp();
  static const int kTimestampFieldNumber = 2;
  inline ::google::protobuf::int64 timestamp() const;
  inline void set_timestamp(::google::protobuf::int64 value);

  // @@protoc_insertion_point(class_scope:charlie.CMsgHeader)
 private:
  inline void set_has_type();
  inline void clear_has_type();
  inline void set_has_timestamp();
  inline void clear_has_timestamp();

  ::google::protobuf::int64 timestamp_;
  int type_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  friend void  protobuf_AddDesc_charlie_2eproto_impl();
  #else
  friend void  protobuf_AddDesc_charlie_2eproto();
  #endif
  friend void protobuf_AssignDesc_charlie_2eproto();
  friend void protobuf_ShutdownFile_charlie_2eproto();

  void InitAsDefaultInstance();
  static CMsgHeader* default_instance_;
};
// -------------------------------------------------------------------

class CMsgContainer : public ::google::protobuf::MessageLite {
 public:
  CMsgContainer();
  virtual ~CMsgContainer();

  CMsgContainer(const CMsgContainer& from);

  inline CMsgContainer& operator=(const CMsgContainer& from) {
    CopyFrom(from);
    return *this;
  }

  static const CMsgContainer& default_instance();

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  // Returns the internal default instance pointer. This function can
  // return NULL thus should not be used by the user. This is intended
  // for Protobuf internal code. Please use default_instance() declared
  // above instead.
  static inline const CMsgContainer* internal_default_instance() {
    return default_instance_;
  }
  #endif

  void Swap(CMsgContainer* other);

  // implements Message ----------------------------------------------

  CMsgContainer* New() const;
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from);
  void CopyFrom(const CMsgContainer& from);
  void MergeFrom(const CMsgContainer& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::std::string GetTypeName() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional bytes signed_timestamp = 1;
  inline bool has_signed_timestamp() const;
  inline void clear_signed_timestamp();
  static const int kSignedTimestampFieldNumber = 1;
  inline const ::std::string& signed_timestamp() const;
  inline void set_signed_timestamp(const ::std::string& value);
  inline void set_signed_timestamp(const char* value);
  inline void set_signed_timestamp(const void* value, size_t size);
  inline ::std::string* mutable_signed_timestamp();
  inline ::std::string* release_signed_timestamp();
  inline void set_allocated_signed_timestamp(::std::string* signed_timestamp);

  // optional bytes body = 2;
  inline bool has_body() const;
  inline void clear_body();
  static const int kBodyFieldNumber = 2;
  inline const ::std::string& body() const;
  inline void set_body(const ::std::string& value);
  inline void set_body(const char* value);
  inline void set_body(const void* value, size_t size);
  inline ::std::string* mutable_body();
  inline ::std::string* release_body();
  inline void set_allocated_body(::std::string* body);

  // optional .charlie.EBodyEncryptionType body_encryption = 3;
  inline bool has_body_encryption() const;
  inline void clear_body_encryption();
  static const int kBodyEncryptionFieldNumber = 3;
  inline ::charlie::EBodyEncryptionType body_encryption() const;
  inline void set_body_encryption(::charlie::EBodyEncryptionType value);

  // optional bytes signed_body_hash = 4;
  inline bool has_signed_body_hash() const;
  inline void clear_signed_body_hash();
  static const int kSignedBodyHashFieldNumber = 4;
  inline const ::std::string& signed_body_hash() const;
  inline void set_signed_body_hash(const ::std::string& value);
  inline void set_signed_body_hash(const char* value);
  inline void set_signed_body_hash(const void* value, size_t size);
  inline ::std::string* mutable_signed_body_hash();
  inline ::std::string* release_signed_body_hash();
  inline void set_allocated_signed_body_hash(::std::string* signed_body_hash);

  // @@protoc_insertion_point(class_scope:charlie.CMsgContainer)
 private:
  inline void set_has_signed_timestamp();
  inline void clear_has_signed_timestamp();
  inline void set_has_body();
  inline void clear_has_body();
  inline void set_has_body_encryption();
  inline void clear_has_body_encryption();
  inline void set_has_signed_body_hash();
  inline void clear_has_signed_body_hash();

  ::std::string* signed_timestamp_;
  ::std::string* body_;
  ::std::string* signed_body_hash_;
  int body_encryption_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  friend void  protobuf_AddDesc_charlie_2eproto_impl();
  #else
  friend void  protobuf_AddDesc_charlie_2eproto();
  #endif
  friend void protobuf_AssignDesc_charlie_2eproto();
  friend void protobuf_ShutdownFile_charlie_2eproto();

  void InitAsDefaultInstance();
  static CMsgContainer* default_instance_;
};
// -------------------------------------------------------------------

class CSaveContainer : public ::google::protobuf::MessageLite {
 public:
  CSaveContainer();
  virtual ~CSaveContainer();

  CSaveContainer(const CSaveContainer& from);

  inline CSaveContainer& operator=(const CSaveContainer& from) {
    CopyFrom(from);
    return *this;
  }

  static const CSaveContainer& default_instance();

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  // Returns the internal default instance pointer. This function can
  // return NULL thus should not be used by the user. This is intended
  // for Protobuf internal code. Please use default_instance() declared
  // above instead.
  static inline const CSaveContainer* internal_default_instance() {
    return default_instance_;
  }
  #endif

  void Swap(CSaveContainer* other);

  // implements Message ----------------------------------------------

  CSaveContainer* New() const;
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from);
  void CopyFrom(const CSaveContainer& from);
  void MergeFrom(const CSaveContainer& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::std::string GetTypeName() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .charlie.CIdentity identity = 1;
  inline bool has_identity() const;
  inline void clear_identity();
  static const int kIdentityFieldNumber = 1;
  inline const ::charlie::CIdentity& identity() const;
  inline ::charlie::CIdentity* mutable_identity();
  inline ::charlie::CIdentity* release_identity();
  inline void set_allocated_identity(::charlie::CIdentity* identity);

  // @@protoc_insertion_point(class_scope:charlie.CSaveContainer)
 private:
  inline void set_has_identity();
  inline void clear_has_identity();

  ::charlie::CIdentity* identity_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  friend void  protobuf_AddDesc_charlie_2eproto_impl();
  #else
  friend void  protobuf_AddDesc_charlie_2eproto();
  #endif
  friend void protobuf_AssignDesc_charlie_2eproto();
  friend void protobuf_ShutdownFile_charlie_2eproto();

  void InitAsDefaultInstance();
  static CSaveContainer* default_instance_;
};
// -------------------------------------------------------------------

class CIdentity : public ::google::protobuf::MessageLite {
 public:
  CIdentity();
  virtual ~CIdentity();

  CIdentity(const CIdentity& from);

  inline CIdentity& operator=(const CIdentity& from) {
    CopyFrom(from);
    return *this;
  }

  static const CIdentity& default_instance();

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  // Returns the internal default instance pointer. This function can
  // return NULL thus should not be used by the user. This is intended
  // for Protobuf internal code. Please use default_instance() declared
  // above instead.
  static inline const CIdentity* internal_default_instance() {
    return default_instance_;
  }
  #endif

  void Swap(CIdentity* other);

  // implements Message ----------------------------------------------

  CIdentity* New() const;
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from);
  void CopyFrom(const CIdentity& from);
  void MergeFrom(const CIdentity& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::std::string GetTypeName() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional bytes private_key = 1;
  inline bool has_private_key() const;
  inline void clear_private_key();
  static const int kPrivateKeyFieldNumber = 1;
  inline const ::std::string& private_key() const;
  inline void set_private_key(const ::std::string& value);
  inline void set_private_key(const char* value);
  inline void set_private_key(const void* value, size_t size);
  inline ::std::string* mutable_private_key();
  inline ::std::string* release_private_key();
  inline void set_allocated_private_key(::std::string* private_key);

  // optional bytes public_key = 2;
  inline bool has_public_key() const;
  inline void clear_public_key();
  static const int kPublicKeyFieldNumber = 2;
  inline const ::std::string& public_key() const;
  inline void set_public_key(const ::std::string& value);
  inline void set_public_key(const char* value);
  inline void set_public_key(const void* value, size_t size);
  inline ::std::string* mutable_public_key();
  inline ::std::string* release_public_key();
  inline void set_allocated_public_key(::std::string* public_key);

  // @@protoc_insertion_point(class_scope:charlie.CIdentity)
 private:
  inline void set_has_private_key();
  inline void clear_has_private_key();
  inline void set_has_public_key();
  inline void clear_has_public_key();

  ::std::string* private_key_;
  ::std::string* public_key_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  friend void  protobuf_AddDesc_charlie_2eproto_impl();
  #else
  friend void  protobuf_AddDesc_charlie_2eproto();
  #endif
  friend void protobuf_AssignDesc_charlie_2eproto();
  friend void protobuf_ShutdownFile_charlie_2eproto();

  void InitAsDefaultInstance();
  static CIdentity* default_instance_;
};
// -------------------------------------------------------------------

class CModule : public ::google::protobuf::MessageLite {
 public:
  CModule();
  virtual ~CModule();

  CModule(const CModule& from);

  inline CModule& operator=(const CModule& from) {
    CopyFrom(from);
    return *this;
  }

  static const CModule& default_instance();

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  // Returns the internal default instance pointer. This function can
  // return NULL thus should not be used by the user. This is intended
  // for Protobuf internal code. Please use default_instance() declared
  // above instead.
  static inline const CModule* internal_default_instance() {
    return default_instance_;
  }
  #endif

  void Swap(CModule* other);

  // implements Message ----------------------------------------------

  CModule* New() const;
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from);
  void CopyFrom(const CModule& from);
  void MergeFrom(const CModule& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::std::string GetTypeName() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional uint32 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::uint32 id() const;
  inline void set_id(::google::protobuf::uint32 value);

  // optional bytes signature = 2;
  inline bool has_signature() const;
  inline void clear_signature();
  static const int kSignatureFieldNumber = 2;
  inline const ::std::string& signature() const;
  inline void set_signature(const ::std::string& value);
  inline void set_signature(const char* value);
  inline void set_signature(const void* value, size_t size);
  inline ::std::string* mutable_signature();
  inline ::std::string* release_signature();
  inline void set_allocated_signature(::std::string* signature);

  // @@protoc_insertion_point(class_scope:charlie.CModule)
 private:
  inline void set_has_id();
  inline void clear_has_id();
  inline void set_has_signature();
  inline void clear_has_signature();

  ::std::string* signature_;
  ::google::protobuf::uint32 id_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  friend void  protobuf_AddDesc_charlie_2eproto_impl();
  #else
  friend void  protobuf_AddDesc_charlie_2eproto();
  #endif
  friend void protobuf_AssignDesc_charlie_2eproto();
  friend void protobuf_ShutdownFile_charlie_2eproto();

  void InitAsDefaultInstance();
  static CModule* default_instance_;
};
// ===================================================================


// ===================================================================

// CMsgHeader

// optional .charlie.EMsgType type = 1;
inline bool CMsgHeader::has_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CMsgHeader::set_has_type() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CMsgHeader::clear_has_type() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CMsgHeader::clear_type() {
  type_ = 0;
  clear_has_type();
}
inline ::charlie::EMsgType CMsgHeader::type() const {
  return static_cast< ::charlie::EMsgType >(type_);
}
inline void CMsgHeader::set_type(::charlie::EMsgType value) {
  assert(::charlie::EMsgType_IsValid(value));
  set_has_type();
  type_ = value;
}

// optional int64 timestamp = 2;
inline bool CMsgHeader::has_timestamp() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CMsgHeader::set_has_timestamp() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CMsgHeader::clear_has_timestamp() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CMsgHeader::clear_timestamp() {
  timestamp_ = GOOGLE_LONGLONG(0);
  clear_has_timestamp();
}
inline ::google::protobuf::int64 CMsgHeader::timestamp() const {
  return timestamp_;
}
inline void CMsgHeader::set_timestamp(::google::protobuf::int64 value) {
  set_has_timestamp();
  timestamp_ = value;
}

// -------------------------------------------------------------------

// CMsgContainer

// optional bytes signed_timestamp = 1;
inline bool CMsgContainer::has_signed_timestamp() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CMsgContainer::set_has_signed_timestamp() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CMsgContainer::clear_has_signed_timestamp() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CMsgContainer::clear_signed_timestamp() {
  if (signed_timestamp_ != &::google::protobuf::internal::kEmptyString) {
    signed_timestamp_->clear();
  }
  clear_has_signed_timestamp();
}
inline const ::std::string& CMsgContainer::signed_timestamp() const {
  return *signed_timestamp_;
}
inline void CMsgContainer::set_signed_timestamp(const ::std::string& value) {
  set_has_signed_timestamp();
  if (signed_timestamp_ == &::google::protobuf::internal::kEmptyString) {
    signed_timestamp_ = new ::std::string;
  }
  signed_timestamp_->assign(value);
}
inline void CMsgContainer::set_signed_timestamp(const char* value) {
  set_has_signed_timestamp();
  if (signed_timestamp_ == &::google::protobuf::internal::kEmptyString) {
    signed_timestamp_ = new ::std::string;
  }
  signed_timestamp_->assign(value);
}
inline void CMsgContainer::set_signed_timestamp(const void* value, size_t size) {
  set_has_signed_timestamp();
  if (signed_timestamp_ == &::google::protobuf::internal::kEmptyString) {
    signed_timestamp_ = new ::std::string;
  }
  signed_timestamp_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* CMsgContainer::mutable_signed_timestamp() {
  set_has_signed_timestamp();
  if (signed_timestamp_ == &::google::protobuf::internal::kEmptyString) {
    signed_timestamp_ = new ::std::string;
  }
  return signed_timestamp_;
}
inline ::std::string* CMsgContainer::release_signed_timestamp() {
  clear_has_signed_timestamp();
  if (signed_timestamp_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = signed_timestamp_;
    signed_timestamp_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void CMsgContainer::set_allocated_signed_timestamp(::std::string* signed_timestamp) {
  if (signed_timestamp_ != &::google::protobuf::internal::kEmptyString) {
    delete signed_timestamp_;
  }
  if (signed_timestamp) {
    set_has_signed_timestamp();
    signed_timestamp_ = signed_timestamp;
  } else {
    clear_has_signed_timestamp();
    signed_timestamp_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional bytes body = 2;
inline bool CMsgContainer::has_body() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CMsgContainer::set_has_body() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CMsgContainer::clear_has_body() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CMsgContainer::clear_body() {
  if (body_ != &::google::protobuf::internal::kEmptyString) {
    body_->clear();
  }
  clear_has_body();
}
inline const ::std::string& CMsgContainer::body() const {
  return *body_;
}
inline void CMsgContainer::set_body(const ::std::string& value) {
  set_has_body();
  if (body_ == &::google::protobuf::internal::kEmptyString) {
    body_ = new ::std::string;
  }
  body_->assign(value);
}
inline void CMsgContainer::set_body(const char* value) {
  set_has_body();
  if (body_ == &::google::protobuf::internal::kEmptyString) {
    body_ = new ::std::string;
  }
  body_->assign(value);
}
inline void CMsgContainer::set_body(const void* value, size_t size) {
  set_has_body();
  if (body_ == &::google::protobuf::internal::kEmptyString) {
    body_ = new ::std::string;
  }
  body_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* CMsgContainer::mutable_body() {
  set_has_body();
  if (body_ == &::google::protobuf::internal::kEmptyString) {
    body_ = new ::std::string;
  }
  return body_;
}
inline ::std::string* CMsgContainer::release_body() {
  clear_has_body();
  if (body_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = body_;
    body_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void CMsgContainer::set_allocated_body(::std::string* body) {
  if (body_ != &::google::protobuf::internal::kEmptyString) {
    delete body_;
  }
  if (body) {
    set_has_body();
    body_ = body;
  } else {
    clear_has_body();
    body_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional .charlie.EBodyEncryptionType body_encryption = 3;
inline bool CMsgContainer::has_body_encryption() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void CMsgContainer::set_has_body_encryption() {
  _has_bits_[0] |= 0x00000004u;
}
inline void CMsgContainer::clear_has_body_encryption() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void CMsgContainer::clear_body_encryption() {
  body_encryption_ = 0;
  clear_has_body_encryption();
}
inline ::charlie::EBodyEncryptionType CMsgContainer::body_encryption() const {
  return static_cast< ::charlie::EBodyEncryptionType >(body_encryption_);
}
inline void CMsgContainer::set_body_encryption(::charlie::EBodyEncryptionType value) {
  assert(::charlie::EBodyEncryptionType_IsValid(value));
  set_has_body_encryption();
  body_encryption_ = value;
}

// optional bytes signed_body_hash = 4;
inline bool CMsgContainer::has_signed_body_hash() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void CMsgContainer::set_has_signed_body_hash() {
  _has_bits_[0] |= 0x00000008u;
}
inline void CMsgContainer::clear_has_signed_body_hash() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void CMsgContainer::clear_signed_body_hash() {
  if (signed_body_hash_ != &::google::protobuf::internal::kEmptyString) {
    signed_body_hash_->clear();
  }
  clear_has_signed_body_hash();
}
inline const ::std::string& CMsgContainer::signed_body_hash() const {
  return *signed_body_hash_;
}
inline void CMsgContainer::set_signed_body_hash(const ::std::string& value) {
  set_has_signed_body_hash();
  if (signed_body_hash_ == &::google::protobuf::internal::kEmptyString) {
    signed_body_hash_ = new ::std::string;
  }
  signed_body_hash_->assign(value);
}
inline void CMsgContainer::set_signed_body_hash(const char* value) {
  set_has_signed_body_hash();
  if (signed_body_hash_ == &::google::protobuf::internal::kEmptyString) {
    signed_body_hash_ = new ::std::string;
  }
  signed_body_hash_->assign(value);
}
inline void CMsgContainer::set_signed_body_hash(const void* value, size_t size) {
  set_has_signed_body_hash();
  if (signed_body_hash_ == &::google::protobuf::internal::kEmptyString) {
    signed_body_hash_ = new ::std::string;
  }
  signed_body_hash_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* CMsgContainer::mutable_signed_body_hash() {
  set_has_signed_body_hash();
  if (signed_body_hash_ == &::google::protobuf::internal::kEmptyString) {
    signed_body_hash_ = new ::std::string;
  }
  return signed_body_hash_;
}
inline ::std::string* CMsgContainer::release_signed_body_hash() {
  clear_has_signed_body_hash();
  if (signed_body_hash_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = signed_body_hash_;
    signed_body_hash_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void CMsgContainer::set_allocated_signed_body_hash(::std::string* signed_body_hash) {
  if (signed_body_hash_ != &::google::protobuf::internal::kEmptyString) {
    delete signed_body_hash_;
  }
  if (signed_body_hash) {
    set_has_signed_body_hash();
    signed_body_hash_ = signed_body_hash;
  } else {
    clear_has_signed_body_hash();
    signed_body_hash_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// -------------------------------------------------------------------

// CSaveContainer

// optional .charlie.CIdentity identity = 1;
inline bool CSaveContainer::has_identity() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CSaveContainer::set_has_identity() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CSaveContainer::clear_has_identity() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CSaveContainer::clear_identity() {
  if (identity_ != NULL) identity_->::charlie::CIdentity::Clear();
  clear_has_identity();
}
inline const ::charlie::CIdentity& CSaveContainer::identity() const {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  return identity_ != NULL ? *identity_ : *default_instance().identity_;
#else
  return identity_ != NULL ? *identity_ : *default_instance_->identity_;
#endif
}
inline ::charlie::CIdentity* CSaveContainer::mutable_identity() {
  set_has_identity();
  if (identity_ == NULL) identity_ = new ::charlie::CIdentity;
  return identity_;
}
inline ::charlie::CIdentity* CSaveContainer::release_identity() {
  clear_has_identity();
  ::charlie::CIdentity* temp = identity_;
  identity_ = NULL;
  return temp;
}
inline void CSaveContainer::set_allocated_identity(::charlie::CIdentity* identity) {
  delete identity_;
  identity_ = identity;
  if (identity) {
    set_has_identity();
  } else {
    clear_has_identity();
  }
}

// -------------------------------------------------------------------

// CIdentity

// optional bytes private_key = 1;
inline bool CIdentity::has_private_key() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CIdentity::set_has_private_key() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CIdentity::clear_has_private_key() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CIdentity::clear_private_key() {
  if (private_key_ != &::google::protobuf::internal::kEmptyString) {
    private_key_->clear();
  }
  clear_has_private_key();
}
inline const ::std::string& CIdentity::private_key() const {
  return *private_key_;
}
inline void CIdentity::set_private_key(const ::std::string& value) {
  set_has_private_key();
  if (private_key_ == &::google::protobuf::internal::kEmptyString) {
    private_key_ = new ::std::string;
  }
  private_key_->assign(value);
}
inline void CIdentity::set_private_key(const char* value) {
  set_has_private_key();
  if (private_key_ == &::google::protobuf::internal::kEmptyString) {
    private_key_ = new ::std::string;
  }
  private_key_->assign(value);
}
inline void CIdentity::set_private_key(const void* value, size_t size) {
  set_has_private_key();
  if (private_key_ == &::google::protobuf::internal::kEmptyString) {
    private_key_ = new ::std::string;
  }
  private_key_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* CIdentity::mutable_private_key() {
  set_has_private_key();
  if (private_key_ == &::google::protobuf::internal::kEmptyString) {
    private_key_ = new ::std::string;
  }
  return private_key_;
}
inline ::std::string* CIdentity::release_private_key() {
  clear_has_private_key();
  if (private_key_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = private_key_;
    private_key_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void CIdentity::set_allocated_private_key(::std::string* private_key) {
  if (private_key_ != &::google::protobuf::internal::kEmptyString) {
    delete private_key_;
  }
  if (private_key) {
    set_has_private_key();
    private_key_ = private_key;
  } else {
    clear_has_private_key();
    private_key_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional bytes public_key = 2;
inline bool CIdentity::has_public_key() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CIdentity::set_has_public_key() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CIdentity::clear_has_public_key() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CIdentity::clear_public_key() {
  if (public_key_ != &::google::protobuf::internal::kEmptyString) {
    public_key_->clear();
  }
  clear_has_public_key();
}
inline const ::std::string& CIdentity::public_key() const {
  return *public_key_;
}
inline void CIdentity::set_public_key(const ::std::string& value) {
  set_has_public_key();
  if (public_key_ == &::google::protobuf::internal::kEmptyString) {
    public_key_ = new ::std::string;
  }
  public_key_->assign(value);
}
inline void CIdentity::set_public_key(const char* value) {
  set_has_public_key();
  if (public_key_ == &::google::protobuf::internal::kEmptyString) {
    public_key_ = new ::std::string;
  }
  public_key_->assign(value);
}
inline void CIdentity::set_public_key(const void* value, size_t size) {
  set_has_public_key();
  if (public_key_ == &::google::protobuf::internal::kEmptyString) {
    public_key_ = new ::std::string;
  }
  public_key_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* CIdentity::mutable_public_key() {
  set_has_public_key();
  if (public_key_ == &::google::protobuf::internal::kEmptyString) {
    public_key_ = new ::std::string;
  }
  return public_key_;
}
inline ::std::string* CIdentity::release_public_key() {
  clear_has_public_key();
  if (public_key_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = public_key_;
    public_key_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void CIdentity::set_allocated_public_key(::std::string* public_key) {
  if (public_key_ != &::google::protobuf::internal::kEmptyString) {
    delete public_key_;
  }
  if (public_key) {
    set_has_public_key();
    public_key_ = public_key;
  } else {
    clear_has_public_key();
    public_key_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// -------------------------------------------------------------------

// CModule

// optional uint32 id = 1;
inline bool CModule::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CModule::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CModule::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CModule::clear_id() {
  id_ = 0u;
  clear_has_id();
}
inline ::google::protobuf::uint32 CModule::id() const {
  return id_;
}
inline void CModule::set_id(::google::protobuf::uint32 value) {
  set_has_id();
  id_ = value;
}

// optional bytes signature = 2;
inline bool CModule::has_signature() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CModule::set_has_signature() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CModule::clear_has_signature() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CModule::clear_signature() {
  if (signature_ != &::google::protobuf::internal::kEmptyString) {
    signature_->clear();
  }
  clear_has_signature();
}
inline const ::std::string& CModule::signature() const {
  return *signature_;
}
inline void CModule::set_signature(const ::std::string& value) {
  set_has_signature();
  if (signature_ == &::google::protobuf::internal::kEmptyString) {
    signature_ = new ::std::string;
  }
  signature_->assign(value);
}
inline void CModule::set_signature(const char* value) {
  set_has_signature();
  if (signature_ == &::google::protobuf::internal::kEmptyString) {
    signature_ = new ::std::string;
  }
  signature_->assign(value);
}
inline void CModule::set_signature(const void* value, size_t size) {
  set_has_signature();
  if (signature_ == &::google::protobuf::internal::kEmptyString) {
    signature_ = new ::std::string;
  }
  signature_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* CModule::mutable_signature() {
  set_has_signature();
  if (signature_ == &::google::protobuf::internal::kEmptyString) {
    signature_ = new ::std::string;
  }
  return signature_;
}
inline ::std::string* CModule::release_signature() {
  clear_has_signature();
  if (signature_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = signature_;
    signature_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void CModule::set_allocated_signature(::std::string* signature) {
  if (signature_ != &::google::protobuf::internal::kEmptyString) {
    delete signature_;
  }
  if (signature) {
    set_has_signature();
    signature_ = signature;
  } else {
    clear_has_signature();
    signature_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace charlie

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_charlie_2eproto__INCLUDED
