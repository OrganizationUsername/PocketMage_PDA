//  ooooooooooooo ooooooo  ooooo ooooooooooooo  //
//  8'   888   `8  `8888    d8'  8'   888   `8  //
//       888         Y888..8P         888       //
//       888          `8888'          888       //
//       888         .8PY888.         888       //
//       888        d8'  `888b        888       //
//      o888o     o888o  o88888o     o888o      //

#include <globals.h>
#if !OTA_APP  // POCKETMAGE_OS
static constexpr const char* TAG = "TXT_NEWz";

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

// Serif
#include <Fonts/FreeSerif9pt8b.h>
#include <Fonts/FreeSerifBold12pt8b.h>
#include <Fonts/FreeSerifBold18pt8b.h>
#include <Fonts/FreeSerifBold24pt8b.h>
#include <Fonts/FreeSerifBold9pt8b.h>
#include <Fonts/FreeSerifBoldItalic12pt8b.h>
#include <Fonts/FreeSerifBoldItalic18pt8b.h>
#include <Fonts/FreeSerifBoldItalic24pt8b.h>
#include <Fonts/FreeSerifBoldItalic9pt8b.h>
#include <Fonts/FreeSerifItalic9pt8b.h>

// Sans
#include <Fonts/FreeSans9pt8b.h>
#include <Fonts/FreeSansBold12pt8b.h>
#include <Fonts/FreeSansBold18pt8b.h>
#include <Fonts/FreeSansBold24pt8b.h>
#include <Fonts/FreeSansBold9pt8b.h>
#include <Fonts/FreeSansBoldOblique12pt8b.h>
#include <Fonts/FreeSansBoldOblique18pt8b.h>
#include <Fonts/FreeSansBoldOblique24pt8b.h>
#include <Fonts/FreeSansBoldOblique9pt8b.h>
#include <Fonts/FreeSansOblique9pt8b.h>

#include "esp32-hal-log.h"
#include "esp_log.h"

// ------------------ General ------------------
enum TXTState_NEW { TXT_, FONT, SAVE_AS, LOAD_FILE, JOURNAL_MODE, NEW_FILE };
TXTState_NEW CurrentTXTState_NEW = TXT_;

// ------------------ Fonts ------------------
#define SPECIAL_PADDING 20      // Padding for lists, code blocks, quote blocks
#define SPACEWIDTH_SYMBOL "n"   // n is roughly the width of a space
#define WORDWIDTH_BUFFER 0      // Add extra spacing to each word
#define DISPLAY_WIDTH_BUFFER 0  // Add margin for text wrap calc
#define HEADING_LINE_PADDING 8  // Padding between each line
#define NORMAL_LINE_PADDING 2

enum FontFamily { serif = 0, sans = 1, mono = 2 };
uint8_t fontStyle = sans;

struct FontMap {
  const GFXfont* normal;
  const GFXfont* normal_B;
  const GFXfont* normal_I;
  const GFXfont* normal_BI;

  const GFXfont* h1;
  const GFXfont* h1_B;
  const GFXfont* h1_I;
  const GFXfont* h1_BI;

  const GFXfont* h2;
  const GFXfont* h2_B;
  const GFXfont* h2_I;
  const GFXfont* h2_BI;

  const GFXfont* h3;
  const GFXfont* h3_B;
  const GFXfont* h3_I;
  const GFXfont* h3_BI;

  const GFXfont* code;
  const GFXfont* code_B;
  const GFXfont* code_I;
  const GFXfont* code_BI;

  const GFXfont* quote;
  const GFXfont* quote_B;
  const GFXfont* quote_I;
  const GFXfont* quote_BI;

  const GFXfont* list;
  const GFXfont* list_B;
  const GFXfont* list_I;
  const GFXfont* list_BI;
};

FontMap fonts[3];

void setFontStyle(FontFamily f) {
  fontStyle = f;
}

// ------------------ Document Variables ------------------
static bool updateScreen = false;
static uint16_t currentLineNum = 0;
static uint16_t topVisibleLine = 0;

#define MAX_LINES 800     // Max number of lines in document
#define LINE_CAP 64       // Max number of characters in a line

struct Line {
  char type = ' ';          // line type
  char text[LINE_CAP + 1];  // line content
  uint16_t len;             // line length
};

struct Document {
  Line lines[MAX_LINES];
  ulong lineCount;
};

Document document;

