//  ooooooooooooo ooooooo  ooooo ooooooooooooo  //
//  8'   888   `8  `8888    d8'  8'   888   `8  //
//       888         Y888..8P         888       //
//       888          `8888'          888       //
//       888         .8PY888.         888       //
//       888        d8'  `888b        888       //
//      o888o     o888o  o88888o     o888o      //

#include <globals.h>
#if !OTA_APP  // POCKETMAGE_OS
enum TXTState { TXT_, WIZ0, WIZ1, WIZ2, WIZ3, FONT };
TXTState CurrentTXTState = TXT_;

#pragma region Font includes
// Mono
#include <Fonts/FreeMono9pt8b.h>
#include <Fonts/FreeMonoBold12pt8b.h>
#include <Fonts/FreeMonoBold18pt8b.h>
#include <Fonts/FreeMonoBold24pt8b.h>
#include <Fonts/FreeMonoBold9pt8b.h>
#include <Fonts/FreeMonoBoldOblique12pt8b.h>
#include <Fonts/FreeMonoBoldOblique18pt8b.h>
#include <Fonts/FreeMonoBoldOblique24pt8b.h>
#include <Fonts/FreeMonoBoldOblique9pt8b.h>
#include <Fonts/FreeMonoOblique9pt8b.h>

void TXT_INIT_OLD() {
  if (PM_SDAUTO().getEditingFile() != "")
    PM_SDAUTO().loadFile();
  CurrentAppState = TXT;
  CurrentTXTState = TXT_;
  KB().setKeyboardState(NORMAL);
  TOUCH().setDynamicScroll(0);
  newLineAdded = true;
}

