#include <BasicTag.h>

// Declare variables
char* charVal = NULL;
float* floatVal = NULL;

FunctionalBasicTag* stringTag = NULL;
FunctionalBasicTag* floatTag = NULL;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Serial is now connected!");
  Serial.println();

  // Initialize the char value to a 256 length string
  size_t stringLen = 256;
  charVal = (char*)malloc(stringLen + 1);
  memset(charVal, '\0', stringLen + 1);

  //Initialize float value
  floatVal = (float*)malloc(sizeof(float));
  *floatVal = 3.14159; // pi

  // create the tags
  Serial.print("Creating tags...");
  stringTag = createStringTag("testing/string 1", charVal, 1, true, false, stringLen);
  floatTag = createFloatTag("testing/float 1", floatVal, 2, true, false);
  Serial.println(" done.");

  // Show tags count
  Serial.print("Tag count: ");
  Serial.println(getTagsCount());

  // Delete a tag
  Serial.print("Deleting tag...");
  if (deleteTag(stringTag)) {
    stringTag = NULL; // Tag has been deallocated
    Serial.print(" done. (Tag count: ");
    Serial.print(getTagsCount());
    Serial.println(")");

    // Now recreate the tag
    Serial.print("Recreating tag...");
    stringTag = createStringTag("testing/string 1", charVal, 1, true, false, stringLen);
    Serial.print(" done. (Tag count: ");
    Serial.print(getTagsCount());
    Serial.println(")");
  } else {
    Serial.println("FAILED!");
  }

  Serial.println("Setup complete, starting loop.");
}

void loop() {
  Serial.println();
  Serial.println();
  // initialize a BasicValue for the write function
  BasicValue newFloat;
  newFloat.isNull = false;

  // make a new value for demo
  if (floatTag->currentValue.value.floatValue < 1.0) {
    newFloat.value.floatValue = *floatVal * float(millis());
  } else {
    newFloat.value.floatValue =  *floatVal / float(millis());
  }

  // write the value to the tag
  writeBasicTag(floatTag, &newFloat);

  // Make a dummy char value for demo
  char newCharVal[stringTag->buffer_value_max_len + 1];
  newCharVal[stringTag->buffer_value_max_len] = '\0';

  // alternate the char value
  for (int i = 0; i < stringTag->buffer_value_max_len; i++) {
    if (stringTag->currentValue.value.stringValue[i] == 'X') {
      newCharVal[i] = '_';
      continue;
    }
    newCharVal[i] = 'X';
  }

  // initialize BasicValue to write to the tag
  BasicValue newString;
  newString.isNull = false;
  newString.value.stringValue = newCharVal;

  // write the new value to the tag
  writeBasicTag(stringTag, &newString);

  // now use the iterTags with the readAndPrint function to read and print the tags
  iterTags(readAndPrint);

  // wait to repeat the loop
  delay(2000);
}

// print functions for demo
void print_buffer(BufferValue* buf) {
  Serial.print("Buffer Written Length: ");
  Serial.println(buf->written_length);
  for (int i = 0; i < buf->written_length; i++) {
    Serial.print(buf->buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void print_basic_value(BasicValue* value) {
    if (value->isNull) {
        Serial.print("null (timestamp: ");
        Serial.print(value->timestamp);
        Serial.print(")");
        return;
    }
    switch(value->datatype) {
      case SparkplugDataType::spInt8:
          Serial.print("Int8 Value: ");
          Serial.print(value->value.int8Value);
          break;
      case SparkplugDataType::spInt16:
          Serial.print("Int16 Value: ");
          Serial.print(value->value.int16Value);
          break;
      case SparkplugDataType::spInt32:
          Serial.print("Int32 Value: ");
          Serial.print(value->value.int32Value);
          break;
      case SparkplugDataType::spInt64:
          Serial.print("Int64 Value: ");
          Serial.print(value->value.int64Value);
          break;
      case SparkplugDataType::spUInt8:
          Serial.print("UInt8 Value: ");
          Serial.print(value->value.uint8Value);
          break;
      case SparkplugDataType::spUInt16:
          Serial.print("UInt16 Value: ");
          Serial.print(value->value.uint16Value);
          break;
      case SparkplugDataType::spUInt32:
          Serial.print("UInt32 Value: ");
          Serial.print(value->value.uint32Value);
          break;
      case SparkplugDataType::spUInt64:
          Serial.print("UInt64 Value: ");
          Serial.print(value->value.uint64Value);
          break;
      case SparkplugDataType::spFloat:
          Serial.print("Float Value: ");
          Serial.print(value->value.floatValue);
          break;
      case SparkplugDataType::spDouble:
          Serial.print("Double Value: ");
          Serial.print(value->value.doubleValue);
          break;
      case SparkplugDataType::spBoolean:
          Serial.print("Boolean Value: ");
          Serial.print(value->value.boolValue ? "true" : "false");
          break;
      case SparkplugDataType::spUUID:
      case SparkplugDataType::spText:
      case SparkplugDataType::spString:
          Serial.print("String Value: ");
          if (value->value.stringValue == NULL) {
            Serial.print("null");
            break;
          }
          Serial.print('"');
          Serial.print(value->value.stringValue);
          Serial.print('"');
          break;
      case SparkplugDataType::spBytes:
          Serial.print("Bytes Value: ");
          if (value->value.bytesValue == NULL) {
            Serial.print("value->value.bytesValue is NULL");
            break;
          }
          print_buffer(value->value.bytesValue);
          break;
      default:
        Serial.print("Unknown value!");
        break;

    }

    Serial.print(" (timestamp: ");
    Serial.print(value->timestamp);
    Serial.print(")");
}

void print_tag(FunctionalBasicTag* tag) {
    if (tag == NULL) {
        Serial.println("Tag is null");
        return;
    }

    Serial.print("Tag: ");
    Serial.println(tag->name);
    Serial.print("Alias: ");
    Serial.println(tag->alias);



    SparkplugDataType typecode = tag->datatype;
    int typecode_int = int(typecode);
    Serial.print("Datatype Code: ");
    Serial.println(typecode_int);

    Serial.print("Previous ");
    print_basic_value(&(tag->previousValue));
    Serial.println();
    Serial.print("Current  ");
    print_basic_value(&(tag->currentValue));
    Serial.println();

    Serial.println();
}

void readAndPrint(FunctionalBasicTag* tag) {
  // read the tag, with a dummy timestamp value
  readBasicTag(tag, uint64_t(1705379309000 + millis()));
  // print the tag
  print_tag(tag);
  Serial.println();
}