#pragma region Mrkdn File Ops
void initLine(Line& line) {
  line.type = ' ';
  line.len = 0;
  line.text[0] = '\0';
}

void saveMarkdownFile(const String& path) {
}

void loadMarkdownFile(const String& path) {
  // Initialize document
  document.lineCount = 1;
  for (ulong i = 0; i < MAX_LINES; i++) {
    initLine(document.lines[i]);
  }

  // Load file
}

#pragma region OLED Editor
// Helpers
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

void cycleLineStyle(Line& line) {
  // Define the cycle order
  static const char styleCycle[] = {'T', '1', '2', '3', '>', 'L', '-', 'C', 'H'};
  static const int numStyles = sizeof(styleCycle) / sizeof(styleCycle[0]);

  // Find current style index
  int currentIndex = 0;
  for (int i = 0; i < numStyles; i++) {
    if (line.type == styleCycle[i]) {
      currentIndex = i;
      break;
    }
  }

  // Move to next style in cycle
  currentIndex = (currentIndex + 1) % numStyles;
  line.type = styleCycle[currentIndex];
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
    case '-':
      lineTypeLabel = "U LIST";
      break;
    case 'L':
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
  /*
  progress (px):
  ((widthUsed % eink.width()) / eink.width()) * oled.width()


  if (lineHasText(lineObj) == true && pixelsUsed > 0) {
    if (pixelsUsed > display.width() - DISPLAY_WIDTH_BUFFER)
      pixelsUsed = display.width() - DISPLAY_WIDTH_BUFFER;
    // uint8_t progress = map(pixelsUsed, 0, display.width() - DISPLAY_WIDTH_BUFFER, 0,
    // u8g2.getDisplayWidth());
    uint8_t progress = map(min(pixelsUsed, display.width() - DISPLAY_WIDTH_BUFFER), 0,
                           display.width() - DISPLAY_WIDTH_BUFFER, 0, u8g2.getDisplayWidth() - 1);

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
    // New line on space animation
    if (pixelsUsed >= display.width() - DISPLAY_WIDTH_BUFFER) {
      // Sawtooth animation
      uint period = 8000;
      uint x1 = map(millis() % period, 0, period, 0, u8g2.getDisplayWidth());
      uint x2 = map((millis() + period / 4) % period, 0, period, 0, u8g2.getDisplayWidth());
      uint x3 = map((millis() + period / 2) % period, 0, period, 0, u8g2.getDisplayWidth());
      uint x4 = map((millis() + (3 * period) / 4) % period, 0, period, 0, u8g2.getDisplayWidth());

      // Draw scrolling box
      u8g2.setDrawColor(0);
      u8g2.drawBox(x1, 0, 10, 2);
      u8g2.drawBox(x2, 0, 10, 2);
      u8g2.drawBox(x3, 0, 10, 2);
      u8g2.drawBox(x4, 0, 10, 2);
      u8g2.setDrawColor(1);
    }
  }
  */

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
// Helpers
const GFXfont* pickFont(char style, bool bold, bool italic) {
  FontMap& fm = fonts[fontStyle];  // currently active family

  switch (style) {
    case '1':  // H1
      if (bold && italic)
        return fm.h1_BI;
      if (bold)
        return fm.h1_B;
      if (italic)
        return fm.h1_I;
      return fm.h1;

    case '2':  // H2
      if (bold && italic)
        return fm.h2_BI;
      if (bold)
        return fm.h2_B;
      if (italic)
        return fm.h2_I;
      return fm.h2;

    case '3':  // H3
      if (bold && italic)
        return fm.h3_BI;
      if (bold)
        return fm.h3_B;
      if (italic)
        return fm.h3_I;
      return fm.h3;

    case '>':  // Quote
      if (bold && italic)
        return fm.quote_BI;
      if (bold)
        return fm.quote_B;
      if (italic)
        return fm.quote_I;
      return fm.quote;

    case '-':  // List
      if (bold && italic)
        return fm.list_BI;
      if (bold)
        return fm.list_B;
      if (italic)
        return fm.list_I;
      return fm.list;

    case 'C':  // Code
      if (bold && italic)
        return fm.code_BI;
      if (bold)
        return fm.code_B;
      if (italic)
        return fm.code_I;
      return fm.code;

    default:  // Normal
      if (bold && italic)
        return fm.normal_BI;
      if (bold)
        return fm.normal_B;
      if (italic)
        return fm.normal_I;
      return fm.normal;
  }
}

uint16_t getLineMaxHeight(Line& line) {
  bool boldExists = false;
  bool italicExists = false;
  bool boldItalicExists = false;
  const GFXfont* font;

  // Text bounds
  int16_t x1, y1;
  uint16_t wpx, hpx;

  // Invalid line
  if (line.type == ' ') return 0;

  // Check if there is a bold or italic or both in the line based on *
  for (uint16_t i = 0; i < line.len; i++) {
    if (line.text[i] == '*') {
      // count consecutive stars if there isn't a star before
      if (i == 0 || line.text[i - 1] != '*') {
        uint8_t starCount = 0;
        while (i + starCount < line.len && line.text[i + starCount] == '*' && starCount < 3) {
          starCount++;
        }
        
        if (starCount == 1) italicExists = true;
        else if (starCount == 2) boldExists = true;
        else if (starCount == 3) boldItalicExists = true;
      }
    }
  }

  if (boldItalicExists) {
    // Make all text bold + italic
    font = pickFont(line.type, true, true);
  }
  else if (boldExists) {
    // Make all text bold
    font = pickFont(line.type, true, false);
  } 
  else if (italicExists) {
    // Make all text italic
    font = pickFont(line.type, false, true);
  }
  else {
    // Make all text normal
    font = pickFont(line.type, false, false);
  }

  // Measure the height
  display.setFont(font);
  display.getTextBounds(line.text, 0, 0, &x1, &y1, &wpx, &hpx);
  return hpx;
}

int getCalculatedLineHeight(Line& line) {
  if (line.type == 'H') return 8;  // Horizontal Rule
  if (line.type == 'B') return 12; // Blank Line

  int h = getLineMaxHeight(line);
  
  // Add appropriate padding
  if (line.type == '1' || line.type == '2' || line.type == '3') {
    return h + HEADING_LINE_PADDING;
  }
  
  return h + NORMAL_LINE_PADDING;
}

int drawLineEink(Line& line, int startX, int startY) {
  char style = line.type;
  int cursorY = startY;

  // ---------- Non-Text Rendered Items ---------- //
  // Horizontal Rules just print a line
  if (style == 'H') {
    display.drawFastHLine(0, cursorY + 3, display.width(), GxEPD_BLACK);
    display.drawFastHLine(0, cursorY + 4, display.width(), GxEPD_BLACK);
    return 8;
  }
  // Blank lines just take up space
  else if (style == 'B') {
    return 12;
  }

  // ---------- Add Padding If Needed ---------- //
  // Lists and Blockquotes are padded on the left
  if (style == '>')
    startX += SPECIAL_PADDING;
  else if (style == '-' || style == 'L')
    startX += 2 * SPECIAL_PADDING;
  // Code blocks are padded on both sides
  else if (style == 'C')
    startX += (SPECIAL_PADDING / 2);

  // ---------- Render Text ---------- //
  bool bold = false;
  bool italic = false;
  int xpos = startX;  // Fix: initialize to startX to retain padding
  char temp[2] = {0, '\0'};

  // Calculate required vertical baseline shift
  int hpx = getCalculatedLineHeight(line);
  int baselineY = cursorY + getLineMaxHeight(line); 

  // Fix: Use a for loop so 'continue' doesn't skip the increment
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
      // Skip display for stars
      continue;
    }

    // Set the font
    const GFXfont* font = pickFont(style, bold, italic);
    display.setFont(font);

    // Draw text & add width
    temp[0] = c;
    display.setCursor(xpos, baselineY); // Fix: draw at baseline
    display.print(temp);

    // Add character space
    int16_t x1, y1;
    uint16_t charW, charH;
    if (c == ' ')
      display.getTextBounds(SPACEWIDTH_SYMBOL, 0, 0, &x1, &y1, &charW, &charH);
    else
      display.getTextBounds(temp, 0, 0, &x1, &y1, &charW, &charH);
    xpos += charW;

    // italics need to overlap a bit
    // if (italic && c != '*') xpos -= 3;
  }

  // ---------- Post-Render Formatting ---------- //
  // Blockquotes get a vertical line on the left
  if (style == '>') {
    display.drawFastVLine(SPECIAL_PADDING / 2, startY, hpx, GxEPD_BLACK);
    display.drawFastVLine((SPECIAL_PADDING / 2) + 1, startY, hpx, GxEPD_BLACK);
  }
  // Code Blocks get a vertical line on each side
  else if (style == 'C') {
    display.drawFastVLine(SPECIAL_PADDING / 4, startY, hpx, GxEPD_BLACK);
    display.drawFastVLine(display.width() - (SPECIAL_PADDING / 4), startY, hpx, GxEPD_BLACK);
    display.drawFastVLine((SPECIAL_PADDING / 4) + 1, startY, hpx, GxEPD_BLACK);
    display.drawFastVLine(display.width() - (SPECIAL_PADDING / 4) - 1, startY, hpx, GxEPD_BLACK);
  }
  // Headings get a horizontal line below them
  else if ((style == '1' || style == '2' || style == '3')) {
    display.drawFastHLine(0, startY + hpx - 2, display.width(), GxEPD_BLACK);
    display.drawFastHLine(0, startY + hpx - 3, display.width(), GxEPD_BLACK);
  }
  // Unordered Lists get a '●'
  else if (style == '-') {
    display.fillCircle(startX - 8, startY + (hpx / 2), 3, GxEPD_BLACK);
  }
  // Ordered Lists get their # (Needs orderedListNumber mapped if retaining feature)
  else if (style == 'L') {
    // Requires mapping orderedListNumber to current line
  }

  return hpx; // Fix: Return consumed height
}

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
    
    // Draw the line
    int heightUsed = drawLineEink(line, 0, currentY); 
    
    currentY += heightUsed;

    // Don't render if offscreen
    if (currentY >= display.height() + 20) {
      break;
    }
  }
}