void processKB_TXT_OLD() {
  OLED().setPowerSave(false);

  disableTimeout = false;

int drawLineEink(Document& doc, ulong lineIndex, int startX, int startY, bool isSelected) {
  // Extract the line using the passed document and index
  Line& line = doc.lines[lineIndex]; 
  
  char style = line.type;
  int cursorY = startY;

  // Determine colors based on selection
  uint16_t fgColor;
  uint16_t bgColor;

  if (style == 'B') {
    fgColor = GxEPD_BLACK;
    bgColor = GxEPD_WHITE;
  }
  else {
    fgColor = isSelected ? GxEPD_WHITE : GxEPD_BLACK;
    bgColor = isSelected ? GxEPD_BLACK : GxEPD_WHITE;
  }


  // Calculate height first so we can draw the background
  int hpx = getCalculatedLineHeight(line);

  // Draw highlight background if selected
  if (isSelected) {
    display.fillRect(0, startY, display.width(), hpx, bgColor);
  }

  // Set text color to contrast with the background
  display.setTextColor(fgColor);

  // ---------- Non-Text Rendered Items ---------- //
  // Horizontal Rules just print a line
  if (style == 'H') {
    display.drawFastHLine(0, cursorY + 3, display.width(), fgColor);
    display.drawFastHLine(0, cursorY + 4, display.width(), fgColor);
    return hpx;
  }
  // Blank lines just take up space
  else if (style == 'B') {
    return 8;
  }

  // ---------- Add Padding If Needed ---------- //
  // Lists and Blockquotes are padded on the left
  if (style == '>')
    startX += SPECIAL_PADDING;
  else if (style == 'U' || style == 'O' || style == 'u' || style == 'o')
    startX += 2 * SPECIAL_PADDING;
  // Code blocks are padded on both sides
  else if (style == 'C')
    startX += (SPECIAL_PADDING / 2);

  // ---------- Render Text ---------- //
  bool bold = false;
  bool italic = false;
  int xpos = startX; 
  char temp[2] = {0, '\0'};

  int baselineY = cursorY + getLineMaxHeight(line); 

  for (uint16_t i = 0; i < line.len; i++) {
    if (xpos > display.width()) continue;
    
    char c = line.text[i];

    if (c == '*') {
      if (i == 0 || line.text[i - 1] != '*') {
        uint8_t starCount = 0;
        while (i + starCount < line.len && line.text[i + starCount] == '*' && starCount < 3)
          starCount++;

        // toggle style based on number of stars
        switch (starCount) {
          case 1: italic = !italic; break;
          case 2: bold = !bold; break;
          case 3: bold = !bold; italic = !italic; break;
        }
      }
      continue;
    }

    // Set the font
    const GFXfont* font = pickFont(style, bold, italic);
    display.setFont(font);

    // Draw text & add width
    temp[0] = c;
    display.setCursor(xpos, baselineY); 
    display.print(temp);

    // Add character space
    int16_t x1, y1;
    uint16_t charW, charH;
    if (c == ' ')
      display.getTextBounds(SPACEWIDTH_SYMBOL, 0, 0, &x1, &y1, &charW, &charH);
    else
      display.getTextBounds(temp, 0, 0, &x1, &y1, &charW, &charH);
    xpos += charW;
  }

  // ---------- Post-Render Formatting ---------- //
  // Blockquotes get a vertical line on the left
  if (style == '>') {
    display.drawFastVLine(SPECIAL_PADDING / 2, startY, hpx, fgColor);
    display.drawFastVLine((SPECIAL_PADDING / 2) + 1, startY, hpx, fgColor);
  }
  // Code Blocks get a vertical line on each side
  else if (style == 'C') {
    display.drawFastVLine(SPECIAL_PADDING / 4, startY, hpx, fgColor);
    display.drawFastVLine(display.width() - (SPECIAL_PADDING / 4), startY, hpx, fgColor);
    display.drawFastVLine((SPECIAL_PADDING / 4) + 1, startY, hpx, fgColor);
    display.drawFastVLine(display.width() - (SPECIAL_PADDING / 4) - 1, startY, hpx, fgColor);
  }
  // Headings get a horizontal line below them ONLY on the last line of the header block
  else if (style == '1' || style == '2' || style == '3') {
    bool isLastLineOfHeader = true;
    if (lineIndex < doc.lineCount - 1) {
       // Peek ahead. If the next line is the exact same heading type, don't draw underline yet
       if (doc.lines[lineIndex + 1].type == style) {
           isLastLineOfHeader = false; 
       }
    }
    
    if (isLastLineOfHeader) {
      display.drawFastHLine(0, startY + hpx - 2, display.width(), fgColor);
      display.drawFastHLine(0, startY + hpx - 3, display.width(), fgColor);
    }
  }
  // Unordered Lists get a '●' (Only uppercase 'U')
  else if (style == 'U') {
    display.fillCircle(startX - 8, startY + (hpx / 2), 3, fgColor);
  }
  // Ordered Lists get their dynamic number (Only uppercase 'O')
  else if (style == 'O') {
    int listNum = 1;
    // Scan up the array to find what number this item should be
    if (lineIndex > 0) {
      for (long k = lineIndex - 1; k >= 0; k--) {
        if (doc.lines[k].type == 'O') listNum++;
        else if (doc.lines[k].type != 'o') break; // Hit a different type, chain broken
      }
    }

    String numStr = String(listNum) + ".";
    display.setFont(pickFont('O', false, false)); // Standard list font
    
    // Measure width to properly right-align it against the padding
    int16_t nx, ny;
    uint16_t nw, nh;
    display.getTextBounds(numStr, 0, 0, &nx, &ny, &nw, &nh);
    
    display.setCursor(startX - nw - 8, baselineY);
    display.print(numStr);
  }

  return hpx; 
}

void insertLineArray(ulong index) {
  if (document.lineCount >= MAX_LINES) return; // Prevent overflow
  
  // Shift everything below the index down by 1
  if (index < document.lineCount) {
    memmove(&document.lines[index + 1], &document.lines[index], sizeof(Line) * (document.lineCount - index));
  }
  
  // Clear the new line
  initLine(document.lines[index]);
  document.lineCount++;
}

void deleteLineArray(ulong index) {
  if (index >= document.lineCount) return;
  
  // Shift everything below the index up by 1
  if (index < document.lineCount - 1) {
    memmove(&document.lines[index], &document.lines[index + 1], sizeof(Line) * (document.lineCount - index - 1));
  }
  
  document.lineCount--;
}

int findWrapIndex(const String& content, char style) {
  int availableWidth = display.width() - DISPLAY_WIDTH_BUFFER;

  // Subtract padding based on style
  if (style == '>') availableWidth -= SPECIAL_PADDING;
  else if (style == 'U' || style == 'O' || style == 'u' || style == 'o') availableWidth -= 2 * SPECIAL_PADDING;
  else if (style == 'C') availableWidth -= (SPECIAL_PADDING / 2);

  bool bold = false;
  bool italic = false;
  int currentWidth = 0;
  int lastSpaceIndex = -1;
  char temp[2] = {0, '\0'};

  // Do not exceed LINE_CAP or the string length
  int maxLen = min((int)content.length(), (int)LINE_CAP);

  for (int i = 0; i < maxLen; i++) {
    char c = content[i];

    if (c == ' ') {
      lastSpaceIndex = i;
    }

    if (c == '*') {
      if (i == 0 || content[i - 1] != '*') {
        uint8_t starCount = 0;
        while (i + starCount < maxLen && content[i + starCount] == '*' && starCount < 3) {
          starCount++;
        }
        switch (starCount) {
          case 1: italic = !italic; break;
          case 2: bold = !bold; break;
          case 3: bold = !bold; italic = !italic; break;
        }
      }
      // Stars don't add to rendered width, skip measurement
      continue; 
    }

    const GFXfont* font = pickFont(style, bold, italic);
    display.setFont(font);

    temp[0] = c;
    int16_t x1, y1;
    uint16_t charW, charH;
    
    if (c == ' ') {
      display.getTextBounds(SPACEWIDTH_SYMBOL, 0, 0, &x1, &y1, &charW, &charH);
    } else {
      display.getTextBounds(temp, 0, 0, &x1, &y1, &charW, &charH);
    }

    if (currentWidth + charW > availableWidth) {
      // Word wrap: break at the last space if possible
      if (lastSpaceIndex > 0) {
        return lastSpaceIndex;
      }
      // Hard break if a single word is longer than the line
      return i > 0 ? i : 1; 
    }

    currentWidth += charW;
  }

  // If the loop finished because it hit LINE_CAP, but the string has more characters
  // attempt to break at a space within the 64 character limit.
  if (maxLen < content.length() && lastSpaceIndex > 0) {
    return lastSpaceIndex;
  }

  return maxLen; // Fits completely within limits
}

void reflowParagraph(ulong startLine, uint16_t& activeCursor) {
  // 1. Identify paragraph boundaries and styles
  char baseStyle = document.lines[startLine].type;
  char contStyle = baseStyle; // Inherit the style directly
  
  // Apply lowercase rule strictly for lists so we don't draw double-bullets
  if (baseStyle == 'U' || baseStyle == 'u') contStyle = 'u';
  else if (baseStyle == 'O' || baseStyle == 'o') contStyle = 'o';

  // 2. Extract all text in this paragraph block
  String fullText = String(document.lines[startLine].text);
  ulong endLine = startLine + 1;
  
  while (endLine < document.lineCount && document.lines[endLine].type == contStyle) {
    if (document.lines[endLine].len > 0) {
      fullText += " " + String(document.lines[endLine].text);
    }
    endLine++;
  }
  
  // 3. Keep track of cursor globally relative to the extracted string
  int absoluteCursor = activeCursor; 

  // 4. Repack the lines
  ulong currWriteIdx = startLine;
  String remainingText = fullText;
  
  while (remainingText.length() > 0) {
    char currentStyle = (currWriteIdx == startLine) ? baseStyle : contStyle;
    int wrapIdx = findWrapIndex(remainingText, currentStyle);
    
    String chunk = remainingText.substring(0, wrapIdx);
    remainingText = remainingText.substring(wrapIdx);
    remainingText.trim(); // Remove leading spaces for the next line
    
    // If we ran out of allocated lines for this paragraph, insert a new one
    if (currWriteIdx >= endLine) {
      insertLineArray(currWriteIdx);
      endLine++;
    }
    
    Line& writeLine = document.lines[currWriteIdx];
    writeLine.type = currentStyle;
    strncpy(writeLine.text, chunk.c_str(), LINE_CAP);
    writeLine.text[LINE_CAP] = '\0';
    writeLine.len = strlen(writeLine.text);
    
    // Track cursor location as it moves between lines
    if (absoluteCursor != -1) {
      if (absoluteCursor <= writeLine.len) {
        currentLineNum = currWriteIdx;
        activeCursor = absoluteCursor;
        absoluteCursor = -1; // Found it, stop tracking
      } else {
        absoluteCursor -= writeLine.len;
        if (remainingText.length() > 0) absoluteCursor--; // Account for removed space
      }
    }
    
    currWriteIdx++;
  }
  
  // --- NEW FAILSAFE BLOCK ---
  // Prevent the cleanup loop from deleting a completely empty line
  if (currWriteIdx == startLine) {
    Line& writeLine = document.lines[currWriteIdx];
    writeLine.type = baseStyle;
    writeLine.text[0] = '\0';
    writeLine.len = 0;
    
    if (absoluteCursor != -1) {
      currentLineNum = currWriteIdx;
      activeCursor = 0;
    }
    currWriteIdx++; // Increment so the cleanup block safely bypasses it
  }
  
  // 5. Cleanup leftover lines if the paragraph shrank due to backspacing
  while (currWriteIdx < endLine) {
    deleteLineArray(currWriteIdx);
    endLine--;
  }
}

void mergeLinesUp(ulong currLine, uint16_t& cursor) {
  if (currLine == 0) return;
  ulong prevLine = currLine - 1;
  char pType = document.lines[prevLine].type;
  
  // If the line above is a blank line or horizontal rule, just delete it
  if (pType == 'B' || pType == 'H') {
    deleteLineArray(prevLine);
    currentLineNum--;
    updateScreen = true;
    return;
  }
  
  // Determine continuation styles for repack
  char contStyle = pType; // Inherit the style directly
  if (pType == 'U' || pType == 'u') contStyle = 'u';
  else if (pType == 'O' || pType == 'o') contStyle = 'o';

  // Combine text EXACTLY as-is (no space added, simulating natural backspace)
  String merged = String(document.lines[prevLine].text) + String(document.lines[currLine].text);
  cursor = document.lines[prevLine].len; // Cursor sits right where the join happened
  
  // Collect the rest of the current line's paragraph to ensure we don't drop text
  char currBase = document.lines[currLine].type;
  char currContStyle = currBase;
  if (currBase == 'U' || currBase == 'u') currContStyle = 'u';
  else if (currBase == 'O' || currBase == 'o') currContStyle = 'o';

  ulong endLine = currLine + 1;
  while (endLine < document.lineCount && document.lines[endLine].type == currContStyle) {
    if (document.lines[endLine].len > 0) {
      merged += " " + String(document.lines[endLine].text);
    }
    endLine++;
  }
  
  // Delete the old disconnected lines
  int linesToDelete = endLine - currLine;
  for (int i = 0; i < linesToDelete; i++) {
    deleteLineArray(currLine);
  }
  
  // Repack all text starting from the previous line
  ulong currWriteIdx = prevLine;
  String remainingText = merged;
  int absoluteCursor = cursor;
  
  while (remainingText.length() > 0) {
    char currentStyle = (currWriteIdx == prevLine) ? pType : contStyle;
    int wrapIdx = findWrapIndex(remainingText, currentStyle);
    
    String chunk = remainingText.substring(0, wrapIdx);
    remainingText = remainingText.substring(wrapIdx);
    remainingText.trim();
    
    if (currWriteIdx >= document.lineCount || currWriteIdx > prevLine) {
      insertLineArray(currWriteIdx);
    }
    
    Line& writeLine = document.lines[currWriteIdx];
    writeLine.type = currentStyle;
    strncpy(writeLine.text, chunk.c_str(), LINE_CAP);
    writeLine.text[LINE_CAP] = '\0';
    writeLine.len = strlen(writeLine.text);
    
    // Update cursor position globally
    if (absoluteCursor != -1) {
      if (absoluteCursor <= writeLine.len) {
        currentLineNum = currWriteIdx;
        cursor = absoluteCursor;
        absoluteCursor = -1;
      } else {
        absoluteCursor -= writeLine.len;
        if (remainingText.length() > 0) absoluteCursor--;
      }
    }
    currWriteIdx++;
  }
  
  // Failsafe for completely deleting all text leaving an empty line
  if (currWriteIdx == prevLine) {
    Line& writeLine = document.lines[currWriteIdx];
    writeLine.type = pType;
    writeLine.text[0] = '\0';
    writeLine.len = 0;
    currentLineNum = currWriteIdx;
    cursor = 0;
  }
  
  updateScreen = true;
}

inline void insertChar(Line& line, uint16_t& cursor, char c) {
  // Ignore if line is full
  if (line.len >= LINE_CAP)
    return;

  // Only shift characters if inserting in the middle
  if (cursor < line.len) {
    // Move the tail, including null terminator
    memmove(&line.text[cursor + 1], &line.text[cursor], line.len - cursor + 1);
  } else {
    // Appending at end: just add character and null terminate
    line.text[line.len] = c;
    line.len++;
    line.text[line.len] = '\0';
    cursor++;
    return;
  }

  // Insert character in middle
  line.text[cursor] = c;
  line.len++;
  cursor++;
}

inline void deleteChar(Line& line, uint16_t& cursor) {
  if (cursor == 0)
    return;

  memmove(&line.text[cursor - 1], &line.text[cursor], line.len - cursor + 1);

  line.len--;
  cursor--;
}

void cycleTextStyle(Line& line, uint16_t& cursor) {
  if (line.len == 0)
    return;

  int leftBound = -1;
  int rightBound = -1;
  uint8_t currentStars = 0;
  uint8_t leftStars = 0;
  uint8_t rightStars = 0;

  // --- 1. Find formatting block containing the cursor ---
  int i = 0;
  int activeStart = -1;
  uint8_t activeStars = 0;

  while (i < line.len) {
    if (line.text[i] == '*') {
      int startIdx = i;
      uint8_t stars = 0;
      while (i < line.len && line.text[i] == '*' && stars < 3) {
        stars++;
        i++;
      }

      if (activeStars == 0) {
        // Potential start of a formatting block
        activeStart = startIdx;
        activeStars = stars;
      } else if (activeStars == stars) {
        // Matching closing stars found
        if (cursor >= activeStart && cursor <= i) {
          // The cursor is inside this block
          leftBound = activeStart;
          rightBound = i;
          currentStars = activeStars;
          leftStars = activeStars;
          rightStars = activeStars;
          break;
        }
        // Cursor is not in this block; reset for the next pair
        activeStars = 0;
        activeStart = -1;
      } else {
        // Mismatched stars; treat the current stars as a new opening
        activeStart = startIdx;
        activeStars = stars;
      }
    } else {
      i++;
    }
  }

  // --- 2. Fallback to current word boundaries if no block was found ---
  if (leftBound == -1) {
    leftBound = cursor;
    while (leftBound > 0 && line.text[leftBound - 1] != ' ') {
      leftBound--;
    }

    rightBound = cursor;
    while (rightBound < line.len && line.text[rightBound] != ' ') {
      rightBound++;
    }

    currentStars = 0;
    leftStars = 0;
    rightStars = 0;
  }

  // --- 3. Determine next formatting ---
  uint8_t nextStars = (currentStars + 1) % 4;  // 0->1->2->3->0

  // Prevent buffer overflow
  int sizeDiff = (nextStars * 2) - (leftStars + rightStars);
  if (line.len + sizeDiff > LINE_CAP)
    return;

  // --- 4. Calculate cursor relative position inside formatting ---
  int contentStart = leftBound + leftStars;
  int contentEnd = rightBound - rightStars;

  int cursorOffset = cursor - contentStart;
  if (cursorOffset < 0)
    cursorOffset = 0;  // Cursor was on left stars
  if (cursorOffset > (contentEnd - contentStart))
    cursorOffset = contentEnd - contentStart;  // Cursor was on right stars

  // --- 5. Remove current formatting ---
  if (rightStars > 0) {
    memmove(&line.text[rightBound - rightStars], &line.text[rightBound],
            line.len - rightBound + 1);  // include null terminator
    line.len -= rightStars;
    rightBound -= rightStars;
  }

  if (leftStars > 0) {
    memmove(&line.text[leftBound], &line.text[leftBound + leftStars],
            line.len - leftBound - leftStars + 1);
    line.len -= leftStars;
    rightBound -= leftStars;
  }

  // --- 6. Insert new formatting ---
  if (nextStars > 0) {
    // Insert right stars
    memmove(&line.text[rightBound + nextStars], &line.text[rightBound], line.len - rightBound + 1);
    for (int j = 0; j < nextStars; j++)
      line.text[rightBound + j] = '*';
    line.len += nextStars;

    // Insert left stars
    memmove(&line.text[leftBound + nextStars], &line.text[leftBound], line.len - leftBound + 1);
    for (int j = 0; j < nextStars; j++)
      line.text[leftBound + j] = '*';
    line.len += nextStars;
  }

  // --- 7. Restore cursor position ---
  cursor = leftBound + nextStars + cursorOffset;
}

void cycleParagraphStyle(ulong& currLine, uint16_t& cursor) {
  char activeType = document.lines[currLine].type;
  ulong startLine = currLine;
  char oldContStyle = activeType;

  // 1. Find the top of the paragraph
  if (activeType == 'u' || activeType == 'U') {
    oldContStyle = 'u';
    while (startLine > 0 && document.lines[startLine].type == 'u') startLine--;
  } else if (activeType == 'o' || activeType == 'O') {
    oldContStyle = 'o';
    while (startLine > 0 && document.lines[startLine].type == 'o') startLine--;
  } else {
    oldContStyle = activeType;
    while (startLine > 0 && document.lines[startLine - 1].type == activeType) {
      startLine--;
    }
  }

  // 2. Cycle to the new base style
  static const char styleCycle[] = {'T', '1', '2', '3', '>', 'O', 'U', 'C', 'H'};
  static const int numStyles = sizeof(styleCycle) / sizeof(styleCycle[0]);
  
  char oldBaseStyle = document.lines[startLine].type;
  int currentIndex = 0;
  for (int i = 0; i < numStyles; i++) {
    if (oldBaseStyle == styleCycle[i]) {
      currentIndex = i;
      break;
    }
  }
  char newBaseStyle = styleCycle[(currentIndex + 1) % numStyles];
  
  // Apply specific lowercase rules for lists
  char newContStyle = newBaseStyle;
  if (newBaseStyle == 'U') newContStyle = 'u';
  else if (newBaseStyle == 'O') newContStyle = 'o';

  // 3. Extract text and track the absolute cursor
  int absoluteCursor = 0;
  String fullText = "";
  ulong endLine = startLine;
  
  while (endLine < document.lineCount) {
    if (endLine > startLine && document.lines[endLine].type != oldContStyle) break;
    
    // Log cursor position globally relative to the entire paragraph string
    if (endLine == currLine) {
      absoluteCursor = fullText.length() + (fullText.length() > 0 ? 1 : 0) + cursor;
    }
    
    if (document.lines[endLine].len > 0) {
      if (fullText.length() > 0) fullText += " ";
      fullText += String(document.lines[endLine].text);
    }
    endLine++;
  }

  // 4. Repack text with the newly selected style
  ulong currWriteIdx = startLine;
  String remainingText = fullText;
  
  while (remainingText.length() > 0) {
    char currentStyle = (currWriteIdx == startLine) ? newBaseStyle : newContStyle;
    int wrapIdx = findWrapIndex(remainingText, currentStyle);
    
    String chunk = remainingText.substring(0, wrapIdx);
    remainingText = remainingText.substring(wrapIdx);
    remainingText.trim(); // remove leading spaces for next line
    
    if (currWriteIdx >= endLine) {
      insertLineArray(currWriteIdx);
      endLine++;
    }
    
    Line& writeLine = document.lines[currWriteIdx];
    writeLine.type = currentStyle;
    strncpy(writeLine.text, chunk.c_str(), LINE_CAP);
    writeLine.text[LINE_CAP] = '\0';
    writeLine.len = strlen(writeLine.text);
    
    // Check if the cursor lands in this chunk and restore it globally
    if (absoluteCursor != -1) {
      if (absoluteCursor <= writeLine.len) {
        currLine = currWriteIdx;
        cursor = absoluteCursor;
        absoluteCursor = -1;
      } else {
        absoluteCursor -= writeLine.len;
        if (remainingText.length() > 0) absoluteCursor--;
      }
    }
    
    currWriteIdx++;
  }
  
  // Failsafe for cycling styles on an empty line
  if (currWriteIdx == startLine) {
    Line& writeLine = document.lines[currWriteIdx];
    writeLine.type = newBaseStyle;
    writeLine.text[0] = '\0';
    writeLine.len = 0;
    currLine = startLine;
    cursor = 0;
    currWriteIdx++;
  }

  // 5. Cleanup leftover lines if the new font made the paragraph shorter
  while (currWriteIdx < endLine) {
    deleteLineArray(currWriteIdx);
    endLine--;
  }
  
  //updateScreen = true;
}

void setFontOLED(bool bold, bool italic) {
  if (bold && italic)
    u8g2.setFont(u8g2_font_luBIS18_tf);  // bold italics
  else if (bold && !italic)
    u8g2.setFont(u8g2_font_luBS18_tf);  // bold
  else if (!bold && italic)
    u8g2.setFont(u8g2_font_luIS18_tf);  // italics
  else
    u8g2.setFont(u8g2_font_lubR18_tf);  // regular
  return;
}

int getLineWidthOLED(Line& line) {
  bool bold = false;
  bool italic = false;
  int width = 0;  // total width in pixels
  char temp[2] = {0, '\0'};

  uint16_t i = 0;
  while (i < line.len) {
    char c = line.text[i];

    if (c == '*') {
      // count consecutive stars if there isn't a star before
      if (i == 0 || line.text[i - 1] != '*') {
        uint8_t starCount = 0;
        while (i + starCount < line.len && line.text[i + starCount] == '*' && starCount < 3)
          starCount++;

        // toggle style based on number of stars
        switch (starCount) {
          case 1:
            italic = !italic;
            break;
          case 2:
            bold = !bold;
            break;
          case 3:
            bold = !bold;
            italic = !italic;
            break;
        }
      }

      //setFontOLED(false, false);
      //  Use tiny font for stars
      u8g2.setFont(u8g2_font_5x7_tf);
    } else
      setFontOLED(bold, italic);

    // Count width
    temp[0] = c;
    width += u8g2.getStrWidth(temp);

    // italics need to overlap a bit
    if (italic && c != '*') width -= 3;

    i++;
  }

  return width;
}

void toolBar(Line& line, bool bold, bool italic) {
  // FN/SHIFT indicator centered
  u8g2.setFont(u8g2_font_5x7_tf);

  switch (KB().getKeyboardState()) {
    case 1:
      u8g2.drawStr((u8g2.getDisplayWidth() - u8g2.getStrWidth("SHIFT")) / 2,
                   u8g2.getDisplayHeight(), "SHIFT");
      break;
    case 2:
      u8g2.drawStr((u8g2.getDisplayWidth() - u8g2.getStrWidth("FN")) / 2, u8g2.getDisplayHeight(),
                   "FN");
      break;
    case 3:
      u8g2.drawStr((u8g2.getDisplayWidth() - u8g2.getStrWidth("FN+SHIFT")) / 2,
                   u8g2.getDisplayHeight(), "FN+SHIFT");
    default:
      break;
  }

  // Show line type
  char currentDocLineType = line.type;
  String lineTypeLabel;

  switch (currentDocLineType) {
    case ' ':
      lineTypeLabel = "ERR";
      break;
    case 'T':
      lineTypeLabel = "BODY";
      break;
    case '1':
      lineTypeLabel = "H1";
      break;
    case '2':
      lineTypeLabel = "H2";
      break;
    case '3':
      lineTypeLabel = "H3";
      break;
    case 'C':
      lineTypeLabel = "CODE";
      break;
    case '>':
      lineTypeLabel = "QUOTE";
      break;
    case 'U':
    case 'u':
      lineTypeLabel = "U LIST";
      break;
    case 'O':
    case 'o':
      lineTypeLabel = "O LIST";
      break;
    case 'H':
      lineTypeLabel = "H RULE";
      break;
    case 'B':
      lineTypeLabel = "BLANK LINE";
      break;
    default:
      lineTypeLabel = "";
      break;  // fallback if none match
  }

  if (lineTypeLabel.length() > 0) {
    u8g2.drawStr(0, u8g2.getDisplayHeight(), lineTypeLabel.c_str());
  }

  // Bold and italic indicator
  if (bold == true && italic == true) {
    u8g2.drawStr(u8g2.getDisplayWidth() - u8g2.getStrWidth("BOLD+ITALIC"), u8g2.getDisplayHeight(),
                 "BOLD+ITALIC");
  } else if (bold == true && italic == false) {
    u8g2.drawStr(u8g2.getDisplayWidth() - u8g2.getStrWidth("BOLD"), u8g2.getDisplayHeight(),
                 "BOLD");
  } else if (bold == false && italic == true) {
    u8g2.drawStr(u8g2.getDisplayWidth() - u8g2.getStrWidth("ITALIC"), u8g2.getDisplayHeight(),
                 "ITALIC");
  } else {
    u8g2.drawStr(u8g2.getDisplayWidth() - u8g2.getStrWidth("NORMAL"), u8g2.getDisplayHeight(),
                 "NORMAL");
  }
}

void displayScrollPreviewOLED(Document& doc, ulong activeCursorLine) {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  int startX = 0; 
  int cursorY = 0;
  int specialPadding = 8; // px
  
  // 1. Dynamic Viewport: Keep active cursor near the top/middle of the OLED
  ulong displayTop = 0;
  if (activeCursorLine > 3) {
    displayTop = activeCursorLine - 3;
  }

  // Draw Vertical Separator at x=76
  u8g2.drawVLine(76, 0, u8g2.getDisplayHeight());

  // 2. Draw Minimap on the left (0 to 74)
  for (ulong i = displayTop; i < doc.lineCount; i++) {
    if (cursorY > u8g2.getDisplayHeight()) break; // Off screen

    Line& line = doc.lines[i];
    char style = line.type;
    int padX = startX;

    // Find height and padding for this line
    int max_hpx = 2;
    switch (style) {
      case '1': max_hpx = 5; break;
      case '2': max_hpx = 4; break;
      case '3': max_hpx = 3; break;
      case 'T': max_hpx = 2; break;
      case 'C': max_hpx = 2; padX += (specialPadding / 2); break;
      case '>': max_hpx = 2; padX += specialPadding; break;
      case 'U': 
      case 'u': 
      case 'O': 
      case 'o': 
        max_hpx = 2; 
        padX += specialPadding; 
        break;
      case 'H': 
        // Horizontal Rules just print a line
        if (cursorY > 0) {
          u8g2.drawHLine(startX, cursorY, 74);
          cursorY += 3;
        }
        continue;
      case 'B': 
        // Blank Lines just take up space
        cursorY += 4; 
        continue; 
      default: 
        max_hpx = 2; 
        break;
    }

    // Fast Width Approximation bounded to the left pane
    uint16_t boxWidth = map(line.len, 0, LINE_CAP, 0, 72 - padX);
    if (boxWidth == 0 && line.len > 0) boxWidth = 2; // Minimum visible blip

    // Draw Line Box
    u8g2.drawBox(padX + 2, cursorY, boxWidth, max_hpx);

    // Draw Style Decorations
    if (style == '>') {
      u8g2.drawVLine(startX + (specialPadding / 2), cursorY, max_hpx+1);
    } 
    else if (style == 'C') {
      u8g2.drawVLine(startX + (specialPadding / 4) - 1, cursorY, max_hpx+1);
      u8g2.drawVLine(padX + 2 + boxWidth + 2, cursorY, max_hpx+1);
    } 
    else if (style == '1' || style == '2' || style == '3') {
      bool isLast = true;
      if (i < doc.lineCount - 1 && doc.lines[i+1].type == style) {
         isLast = false; 
      }
      if (isLast && (cursorY + max_hpx + 1) < u8g2.getDisplayHeight()) {
        u8g2.drawHLine(startX, cursorY + max_hpx + 1, 74);
      }
    } 
    else if (style == 'U') {
      u8g2.drawHLine(startX + specialPadding - 3, cursorY + (max_hpx / 2), 2);
    } 
    else if (style == 'O') {
      u8g2.drawVLine(startX + specialPadding - 3, cursorY, 2);
      u8g2.drawPixel(startX + specialPadding - 1, cursorY + 1);
    }

    // Draw Selection Indicator tracking the active line
    if (i == activeCursorLine) {
      u8g2.drawFrame(padX, cursorY - 1, boxWidth + 4, max_hpx + 2);
      // Draw left-pointing triangle on the separator wall
      u8g2.drawTriangle(74, cursorY-3, 74, cursorY + 3, 70, cursorY); 
    }

    // Move down for next line
    cursorY += max_hpx + 2; 
    if (style == '1' || style == '2' || style == '3') cursorY += 2; // Extra padding
  }

  // 3. Draw Text Info on the Right Pane (78 to 128)
  Line& activeLine = doc.lines[activeCursorLine];
  String typeLabel = "";
  
  switch (activeLine.type) {
    case '1': typeLabel = "H1"; break;
    case '2': typeLabel = "H2"; break;
    case '3': typeLabel = "H3"; break;
    case '>': typeLabel = "QUOTE"; break;
    case 'C': typeLabel = "CODE"; break;
    case 'U': 
    case 'u': typeLabel = "U LIST"; break;
    case 'O': 
    case 'o': typeLabel = "O LIST"; break;
    case 'H': typeLabel = "H RULE"; break;
    case 'B': typeLabel = "BLANK"; break;
    case ' ': typeLabel = "ERR"; break;
    default:  typeLabel = "BODY"; break;
  }

  u8g2.setFont(u8g2_font_5x7_tf);
  
  // Print Line Number
  String lineStr = "L: " + String(activeCursorLine);
  u8g2.drawStr(80, 7, lineStr.c_str());
  
  // Print Line Type
  u8g2.drawStr(u8g2.getDisplayWidth() - u8g2.getStrWidth(typeLabel.c_str()), 7, typeLabel.c_str());
  
  // Draw a horizontal line under the header info
  u8g2.drawHLine(78, 10, u8g2.getDisplayWidth()-78);

  // Print Full Text Preview (Single line, extending offscreen)
  u8g2.setFont(u8g2_font_lubR18_tf);

  if (activeLine.len > 0) {
    int prevCursorX = 80;
    int rightEdge = u8g2.getDisplayWidth(); 
    char tChar[2] = {0, '\0'};
    
    for (int j = 0; j < activeLine.len; j++) {
      char c = activeLine.text[j];
      
      // Skip formatting stars so they don't clog up the preview
      if (c == '*') continue; 
      
      tChar[0] = c;
      int charWidth = u8g2.getStrWidth(tChar);
      
      // If adding this char goes past the screen, stop drawing to save cycles
      if (prevCursorX > rightEdge) {
        break; 
      }
      
      // Draw character
      u8g2.drawStr(prevCursorX, u8g2.getDisplayHeight(), tChar);
      prevCursorX += charWidth;
    }
  }

  u8g2.sendBuffer();
}

#pragma region Mrkdn File Ops
void saveMarkdownFile(const String& path) {
  if (PM_SDAUTO().getNoSD()) {
    OLED().oledWord("SAVE FAILED - No SD!");
    delay(3000);
    return;
  }

  SDActive = true;
  pocketmage::setCpuSpeed(240); // Boost clock for SD operation
  delay(50);

  // Determine save path
  String savePath = path;
  if (savePath == "" || savePath == "-")
    savePath = "/temp.txt";
  if (!savePath.startsWith("/"))
    savePath = "/" + savePath;

  File file = global_fs->open(savePath.c_str(), FILE_WRITE);
  if (!file) {
    OLED().oledWord("SAVE FAILED - OPEN ERR");
    delay(2000);
    ESP_LOGE("SD", "Failed to open file for writing: %s", savePath.c_str());
    SDActive = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(80);
    return;
  }

  for (ulong i = 0; i < document.lineCount; i++) {
    Line& line = document.lines[i];
    
    // Determine if this line is a word-wrap continuation of the previous line
    bool isContinuation = false;
    if (i > 0) {
      char pType = document.lines[i-1].type;
      if (line.type == 'T' && pType == 'T') isContinuation = true;
      else if (line.type == 'u' && (pType == 'U' || pType == 'u')) isContinuation = true;
      else if (line.type == 'o' && (pType == 'O' || pType == 'o')) isContinuation = true;
      else if (line.type == '1' && pType == '1') isContinuation = true;
      else if (line.type == '2' && pType == '2') isContinuation = true;
      else if (line.type == '3' && pType == '3') isContinuation = true;
      else if (line.type == '>' && pType == '>') isContinuation = true;
      else if (line.type == 'C' && pType == 'C') isContinuation = true;
    }

    // If it is NOT a continuation, we must close the previous line and start a new Markdown block
    if (!isContinuation) {
      // Close the previous line with a newline (and closing backtick if it was code)
      if (i > 0) {
        if (document.lines[i-1].type == 'C') file.print("`");
        file.println();
      }

      // Print the Markdown prefix for the new line
      switch (line.type) {
        case '1': file.print("# "); break;
        case '2': file.print("## "); break;
        case '3': file.print("### "); break;
        case '>': file.print("> "); break;
        case 'U': file.print("- "); break;
        case 'O': file.print("1. "); break;
        case 'C': file.print("`"); break;
        case 'H': file.print("---"); break;
        default: break; // 'T', 'B', 'u', 'o' have no prefix
      }
    } else {
      // It is a continuation line. 
      // Re-inject a space to re-join the wrapped words correctly.
      file.print(" ");
    }

    // Print the actual text
    if (line.len > 0) {
      file.print(line.text);
    }
  }

  // Close the very last line in the document
  if (document.lineCount > 0) {
    if (document.lines[document.lineCount - 1].type == 'C') file.print("`");
    file.println();
  }

  file.close();

  // Save metadata
  PM_SDAUTO().writeMetadata(savePath);
  PM_SDAUTO().setEditingFile(savePath);

  OLED().oledWord("Saved: " + savePath);
  delay(1000);

  if (SAVE_POWER) pocketmage::setCpuSpeed(80); // Return to power save
  SDActive = false;
}

bool loadMarkdownFile(const String& path) {
  pocketmage::setCpuSpeed(240);

  // Invalid file
  if (path == "" || path == " " || path == "-") {
    return false;
    /*OLED().oledWord("Creating new file.");
    delay(500);

    document.lineCount = 1;
    initLine(document.lines[0]);
    document.lines[0].type = 'T';
    currentLineNum = 0;
    return;*/
  }

  if (PM_SDAUTO().getNoSD()) {
    OLED().oledWord("LOAD FAILED - No SD!");
    delay(5000);
    return false;
  }

  delay(50);

  File file = global_fs->open(path.c_str(), FILE_READ);
  if (!file) {
    return false;
    /*ESP_LOGE("SD", "File does not exist: %s", path.c_str());
    OLED().oledWord("LOAD FAILED - FILE MISSING");
    delay(2000);

    document.lineCount = 1;
    initLine(document.lines[0]);
    document.lines[0].type = 'T';
    currentLineNum = 0;
    return;*/
  }

  // Initialize document
  document.lineCount = 0;
  for (ulong i = 0; i < MAX_LINES; i++) {
    initLine(document.lines[i]);
  }

  while (file.available() && document.lineCount < MAX_LINES) {
    String line = file.readStringUntil('\n');
    line.trim();
    char style = 'T';
    String content = line;  // default is full line

    // Parse Markdown Prefix
    if (line.length() == 0) {
      style = 'B'; // Blank line
      content = "";
    } else if (line.startsWith("### ")) {
      style = '3'; // Heading 3
      content = line.substring(4);  
    } else if (line.startsWith("## ")) {
      style = '2'; // Heading 2
      content = line.substring(3);  
    } else if (line.startsWith("# ")) {
      style = '1'; // Heading 1
      content = line.substring(2);  
    } else if (line.startsWith("> ")) {
      style = '>'; // Quote Block
      content = line.substring(2);  
    } else if (line.startsWith("- ")) {
      style = 'U'; 
      content = line.substring(2); 
    } else if (line == "---") {
      style = 'H'; // Horizontal Rule
      content = "";  
    } else if ((line.startsWith("```")) || (line.startsWith("`") && line.endsWith("`")) || (line.startsWith("```") && line.endsWith("```"))) {
      if (line.startsWith("```"))
        content = line.substring(3);
      else if (line.startsWith("```") && line.endsWith("```"))
        content = line.substring(3, line.length() - 3);
      else if (line.startsWith("`") && line.endsWith("`"))
        content = line.substring(1, line.length() - 1);

      style = 'C'; // Code Block
    } else if (line.length() > 2 && isDigit(line.charAt(0)) && line.charAt(1) == '.' && line.charAt(2) == ' ') {
      style = 'O'; 
      content = line.substring(3); 
    }

    // Split content if it exceeds pixel bounds OR the fixed array LINE_CAP bounds
    while (content.length() > 0 && document.lineCount < MAX_LINES) {
      int splitIndex = findWrapIndex(content, style);

      String chunk = content.substring(0, splitIndex);
      content = content.substring(splitIndex);
      content.trim(); // remove leading space from remainder

      Line& docLine = document.lines[document.lineCount];
      docLine.type = style;
      strncpy(docLine.text, chunk.c_str(), LINE_CAP);
      docLine.text[LINE_CAP] = '\0';
      docLine.len = strlen(docLine.text);
      
      document.lineCount++;
      
      // --- Setup style for the continuation line ---
      if (content.length() > 0) {
        if (style == 'U') style = 'u';
        else if (style == 'O') style = 'o';
        // Removed the rule that downgrades '1', '2', '3' to 'T'. 
        // Headers will now retain their style across word wraps.
      }
    }
    
    // Catch empty lines (like parsed 'B' or 'H') that bypassed the length split loop
    if (content.length() == 0 && (style == 'B' || style == 'H') && document.lineCount < MAX_LINES) {
        Line& docLine = document.lines[document.lineCount];
        docLine.type = style;
        docLine.text[0] = '\0';
        docLine.len = 0;
        document.lineCount++;
    }
  }

  file.close();

  if (document.lineCount == 0) {
    document.lineCount = 1;
    initLine(document.lines[0]);
    document.lines[0].type = 'T';
    currentLineNum = 0;
  } else {
    currentLineNum = document.lineCount - 1;
  }

  if (SAVE_POWER)
    pocketmage::setCpuSpeed(80);
  SDActive = false;

  OLED().oledWord("FILE LOADED");
  delay(500);

  return true;
}

void newMarkdownFile(const String& path) {
  if (PM_SDAUTO().getNoSD()) {
    OLED().oledWord("CREATE FAILED - No SD!");
    delay(3000);
    return;
  }

  SDActive = true;
  pocketmage::setCpuSpeed(240); // Boost clock for SD operation
  delay(50);

  // Sanitize path
  String savePath = path;
  if (savePath == "" || savePath == "-") savePath = "/notes/untitled.txt";
  if (!savePath.startsWith("/")) savePath = "/" + savePath;

  // Create an empty file
  File file = global_fs->open(savePath.c_str(), FILE_WRITE);
  if (!file) {
    OLED().oledWord("CREATE FAILED");
    delay(2000);
    ESP_LOGE("SD", "Failed to create file: %s", savePath.c_str());
    SDActive = false;
    if (SAVE_POWER) pocketmage::setCpuSpeed(80);
    return;
  }
  file.close(); // Close immediately to leave it blank

  // Save metadata and update the system's active editing file
  PM_SDAUTO().writeMetadata(savePath);
  PM_SDAUTO().setEditingFile(savePath);

  OLED().oledWord("Created File");
  delay(1000);

  // Load the newly created blank file into the editor memory
  loadMarkdownFile(savePath);
  
  // Reset scrolling and cursor variables
  currentLineNum = 0;
  topVisibleLine = 0;
  updateScreen = true;

  if (SAVE_POWER) pocketmage::setCpuSpeed(80); // Return to power save
  SDActive = false;
}

#pragma region OLED Editor
// OLED Editor
void editorOledDisplay(Line& line, uint16_t cursor_pos, bool currentlyTyping) {
  u8g2.clearBuffer();

  bool currentWordBold = false;
  bool currentWordItalic = false;

  // Draw line text
  if (getLineWidthOLED(line) < (u8g2.getDisplayWidth() - 5)) {
    bool bold = false;
    bool italic = false;
    int xpos = 0;  // total width in pixels
    char temp[2] = {0, '\0'};
    uint16_t i = 0;

    // Draw cursor at position 0
    if (cursor_pos == 0) {
      u8g2.drawVLine(xpos, 1, 22);
    }

    while (i < line.len) {
      char c = line.text[i];

      if (c == '*') {
        if (i == 0 || line.text[i - 1] != '*') {
          uint8_t starCount = 0;
          while (i + starCount < line.len && line.text[i + starCount] == '*' && starCount < 3)
            starCount++;

          // toggle style based on number of stars
          switch (starCount) {
            case 1:
              italic = !italic;
              break;
            case 2:
              bold = !bold;
              break;
            case 3:
              bold = !bold;
              italic = !italic;
              break;
          }
        }
        // setFontOLED(false, false);
        //  Use tiny font for stars
        u8g2.setFont(u8g2_font_5x7_tf);
      } else {
        setFontOLED(bold, italic);
      }

      // Draw text & add width
      temp[0] = c;
      if (c == '*') u8g2.drawStr(xpos, 8, temp);
      else u8g2.drawStr(xpos, 20, temp);
      xpos += u8g2.getStrWidth(temp);

      // italics need to overlap a bit
      if (italic && c != '*') xpos -= 3;

      // Draw cursor and capture word formatting state
      if (cursor_pos == i + 1) {
        u8g2.drawVLine(xpos, 1, 22);
        currentWordBold = bold;
        currentWordItalic = italic;
      }

      i++;
    }

  } else {
    // Line is too long to fit, calculate scrolling offset
    int total_pixel_width = getLineWidthOLED(line);
    int display_w = u8g2.getDisplayWidth();

    // 1. Calculate the exact pixel offset of the cursor dynamically
    int cursor_pixel_offset = 0;
    bool bold = false;
    bool italic = false;
    char temp[2] = {0, '\0'};

    // This loop evaluates the line exactly up to the cursor position
    for (uint16_t j = 0; j < cursor_pos; j++) {
      char c = line.text[j];
      if (c == '*') {
        if (j == 0 || line.text[j - 1] != '*') {
          uint8_t starCount = 0;
          while (j + starCount < line.len && line.text[j + starCount] == '*' && starCount < 3)
            starCount++;
          switch (starCount) {
            case 1:
              italic = !italic;
              break;
            case 2:
              bold = !bold;
              break;
            case 3:
              bold = !bold;
              italic = !italic;
              break;
          }
        }
        // setFontOLED(false, false);
        //  Use tiny font for stars
        u8g2.setFont(u8g2_font_5x7_tf);
      } else {
        setFontOLED(bold, italic);
      }
      temp[0] = c;
      cursor_pixel_offset += u8g2.getStrWidth(temp);

      // italics need to overlap a bit
      if (italic && c != '*') cursor_pixel_offset -= 3;
    }

    // Capture the state at the end of the pre-computation loop
    currentWordBold = bold;
    currentWordItalic = italic;

    // 2. Determine horizontal shift to keep cursor centered
    int line_start = 0;

    if (cursor_pos == 0) {
      line_start = 0;
    } else if (cursor_pos == line.len) {
      // Show end of line, input scrolls left
      line_start = display_w - 8 - total_pixel_width;
    } else {
      if (cursor_pixel_offset > (display_w - 8) / 2) {
        // Shift left
        line_start = ((display_w - 8) / 2) - cursor_pixel_offset;

        // Prevent scrolling too far left if the right edge is visible
        if (line_start + total_pixel_width < display_w - 8) {
          line_start = display_w - 8 - total_pixel_width;
        }
      }
    }

    // 3. Draw the shifted line
    int xpos = line_start;
    bold = false;
    italic = false;
    uint16_t i = 0;

    // Draw cursor at position 0
    if (cursor_pos == 0) {
      u8g2.drawVLine(xpos, 1, 22);
    }

    while (i < line.len) {
      char c = line.text[i];

      if (c == '*') {
        if (i == 0 || line.text[i - 1] != '*') {
          uint8_t starCount = 0;
          while (i + starCount < line.len && line.text[i + starCount] == '*' && starCount < 3)
            starCount++;
          switch (starCount) {
            case 1:
              italic = !italic;
              break;
            case 2:
              bold = !bold;
              break;
            case 3:
              bold = !bold;
              italic = !italic;
              break;
          }
        }
        // setFontOLED(false, false);
        //  Use tiny font for stars
        u8g2.setFont(u8g2_font_5x7_tf);
      } else {
        setFontOLED(bold, italic);
      }

      temp[0] = c;
      int char_w = u8g2.getStrWidth(temp);

      // Draw character only if it is within screen bounds
      if (xpos + char_w >= 0 && xpos <= display_w) {
        if (c == '*') u8g2.drawStr(xpos, 8, temp);
        else u8g2.drawStr(xpos, 20, temp);
      }
      xpos += char_w;

      // italics need to overlap a bit
      if (italic && c != '*') xpos -= 3;

      // Draw cursor
      if (cursor_pos == i + 1) {
        u8g2.drawVLine(xpos, 1, 22);
      }

      i++;
    }
  }

  // PROGRESS BAR
  //progress (px):
  //((widthUsed % eink.width()) / eink.width()) * oled.width()


  /*if (line.len > 0) {
    int total_pixel_width = getLineWidthOLED(line);
    int progress = ((total_pixel_width % display.width()) / display.width()) * u8g2.getDisplayWidth();

    u8g2.drawVLine(u8g2.getDisplayWidth(), 0, 2);
    u8g2.drawVLine(0, 0, 2);

    u8g2.drawHLine(0, 0, progress);
    u8g2.drawHLine(0, 1, progress);
    // u8g2.drawHLine(0, 2, progress);

    // LINE END WARNING INDICATOR
    if (progress > (u8g2.getDisplayWidth() * 0.8)) {
      if ((millis() / 400) % 2 == 0) {  // ON for 200ms, OFF for 200ms
        u8g2.drawVLine(u8g2.getDisplayWidth() - 1, 8, 32 - 16);
        u8g2.drawLine(u8g2.getDisplayWidth() - 1, 15, u8g2.getDisplayWidth() - 4, 12);
        u8g2.drawLine(u8g2.getDisplayWidth() - 1, 15, u8g2.getDisplayWidth() - 4, 18);
      }
    }
  }*/

  if (currentlyTyping) {
    // Show toolbar
    toolBar(line, currentWordBold, currentWordItalic);
  } else {
    // Show infobar
    OLED().infoBar();
  }

  u8g2.sendBuffer();
}

#pragma region E-Ink Editor
// E-Ink Editor
void editorEinkDisplay(Document& doc, uint16_t currentLineNum) {
  // Scroll up boundary
  if (currentLineNum < topVisibleLine) {
    topVisibleLine = currentLineNum;
  }

  // Scroll down boundary
  uint16_t j = 0;
  while (j <= doc.lineCount) {
    int cursorBottomY = 0;
    
    for (uint16_t i = topVisibleLine; i <= currentLineNum; i++) {
      cursorBottomY += getCalculatedLineHeight(doc.lines[i]);
    }

    // Current line offscreen, move window and retest
    if (cursorBottomY > display.height()) {
      topVisibleLine++;
    } else {
      break;
    }

    // Ensure loop doesn't get stuck
    j++;
  }

  // Render the viewport
  display.fillScreen(GxEPD_WHITE); 
  
  int currentY = 0;

  for (uint16_t i = topVisibleLine; i < doc.lineCount; i++) {
    Line& line = doc.lines[i];
    bool isSelected = (i == currentLineNum);
    
    // Pass the document and index
    int heightUsed = drawLineEink(doc, i, 0, currentY, isSelected); 
    
    currentY += heightUsed;
    if (currentY >= display.height() + 20) break;
  }
}

void editor(char inchar) {
  static uint16_t cursor_pos = 0;
  static long lastInput = millis();
  bool currentlyTyping = false;
  
  // Style cycling debounce variables
  static unsigned long lastStyleCycleMillis = 0;
  static bool pendingStyleRefresh = false;

  Line& line = document.lines[currentLineNum];

  // Prevent memory corruption if scrolling to a shorter line
  if (cursor_pos > line.len) {
    cursor_pos = line.len;
  }

  // 1. Process Input if a key was pressed
  if (inchar != 0) {
    lastInput = millis();

    // CR Recieved (ENTER KEY)
    if (inchar == 13) {
      Line& activeLine = document.lines[currentLineNum];

      // 1. Split text at the cursor
      String leftHalf = String(activeLine.text).substring(0, cursor_pos);
      String rightHalf = String(activeLine.text).substring(cursor_pos);
      
      // Update current line
      strncpy(activeLine.text, leftHalf.c_str(), LINE_CAP);
      activeLine.text[LINE_CAP] = '\0';
      activeLine.len = leftHalf.length();

      // 2. Insert blank line ('B') to create the paragraph break
      insertLineArray(currentLineNum + 1);
      document.lines[currentLineNum + 1].type = 'B';
      
      // 3. Insert new text line for the right half of the split
      insertLineArray(currentLineNum + 2);
      Line& newLine = document.lines[currentLineNum + 2];
      
      // Determine style for the new line
      if (activeLine.type == 'U' || activeLine.type == 'u') newLine.type = 'U'; 
      else if (activeLine.type == 'O' || activeLine.type == 'o') newLine.type = 'O'; 
      else if (activeLine.type == '>') newLine.type = '>';
      else if (activeLine.type == 'C') newLine.type = 'C';
      else newLine.type = 'T';

      strncpy(newLine.text, rightHalf.c_str(), LINE_CAP);
      newLine.text[LINE_CAP] = '\0';
      newLine.len = rightHalf.length();

      // 4. Move cursor to the new line
      currentLineNum += 2;
      cursor_pos = 0;
      updateScreen = true;
      
      // Reflow the new line in case the rightHalf was huge
      reflowParagraph(currentLineNum, cursor_pos);
    }

    // SHIFT Recieved
    else if (inchar == 17) {
      if (KB().getKeyboardState() == SHIFT || KB().getKeyboardState() == FN_SHIFT) KB().setKeyboardState(NORMAL);
      else if (KB().getKeyboardState() == FUNC) KB().setKeyboardState(FN_SHIFT);
      else KB().setKeyboardState(SHIFT);
    }

    // FN Recieved
    else if (inchar == 18) {
      if (KB().getKeyboardState() == FUNC || KB().getKeyboardState() == FN_SHIFT) KB().setKeyboardState(NORMAL);
      else if (KB().getKeyboardState() == SHIFT) KB().setKeyboardState(FN_SHIFT);
      else KB().setKeyboardState(FUNC);
    }

    // BKSP Recieved
    else if (inchar == 8) {
      if (cursor_pos > 0) {
        ulong preReflowLine = currentLineNum; // Capture state
        
        deleteChar(document.lines[currentLineNum], cursor_pos);
        reflowParagraph(currentLineNum, cursor_pos); // Reflow text inwards
        
        // Trigger E-ink update if word-wrap pulled the cursor up a line
        if (currentLineNum != preReflowLine) {
          updateScreen = true;
        }
      } else {
        mergeLinesUp(currentLineNum, cursor_pos);
      }
    }

    // LEFT
    else if (inchar == 19) {
      if (cursor_pos > 0) {
        cursor_pos--;
      } else if (currentLineNum > 0) {
        currentLineNum--;
        cursor_pos = document.lines[currentLineNum].len;
        updateScreen = true;
      }
    }

    // RIGHT
    else if (inchar == 21) {
      if (cursor_pos < document.lines[currentLineNum].len) {
        cursor_pos++;
      } else if (currentLineNum < document.lineCount - 1) {
        currentLineNum++;
        cursor_pos = 0;
        updateScreen = true;
      }
    }

    // CENTER
    else if (inchar == 20) {}

    // SHIFT+LEFT (Cycle Paragraph Style)
    else if (inchar == 28) {
      cycleParagraphStyle(currentLineNum, cursor_pos);
      // Log the time and flag that we need an E-ink update soon
      lastStyleCycleMillis = millis();
      pendingStyleRefresh = true;
    }

    // SHIFT+RIGHT
    else if (inchar == 30) {
      cycleTextStyle(document.lines[currentLineNum], cursor_pos);
    }

    // SHIFT+CENTER (New File)
    else if (inchar == 29) {
      if (CurrentTXTState_NEW != JOURNAL_MODE) {
        CurrentTXTState_NEW = NEW_FILE;
        KB().setKeyboardState(NORMAL);
      }
    }

    // FN+LEFT (Home)
    else if (inchar == 12) {
      if (CurrentTXTState_NEW != JOURNAL_MODE) {
        HOME_INIT();
      } else {
        JOURNAL_INIT(); 
      }
    }

    // FN+RIGHT (Save File)
    else if (inchar == 6) {
      if (CurrentTXTState_NEW != JOURNAL_MODE) {
        String savePath = PM_SDAUTO().getEditingFile();
        if (savePath == "" || savePath == "-" || savePath == "/temp.txt") {
          KB().setKeyboardState(NORMAL);
          CurrentTXTState_NEW = SAVE_AS;
        } else {
          if (!savePath.startsWith("/")) savePath = "/" + savePath;
          saveMarkdownFile(savePath);
        }
      } else {
        // Journal save
        String savePath = getCurrentJournal();
        if (!savePath.startsWith("/")) savePath = "/" + savePath;
        saveMarkdownFile(savePath);
      }
    }

    // FN+CENTER (Load File)
    else if (inchar == 7) {
      if (CurrentTXTState_NEW != JOURNAL_MODE) {
        CurrentTXTState_NEW = LOAD_FILE;
        KB().setKeyboardState(NORMAL);
      } else {
        // Journal load
        String outPath = getCurrentJournal();
        if (!outPath.startsWith("/")) outPath = "/" + outPath;
        loadMarkdownFile(outPath);
      }
    }

    // FN+SHIFT+LEFT
    else if (inchar == 24) {
      cursor_pos = 0;
    }

    // FN+SHIFT+RIGHT
    else if (inchar == 26) {
      cursor_pos = document.lines[currentLineNum].len;
    }

    // FN+SHIFT+CENTER
    else if (inchar == 25) {}

    // TAB / SHIFT+TAB (Word Navigation)
    else if (inchar == 9 || inchar == 14) {
      if (KB().getKeyboardState() == SHIFT || KB().getKeyboardState() == FN_SHIFT || inchar == 14) {
        // --- PREVIOUS WORD (Shift + Tab) ---
        if (cursor_pos == 0) {
          if (currentLineNum > 0) {
            currentLineNum--;
            cursor_pos = document.lines[currentLineNum].len;
            updateScreen = true;
          }
        } else {
          while (cursor_pos > 0 && document.lines[currentLineNum].text[cursor_pos - 1] == ' ') cursor_pos--;
          while (cursor_pos > 0 && document.lines[currentLineNum].text[cursor_pos - 1] != ' ') cursor_pos--;
        }
      } else {
        // --- NEXT WORD (Tab) ---
        if (cursor_pos >= line.len) {
          if (currentLineNum < document.lineCount - 1) {
            currentLineNum++;
            cursor_pos = 0;
            updateScreen = true;
          }
        } else {
          while (cursor_pos < line.len && document.lines[currentLineNum].text[cursor_pos] != ' ') cursor_pos++;
          while (cursor_pos < line.len && document.lines[currentLineNum].text[cursor_pos] == ' ') cursor_pos++;
        }
      }
    }

    // Normal character input
    else {
      ulong preReflowLine = currentLineNum;
      
      if (document.lines[currentLineNum].type == ' ' || document.lines[currentLineNum].type == 'B') {
        document.lines[currentLineNum].type = 'T';
      }

      insertChar(document.lines[currentLineNum], cursor_pos, inchar);
      
      reflowParagraph(currentLineNum, cursor_pos);

      if (currentLineNum != preReflowLine) {
        updateScreen = true;
      }

      if (inchar < 48 || inchar > 57) {
        if (KB().getKeyboardState() != NORMAL) KB().setKeyboardState(NORMAL);
      }
    }
  }

  // Debounce Timer for Style Cycling
  if (pendingStyleRefresh && (millis() - lastStyleCycleMillis > 1000)) {
    updateScreen = true;
    pendingStyleRefresh = false;
  }

  // Determine Toolbar State
  if (millis() - lastInput > IDLE_TIME) currentlyTyping = false;
  else currentlyTyping = true;

  // Render OLED
  unsigned long currentMillis = millis();
  if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
    char inchar = KB().updateKeypress();
    switch (CurrentTXTState) {
      case TXT_:
        // SET MAXIMUMS AND FONT
        EINK().setTXTFont(EINK().getCurrentFont());


        // HANDLE INPUTS
        // No char recieved
        if (inchar == 0)
          ;
        else if (inchar == 12) {
          CurrentAppState = HOME;
          currentLine = "";
          newState = true;
          KB().setKeyboardState(NORMAL);
        }
        // TAB Recieved
        else if (inchar == 9) {
          currentLine += "    ";
        }
        // SHIFT Recieved
        else if (inchar == 17) {
          if (KB().getKeyboardState() == SHIFT || KB().getKeyboardState() == FN_SHIFT) {
            KB().setKeyboardState(NORMAL);
          } else if (KB().getKeyboardState() == FUNC) {
            KB().setKeyboardState(FN_SHIFT);
          } else {
            KB().setKeyboardState(SHIFT);
          }
        }
        // FN Recieved
        else if (inchar == 18) {
          if (KB().getKeyboardState() == FUNC || KB().getKeyboardState() == FN_SHIFT) {
            KB().setKeyboardState(NORMAL);
          } else if (KB().getKeyboardState() == SHIFT) {
            KB().setKeyboardState(FN_SHIFT);
          } else {
            KB().setKeyboardState(FUNC);
          }
        }
        // Space Recieved
        else if (inchar == 32) {
          currentLine += " ";
        }
        // CR Recieved
        else if (inchar == 13) {
          allLines.push_back(currentLine);
          currentLine = "";
          newLineAdded = true;
        }
        // ESC / CLEAR Recieved
        else if (inchar == 20) {
          allLines.clear();
          currentLine = "";
          OLED().oledWord("Clearing...");
          doFull = true;
          newLineAdded = true;
          delay(300);
        }
        // LEFT
        else if (inchar == 19) {
        }
        // RIGHT
        else if (inchar == 21) {
        }
        // BKSP Recieved
        else if (inchar == 8) {
          if (currentLine.length() > 0) {
            currentLine.remove(currentLine.length() - 1);
          }
        }
        // SAVE Recieved
        else if (inchar == 6) {
          // File exists, save normally
          if (PM_SDAUTO().getEditingFile() != "" && PM_SDAUTO().getEditingFile() != "-") {
            PM_SDAUTO().saveFile();
            KB().setKeyboardState(NORMAL);
            newLineAdded = true;
          }
          // File does not exist, make a new one
          else {
            CurrentTXTState = WIZ3;
            currentLine = "";
            KB().setKeyboardState(NORMAL);
            doFull = true;
            newState = true;
          }
        }
        // LOAD Recieved
        else if (inchar == 5) {
          PM_SDAUTO().loadFile();
          KB().setKeyboardState(NORMAL);
          newLineAdded = true;
        }
        // FILE Recieved
        else if (inchar == 7) {
          CurrentTXTState = WIZ0;
          KB().setKeyboardState(NORMAL);
          newState = true;
        }
        // Font Switcher
        else if (inchar == 14) {
          CurrentTXTState = FONT;
          KB().setKeyboardState(FUNC);
          newState = true;
        } else {
          currentLine += inchar;
          if (inchar >= 48 && inchar <= 57) {
          }  // Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        // Make sure oled only updates at 60fps
        if (currentMillis - OLEDFPSMillis >= (1000 / 60)) {
          OLEDFPSMillis = currentMillis;
          // ONLY SHOW OLEDLINE WHEN NOT IN SCROLL MODE
          if (TOUCH().getLastTouch() == -1) {
            OLED().oledLine(currentLine, currentLine.length());
            if (TOUCH().getPrevDynamicScroll() != TOUCH().getDynamicScroll())
              TOUCH().setPrevDynamicScroll(TOUCH().getDynamicScroll());
          } else
            OLED().oledScroll();
        }
      }
      break;

        if (currentLine.length() > 0) {
          int16_t x1, y1;
          uint16_t charWidth, charHeight;
          display.getTextBounds(currentLine, 0, 0, &x1, &y1, &charWidth, &charHeight);

          if (charWidth >= display.width() - 5) {
            // If currentLine ends with a space, just start a new line
            if (currentLine.endsWith(" ")) {
              allLines.push_back(currentLine);
              currentLine = "";
            }
            // If currentLine ends with a letter, we are in the middle of a word
            else {
              int lastSpace = currentLine.lastIndexOf(' ');
              String partialWord;

              if (lastSpace != -1) {
                partialWord = currentLine.substring(lastSpace + 1);
                currentLine = currentLine.substring(0, lastSpace);  // Strip partial word
                allLines.push_back(currentLine);
                currentLine = partialWord;  // Start new line with the partial word
              }
              // No spaces found, whole line is a single word
              else {
                allLines.push_back(currentLine);
                currentLine = "";
              }
            }
            newLineAdded = true;
          }
        }

        break;
      case WIZ0:
        // No char recieved
        if (inchar == 0)
          ;
        // BKSP Recieved
        else if (inchar == 127 || inchar == 8) {
          CurrentTXTState = TXT_;
          KB().setKeyboardState(NORMAL);
          newLineAdded = true;
          currentWord = "";
          currentLine = "";
          display.fillScreen(GxEPD_WHITE);
        } else if (inchar >= '0' && inchar <= '9') {
          int fileIndex = (inchar == '0') ? 10 : (inchar - '0');
          // Edit a new file
          if (PM_SDAUTO().getFilesListIndex(fileIndex - 1) != PM_SDAUTO().getEditingFile()) {
            // Selected file does not exist, create a new one
            if (PM_SDAUTO().getFilesListIndex(fileIndex - 1) == "-") {
              CurrentTXTState = WIZ3;
              EINK().setFullRefreshAfter(FULL_REFRESH_AFTER + 1);
              newState = true;
              display.fillScreen(GxEPD_WHITE);
            }
            // Selected file exists, prompt to save current file
            else {
              prevEditingFile = PM_SDAUTO().getEditingFile();
              PM_SDAUTO().setEditingFile(PM_SDAUTO().getFilesListIndex(fileIndex - 1));
              CurrentTXTState = WIZ1;
              EINK().setFullRefreshAfter(FULL_REFRESH_AFTER + 1);
              newState = true;
              display.fillScreen(GxEPD_WHITE);
            }
          }
          // Selected file is current file, return to editor
          else {
            KB().setKeyboardState(NORMAL);
            CurrentTXTState = TXT_;
            newLineAdded = true;
            currentWord = "";
            currentLine = "";
            display.fillScreen(GxEPD_WHITE);
          }
        }

        currentMillis = millis();
        // Make sure oled only updates at 60fps
        if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentWord, currentWord.length(), false);
        }
        break;
      case WIZ1:
        // No char recieved
        if (inchar == 0)
          ;
        // BKSP Recieved
        else if (inchar == 127 || inchar == 8) {
          CurrentTXTState = WIZ0;
          KB().setKeyboardState(FUNC);
          EINK().setFullRefreshAfter(FULL_REFRESH_AFTER + 1);
          newState = true;
          display.fillScreen(GxEPD_WHITE);
        } else if (inchar >= '0' && inchar <= '9') {
          int numSelect = (inchar == '0') ? 10 : (inchar - '0');
          // YES (save current file)
          if (numSelect == 1) {
            // File to be saved does not exist
            if (prevEditingFile == "" || prevEditingFile == "-") {
              CurrentTXTState = WIZ2;
              currentWord = "";
              KB().setKeyboardState(NORMAL);
              EINK().setFullRefreshAfter(FULL_REFRESH_AFTER + 1);
              newState = true;
              display.fillScreen(GxEPD_WHITE);
            }
            // File to be saved exists
            else {
              // Save current file
              PM_SDAUTO().saveFile();

              delay(200);
              // Load new file
              PM_SDAUTO().loadFile();
              // Return to TXT
              CurrentTXTState = TXT_;
              KB().setKeyboardState(NORMAL);
              newLineAdded = true;
              currentWord = "";
              currentLine = "";
              display.fillScreen(GxEPD_WHITE);
            }
          }
          // NO  (don't save current file)
          else if (numSelect == 2) {
            // Just load new file
            PM_SDAUTO().loadFile();
            // Return to TXT
            CurrentTXTState = TXT_;
            KB().setKeyboardState(NORMAL);
            newLineAdded = true;
            currentWord = "";
            currentLine = "";
            display.fillScreen(GxEPD_WHITE);
          }
        }

        currentMillis = millis();
        // Make sure oled only updates at 60fps
        if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentWord, currentWord.length(), false);
        }
        break;

      case WIZ2:
        // No char recieved
        if (inchar == 0)
          ;
        // SHIFT Recieved
        else if (inchar == 17) {
          if (KB().getKeyboardState() == SHIFT || KB().getKeyboardState() == FN_SHIFT) {
            KB().setKeyboardState(NORMAL);
          } else if (KB().getKeyboardState() == FUNC) {
            KB().setKeyboardState(FN_SHIFT);
          } else {
            OLED().oledWord("Invalid Name");
            delay(2000);
          }
          inputBuffer = "";
        }
        // FN Recieved
        else if (inchar == 18) {
          if (KB().getKeyboardState() == FUNC || KB().getKeyboardState() == FN_SHIFT) {
            KB().setKeyboardState(NORMAL);
          } else if (KB().getKeyboardState() == SHIFT) {
            KB().setKeyboardState(FN_SHIFT);
          } else {
            KB().setKeyboardState(FUNC);
          }
          newState = true;
        }
        // Space Recieved
        else if (inchar == 32) {
        }
        // ESC / CLEAR Recieved
        else if (inchar == 20) {
          currentWord = "";
        }
        // BKSP Recieved
        else if (inchar == 8) {
          if (currentWord.length() > 0) {
            currentWord.remove(currentWord.length() - 1);
          }
        }
        // ENTER Recieved
        else if (inchar == 13) {
          prevEditingFile = "/" + currentWord + ".txt";

          // Save the file
          PM_SDAUTO().saveFile();

          delay(200);
          // Load new file
          PM_SDAUTO().loadFile();

          keypad.enableInterrupts();

          // Return to TXT_
          CurrentTXTState = TXT_;
          KB().setKeyboardState(NORMAL);
          newLineAdded = true;
          currentWord = "";
          currentLine = "";
        }
        // All other chars
        else {
          // Only allow char to be added if it's an allowed char
          if (isalnum(inchar) || inchar == '_' || inchar == '-' || inchar == '.')
            currentWord += inchar;
          if (inchar >= 48 && inchar <= 57) {
          }  // Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        // Make sure oled only updates at 60fps
        if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(inputBuffer, inputBuffer.length(), false, "Name New File");
        }
      }
      break;

    case LOAD_FILE:
      String outPath = fileWizardMini(false, "/notes", inchar);
      if (outPath == "_EXIT_") {
        // Return to TXT
        CurrentTXTState_NEW = TXT_;
        break;
      case WIZ3:
        // No char recieved
        if (inchar == 0)
          ;
        // SHIFT Recieved
        else if (inchar == 17) {
          if (KB().getKeyboardState() == SHIFT || KB().getKeyboardState() == FN_SHIFT) {
            KB().setKeyboardState(NORMAL);
          } else if (KB().getKeyboardState() == FUNC) {
            KB().setKeyboardState(FN_SHIFT);
          } else {
            KB().setKeyboardState(SHIFT);
          }
        }
        // FN Recieved
        else if (inchar == 18) {
          if (KB().getKeyboardState() == FUNC || KB().getKeyboardState() == FN_SHIFT) {
            KB().setKeyboardState(NORMAL);
          } else if (KB().getKeyboardState() == SHIFT) {
            KB().setKeyboardState(FN_SHIFT);
          } else {
            KB().setKeyboardState(FUNC);
          }
        }
        // Space Recieved
        else if (inchar == 32) {
        }
        // ESC / CLEAR Recieved
        else if (inchar == 20) {
          currentWord = "";
        }
        // BKSP Recieved
        else if (inchar == 8) {
          if (currentWord.length() > 0) {
            currentWord.remove(currentWord.length() - 1);
          }
        }
        // ENTER Recieved
        else if (inchar == 13) {
          prevEditingFile = "/" + currentWord + ".txt";

          // Save the file
          PM_SDAUTO().saveFile();
          // Ask to save prev file

          // Return to TXT_
          CurrentTXTState = TXT_;
          KB().setKeyboardState(NORMAL);
          newLineAdded = true;
          currentWord = "";
          currentLine = "";
        }
        // All other chars
        else {
          // Only allow char to be added if it's an allowed char
          if (isalnum(inchar) || inchar == '_' || inchar == '-' || inchar == '.')
            currentWord += inchar;
          if (inchar >= 48 && inchar <= 57) {
          }  // Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        // Make sure oled only updates at 60fps
        if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentWord, currentWord.length(), false);
        }
        break;
      case FONT:
        // No char recieved
        if (inchar == 0)
          ;
        // BKSP Recieved
        else if (inchar == 127 || inchar == 8) {
          CurrentTXTState = TXT_;
          KB().setKeyboardState(NORMAL);
          newLineAdded = true;
          currentWord = "";
          currentLine = "";
          display.fillScreen(GxEPD_WHITE);
        } else if (inchar >= '0' && inchar <= '9') {
          int fontIndex = (inchar == '0') ? 10 : (inchar - '0');
          switch (fontIndex) {
            case 1:
              EINK().setCurrentFont(&FreeMonoBold9pt8b);
              break;
            case 2:
              EINK().setCurrentFont(&FreeSans9pt8b);
              break;
            case 3:
              EINK().setCurrentFont(&FreeSerif9pt8b);
              break;
            case 4:
              EINK().setCurrentFont(&FreeSerifBold9pt8b);
              break;
            case 5:
              EINK().setCurrentFont(&FreeMono12pt8b);
              break;
            case 6:
              EINK().setCurrentFont(&FreeSans12pt8b);
              break;
            case 7:
              EINK().setCurrentFont(&FreeSerif12pt8b);
              break;
            default:
              EINK().setCurrentFont(&FreeMonoBold9pt8b);
              break;
          }
          // SET THE FONT
          EINK().setTXTFont(EINK().getCurrentFont());

          // UPDATE THE ARRAY TO MATCH NEW FONT SIZE
          String fullTextStr = vectorToString();
          stringToVector(fullTextStr);

          CurrentTXTState = TXT_;
          KB().setKeyboardState(NORMAL);
          newLineAdded = true;
          currentWord = "";
          currentLine = "";
          display.fillScreen(GxEPD_WHITE);
        }

        currentMillis = millis();
        // Make sure oled only updates at 60fps
        if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentWord, currentWord.length(), false);
        }
        break;
    }
    KBBounceMillis = currentMillis;
  }
}

