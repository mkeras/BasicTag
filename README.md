# BasicTag
 Basic tag abstraction written in C. Designed to be used as a building block for other libraries to enable IOT/IIOT functionality (eg. I will use this for implementing sparkplug B/sparkplug 3 edge node). Purpose is to form a base for organizing and monitoring variables, detecting changes and writing new values.

# Overview
The Basic Tag Library is a C library designed for managing and manipulating various data types within a structured system. It provides a comprehensive suite of functions and data structures to handle a wide range of data types, including basic types (like integers and floats), complex types (like strings and byte arrays), and specialized types (like UUIDs and DateTime). The SparkplugDataType Enum is a direct copy from the Sparkplug 3.0 specification. More complex datatypes like arrays, PropertySet, and Template have not been implemented yet. It is undecided as of yet whether it is necessary to implement these more complex datatypes, as in a microcontroller environment that this library is designed for, they are generally not neccessary.

# Key Features
- No Dependencies: written in C using only standard datatypes and structures, can ompile to basically any platform
- Support for Basic Tag Data Types: integers, floats, booleans, strings, byte arrays, UUIDs, DateTime.
- Functional Library: written in pure C, this library uses functions and structs for siplicity and performance.
- Custom Compare Functions: Ability to define custom comparison logic for value changes, enabling different types of deadband, etc.
- Tag Management Functions: Create, read, write, and delete functions for tags.
- Iterative Operations: When tags are created they are stored in a singley linked list, enabling easy iteration over all tags, with a custom callback function being called on each tag (iterTags function).
- Timestamp Support: Designed to be used with millisecond timestamps, provided to the read function via an external source
- SparkplugDataType (Enum): Represents various data types supported by the library, and follows the Sparkplug 3.0 specification.
- BufferValue (Struct): A helpful struct for managing buffer data. Stores a buffer pointer, its size (allocated), and the written length.
- Value (Union): The base union to store values across different data types.
- BasicValue (Struct): Combines value with datatype and timestamp information to allow all datatypes to have the same structure.
- FunctionalBasicTag (Struct): Represents a tag, storing its various attributes like name, data type, value, compare function, etc.
- FunctionalBasicTagNode (Struct): Under the hood Node struct for creating the linked list of FunctionalBasicTag.
# Functions
- Tag Creation Functions: Functions to create tags for different data types (e.g., createInt8Tag, createStringTag). They are shorthand for calling the createTag function and all return a newly allocated pointer to a FunctionalBasicTag struct. Also handles adding to the linked list.
- deleteTag Function: Deletes a tag, handling deallocation and removal from the linked list.
- DefaultCompareFn Function: A default comparison function that checks basic equality when reading tags.
- readBasicTag and writeBasicTag Functions: Read and write operations for basic tags with NULL pointer checks to prevent memory leaks. writeBasicTag also checks if either of the 2 writable flags of the tag are set before writing. 
- getTagsCount Function: Returns the count of the current number of tags.
- iterTags Function: Iterates over tags and applies a user supplied function to each FunctionalBasicTag instance.
# API Reference

## v1.1.0
Function for adding onChange callback to a tag:
```c
bool addOnChangeCallback(FunctionalBasicTag* tag, onValueChangeFunction callbackFn);
```

Tag find/search functionality: Function typedef that is a bool returning function that determines if a tag matches some criteria. findTag iterates and checks the matcherFn until it returns true, then returns that tag's pointer, or it returns NULL when no match is found.
```c
typedef bool (*TagFindFunction)(FunctionalBasicTag* tag, void* arg); // Returns true if the tag matches a certain criteria set out in the function
FunctionalBasicTag* findTag(TagFindFunction matcherFn, void* arg); // Returns pointer to first tag for which the matcherFn returns true, returns NULL of no match found
```

Two Helper functions for dealing with aliases have been added:
aliasValid validates an alias, checking if alias has already been used or not.
getNextAlias returns the max value of the current aliases plus 1.
In a future release aliases might become fully handled under the hood.
```c
bool aliasValid(int alias); // 
int getNextAlias(); // Iterate through tags, get the max alias and return max_alias + 1
```

## v1.0.0
## createTag
createTag is the base function for creating new tags. It creates a tag based on the supplied arguments, and handles memory allocations and linked list backend. Returns a pointer to the newly created FunctionalBasicTag struct instance.
```c
FunctionalBasicTag* createTag(const char* name, void* value_address, int alias, SparkplugDataType datatype, bool local_writable, bool remote_writable, size_t buffer_value_max_len);
```
There are also helpful shorthand functions for creating each type of tag:
```c
/* String & Buffer Types */
FunctionalBasicTag* createStringTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable, size_t string_max_len);
FunctionalBasicTag* createTextTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable, size_t string_max_len);
FunctionalBasicTag* createUUIDTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable);
FunctionalBasicTag* createBytesTag(const char* name, BufferValue* value_address, int alias, bool local_writable, bool remote_writable, size_t );

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
```
### Arguments:
- **name**: the const char* string of the name that the tag will have
- **value_address**: This is the pointer to the value that the tag will read from. When using createTag it is a void* pointer, otherwise it matches its function *eg. createFloatTag expects a float* pointer*. The value_address must be allocated for *at least* the lifetime of the tag to prevent memory issues.
- **alias**: The unique integer value identifier for the tag. This should be unique across all tags, although it is not yet enforced.
- **local_writeable**: Boolean value to indicate if the tag should be considered writable locally (within the program) or not.
- **remote_writeable**: Boolean value to indicate if the tag should be considered writable remotely (ie, via some external source, internet, etc) or not.
- **buffer_value_max_len**: a size_t integer to indicate the max length, if the datatype is a buffer or string value. For strings it is exlusive of the null ('\0') terminator, the underlying functions will allocate buffer_value_max_len + 1 to help ensure string safety. It should also not be larger than the allocated length of the value_address + 1 to account for the the null ('\0') terminator.

