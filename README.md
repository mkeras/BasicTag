# BasicTag
 Basic tag abstraction written in C. Designed to be used in other libraries for more complex tag functionality (eg. I will use this for implementing sparkplug B/sparkplug 3 edge node). Purpose is to form a base for organizing and monitoring variables, detecting changes and writing new values.

## Overview
The Basic Tag Library is a C library designed for managing and manipulating various data types within a structured system. It provides a comprehensive suite of functions and data structures to handle a wide range of data types, including basic types (like integers and floats), complex types (like strings and byte arrays), and specialized types (like UUIDs and DateTime). The SparkplugDataType Enum is a direct copy from the Sparkplug 3.0 specification. More complex datatypes like arrays, PropertySet, and Template have not been implemented yet. It is undecided as of yet whether it is necessary to implement these more complex datatypes, as in a microcontroller environment that this library is designed for, they are generally not neccessary.

## Key Features
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
## Functions
- Tag Creation Functions: Functions to create tags for different data types (e.g., createInt8Tag, createStringTag). They are shorthand for calling the createTag function and all return a newly allocated pointer to a FunctionalBasicTag struct. Also handles adding to the linked list.
- deleteTag Function: Deletes a tag, handling deallocation and removal from the linked list.
- DefaultCompareFn Function: A default comparison function that checks basic equality when reading tags.
- readBasicTag and writeBasicTag Functions: Read and write operations for basic tags with NULL pointer checks to prevent memory leaks. writeBasicTag also checks if either of the 2 writable flags of the tag are set before writing. 
- getTagsCount Function: Returns the count of the current number of tags.
- iterTags Function: Iterates over tags and applies a user supplied function to each FunctionalBasicTag instance.
## API Reference
### createTag
createTag is the base function for creating new tags. It creates a tag based on the supplied arguments, and handles memory allocations and linked list backend. 
```c
FunctionalBasicTag* createTag(const char* name, void* value_address, int alias, SparkplugDataType datatype, bool local_writable, bool remote_writable, size_t buffer_value_max_len);
```
There are also helpful shorthand functions for creating each type:
```c
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
```

### deleteTag
```c

```