void einkHandler_TXT_OLD() {
  if (newLineAdded || newState) {
    switch (CurrentTXTState) {
      case TXT_:
        if (newState && doFull) {
          EINK().resetDisplay();
          EINK().refresh();
        }
        if (newLineAdded && !newState) {
          EINK().einkTextDynamic(true);
          EINK().refresh();
        }
        break;
      case WIZ0:
        display.setFullWindow();
        EINK().einkTextDynamic(true, true);
        display.setFont(&FreeMonoBold9pt8b);

        display.fillRect(0, display.height() - 26, display.width(), 26, GxEPD_WHITE);
        display.drawRect(0, display.height() - 20, display.width(), 20, GxEPD_BLACK);
        display.setCursor(4, display.height() - 6);
        // display.print("W:" + String(countWords(allText)) + " C:" +
        // String(countVisibleChars(allText)) + " L:" + String(EINK().countLines(allText)));
        display.drawBitmap(display.width() - 30, display.height() - 20, KBStatusallArray[6], 30, 20,
                           GxEPD_BLACK);

        display.fillRect(60, 0, 200, 218, GxEPD_WHITE);
        display.drawBitmap(60, 0, fileWizLiteallArray[0], 200, 218, GxEPD_BLACK);

        keypad.disableInterrupts();
        PM_SDAUTO().listDir(*global_fs, "/");
        keypad.enableInterrupts();

        for (int i = 0; i < MAX_FILES; i++) {
          display.setCursor(88, 54 + (17 * i));
          display.print(PM_SDAUTO().getFilesListIndex(i));
        }

        EINK().refresh();
        KB().setKeyboardState(FUNC);
        break;
      case WIZ1:
        display.setFont(&FreeMonoBold9pt8b);

        display.fillRect(0, display.height() - 26, display.width(), 26, GxEPD_WHITE);
        display.drawRect(0, display.height() - 20, display.width(), 20, GxEPD_BLACK);
        display.setCursor(4, display.height() - 6);
        // display.print("W:" + String(countWords(allText)) + " C:" +
        // String(countVisibleChars(allText)) + " L:" + String(EINK().countLines(allText)));
        display.drawBitmap(display.width() - 30, display.height() - 20, KBStatusallArray[6], 30, 20,
                           GxEPD_BLACK);

        display.fillRect(60, 0, 200, 218, GxEPD_WHITE);
        display.drawBitmap(60, 0, fileWizLiteallArray[1], 200, 218, GxEPD_BLACK);

        EINK().refresh();
        KB().setKeyboardState(FUNC);
        break;
      case WIZ2:
        display.setFont(&FreeMonoBold9pt8b);

        display.fillRect(0, display.height() - 26, display.width(), 26, GxEPD_WHITE);
        display.drawRect(0, display.height() - 20, display.width(), 20, GxEPD_BLACK);
        display.setCursor(4, display.height() - 6);
        // display.print("W:" + String(countWords(allText)) + " C:" +
        // String(countVisibleChars(allText)) + " L:" + String(EINK().countLines(allText)));
        display.drawBitmap(display.width() - 30, display.height() - 20, KBStatusallArray[6], 30, 20,
                           GxEPD_BLACK);

        display.fillRect(60, 0, 200, 218, GxEPD_WHITE);
        display.drawBitmap(60, 0, fileWizLiteallArray[2], 200, 218, GxEPD_BLACK);

        EINK().refresh();
        KB().setKeyboardState(NORMAL);
        break;
      case WIZ3:
        display.setFullWindow();
        EINK().einkTextDynamic(true, true);
        display.setFont(&FreeMonoBold9pt8b);

        display.fillRect(0, display.height() - 26, display.width(), 26, GxEPD_WHITE);
        display.drawRect(0, display.height() - 20, display.width(), 20, GxEPD_BLACK);
        display.setCursor(4, display.height() - 6);
        // display.print("W:" + String(countWords(allText)) + " C:" +
        // String(countVisibleChars(allText)) + " L:" + String(EINK().countLines(allText)));
        display.drawBitmap(display.width() - 30, display.height() - 20, KBStatusallArray[6], 30, 20,
                           GxEPD_BLACK);

        display.fillRect(60, 0, 200, 218, GxEPD_WHITE);
        display.drawBitmap(60, 0, fileWizLiteallArray[3], 200, 218, GxEPD_BLACK);

        EINK().refresh();
        KB().setKeyboardState(NORMAL);
        break;
      case FONT:
        display.setFullWindow();
        EINK().einkTextDynamic(true, true);

        EINK().drawStatusBar("Select a Font (0-9)");

        display.fillRect(60, 0, 200, 218, GxEPD_WHITE);
        display.drawBitmap(60, 0, fontfont0, 200, 218, GxEPD_BLACK);

        keypad.disableInterrupts();
        PM_SDAUTO().listDir(*global_fs, "/");
        keypad.enableInterrupts();

        for (int i = 0; i < 7; i++) {
          display.setCursor(88, 54 + (17 * i));
          switch (i) {
            case 0:
              display.setFont(&FreeMonoBold9pt8b);
              break;
            case 1:
              display.setFont(&FreeSans9pt8b);
              break;
            case 2:
              display.setFont(&FreeSerif9pt8b);
              break;
            case 3:
              display.setFont(&FreeSerifBold9pt8b);
              break;
            case 4:
              display.setFont(&FreeMono12pt8b);
              break;
            case 5:
              display.setFont(&FreeSans12pt8b);
              break;
            case 6:
              display.setFont(&FreeSerif12pt8b);
              break;
          }
          display.print("Font Number " + String(i + 1));
        }

        EINK().refresh();
        KB().setKeyboardState(FUNC);
        break;
    }
    newState = false;
    newLineAdded = false;
  }
}
#endif
