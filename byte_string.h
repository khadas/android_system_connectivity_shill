// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_BYTE_STRING_
#define SHILL_BYTE_STRING_

#include <string>
#include <vector>

#include <base/basictypes.h>

namespace shill {

// Provides a holder of a string of bytes
class ByteString {
 public:
  ByteString() {}
  ByteString(const ByteString &b) : data_(b.data_) {}
  explicit ByteString(size_t length) : data_(length) {}
  ByteString(const unsigned char *data, size_t length)
      : data_(data, data + length) {}
  ByteString(const std::string &data, bool copy_terminator)
    : data_(reinterpret_cast<const unsigned char *>(data.c_str()),
            reinterpret_cast<const unsigned char *>(data.c_str() +
                                                    data.length() +
                                                    (copy_terminator ?
                                                     1 : 0))) {}

  ByteString &operator=(const ByteString &b) {
    data_ = b.data_;
    return *this;
  }

  unsigned char *GetData() { return data_.data(); }
  const unsigned char *GetConstData() const { return data_.data(); }
  size_t GetLength() const { return data_.size(); }

  // Returns a ByteString containing |length| bytes from the ByteString
  // starting at |offset|.  This function truncates the returned string
  // if part (or all) of this requested data lies outside the bounds of
  // this ByteString.
  ByteString GetSubstring(size_t offset, size_t length) const;

  // Inserts a uint32 into a ByteString in cpu-order
  static ByteString CreateFromCPUUInt32(uint32 val);
  // Inserts a uint32 into a ByteString in network-order
  static ByteString CreateFromNetUInt32(uint32 val);

  // Converts to a uint32 from a host-order value stored in the ByteString
  // Returns true on success
  bool ConvertToCPUUInt32(uint32 *val) const;
  // Converts to a uint32 from a network-order value stored in the ByteString
  // Returns true on success
  bool ConvertToNetUInt32(uint32 *val) const;

  bool IsEmpty() const { return GetLength() == 0; }

  // Returns true if every element of |this| is zero, false otherwise.
  bool IsZero() const;

  // Perform an AND operation between each element of |this| with the
  // corresponding byte of |b|.  Returns true if both |this| and |b|
  // are the same length, and as such the operation succeeds; false
  // if they are not.  The result of the operation is stored in |this|.
  bool BitwiseAnd(const ByteString &b);

  // Perform an OR operation between each element of |this| with the
  // corresponding byte of |b|.  Returns true if both |this| and |b|
  // are the same length, and as such the operation succeeds; false
  // if they are not.  The result of the operation is stored in |this|.
  bool BitwiseOr(const ByteString &b);

  // Perform an inversion operation on each of the bits this string.
  void BitwiseInvert();

  bool Equals(const ByteString &b) const;
  void Append(const ByteString &b);
  void Clear() { data_.clear(); }
  void Resize(int size) {
    data_.resize(size, 0);
  }

  std::string HexEncode() const;

 private:
  std::vector<unsigned char> data_;
  // NO DISALLOW_COPY_AND_ASSIGN -- we assign ByteStrings in STL hashes
};

}  // namespace shill


#endif  // SHILL_BYTE_STRING_