## deleteTag
Handles deletion of a tag, deallocates any allocations done by the createTag function and removes it from the linked list of tags. Returns true on success, false if not.
```c
bool deleteTag(FunctionalBasicTag* tag);
```
### Arguments:
- tag: The pointer of the tag (FunctionalBasicTag instance) that you want to delete.

## readBasicTag:
Reads a tag. User must supply a tag pointer and a uint64_t millisecond timestamp. Return value indicates if tag value has changed or not. If the compareFn of the tag is set, it is the return value of that.
```c
bool readBasicTag(FunctionalBasicTag* tag, uint64_t timestamp);
```
### Arguments:
- tag: The FunctionalBasicTag* pointer of the tag to read.
- timestamp: The uint64_t value of the millisecond epoch timestamp

## writeBasicTag:
Writes a value to a tag. Performs necessary NULL checks and then writes the value of the supplied BasicValue* newValue to the value_address of the tag.
```c
bool writeBasicTag(FunctionalBasicTag* tag, BasicValue* newValue);
```
### Arguments:
- tag: The FunctionalBasicTag* pointer of the tag to write to.
- newValue: A pointer to a BasicValue struct that has it's value union set according to the datatype of the tag it is to be written to. Currently (1.0.0) doesn't use the timestamp, isNull or datatype of the newValue, so they don't need to be set when making a BasicValue to use with this function.

## getTagsCount
Returns the number of tags currently configured.
```c
unsigned int getTagsCount();
```

# Data Structures

This is the enum for datatypes, based off of the sparkplug 3.0 standard. The datatype is what lets the functions know which value type to use with the tags and values:
```c
typedef enum {
  Int8 = 1,
  Int16 = 2,
  Int32 = 3,
  Int64 = 4,
  UInt8 = 5,
  UInt16 = 6,
  UInt32 = 7,
  UInt64 = 8,
  Float = 9,
  Double = 10,
  Boolean = 11,
  String = 12,
  DateTime = 13,  // uint64, epoch milliseconds
  Text = 14,
  UUID = 15,   // a string of 36 characters
  Bytes = 17,
} SparkplugDataType;
```

The BufferValue struct is for using with bytes values. It stores the pointer to the allocated buffer, the allocated length of the buffer, and a written_length:
```c
typedef struct {
  uint8_t* buffer;  // Pointer to an array of uint8_t
  size_t written_length;  // Length of the array
  size_t allocated_length;  // allocated/max length
} BufferValue;  // Size 12 bytes + length of buffer
```

The Value union is the under the hood component for storing the different datatypes:
```c
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
```

The BasicValue is the base of a tag, it contains the datatype, value, timestamp, and a helpful isNull flag:
```c
typedef struct {
    uint64_t timestamp;
    SparkplugDataType datatype;
    Value value;
    bool isNull;
} BasicValue; // Basic Value size is 32 bytes
```

The CompareFunction type is used by the readBasicTag function to compare tag values when reading, return true if value should be considered changed, false if not. It's purpose is to enable any type of deadband processing on a tag:
```c
typedef bool (*CompareFunction)(BasicValue* previousValue, BasicValue* newValue);
```

The FunctionalBasicTag is the 'tag'. It stores all the tag's information. The user should generally only be reading/checking a FunctionalBasicTags variables, and allow the api functions to handle the changing of any of it's values:
```c
typedef struct FunctionalBasicTag FunctionalBasicTag; // v1.1.0: Forward declaration
```
New Function type in v1.1.0: Optional function that can be registered for a tag that is triggered be read, when the value has changed. Called after tag values have been updated, before returning.
```c
typedef void (*onValueChangeFunction)(FunctionalBasicTag* tag);

struct {
  const char* name;
  int alias;
  void* value_address;
  bool local_writable;
  bool remote_writable;
  bool valueChanged;  // Set every time read is called, the result of compareFunc
  uint64_t lastRead;  // timestamp of when readBasicTag was last called on it
  size_t buffer_value_max_len;
  SparkplugDataType datatype;
  BasicValue currentValue;
  BasicValue previousValue;
  CompareFunction compareFunc;
  onValueChangeFunction onChange; // New in v1.1.0
} FunctionalBasicTag;  // Size is 108 bytes + bytes / char values
```

The FunctionalBasicTagNode is the basic linked list structure. It should generally not be used manually by the user as the linked list is managed automatically by the api functions:
```c
typedef struct {
  FunctionalBasicTag* tag_ptr;
  void* next_node;
} FunctionalBasicTagNode;  // Linked List Node Wrapper for FunctionalBasicTag
```

This function type is used by the iterTags function. It is simply a function that does something to a tag with no return value (void).
```c
typedef void (*TagFunction)(FunctionalBasicTag* tag);
```