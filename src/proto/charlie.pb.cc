// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: charlie.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "charlie.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
// @@protoc_insertion_point(includes)

namespace charlie {

void protobuf_ShutdownFile_charlie_2eproto() {
  delete CMsgHeader::default_instance_;
  delete CMsgContainer::default_instance_;
  delete CSaveContainer::default_instance_;
  delete CIdentity::default_instance_;
  delete CModule::default_instance_;
}

#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
void protobuf_AddDesc_charlie_2eproto_impl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#else
void protobuf_AddDesc_charlie_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#endif
  CMsgHeader::default_instance_ = new CMsgHeader();
  CMsgContainer::default_instance_ = new CMsgContainer();
  CSaveContainer::default_instance_ = new CSaveContainer();
  CIdentity::default_instance_ = new CIdentity();
  CModule::default_instance_ = new CModule();
  CMsgHeader::default_instance_->InitAsDefaultInstance();
  CMsgContainer::default_instance_->InitAsDefaultInstance();
  CSaveContainer::default_instance_->InitAsDefaultInstance();
  CIdentity::default_instance_->InitAsDefaultInstance();
  CModule::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_charlie_2eproto);
}

#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AddDesc_charlie_2eproto_once_);
void protobuf_AddDesc_charlie_2eproto() {
  ::google::protobuf::::google::protobuf::GoogleOnceInit(&protobuf_AddDesc_charlie_2eproto_once_,
                 &protobuf_AddDesc_charlie_2eproto_impl);
}
#else
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_charlie_2eproto {
  StaticDescriptorInitializer_charlie_2eproto() {
    protobuf_AddDesc_charlie_2eproto();
  }
} static_descriptor_initializer_charlie_2eproto_;
#endif
bool EMsgType_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 9000:
      return true;
    default:
      return false;
  }
}

bool EBodyEncryptionType_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
    case 2:
      return true;
    default:
      return false;
  }
}


// ===================================================================

#ifndef _MSC_VER
const int CMsgHeader::kTypeFieldNumber;
const int CMsgHeader::kTimestampFieldNumber;
#endif  // !_MSC_VER

CMsgHeader::CMsgHeader()
  : ::google::protobuf::MessageLite() {
  SharedCtor();
}

void CMsgHeader::InitAsDefaultInstance() {
}

CMsgHeader::CMsgHeader(const CMsgHeader& from)
  : ::google::protobuf::MessageLite() {
  SharedCtor();
  MergeFrom(from);
}

void CMsgHeader::SharedCtor() {
  _cached_size_ = 0;
  type_ = 0;
  timestamp_ = GOOGLE_LONGLONG(0);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CMsgHeader::~CMsgHeader() {
  SharedDtor();
}

void CMsgHeader::SharedDtor() {
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void CMsgHeader::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const CMsgHeader& CMsgHeader::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_charlie_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_charlie_2eproto();
#endif
  return *default_instance_;
}

CMsgHeader* CMsgHeader::default_instance_ = NULL;

CMsgHeader* CMsgHeader::New() const {
  return new CMsgHeader;
}

void CMsgHeader::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    type_ = 0;
    timestamp_ = GOOGLE_LONGLONG(0);
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

bool CMsgHeader::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional .charlie.EMsgType type = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::charlie::EMsgType_IsValid(value)) {
            set_type(static_cast< ::charlie::EMsgType >(value));
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_timestamp;
        break;
      }

      // optional int64 timestamp = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_timestamp:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int64, ::google::protobuf::internal::WireFormatLite::TYPE_INT64>(
                 input, &timestamp_)));
          set_has_timestamp();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void CMsgHeader::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional .charlie.EMsgType type = 1;
  if (has_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      1, this->type(), output);
  }

  // optional int64 timestamp = 2;
  if (has_timestamp()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64(2, this->timestamp(), output);
  }

}

int CMsgHeader::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional .charlie.EMsgType type = 1;
    if (has_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->type());
    }

    // optional int64 timestamp = 2;
    if (has_timestamp()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int64Size(
          this->timestamp());
    }

  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void CMsgHeader::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const CMsgHeader*>(&from));
}

void CMsgHeader::MergeFrom(const CMsgHeader& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_type()) {
      set_type(from.type());
    }
    if (from.has_timestamp()) {
      set_timestamp(from.timestamp());
    }
  }
}

