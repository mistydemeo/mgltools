#include "csv_utf8.h"

int filesize(std::istream& ifs) {
  int old = ifs.tellg();
  ifs.seekg(0, std::ios_base::end);
  int sz = ifs.tellg();
  ifs.seekg(old);
  return sz;
}

int findDelimiter(BigChars& chars, int pos, std::string delimiters) {
/*  bool escaping = false;
  while (pos < chars.size()) {
    if (chars[pos] == (int)'"') escaping = !escaping;
    else if (!escaping) {
      // check for each delimiter
      for (int j = 0; j < delimiters.size(); j++) {
        if (chars[pos] == (int)(delimiters[j])) return pos;
      }
    }
    
    ++pos;
  }
  
  // reached EOF: assume end of row
  return pos; */
  
  while (pos < chars.size()) {
    // check for each delimiter
    for (int j = 0; j < delimiters.size(); j++) {
      if (chars[pos] == (int)(delimiters[j])) return pos;
    }
    
    ++pos;
  }
  
  // reached EOF: assume end of row
  return pos;
}

void bigCharsToString(BigChars& chars, int pos, int endpos, std::string& dst) {
  for (int i = pos; i < endpos; i++) {
    // input better be ascii!
    dst += (char)(chars[i]);
  }
}

void bigCharsToString(BigChars& chars, std::string& dst) {
  // input better be ascii!
  for (int i = 0; i < chars.size(); i++) {
    dst += (char)(chars[i]);
  }
}

int readCsvFieldToBigChars(BigChars& chars, int pos, BigChars& dst) {
  int endpos;
  bool isEscaped = false;
  // if cell is escaped, search for quotation mark delimiter
  if (chars[pos] == '"') {
    isEscaped = true;
    ++pos;
    endpos = findDelimiter(chars, pos, "\"");
  }
  // otherwise, search for comma or newline
  else {
    endpos = findDelimiter(chars, pos, ",\n");
  }
  
  for (int i = pos; i < endpos; i++) {
    // fuck fucking shit
    // why is libreoffice using double right quotes instead of real ones
    if (chars[i] == 0x201D) {
      dst.push_back('"');
    }
    else {
      dst.push_back(chars[i]);
    }
  }
  
  // if cell is escaped, skip end quote
  if (isEscaped) ++endpos;
  
  return endpos + 1;
}

int readCsvFieldToString(BigChars& chars, int pos, std::string& dst) {
  BigChars temp;
  int endpos = readCsvFieldToBigChars(chars, pos, temp);
  bigCharsToString(temp, dst);
  return endpos;
}

int readCsvRow(BigChars& chars, int pos, std::vector<BigChars>& dst) {
  // Read first row to determine dimensions
  int secondRowStart = pos;
  do {
    dst.push_back(BigChars());
    secondRowStart = readCsvFieldToBigChars(chars,
                                            secondRowStart,
                                            dst[dst.size() - 1]);
    
  } while (chars[secondRowStart - 1] != '\n');
  
  return secondRowStart;
}

void readCsvUtf8(std::istream& ifs,
                 std::vector< std::vector<BigChars> >& dst) {
  // Read input
  int sz = filesize(ifs);
  char* buffer = new char[sz];
  ifs.read(buffer, sz);
  std::string rawString(buffer, sz);
  delete buffer;
  
  // Convert the content to UTF-16
  BigChars csv16;
  utf8::utf8to16(rawString.begin(), rawString.end(), back_inserter(csv16));

  int pos = 0;
  do {
    dst.push_back(std::vector<BigChars>());
    pos = readCsvRow(csv16, pos, dst[dst.size() - 1]);
  } while (pos < csv16.size());

/*  std::vector<BigChars> firstRow;
  
  // Read first row to determine dimensions
  int secondRowStart = 0;
  do {
    firstRow.push_back(BigChars());
    secondRowStart = readCsvFieldToBigChars(csv16,
                                            secondRowStart,
                                            firstRow[firstRow.size() - 1]);
    
  } while (csv16[secondRowStart - 1] != '\n'); */
}
