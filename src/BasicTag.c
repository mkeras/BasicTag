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

#include "BasicTag.h"

bool DefaultCompareFn(BasicValue* currentValue, BasicValue* newValue) {
  /* Return true if values have changed, false if not
  false: new value ignored; true: current becomes previous and new value becomes current
  The CompareFunction is where deadband and report by exception is enabled.
  */
  switch (currentValue->datatype) {
      case Int8:
        return currentValue->value.int8Value != newValue->value.int8Value;
      case Int16:
        return currentValue->value.int16Value != newValue->value.int16Value;
      case Int32:
        return currentValue->value.int32Value != newValue->value.int32Value;
      case Int64:
        return currentValue->value.int64Value != newValue->value.int64Value;
      case UInt8:
        return currentValue->value.uint8Value != newValue->value.uint8Value;
      case UInt16:
        return currentValue->value.uint16Value != newValue->value.uint16Value;
      case UInt32:
        return currentValue->value.uint32Value != newValue->value.uint32Value;
      case DateTime:  // Datetime is a uint64
      case UInt64:
        return currentValue->value.uint64Value != newValue->value.uint64Value;
      case Float:
        return currentValue->value.floatValue != newValue->value.floatValue;
      case Double:
        return currentValue->value.doubleValue != newValue->value.doubleValue;
      case Boolean:
        return currentValue->value.boolValue != newValue->value.boolValue;
      case Text:  // Text is a string
      case UUID: // UUID is a string
      case String:
        return strcmp(currentValue->value.stringValue, newValue->value.stringValue) != 0;
      case Bytes:
        return true;
      default:
        // For unknown types, you might not need to do anything
        return true;
    }
}

/*
Tag Linked List Functions
Tag allocation deallocation handled elsewhere, these are designed to be called after a creation or deletion of tag
*/

static FunctionalBasicTagNode* _head = NULL;
static unsigned int _NodeCount = 0;
// v1.2.0 maintain array of tags for quick indexing
static FunctionalBasicTag** _tags_array = NULL;

unsigned int getTagsCount() {
    return _NodeCount;
}

// New addition in v1.2.0
static bool _update_tags_array() {
    if (_NodeCount == 0) {
      if (_tags_array != NULL) free(_tags_array);
      _tags_array = NULL;
      return true;
    }

    // Allocate to the number of Nodes
    if (_tags_array == NULL) {
      // malloc
      _tags_array = malloc(_NodeCount * sizeof(FunctionalBasicTag*));
    } else {
      // realloc
      _tags_array = realloc(_tags_array, _NodeCount * sizeof(FunctionalBasicTag*));
    }

    // Check if memory operation failed
    if (_tags_array == NULL) return false;

    // Walk list and add addresses to _tags_array
    FunctionalBasicTagNode* current = _head;
    size_t count = _NodeCount - 1; // start at end and work backwards
    while (current != NULL) {
        _tags_array[count] = current->tag_ptr;
        current = current->next_node;
        count--;
    }
    return true;
}

static FunctionalBasicTagNode* _add_tag_to_list(FunctionalBasicTag* tag) {
    FunctionalBasicTagNode* new = malloc(sizeof(FunctionalBasicTagNode));
    if (new == NULL)
        return NULL;
    new->tag_ptr = tag;

    if (_head == NULL) {
        new->next_node = NULL;
    } else {
        new->next_node = _head;
    }

    _head = new;
    _NodeCount += 1;
    
    _update_tags_array();

    return new;
}

static bool _remove_tag_from_list(FunctionalBasicTag* tag) {
    FunctionalBasicTagNode* current = _head;
    FunctionalBasicTagNode* prev = _head;
     while (current != NULL) {
        if (current->tag_ptr == tag) {
            if (current == _head) {
                // remove current reference from head
                _head = current->next_node;
            } else {
                // Remove current reference from list
                prev->next_node = current->next_node;
            }

            free(current);
            _NodeCount -= 1;
            _update_tags_array();
            return true;
        }
        prev = current;
        current = current->next_node;
     }
    return false;
}

