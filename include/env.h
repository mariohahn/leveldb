// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// An Env is an interface used by the leveldb implementation to access
// operating system functionality like the filesystem etc.  Callers
// may wish to provide a custom Env object when opening a database to
// get fine gain control; e.g., to rate limit file system operations.

#ifndef STORAGE_LEVELDB_INCLUDE_ENV_H_
#define STORAGE_LEVELDB_INCLUDE_ENV_H_

#include <cstdarg>
#include <string>
#include <vector>
#include <stdint.h>
#include "include/status.h"

namespace leveldb {

class FileLock;
class RandomAccessFile;
class SequentialFile;
class Slice;
class WritableFile;

class Env {
 public:
  Env() { }
  virtual ~Env();

  // Return a default environment suitable for the current operating
  // system.  Sophisticated users may wish to provide their own Env
  // implementation instead of relying on this default environment.
  //
  // The result of Default() belongs to leveldb and must never be deleted.
  static Env* Default();

  // Create a brand new sequentially-readable file with the specified name.
  // On success, stores a pointer to the new file in *result and returns OK.
  // On failure stores NULL in *result and returns non-OK.  If the file does
  // not exist, returns a non-OK status.
  //
  // The returned file will only be accessed by one thread at a time.
  virtual Status NewSequentialFile(const std::string& fname,
                                   SequentialFile** result) = 0;

  // Create a brand new random access read-only file with the
  // specified name.  On success, stores a pointer to the new file in
  // *result and returns OK.  On failure stores NULL in *result and
  // returns non-OK.  If the file does not exist, returns a non-OK
  // status.
  //
  // The returned file may be concurrently accessed by multiple threads.
  virtual Status NewRandomAccessFile(const std::string& fname,
                                     RandomAccessFile** result) = 0;

  // Create an object that writes to a new file with the specified
  // name.  Deletes any existing file with the same name and creates a
  // new file.  On success, stores a pointer to the new file in
  // *result and returns OK.  On failure stores NULL in *result and
  // returns non-OK.
  //
  // The returned file will only be accessed by one thread at a time.
  virtual Status NewWritableFile(const std::string& fname,
                                 WritableFile** result) = 0;

  // Returns true iff the named file exists.
  virtual bool FileExists(const std::string& fname) = 0;

  // Store in *result the names of the children of the specified directory.
  // The names are relative to "dir".
  // Original contents of *results are dropped.
  virtual Status GetChildren(const std::string& dir,
                             std::vector<std::string>* result) = 0;

  // Delete the named file.
  virtual Status DeleteFile(const std::string& fname) = 0;

  // Create the specified directory.
  virtual Status CreateDir(const std::string& dirname) = 0;

  // Delete the specified directory.
  virtual Status DeleteDir(const std::string& dirname) = 0;

  // Store the size of fname in *file_size.
  virtual Status GetFileSize(const std::string& fname, uint64_t* file_size) = 0;

  // Rename file src to target.
  virtual Status RenameFile(const std::string& src,
                            const std::string& target) = 0;

  // Lock the specified file.  Used to prevent concurrent access to
  // the same db by multiple processes.  On failure, stores NULL in
  // *lock and returns non-OK.
  //
  // On success, stores a pointer to the object that represents the
  // acquired lock in *lock and returns OK.  The caller should call
  // UnlockFile(*lock) to release the lock.  If the process exits,
  // the lock will be automatically released.
  //
  // If somebody else already holds the lock, finishes immediately
  // with a failure.  I.e., this call does not wait for existing locks
  // to go away.
  //
  // May create the named file if it does not already exist.
  virtual Status LockFile(const std::string& fname, FileLock** lock) = 0;

  // Release the lock acquired by a previous successful call to LockFile.
  // REQUIRES: lock was returned by a successful LockFile() call
  // REQUIRES: lock has not already been unlocked.
  virtual Status UnlockFile(FileLock* lock) = 0;

  // Arrange to run "(*function)(arg)" once in a background thread.
  //
  // "function" may run in an unspecified thread.  Multiple functions
  // added to the same Env may run concurrently in different threads.
  // I.e., the caller may not assume that background work items are
  // serialized.
  virtual void Schedule(
      void (*function)(void* arg),
      void* arg) = 0;

  // Start a new thread, invoking "function(arg)" within the new thread.
  // When "function(arg)" returns, the thread will be destroyed.
  virtual void StartThread(void (*function)(void* arg), void* arg) = 0;

  // *path is set to a temporary directory that can be used for testing. It may
  // or many not have just been created. The directory may or may not differ
  // between runs of the same process, but subsequent calls will return the
  // same directory.
  virtual Status GetTestDirectory(std::string* path) = 0;

  // Write an entry to the log file with the specified format.
  virtual void Logv(WritableFile* log, const char* format, va_list ap) = 0;

  // Returns the number of micro-seconds since some fixed point in time. Only
  // useful for computing deltas of time.
  virtual uint64_t NowMicros() = 0;

  // Sleep/delay the thread for the perscribed number of micro-seconds.
  virtual void SleepForMicroseconds(int micros) = 0;

