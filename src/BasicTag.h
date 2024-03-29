/*
Copyright 2024 Michael Keras

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef BASIC_TAG_H
#define BASIC_TAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

/* Data structures definitions */

typedef uint64_t (*TimestampFunction)();  // Function that returns a uint64_t millisecond timestamp

typedef struct FunctionalBasicTag FunctionalBasicTag; // Forward declaration

typedef enum {
  // Indexes of Data Types matching the Sparkplug 3 specification
  // Unimplemented Datatypes are commented out
  // Unknown placeholder for future expansion.
  //spUnknown = 0,
  // Basic Types
  spInt8 = 1,
  spInt16 = 2,
  spInt32 = 3,
  spInt64 = 4,
  spUInt8 = 5,
  spUInt16 = 6,
  spUInt32 = 7,
  spUInt64 = 8,
  spFloat = 9,
  spDouble = 10,
  spBoolean = 11,
  spString = 12,
  spDateTime = 13,  // uint64, epoch milliseconds
  spText = 14,
  // Additional Metric Types
  spUUID = 15,   // a string of 36 characters
  /*spDataSet = 16,*/
  spBytes = 17,
  /*spFile = 18,
  spTemplate = 19,
  // Additional PropertyValue Types
  spPropertySet = 20,
  spPropertySetList = 21,
  // Array Types
  spInt8Array = 22,
  spInt16Array = 23,
  spInt32Array = 24,
  spInt64Array = 25,
  spUInt8Array = 26,
  spUInt16Array = 27,
  spUInt32Array = 28,
  spUInt64Array = 29,
  spFloatArray = 30,
  spDoubleArray = 31,
  spBooleanArray = 32,
  spStringArray = 33,
  spDateTimeArray = 34*/
} SparkplugDataType; // v1.3.0 added sp prefix to types to avoid any name overlap (eg. arduino String class)


typedef struct {
  uint8_t* buffer;  // Pointer to an array of uint8_t
  size_t written_length;    // Length of the array
  size_t allocated_length;
} BufferValue;  // Size 12 bytes + length of buffer


// Union to accommodate various data types
typedef union {
    int8_t int8Value;
    int16_t int16Value;
    int32_t int32Value;
    int64_t int64Value;
    uint8_t uint8Value;
    uint16_t uint16Value;
    uint32_t uint32Value;
    uint64_t uint64Value;
    float floatValue;
    double doubleValue;
    bool boolValue;
    char* stringValue;
    BufferValue* bytesValue;  // Use BufferValue struct for bytes data types
    void* other; // Void pointer for use with other complex SparkplugDataTypes
} Value; // Size is 8 bytes + value of string, if the value is a string, + 12 bytes Buffer Value + buffer size if it's a bytesValue


typedef struct {
    uint64_t timestamp;
    SparkplugDataType datatype;
    Value value;
    bool isNull;
} BasicValue; // Basic Value size is 32 bytes

typedef bool (*CompareFunction)(BasicValue* previousValue, BasicValue* newValue);  // Compare the values, return True if value should be considered changed False if not
typedef void (*onValueChangeFunction)(FunctionalBasicTag* tag);  // Optional function that can be registered for a tag that is triggered be read, when the value has changed. Called after tag values have been updated, before returning
typedef bool (*ValidateWriteFunction)(BasicValue* newValue);

struct FunctionalBasicTag {
  const char* name;
  int alias;
  void* value_address;
  bool local_writable;
  bool remote_writable;
  bool valueChanged;  // Set every time read is called
  uint64_t lastRead;
  size_t buffer_value_max_len;
  SparkplugDataType datatype;
  BasicValue currentValue;
  BasicValue previousValue;
  CompareFunction compareFunc;
  onValueChangeFunction onChange;
  ValidateWriteFunction validateWrite;  // New addition for v1.3.0
  /*void* _extra_data; // New addition for v1.3.0 Unsure if this will be added or not */
};  // Size is 112 bytes + bytes / char values


typedef struct {
  FunctionalBasicTag* tag_ptr;
  void* next_node;
  // void* previous_node; TODO implement later
} FunctionalBasicTagNode;  // Linked List Wrapper for FunctionalBasicTag


/* Functions definitions */

FunctionalBasicTag* createTag(const char* name, void* value_address, int alias, SparkplugDataType datatype, bool local_writable, bool remote_writable, size_t buffer_value_max_len);

/* String & Buffer Types */
FunctionalBasicTag* createStringTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable, size_t string_max_len);
FunctionalBasicTag* createTextTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable, size_t string_max_len);
FunctionalBasicTag* createUUIDTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createBytesTag(const char* name, BufferValue* value_address, int alias, bool local_writable, bool remote_writable, size_t buffer_value_max_len);

/* Int Types */
FunctionalBasicTag* createInt8Tag(const char* name, int8_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createInt16Tag(const char* name, int16_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createInt32Tag(const char* name, int32_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createInt64Tag(const char* name, int64_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createUInt8Tag(const char* name, uint8_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createUInt16Tag(const char* name, uint16_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createUInt32Tag(const char* name, uint32_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createUInt64Tag(const char* name, uint64_t* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createDateTimeTag(const char* name, uint64_t* value_address, int alias, bool local_writable, bool remote_writable);

/* Float Types */
FunctionalBasicTag* createFloatTag(const char* name, float* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createDoubleTag(const char* name, double* value_address, int alias, bool local_writable, bool remote_writable);

/* Bool Type */
FunctionalBasicTag* createBoolTag(const char* name, bool* value_address, int alias, bool local_writable, bool remote_writable);


bool deleteTag(FunctionalBasicTag* tag);

bool DefaultCompareFn(BasicValue* currentValue, BasicValue* newValue);

bool readBasicTag(FunctionalBasicTag* tag, uint64_t timestamp);

bool writeBasicTag(FunctionalBasicTag* tag, BasicValue* newValue);

unsigned int getTagsCount();

typedef void (*TagFunction)(FunctionalBasicTag* tag);  // Function that is called on each tag by the iterTags function

void iterTags(TagFunction tagFn);

/*
Version 1.1.0 Additions
*/

// When read is called
bool addOnChangeCallback(FunctionalBasicTag* tag, onValueChangeFunction callbackFn);  // returns true when added succesfully

typedef bool (*TagFindFunction)(FunctionalBasicTag* tag, void* arg); // Returns true if the tag matches a certain criterea set out in the function
FunctionalBasicTag* findTag(TagFindFunction matcherFn, void* arg); // Returns pointer to first tag for which the matcherFn returns true, returns NULL of no match found

bool aliasValid(int alias); // Iterate through tags, check if alias has already been used or not
int getNextAlias(); // Iterate through tags, get the max alias and return max_alias + 1

/*
New Functions for Version 1.2.0
*/

FunctionalBasicTag* getTagByName(const char* name);
FunctionalBasicTag* getTagByAlias(int alias);

FunctionalBasicTag* getTagByIdx(size_t idx);

/*
New Functions for Version 1.3.0
*/

bool addValidateWriteCallback(FunctionalBasicTag* tag, ValidateWriteFunction callbackFn);


bool setBasicTagTimestampFunction(TimestampFunction fn);

bool readAllBasicTags(); // Read all tags, return if true if any values have changed, otherwise false

// Buffer Allocate/Deallocators
bool allocateStringValue(BasicValue* value, size_t max_str_length);
bool deallocateStringValue(BasicValue* value);

bool allocateBufferValue(BasicValue* value, size_t buffer_size);
bool deallocateBufferValue(BasicValue* value);

#ifdef __cplusplus
}
#endif

#endif // BASIC_TAG_H