void editor() {
  static uint16_t cursor_pos = 0;
  static int currentMillis = millis();
  static long lastInput = millis();
  bool currentlyTyping = false;

  Line& line = document.lines[currentLineNum];

  if (currentMillis - KBBounceMillis >= 10) {
    char inchar = KB().updateKeypress();

    // HANDLE INPUTS
    if (inchar != 0 ) lastInput = millis();

    // No char recieved
    if (inchar == 0) ;

    // CR Recieved
    else if (inchar == 13) {
      //cursor_pos = 0;
      updateScreen = true;
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

    // BKSP Recieved
    else if (inchar == 8) {
      deleteChar(line, cursor_pos);
    }

    // LEFT
    else if (inchar == 19) {
      if (cursor_pos > 0) {
        cursor_pos--;
      }
    }

    // RIGHT
    else if (inchar == 21) {
      if (cursor_pos < line.len) {
        cursor_pos++;
      }
    }

    // CENTER
    else if (inchar == 20) {
    }

    // SHIFT+LEFT
    else if (inchar == 28) {
      // Define the cycle order
      static const char styleCycle[] = {'T', '1', '2', '3', '>', 'L', '-', 'C', 'H'};
      static const int numStyles = sizeof(styleCycle) / sizeof(styleCycle[0]);

      // Find current style index
      int currentIndex = 0;
      for (int i = 0; i < numStyles; i++) {
        if (line.type == styleCycle[i]) {
          currentIndex = i;
          break;
        }
      }

      // Move to next style in cycle
      currentIndex = (currentIndex + 1) % numStyles;
      line.type = styleCycle[currentIndex];
    }

    // SHIFT+RIGHT
    else if (inchar == 30) {
      cycleTextStyle(line, cursor_pos);
    }

    // SHIFT+CENTER
    else if (inchar == 29) {
    }

    // FN+LEFT
    else if (inchar == 12) {
      HOME_INIT();
    }

    // FN+RIGHT
    else if (inchar == 6) {
    }

    // FN+CENTER
    else if (inchar == 7) {
    }

    // FN+SHIFT+LEFT
    else if (inchar == 24) {
    }

    // FN+SHIFT+RIGHT
    else if (inchar == 26) {
    }

    // FN+SHIFT+CENTER
    else if (inchar == 25) {
    }

    // TAB, SHIFT+TAB / FN+TAB, FN+SHIFT+TAB
    else if (inchar == 9 || inchar == 14) {
    }

    // Normal character input
    else {
      insertChar(line, cursor_pos, inchar);

      if (inchar < 48 || inchar > 57) {
        if (KB().getKeyboardState() != NORMAL) {
          KB().setKeyboardState(NORMAL);
        }
      }
    }

    currentMillis = millis();

    if (millis() - lastInput > IDLE_TIME) currentlyTyping = false;
    else currentlyTyping = true;

    // Make sure oled only updates at OLED_MAX_FPS
    if (currentMillis - OLEDFPSMillis >= (1000 / OLED_MAX_FPS)) {
      OLEDFPSMillis = currentMillis;
      // OLED().oledLine(line.text, cursor_pos, false);
      editorOledDisplay(line, cursor_pos, currentlyTyping);
    }
  }
}

#pragma region INIT
void initFonts() {
  // Mono
  fonts[mono].normal = &FreeMono9pt8b;
  fonts[mono].normal_B = &FreeMonoBold9pt8b;
  fonts[mono].normal_I = &FreeMonoOblique9pt8b;
  fonts[mono].normal_BI = &FreeMonoBoldOblique9pt8b;

  fonts[mono].h1 = &FreeMonoBold24pt8b;
  fonts[mono].h1_B = &FreeMonoBold24pt8b;  // Already bold
  fonts[mono].h1_I = &FreeMonoBoldOblique24pt8b;
  fonts[mono].h1_BI = &FreeMonoBoldOblique24pt8b;

  fonts[mono].h2 = &FreeMonoBold18pt8b;
  fonts[mono].h2_B = &FreeMonoBold18pt8b;
  fonts[mono].h2_I = &FreeMonoBoldOblique18pt8b;
  fonts[mono].h2_BI = &FreeMonoBoldOblique18pt8b;

  fonts[mono].h3 = &FreeMonoBold12pt8b;
  fonts[mono].h3_B = &FreeMonoBold12pt8b;
  fonts[mono].h3_I = &FreeMonoBoldOblique12pt8b;
  fonts[mono].h3_BI = &FreeMonoBoldOblique12pt8b;

  fonts[mono].code = &FreeMono9pt8b;
  fonts[mono].code_B = &FreeMono9pt8b;
  fonts[mono].code_I = &FreeMono9pt8b;
  fonts[mono].code_BI = &FreeMono9pt8b;

  fonts[mono].quote = &FreeMono9pt8b;
  fonts[mono].quote_B = &FreeMonoBold9pt8b;
  fonts[mono].quote_I = &FreeMonoOblique9pt8b;
  fonts[mono].quote_BI = &FreeMonoBoldOblique9pt8b;

  fonts[mono].list = &FreeMono9pt8b;
  fonts[mono].list_B = &FreeMonoBold9pt8b;
  fonts[mono].list_I = &FreeMonoOblique9pt8b;
  fonts[mono].list_BI = &FreeMonoBoldOblique9pt8b;

  // Serif
  fonts[serif].normal = &FreeSerif9pt8b;
  fonts[serif].normal_B = &FreeSerifBold9pt8b;
  fonts[serif].normal_I = &FreeSerifItalic9pt8b;
  fonts[serif].normal_BI = &FreeSerifBoldItalic9pt8b;

  fonts[serif].h1 = &FreeSerifBold24pt8b;
  fonts[serif].h1_B = &FreeSerifBold24pt8b;
  fonts[serif].h1_I = &FreeSerifBoldItalic24pt8b;
  fonts[serif].h1_BI = &FreeSerifBoldItalic24pt8b;

  fonts[serif].h2 = &FreeSerifBold18pt8b;
  fonts[serif].h2_B = &FreeSerifBold18pt8b;
  fonts[serif].h2_I = &FreeSerifBoldItalic18pt8b;
  fonts[serif].h2_BI = &FreeSerifBoldItalic18pt8b;

  fonts[serif].h3 = &FreeSerifBold12pt8b;
  fonts[serif].h3_B = &FreeSerifBold12pt8b;
  fonts[serif].h3_I = &FreeSerifBoldItalic12pt8b;
  fonts[serif].h3_BI = &FreeSerifBoldItalic12pt8b;

  fonts[serif].code = &FreeMono9pt8b;
  fonts[serif].code_B = &FreeMono9pt8b;
  fonts[serif].code_I = &FreeMono9pt8b;
  fonts[serif].code_BI = &FreeMono9pt8b;

  fonts[serif].quote = &FreeSerif9pt8b;
  fonts[serif].quote_B = &FreeSerifBold9pt8b;
  fonts[serif].quote_I = &FreeSerifItalic9pt8b;
  fonts[serif].quote_BI = &FreeSerifBoldItalic9pt8b;

  fonts[serif].list = &FreeSerif9pt8b;
  fonts[serif].list_B = &FreeSerifBold9pt8b;
  fonts[serif].list_I = &FreeSerifItalic9pt8b;
  fonts[serif].list_BI = &FreeSerifBoldItalic9pt8b;

  // Sans
  fonts[sans].normal = &FreeSans9pt8b;
  fonts[sans].normal_B = &FreeSansBold9pt8b;
  fonts[sans].normal_I = &FreeSansOblique9pt8b;
  fonts[sans].normal_BI = &FreeSansBoldOblique9pt8b;

  fonts[sans].h1 = &FreeSansBold24pt8b;
  fonts[sans].h1_B = &FreeSansBold24pt8b;
  fonts[sans].h1_I = &FreeSansBoldOblique24pt8b;
  fonts[sans].h1_BI = &FreeSansBoldOblique24pt8b;

  fonts[sans].h2 = &FreeSansBold18pt8b;
  fonts[sans].h2_B = &FreeSansBold18pt8b;
  fonts[sans].h2_I = &FreeSansBoldOblique18pt8b;
  fonts[sans].h2_BI = &FreeSansBoldOblique18pt8b;

  fonts[sans].h3 = &FreeSansBold12pt8b;
  fonts[sans].h3_B = &FreeSansBold12pt8b;
  fonts[sans].h3_I = &FreeSansBoldOblique12pt8b;
  fonts[sans].h3_BI = &FreeSansBoldOblique12pt8b;

  fonts[sans].code = &FreeMono9pt8b;
  fonts[sans].code_B = &FreeMono9pt8b;
  fonts[sans].code_I = &FreeMono9pt8b;
  fonts[sans].code_BI = &FreeMono9pt8b;

  fonts[sans].quote = &FreeSans9pt8b;
  fonts[sans].quote_B = &FreeSansBold9pt8b;
  fonts[sans].quote_I = &FreeSansOblique9pt8b;
  fonts[sans].quote_BI = &FreeSansBoldOblique9pt8b;

  fonts[sans].list = &FreeSans9pt8b;
  fonts[sans].list_B = &FreeSansBold9pt8b;
  fonts[sans].list_I = &FreeSansOblique9pt8b;
  fonts[sans].list_BI = &FreeSansBoldOblique9pt8b;
}

void TXT_INIT(String inPath) {
  /*
  if (inPath == "") loadMarkdownFile(PM_SDAUTO().getEditingFile());
  else {
    PM_SDAUTO().setEditingFile(inPath);
    loadMarkdownFile(inPath);
  }

  setFontStyle(serif);

  lineScroll = 0;
  updateScreen = true;
  */
  initFonts();
  loadMarkdownFile(inPath);
  CurrentAppState = TXT;
  CurrentTXTState_NEW = TXT_;
}

void TXT_INIT_JournalMode() {
  /*
  initFonts();

  String outPath = getCurrentJournal();
  if (!outPath.startsWith("/")) outPath = "/" + outPath;
  loadMarkdownFile(outPath);

  setFontStyle(serif);

  lineScroll = 0;
  updateScreen = true;
  CurrentAppState = TXT;
  CurrentTXTState_NEW = JOURNAL_MODE;
  */
}

#pragma region Loops
void einkHandler_TXT_NEW() {
  if (updateScreen) {
    updateScreen = false;
    display.setFullWindow();
    display.setTextColor(GxEPD_BLACK);
    editorEinkDisplay(document, currentLineNum);
    EINK().refresh();
  }
}

void processKB_TXT_NEW() {
  editor();
  /*OLED().setPowerSave(false);
  disableTimeout = false;
  String outPath = "";
  char inchar;

  unsigned long currentMillis = millis();

  switch (CurrentTXTState_NEW) {
    case TXT_:
      inchar = KB().updateKeypress();
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
        // update scroll
        if (TOUCH().updateScroll(getTotalDisplayLines(), lineScroll)) {
          updateScreen = true;
        }
        switch (currentEditMode) {
          case edit_append:
            editAppend(inchar);
            break;
          case edit_inline:

            break;
        }
      }
      break;
    case JOURNAL_MODE: // Stripped down version of TXT_ for journal
      inchar = KB().updateKeypress();
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
        // update scroll
        if (TOUCH().updateScroll(getTotalDisplayLines(), lineScroll)) {
          updateScreen = true;
        }
        switch (currentEditMode) {
          case edit_append:
            editAppend(inchar);
            break;
          case edit_inline:

            break;
        }
      }
      break;
    case SAVE_AS:
      inchar = KB().updateKeypress();
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
        // HANDLE INPUTS
        //No char recieved
        if (inchar == 0);
        //CR Recieved
        else if (inchar == 13) {
          if (currentLine != "" && currentLine != "-") {
            if (!currentLine.startsWith("/notes/")) currentLine = "/notes/" + currentLine;
            if (!currentLine.endsWith(".txt")) currentLine = currentLine + ".txt";
            saveMarkdownFile(currentLine);
            CurrentTXTState_NEW = TXT_;
          } else {
            OLED().oledWord("Invalid Name");
            delay(2000);
          }

          currentLine = "";
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
        //Space Recieved
        else if (inchar == 32) {
          // Spaces not allowed in filenames
        }
        //ESC / CLEAR Recieved
        else if (inchar == 20) {
          currentLine = "";
        }
        //BKSP Recieved
        else if (inchar == 8) {
          if (currentLine.length() > 0) {
            currentLine.remove(currentLine.length() - 1);
          }
        }
        // Home recieved
        else if (inchar == 12) {
          CurrentTXTState_NEW = TXT_;
        }
        else {
          currentLine += inchar;
          if (inchar >= 48 && inchar <= 57) {}  //Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        //Make sure oled only updates at OLED_MAX_FPS
        if (currentMillis - OLEDFPSMillis >= (1000/OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentLine, currentLine.length(), false, "Input Filename");
        }
      }
      break;
    case NEW_FILE:
      inchar = KB().updateKeypress();
      if (currentMillis - KBBounceMillis >= KB_COOLDOWN) {
        // HANDLE INPUTS
        //No char recieved
        if (inchar == 0);
        //CR Recieved
        else if (inchar == 13) {
          if (currentLine != "" && currentLine != "-") {
            if (!currentLine.startsWith("/notes/")) currentLine = "/notes/" + currentLine;
            if (!currentLine.endsWith(".txt")) currentLine = currentLine + ".txt";
            newMarkdownFile(currentLine);
            CurrentTXTState_NEW = TXT_;
          } else {
            OLED().oledWord("Invalid Name");
            delay(2000);
          }

          currentLine = "";
        }
        //SHIFT Recieved
        else if (inchar == 17) {
          if (KB().getKeyboardState() == SHIFT) KB().setKeyboardState(NORMAL);
          else KB().setKeyboardState(SHIFT);
        }
        //FN Recieved
        else if (inchar == 18) {
          if (KB().getKeyboardState() == FUNC) KB().setKeyboardState(NORMAL);
          else KB().setKeyboardState(FUNC);
        }
        //Space Recieved
        else if (inchar == 32) {
          // Spaces not allowed in filenames
        }
        //ESC / CLEAR Recieved
        else if (inchar == 20) {
          currentLine = "";
        }
        //BKSP Recieved
        else if (inchar == 8) {
          if (currentLine.length() > 0) {
            currentLine.remove(currentLine.length() - 1);
          }
        }
        // Home recieved
        else if (inchar == 12) {
          CurrentTXTState_NEW = TXT_;
        }
        else {
          currentLine += inchar;
          if (inchar >= 48 && inchar <= 57) {}  //Only leave FN on if typing numbers
          else if (KB().getKeyboardState() != NORMAL) {
            KB().setKeyboardState(NORMAL);
          }
        }

        currentMillis = millis();
        //Make sure oled only updates at OLED_MAX_FPS
        if (currentMillis - OLEDFPSMillis >= (1000/OLED_MAX_FPS)) {
          OLEDFPSMillis = currentMillis;
          OLED().oledLine(currentLine, currentLine.length(), false, "Input Name for New File");
        }
      }
      break;
    case LOAD_FILE:
      outPath = fileWizardMini(false, "/notes");
      if (outPath == "_EXIT_") {
        // Return to TXT
        CurrentTXTState_NEW = TXT_;
        //newState = true;
        break;
      }
      else if (outPath != "") {
        // Ensure file is a .txt or .md
        if (outPath.endsWith(".txt") || outPath.endsWith(".md")) {
          if (!outPath.startsWith("/")) outPath = "/" + outPath;
          loadMarkdownFile(outPath);
          PM_SDAUTO().setEditingFile(outPath);
          CurrentTXTState_NEW = TXT_;
          updateScreen = true;
        } else {
          OLED().oledWord("Incompatible Filetype!");
          delay(2000);
          CurrentTXTState_NEW = TXT_;
        }
      }
      break;
  }
  */
}
#endif