void CMsgHeader::CopyFrom(const CMsgHeader& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CMsgHeader::IsInitialized() const {

  return true;
}

void CMsgHeader::Swap(CMsgHeader* other) {
  if (other != this) {
    std::swap(type_, other->type_);
    std::swap(timestamp_, other->timestamp_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::std::string CMsgHeader::GetTypeName() const {
  return "charlie.CMsgHeader";
}


// ===================================================================

#ifndef _MSC_VER
const int CMsgContainer::kSignedTimestampFieldNumber;
const int CMsgContainer::kBodyFieldNumber;
const int CMsgContainer::kBodyEncryptionFieldNumber;
const int CMsgContainer::kSignedBodyHashFieldNumber;
#endif  // !_MSC_VER

CMsgContainer::CMsgContainer()
  : ::google::protobuf::MessageLite() {
  SharedCtor();
}

void CMsgContainer::InitAsDefaultInstance() {
}

CMsgContainer::CMsgContainer(const CMsgContainer& from)
  : ::google::protobuf::MessageLite() {
  SharedCtor();
  MergeFrom(from);
}

void CMsgContainer::SharedCtor() {
  _cached_size_ = 0;
  signed_timestamp_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  body_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  body_encryption_ = 0;
  signed_body_hash_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CMsgContainer::~CMsgContainer() {
  SharedDtor();
}

void CMsgContainer::SharedDtor() {
  if (signed_timestamp_ != &::google::protobuf::internal::kEmptyString) {
    delete signed_timestamp_;
  }
  if (body_ != &::google::protobuf::internal::kEmptyString) {
    delete body_;
  }
  if (signed_body_hash_ != &::google::protobuf::internal::kEmptyString) {
    delete signed_body_hash_;
  }
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void CMsgContainer::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const CMsgContainer& CMsgContainer::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_charlie_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_charlie_2eproto();
#endif
  return *default_instance_;
}

CMsgContainer* CMsgContainer::default_instance_ = NULL;

CMsgContainer* CMsgContainer::New() const {
  return new CMsgContainer;
}

void CMsgContainer::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_signed_timestamp()) {
      if (signed_timestamp_ != &::google::protobuf::internal::kEmptyString) {
        signed_timestamp_->clear();
      }
    }
    if (has_body()) {
      if (body_ != &::google::protobuf::internal::kEmptyString) {
        body_->clear();
      }
    }
    body_encryption_ = 0;
    if (has_signed_body_hash()) {
      if (signed_body_hash_ != &::google::protobuf::internal::kEmptyString) {
        signed_body_hash_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

bool CMsgContainer::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional bytes signed_timestamp = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_signed_timestamp()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_body;
        break;
      }

      // optional bytes body = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_body:
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_body()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_body_encryption;
        break;
      }

      // optional .charlie.EBodyEncryptionType body_encryption = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_body_encryption:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::charlie::EBodyEncryptionType_IsValid(value)) {
            set_body_encryption(static_cast< ::charlie::EBodyEncryptionType >(value));
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(34)) goto parse_signed_body_hash;
        break;
      }

      // optional bytes signed_body_hash = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_signed_body_hash:
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_signed_body_hash()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void CMsgContainer::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional bytes signed_timestamp = 1;
  if (has_signed_timestamp()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      1, this->signed_timestamp(), output);
  }

  // optional bytes body = 2;
  if (has_body()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      2, this->body(), output);
  }

  // optional .charlie.EBodyEncryptionType body_encryption = 3;
  if (has_body_encryption()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      3, this->body_encryption(), output);
  }

  // optional bytes signed_body_hash = 4;
  if (has_signed_body_hash()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      4, this->signed_body_hash(), output);
  }

}

int CMsgContainer::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional bytes signed_timestamp = 1;
    if (has_signed_timestamp()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->signed_timestamp());
    }

    // optional bytes body = 2;
    if (has_body()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->body());
    }

    // optional .charlie.EBodyEncryptionType body_encryption = 3;
    if (has_body_encryption()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->body_encryption());
    }

    // optional bytes signed_body_hash = 4;
    if (has_signed_body_hash()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->signed_body_hash());
    }

  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void CMsgContainer::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const CMsgContainer*>(&from));
}