void iterTags(TagFunction tagFn) {
    if (_head == NULL) return;  // If there are no tags return right away
    FunctionalBasicTagNode* current = _head;
    while (current != NULL) {
        tagFn(current->tag_ptr);
        current = current->next_node;
    }
}


/* Data Type Allocators and Deallocators */


// string value (char*) allocate/deallocate

static bool _init_string_value(Value* value, size_t max_str_length) {
  if (value->stringValue != NULL) return false; // Don't allocate to an already allocated
  if (max_str_length < 1) {
    value->stringValue = NULL;
    return true;
  }

  value->stringValue = (char*)malloc(max_str_length + 1);
  if (!value->stringValue) return false; // Check for malloc failure
  memset(value->stringValue, '\0', max_str_length + 1);
  return true;
}

static bool _deallocate_string_value(Value* value) {
  if (value->stringValue == NULL) return false;  // If it's already NULL there's nothing to free
  free(value->stringValue);
  value->stringValue = NULL;
  return true;
}


// BufferValue allocate/deallocate

static bool _init_buffer_value(Value* value, size_t buffer_size) {
  if (value->bytesValue != NULL) return false; // Don't allocate to an already allocated

  // Allocate the BufferValue
  value->bytesValue = (BufferValue*)malloc(sizeof(BufferValue));
  if (value->bytesValue == NULL) {
    return false; // Allocation failed
  }

  // 0 Length case, no need to allocate; this would be a useless tag and indicative of config error
  if (buffer_size < 1) {
    value->bytesValue->buffer = NULL;
    value->bytesValue->written_length = 0;
    value->bytesValue->allocated_length = 0;
    return true;
  }

  value->bytesValue->buffer = (uint8_t*)malloc(buffer_size);
  // Handle allocation failure
  if (value->bytesValue->buffer == NULL) {
    value->bytesValue->written_length = 0;
    value->bytesValue->allocated_length = 0;
    return false;
  }

  // Successfully initialized the buffer
  value->bytesValue->written_length = 0;
  value->bytesValue->allocated_length = buffer_size;
  return true;
}

static bool _deallocate_buffer_value(Value* value) {
  if (value->bytesValue == NULL) return false; // There is no buffer value allocated

  if (value->bytesValue->buffer != NULL) free(value->bytesValue->buffer);  // If it's already NULL there's nothing to free
  
  free(value->bytesValue);  // free the BufferValue instance
  value->bytesValue = NULL; // Reset pointer to NULL

  return true;
}


/* Tag Create and Delete Functions */


static bool _init_functional_basic_tag(FunctionalBasicTag* tag, char* name, void* value_address, int alias, SparkplugDataType datatype, bool local_writable, bool remote_writable, size_t buffer_value_max_len) {
  if (tag == NULL) return false; // Safety check to ensure the tag pointer is not null

  if (!aliasValid(alias)) alias = getNextAlias();  // If alias is not unique, make it so

  // Set the name, value_address, alias, and datatype
  tag->name = name;
  tag->value_address = value_address;
  tag->alias = alias;
  tag->datatype = datatype;
  tag->local_writable = local_writable;
  tag->remote_writable = remote_writable;
  tag->buffer_value_max_len = buffer_value_max_len;
  tag->compareFunc = DefaultCompareFn;
  tag->valueChanged = false;
  tag->lastRead = 0;
  tag->onChange = NULL; // Must be set by addOnChangeCallback to keep backward compatibility

  // Initialize currentValue and previousValue
  tag->currentValue.timestamp = 0;
  tag->currentValue.datatype = datatype;
  tag->currentValue.isNull = true;
  tag->previousValue.timestamp = 0;
  tag->previousValue.datatype = datatype;
  tag->previousValue.isNull = true;

  switch (datatype) {
    case UUID:
      buffer_value_max_len = 36;  // Override buffer max value to length of UUID
    case Text:
    case String:
      // Allocate the strings for values
      {
      // WARNING: ASSUMES THE VALUES HAVE NOT YET BEEN ALLOCATED/INITIALIZED
      // CALLING __init_functional_basic_tag twice on same tag will cause memory leak
      tag->currentValue.value.stringValue = NULL;
      tag->previousValue.value.stringValue = NULL;

      _init_string_value(&(tag->currentValue.value), buffer_value_max_len);
      _init_string_value(&(tag->previousValue.value), buffer_value_max_len);
      }
      break;
    case Bytes:
      // WARNING: ASSUMES THE VALUES HAVE NOT YET BEEN ALLOCATED/INITIALIZED
      // CALLING __init_functional_basic_tag twice on same tag will cause memory leak
      tag->currentValue.value.bytesValue = NULL;
      tag->previousValue.value.bytesValue = NULL;
      // Allocate bytes buffer
      _init_buffer_value(&(tag->currentValue.value), buffer_value_max_len);
      _init_buffer_value(&(tag->previousValue.value), buffer_value_max_len);
      break;
    default:
      break;
  }

  return true;
}