 private:
  // No copying allowed
  Env(const Env&);
  void operator=(const Env&);
};

// A file abstraction for reading sequentially through a file
class SequentialFile {
 public:
  SequentialFile() { }
  virtual ~SequentialFile();

  // Read up to "n" bytes from the file.  "scratch[0..n-1]" may be
  // written by this routine.  Sets "*result" to the data that was
  // read (including if fewer than "n" bytes were successfully read).
  // If an error was encountered, returns a non-OK status.
  //
  // REQUIRES: External synchronization
  virtual Status Read(size_t n, Slice* result, char* scratch) = 0;
};

// A file abstraction for randomly reading the contents of a file.
class RandomAccessFile {
 public:
  RandomAccessFile() { }
  virtual ~RandomAccessFile();

  // Return the length of this file in bytes.
  virtual uint64_t Size() const = 0;

  // Read up to "n" bytes from the file starting at "offset".
  // "scratch[0..n-1]" may be written by this routine.  Sets "*result"
  // to the data that was read (including if fewer than "n" bytes were
  // successfully read).  If an error was encountered, returns a
  // non-OK status.
  //
  // Safe for concurrent use by multiple threads.
  virtual Status Read(uint64_t offset, size_t n, Slice* result,
                      char* scratch) const = 0;
};

// A file abstraction for sequential writing.  The implementation
// must provide buffering since callers may append small fragments
// at a time to the file.
class WritableFile {
 public:
  WritableFile() { }
  virtual ~WritableFile();

  virtual Status Append(const Slice& data) = 0;
  virtual Status Close() = 0;
  virtual Status Flush() = 0;
  virtual Status Sync() = 0;

 private:
  // No copying allowed
  WritableFile(const WritableFile&);
  void operator=(const WritableFile&);
};

// Identifies a locked file.
class FileLock {
 public:
  FileLock() { }
  virtual ~FileLock();
 private:
  // No copying allowed
  FileLock(const FileLock&);
  void operator=(const FileLock&);
};

// Log the specified data to *info_log if info_log is non-NULL.
extern void Log(Env* env, WritableFile* info_log, const char* format, ...)
#   if defined(__GNUC__) || defined(__clang__)
    __attribute__((__format__ (__printf__, 3, 4)))
#   endif
    ;

// A utility routine: write "data" to the named file.
extern Status WriteStringToFile(Env* env, const Slice& data,
                                const std::string& fname);

// A utility routine: read contents of named file into *data
extern Status ReadFileToString(Env* env, const std::string& fname,
                               std::string* data);

// An implementation of Env that forwards all calls to another Env.
// May be useful to clients who wish to override just part of the
// functionality of another Env.
class EnvWrapper : public Env {
 public:
  // Initialize an EnvWrapper that delegates all calls to *target
  explicit EnvWrapper(Env* target) : target_(target) { }
  virtual ~EnvWrapper();

  // Return the target to which this Env forwards all calls
  Env* target() const { return target_; }

  // The following text is boilerplate that forwards all methods to target()
  Status NewSequentialFile(const std::string& f, SequentialFile** r) {
    return target_->NewSequentialFile(f, r);
  }
  Status NewRandomAccessFile(const std::string& f, RandomAccessFile** r) {
    return target_->NewRandomAccessFile(f, r);
  }
  Status NewWritableFile(const std::string& f, WritableFile** r) {
    return target_->NewWritableFile(f, r);
  }
  bool FileExists(const std::string& f) { return target_->FileExists(f); }
  Status GetChildren(const std::string& dir, std::vector<std::string>* r) {
    return target_->GetChildren(dir, r);
  }
  Status DeleteFile(const std::string& f) { return target_->DeleteFile(f); }
  Status CreateDir(const std::string& d) { return target_->CreateDir(d); }
  Status DeleteDir(const std::string& d) { return target_->DeleteDir(d); }
  Status GetFileSize(const std::string& f, uint64_t* s) {
    return target_->GetFileSize(f, s);
  }
  Status RenameFile(const std::string& s, const std::string& t) {
    return target_->RenameFile(s, t);
  }
  Status LockFile(const std::string& f, FileLock** l) {
    return target_->LockFile(f, l);
  }
  Status UnlockFile(FileLock* l) { return target_->UnlockFile(l); }
  void Schedule(void (*f)(void*), void* a) {
    return target_->Schedule(f, a);
  }
  void StartThread(void (*f)(void*), void* a) {
    return target_->StartThread(f, a);
  }
  virtual Status GetTestDirectory(std::string* path) {
    return target_->GetTestDirectory(path);
  }
  virtual void Logv(WritableFile* log, const char* format, va_list ap) {
    return target_->Logv(log, format, ap);
  }
  uint64_t NowMicros() {
    return target_->NowMicros();
  }
  void SleepForMicroseconds(int micros) {
    target_->SleepForMicroseconds(micros);
  }
 private:
  Env* target_;
};

}

#endif  // STORAGE_LEVELDB_INCLUDE_ENV_H_