void CMsgContainer::MergeFrom(const CMsgContainer& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_signed_timestamp()) {
      set_signed_timestamp(from.signed_timestamp());
    }
    if (from.has_body()) {
      set_body(from.body());
    }
    if (from.has_body_encryption()) {
      set_body_encryption(from.body_encryption());
    }
    if (from.has_signed_body_hash()) {
      set_signed_body_hash(from.signed_body_hash());
    }
  }
}

void CMsgContainer::CopyFrom(const CMsgContainer& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CMsgContainer::IsInitialized() const {

  return true;
}

void CMsgContainer::Swap(CMsgContainer* other) {
  if (other != this) {
    std::swap(signed_timestamp_, other->signed_timestamp_);
    std::swap(body_, other->body_);
    std::swap(body_encryption_, other->body_encryption_);
    std::swap(signed_body_hash_, other->signed_body_hash_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::std::string CMsgContainer::GetTypeName() const {
  return "charlie.CMsgContainer";
}


// ===================================================================

#ifndef _MSC_VER
const int CSaveContainer::kIdentityFieldNumber;
#endif  // !_MSC_VER

CSaveContainer::CSaveContainer()
  : ::google::protobuf::MessageLite() {
  SharedCtor();
}

void CSaveContainer::InitAsDefaultInstance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  identity_ = const_cast< ::charlie::CIdentity*>(
      ::charlie::CIdentity::internal_default_instance());
#else
  identity_ = const_cast< ::charlie::CIdentity*>(&::charlie::CIdentity::default_instance());
#endif
}

CSaveContainer::CSaveContainer(const CSaveContainer& from)
  : ::google::protobuf::MessageLite() {
  SharedCtor();
  MergeFrom(from);
}

void CSaveContainer::SharedCtor() {
  _cached_size_ = 0;
  identity_ = NULL;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CSaveContainer::~CSaveContainer() {
  SharedDtor();
}

void CSaveContainer::SharedDtor() {
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
    delete identity_;
  }
}

void CSaveContainer::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const CSaveContainer& CSaveContainer::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_charlie_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_charlie_2eproto();
#endif
  return *default_instance_;
}

CSaveContainer* CSaveContainer::default_instance_ = NULL;

CSaveContainer* CSaveContainer::New() const {
  return new CSaveContainer;
}

void CSaveContainer::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_identity()) {
      if (identity_ != NULL) identity_->::charlie::CIdentity::Clear();
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

bool CSaveContainer::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional .charlie.CIdentity identity = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_identity()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void CSaveContainer::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional .charlie.CIdentity identity = 1;
  if (has_identity()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessage(
      1, this->identity(), output);
  }

}

int CSaveContainer::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional .charlie.CIdentity identity = 1;
    if (has_identity()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->identity());
    }

  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void CSaveContainer::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const CSaveContainer*>(&from));
}

void CSaveContainer::MergeFrom(const CSaveContainer& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_identity()) {
      mutable_identity()->::charlie::CIdentity::MergeFrom(from.identity());
    }
  }
}

void CSaveContainer::CopyFrom(const CSaveContainer& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CSaveContainer::IsInitialized() const {

  return true;
}

void CSaveContainer::Swap(CSaveContainer* other) {
  if (other != this) {
    std::swap(identity_, other->identity_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::std::string CSaveContainer::GetTypeName() const {
  return "charlie.CSaveContainer";
}


// ===================================================================

#ifndef _MSC_VER
const int CIdentity::kPrivateKeyFieldNumber;
const int CIdentity::kPublicKeyFieldNumber;
#endif  // !_MSC_VER

CIdentity::CIdentity()
  : ::google::protobuf::MessageLite() {
  SharedCtor();
}

void CIdentity::InitAsDefaultInstance() {
}

CIdentity::CIdentity(const CIdentity& from)
  : ::google::protobuf::MessageLite() {
  SharedCtor();
  MergeFrom(from);
}

void CIdentity::SharedCtor() {
  _cached_size_ = 0;
  private_key_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  public_key_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CIdentity::~CIdentity() {
  SharedDtor();
}

void CIdentity::SharedDtor() {
  if (private_key_ != &::google::protobuf::internal::kEmptyString) {
    delete private_key_;
  }
  if (public_key_ != &::google::protobuf::internal::kEmptyString) {
    delete public_key_;
  }
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void CIdentity::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const CIdentity& CIdentity::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_charlie_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_charlie_2eproto();
#endif
  return *default_instance_;
}

CIdentity* CIdentity::default_instance_ = NULL;

CIdentity* CIdentity::New() const {
  return new CIdentity;
}

void CIdentity::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_private_key()) {
      if (private_key_ != &::google::protobuf::internal::kEmptyString) {
        private_key_->clear();
      }
    }
    if (has_public_key()) {
      if (public_key_ != &::google::protobuf::internal::kEmptyString) {
        public_key_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

bool CIdentity::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional bytes private_key = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_private_key()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_public_key;
        break;
      }

      // optional bytes public_key = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_public_key:
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_public_key()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void CIdentity::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional bytes private_key = 1;
  if (has_private_key()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      1, this->private_key(), output);
  }

  // optional bytes public_key = 2;
  if (has_public_key()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      2, this->public_key(), output);
  }

}

int CIdentity::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional bytes private_key = 1;
    if (has_private_key()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->private_key());
    }

    // optional bytes public_key = 2;
    if (has_public_key()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->public_key());
    }

  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void CIdentity::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const CIdentity*>(&from));
}