FunctionalBasicTag* createTag(const char* name, void* value_address, int alias, SparkplugDataType datatype, bool local_writable, bool remote_writable, size_t buffer_value_max_len) {
    /* 
    handles the creation of a new tag, including adding to a tags linked list
    This should always be used to create tags
    */

    FunctionalBasicTag* newTag = (FunctionalBasicTag*)malloc(sizeof(FunctionalBasicTag));
    if (newTag == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    if(!_init_functional_basic_tag(newTag, name, value_address, alias, datatype, local_writable, remote_writable, buffer_value_max_len)) {
        free(newTag);
        return NULL;
    }

    _add_tag_to_list(newTag);
    return newTag;
}

/*Create String & Buffer Types*/
FunctionalBasicTag* createStringTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable, size_t string_max_len) {
  return createTag(name, (void*)value_address, alias, String, local_writable, remote_writable, string_max_len);
}
FunctionalBasicTag* createTextTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable, size_t string_max_len) {
  return createTag(name, (void*)value_address, alias, Text, local_writable, remote_writable, string_max_len);
}
FunctionalBasicTag* createUUIDTag(const char* name, char* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, UUID, local_writable, remote_writable, 36);
}
FunctionalBasicTag* createBytesTag(const char* name, BufferValue* value_address, int alias, bool local_writable, bool remote_writable, size_t buffer_value_max_len) {
  return createTag(name, (void*)value_address, alias, Bytes, local_writable, remote_writable, buffer_value_max_len);
}
/*Create Int Types*/
FunctionalBasicTag* createInt8Tag(const char* name, int8_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, Int8, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createInt16Tag(const char* name, int16_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, Int16, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createInt32Tag(const char* name, int32_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, Int32, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createInt64Tag(const char* name, int64_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, Int64, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createUInt8Tag(const char* name, uint8_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, UInt8, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createUInt16Tag(const char* name, uint16_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, UInt16, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createUInt32Tag(const char* name, uint32_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, UInt32, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createUInt64Tag(const char* name, uint64_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, UInt64, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createDateTimeTag(const char* name, uint64_t* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, DateTime, local_writable, remote_writable, 0);
}
/*Create Float Types*/
FunctionalBasicTag* createFloatTag(const char* name, float* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, Float, local_writable, remote_writable, 0);
}
FunctionalBasicTag* createDoubleTag(const char* name, double* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, Double, local_writable, remote_writable, 0);
}
/*Create Bool Types*/
FunctionalBasicTag* createBoolTag(const char* name, bool* value_address, int alias, bool local_writable, bool remote_writable) {
  return createTag(name, (void*)value_address, alias, Boolean, local_writable, remote_writable, 0);
}


static bool _deallocate_functional_basic_tag(FunctionalBasicTag* tag) {
    /*
    Deallocate any malloc'd variables before tag is deallocated
    */
    switch (tag->datatype) {
        case Text:
        case UUID:
        case String:
            // Deallocate char buffers
            _deallocate_string_value(&(tag->currentValue.value));
            _deallocate_string_value(&(tag->previousValue.value));
            break;
        case Bytes:
            // Deallocate bytes buffer
            _deallocate_buffer_value(&(tag->currentValue.value));
            _deallocate_buffer_value(&(tag->previousValue.value));
            break;
        default:
            break;
    }

    free(tag);

    return true;
}


bool deleteTag(FunctionalBasicTag* tag) {
    /*
    Used for deleting a tag created by createTag
    also handles removing from linked list
    */
    if(_deallocate_functional_basic_tag(tag)) return _remove_tag_from_list(tag);
    return false;
}


/*
Value Copy Functions
functions used for copying BasicValue structs or Value unions
*/

static bool _copy_string_value(char* reference_str, char* target_str, size_t buffer_value_max_len) {
  // Check for Null values or zero length buffer
  if (reference_str == NULL || target_str == NULL || buffer_value_max_len < 1) return false;

  // check if the string is empty; empty string doesn't mean operation failed
  if (reference_str[0] == '\0') return true;

  size_t length = strlen(reference_str);
  // Check string length, safety check to makesure char buffer is never overflowed
  if (length > buffer_value_max_len) length = buffer_value_max_len;

  // TODO Evaluate neccesity of performing strcmp before copying, to reduce copy operations

  // Copy string from reference to target
  memcpy(target_str, reference_str, length);
  // Ensure that null terminator is there in correct spot
  target_str[length] = '\0';
  return true;
}

static bool _copy_buffer_value(BufferValue* reference_buf, BufferValue* target_buf) {
  // Check for Null values or zero length buffer
  if (reference_buf == NULL || target_buf == NULL || target_buf->allocated_length < 1) return false;

  // check if the string is empty; empty string doesn't mean operation failed
  if (reference_buf->written_length == 0) {
    memset(target_buf->buffer, 0x0, target_buf->allocated_length);
    target_buf->written_length = 0;
    return true;
  }

  size_t length = reference_buf->written_length;
  // Check buffer length, safety check to makesure target buffer is never overflowed
  if (length > target_buf->allocated_length) length = target_buf->allocated_length;

  // TODO Evaluate neccesity of performing strcmp before copying, to reduce copy operations

  // Copy string from reference to target
  memcpy(target_buf, reference_buf, length);
  target_buf->written_length = length;
  return true;
}

static void _copyBasicValue(BasicValue* reference, BasicValue* target, size_t buffer_value_max_len) {
    if (reference == NULL || target == NULL) return;

    // Copy simple fields
    target->timestamp = reference->timestamp;
    target->datatype = reference->datatype;
    target->isNull = reference->isNull;

    // If it's null there's nothing else to copy
    if (reference->isNull) return;

    // Depending on the SparkplugDataType, handle the value copying
    switch (reference->datatype) {
      case Int8:
      case Int16:
      case Int32:
      case Int64:
      case UInt8:
      case UInt16:
      case UInt32:
      case UInt64:
      case Float:
      case Double:
      case Boolean:
      case DateTime:
        // For these types, direct assignment is fine
        target->value = reference->value;
        break;
      case Text:  // Text is a string
      case UUID: // UUID is a string
      case String:
        _copy_string_value(reference->value.stringValue, target->value.stringValue, buffer_value_max_len);
        break;
      case Bytes:
        _copy_buffer_value(reference->value.bytesValue, target->value.bytesValue);
        break;
      default:
        // For unknown types, you might not need to do anything
        target->isNull = true;
        break;
    }
}


/* Tag read/write Functions */

bool readBasicTag(FunctionalBasicTag* tag, uint64_t timestamp) {
  if (tag == NULL) return false; // Safety check
  
  tag->lastRead = timestamp;

  BasicValue newValue = {timestamp, tag->datatype, {0}, false};
  

  if (tag->value_address == NULL) {
    newValue.isNull = true;
    return true;
  }

  switch (tag->datatype) {
    case Int8: // Handle Int8 type
        newValue.value.int8Value = *((int8_t*)(tag->value_address));
        break;
    case Int16:  // Handle Int16 type
        newValue.value.int16Value = *((int16_t*)(tag->value_address));
        break;
    case Int32: // Handle Int32 type
        newValue.value.int32Value = *((int32_t*)(tag->value_address));
        break;
    case Int64: // Handle Int64 type
        newValue.value.int64Value = *((int64_t*)(tag->value_address));
        break;
    case UInt8: // Handle UInt8 type
        newValue.value.uint8Value = *((uint8_t*)(tag->value_address));
        break;
    case UInt16: // Handle UInt16 type
        newValue.value.uint16Value = *((uint16_t*)(tag->value_address));
        break;
    case UInt32: // Handle UInt32 type
        newValue.value.uint32Value = *((uint32_t*)(tag->value_address));
        break;
    case DateTime: // DateTime is epoch milliseconds as uint64
    case UInt64: // Handle UInt64 type
        newValue.value.uint64Value = *((uint64_t*)(tag->value_address));
        break;
    case Float: // Handle Float type
        newValue.value.floatValue = *((float*)(tag->value_address));
        break;
    case Double: // Handle Double type
        newValue.value.doubleValue = *((double*)(tag->value_address));
        break;
    case Boolean: // Handle Boolean type
        newValue.value.boolValue = *((bool*)(tag->value_address));
        break;
    case Text:  // Test is a string
    case UUID: // UUID is a string
    case String: // Handle String type, cast the tag value adress directly to newValue.value.stringValue
        newValue.value.stringValue = (char*)(tag->value_address);
        // check for 0 length char or empty string
        if (tag->buffer_value_max_len < 1 || strlen(newValue.value.stringValue) == 0) {
          newValue.value.stringValue = NULL;
          newValue.isNull = true;
        }
        break;
    case Bytes: // Buffer Type, value_address must be a pointer to a BufferValue
      newValue.value.bytesValue = (BufferValue*)(tag->value_address);
    default:
        newValue.isNull = true;
        break; // For unknown types, you might not need to do anything
  }

  bool valueChanged = true;
  // If there is a compare function, set valueChanged to indicate if value has changed or not, otherwise defaults to true
  // If the currentValue timestamp is 0, skip compareFunc and automatically indicate true as it's first read
  if (tag->currentValue.timestamp == 0) {
    // timestamp 0 means it's initial read
  } else if (tag->currentValue.isNull || newValue.isNull) {
    valueChanged = tag->currentValue.isNull != newValue.isNull;
  } else if (tag->compareFunc != NULL) {
    // compare func, returns if the value should be considered changed or not (the compare Function handles any deadband, etc)
    valueChanged = tag->compareFunc(&(tag->currentValue), &newValue);
  }
  tag->valueChanged = valueChanged;
  if (!valueChanged) return false;

  // Update the current and previous values only if the value is considered changed
  _copyBasicValue(&(tag->currentValue), &(tag->previousValue), tag->buffer_value_max_len);
  _copyBasicValue(&newValue, &(tag->currentValue), tag->buffer_value_max_len);

  if (tag->onChange != NULL) tag->onChange(tag);
  return true;
}

bool writeBasicTag(FunctionalBasicTag* tag, BasicValue* newValue) {
  /* It is up to the user to validate if the write is remote or local.
  User must correctly set the newValue to match the datatype of the tag, otherwise errors can occur */
  if (tag->value_address == NULL) return false;
  if (!tag->local_writable && !tag->remote_writable) return false;

  switch (tag->datatype) {
    case Int8:
      *(int8_t*)(tag->value_address) = newValue->value.int8Value;
      break;
    case Int16:
        *(int16_t*)(tag->value_address) = newValue->value.int16Value;
        break;
    case Int32:
        *(int32_t*)(tag->value_address) = newValue->value.int32Value;
        break;
    case Int64:
        *(int64_t*)(tag->value_address) = newValue->value.int64Value;
        break;
    case UInt8:
        *(uint8_t*)(tag->value_address) = newValue->value.uint8Value;
        break;
    case UInt16:
        *(uint16_t*)(tag->value_address) = newValue->value.uint16Value;
        break;
    case UInt32:
        *(uint32_t*)(tag->value_address) = newValue->value.uint32Value;
        break;
    case Float:
      *(float*)(tag->value_address) = newValue->value.floatValue;
      break;
    case Double:
      *(double*)(tag->value_address) = newValue->value.doubleValue;
      break;
    case Boolean:
      *(bool*)(tag->value_address) = newValue->value.boolValue;
      break;
    case DateTime:
    case UInt64:
      *(uint64_t*)(tag->value_address) = newValue->value.uint64Value;
      break;
    case Text:  // Text is a string
    case UUID: // UUID is a string
    case String:
      // NULL checks
      if (newValue->isNull || newValue->value.stringValue == NULL || newValue->value.stringValue[0] == '\0') {
        // set the value at tag->value_address to null according to the max length
        memset(tag->value_address, '\0', tag->buffer_value_max_len);
        break;
      }
      _copy_string_value(newValue->value.stringValue, (char*)(tag->value_address), tag->buffer_value_max_len);
      break;
    case Bytes:
      // NULL checks
      if (newValue->isNull || newValue->value.bytesValue == NULL || newValue->value.bytesValue->buffer == NULL || newValue->value.bytesValue->allocated_length < 1 || newValue->value.bytesValue->written_length < 1) {
        // set the value at tag->value_address to null according to the max length
        memset(tag->value_address, 0x0, tag->buffer_value_max_len);
        break;
      }
      _copy_buffer_value(newValue->value.bytesValue, (BufferValue*)(tag->value_address));
      break;
    default:
      // For unknown types, you might not need to do anything
      return false;
    }

  return true;
}


/*
New Functions for Version 1.1.0
*/

bool addOnChangeCallback(FunctionalBasicTag* tag, onValueChangeFunction callbackFn) {
  if (callbackFn == NULL || tag == NULL) return false;
  tag->onChange = callbackFn;
  return true;
}


// Linked list functionality

FunctionalBasicTag* findTag(TagFindFunction matcherFn, void* arg) {
  /* Returns the first tag found for which the mathcherFn returns true */
  if (_head == NULL || matcherFn == NULL) return NULL;  // If there are no tags return right away
  FunctionalBasicTagNode* current = _head;
  while (current != NULL) {
      if (matcherFn(current->tag_ptr, arg)) return current->tag_ptr;
      current = current->next_node;
  }
  return NULL;
}

static bool _tag_has_alias(FunctionalBasicTag* tag, void* arg) {
  return tag->alias == *(int*)arg;
}

bool aliasValid(int alias) {
  return findTag(_tag_has_alias, (void*)&alias) == NULL;
}

int getNextAlias() {
  if (_head == NULL) return 1;  // If there are no tags start aliases at 1
  int max = 0;
  FunctionalBasicTagNode* current = _head;
  while (current != NULL) {
      if (current->tag_ptr->alias > max) max = current->tag_ptr->alias;
      current = current->next_node;
  }
  return max + 1;
}


/*
New Functions for Version 1.2.0
*/

static bool _tag_has_name(FunctionalBasicTag* tag, void* arg) {
  const char* tagName = (char*)arg;
  return strcmp(tag->name, tagName) == 0;
}

FunctionalBasicTag* getTagByName(const char* name) {
  // Returns the first tag that has a given name
  return findTag(_tag_has_name, (void*)name);
}

FunctionalBasicTag* getTagByAlias(int alias) {
  // Returns the first tag that has a given alias
  return findTag(_tag_has_alias, (void*)&alias);
}

FunctionalBasicTag* getTagByIdx(size_t idx) {
  // idx is bigger than arraylen
  if (idx < _NodeCount) return _tags_array[idx];
  return NULL;
}