void CIdentity::MergeFrom(const CIdentity& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_private_key()) {
      set_private_key(from.private_key());
    }
    if (from.has_public_key()) {
      set_public_key(from.public_key());
    }
  }
}

void CIdentity::CopyFrom(const CIdentity& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CIdentity::IsInitialized() const {

  return true;
}

void CIdentity::Swap(CIdentity* other) {
  if (other != this) {
    std::swap(private_key_, other->private_key_);
    std::swap(public_key_, other->public_key_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::std::string CIdentity::GetTypeName() const {
  return "charlie.CIdentity";
}


// ===================================================================

#ifndef _MSC_VER
const int CModule::kIdFieldNumber;
const int CModule::kSignatureFieldNumber;
#endif  // !_MSC_VER

CModule::CModule()
  : ::google::protobuf::MessageLite() {
  SharedCtor();
}

void CModule::InitAsDefaultInstance() {
}

CModule::CModule(const CModule& from)
  : ::google::protobuf::MessageLite() {
  SharedCtor();
  MergeFrom(from);
}

void CModule::SharedCtor() {
  _cached_size_ = 0;
  id_ = 0u;
  signature_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CModule::~CModule() {
  SharedDtor();
}

void CModule::SharedDtor() {
  if (signature_ != &::google::protobuf::internal::kEmptyString) {
    delete signature_;
  }
  #ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  if (this != &default_instance()) {
  #else
  if (this != default_instance_) {
  #endif
  }
}

void CModule::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const CModule& CModule::default_instance() {
#ifdef GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER
  protobuf_AddDesc_charlie_2eproto();
#else
  if (default_instance_ == NULL) protobuf_AddDesc_charlie_2eproto();
#endif
  return *default_instance_;
}

CModule* CModule::default_instance_ = NULL;

CModule* CModule::New() const {
  return new CModule;
}

void CModule::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    id_ = 0u;
    if (has_signature()) {
      if (signature_ != &::google::protobuf::internal::kEmptyString) {
        signature_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

bool CModule::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional uint32 id = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &id_)));
          set_has_id();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_signature;
        break;
      }

      // optional bytes signature = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_signature:
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_signature()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void CModule::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // optional uint32 id = 1;
  if (has_id()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->id(), output);
  }

  // optional bytes signature = 2;
  if (has_signature()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytes(
      2, this->signature(), output);
  }

}

int CModule::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional uint32 id = 1;
    if (has_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->id());
    }

    // optional bytes signature = 2;
    if (has_signature()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->signature());
    }

  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void CModule::CheckTypeAndMergeFrom(
    const ::google::protobuf::MessageLite& from) {
  MergeFrom(*::google::protobuf::down_cast<const CModule*>(&from));
}

void CModule::MergeFrom(const CModule& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_id()) {
      set_id(from.id());
    }
    if (from.has_signature()) {
      set_signature(from.signature());
    }
  }
}

void CModule::CopyFrom(const CModule& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CModule::IsInitialized() const {

  return true;
}

void CModule::Swap(CModule* other) {
  if (other != this) {
    std::swap(id_, other->id_);
    std::swap(signature_, other->signature_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::std::string CModule::GetTypeName() const {
  return "charlie.CModule";
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace charlie

// @@protoc_insertion_point(global_scope)
