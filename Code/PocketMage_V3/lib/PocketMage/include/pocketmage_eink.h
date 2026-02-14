//  oooooooooooo         ooooo ooooo      ooo oooo    oooo  //
//  `888'     `8         `888' `888b.     `8' `888   .8P'   //
//   888                  888   8 `88b.    8   888  d8'     //
//   888oooo8    8888888  888   8   `88b.  8   88888[       //
//   888    "             888   8     `88b.8   888`88b.     //
//   888       o          888   8       `888   888  `88b.   //
//  o888ooooood8         o888o o8o        `8  o888o  o888o  //

#pragma once
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <config.h>  // for FULL_REFRESH_AFTER

#include <vector>
#pragma region fonts
// FONTS
// Small
#include "Fonts/Font3x7FixedNum.h"
#include "Fonts/Font4x5Fixed.h"
#include "Fonts/Font5x7Fixed.h"

// 9x8
#include <Fonts/FreeMono9pt8b.h>
#include <Fonts/FreeMonoBold9pt8b.h>
// #include <Fonts/FreeMonoBoldItalic9pt8b.h>
#include <Fonts/FreeMonoBoldOblique9pt8b.h>
// #include <Fonts/FreeMonoItalic9pt8b.h>
#include <Fonts/FreeMonoOblique9pt8b.h>
#include <Fonts/FreeSans9pt8b.h>
#include <Fonts/FreeSansBold9pt8b.h>
// #include <Fonts/FreeSansBoldItalic9pt8b.h>
#include <Fonts/FreeSansBoldOblique9pt8b.h>
// #include <Fonts/FreeSansItalic9pt8b.h>
#include <Fonts/FreeSansOblique9pt8b.h>
#include <Fonts/FreeSerif9pt8b.h>
#include <Fonts/FreeSerifBold9pt8b.h>
#include <Fonts/FreeSerifBoldItalic9pt8b.h>
// #include <Fonts/FreeSerifBoldOblique9pt8b.h>
#include <Fonts/FreeSerifItalic9pt8b.h>
// #include <Fonts/FreeSerifOblique9pt8b.h>

// 12x8
#include <Fonts/FreeMono12pt8b.h>
#include <Fonts/FreeMonoBold12pt8b.h>
// #include <Fonts/FreeMonoBoldItalic12pt8b.h>
#include <Fonts/FreeMonoBoldOblique12pt8b.h>
// #include <Fonts/FreeMonoItalic12pt8b.h>
// #include <Fonts/FreeMonoOblique12pt8b.h>
#include <Fonts/FreeSans12pt8b.h>
#include <Fonts/FreeSansBold12pt8b.h>
// #include <Fonts/FreeSansBoldItalic12pt8b.h>
#include <Fonts/FreeSansBoldOblique12pt8b.h>
// #include <Fonts/FreeSansItalic12pt8b.h>
// #include <Fonts/FreeSansOblique12pt8b.h>
#include <Fonts/FreeSerif12pt8b.h>
#include <Fonts/FreeSerifBold12pt8b.h>
#include <Fonts/FreeSerifBoldItalic12pt8b.h>
// #include <Fonts/FreeSerifBoldOblique12pt8b.h>
// #include <Fonts/FreeSerifItalic12pt8b.h>
// #include <Fonts/FreeSerifOblique12pt8b.h>

// 18x8
// #include <Fonts/FreeMono18pt8b.h>
#include <Fonts/FreeMonoBold18pt8b.h>
// #include <Fonts/FreeMonoBoldItalic18pt8b.h>
#include <Fonts/FreeMonoBoldOblique18pt8b.h>
// #include <Fonts/FreeMonoItalic18pt8b.h>
// #include <Fonts/FreeMonoOblique18pt8b.h>
// #include <Fonts/FreeSans18pt8b.h>
#include <Fonts/FreeSansBold18pt8b.h>
// #include <Fonts/FreeSansBoldItalic18pt8b.h>
#include <Fonts/FreeSansBoldOblique18pt8b.h>
// #include <Fonts/FreeSansItalic18pt8b.h>
// #include <Fonts/FreeSansOblique18pt8b.h>
// #include <Fonts/FreeSerif18pt8b.h>
#include <Fonts/FreeSerifBold18pt8b.h>
#include <Fonts/FreeSerifBoldItalic18pt8b.h>
// #include <Fonts/FreeSerifBoldOblique18pt8b.h>
// #include <Fonts/FreeSerifItalic18pt8b.h>
// #include <Fonts/FreeSerifOblique18pt8b.h>

// 24x8
// #include <Fonts/FreeMono24pt8b.h>
#include <Fonts/FreeMonoBold24pt8b.h>
// #include <Fonts/FreeMonoBoldItalic24pt8b.h>
#include <Fonts/FreeMonoBoldOblique24pt8b.h>
// #include <Fonts/FreeMonoItalic24pt8b.h>
// #include <Fonts/FreeMonoOblique24pt8b.h>
// #include <Fonts/FreeSans24pt8b.h>
#include <Fonts/FreeSansBold24pt8b.h>
// #include <Fonts/FreeSansBoldItalic24pt8b.h>
#include <Fonts/FreeSansBoldOblique24pt8b.h>
// #include <Fonts/FreeSansItalic24pt8b.h>
// #include <Fonts/FreeSansOblique24pt8b.h>
// #include <Fonts/FreeSerif24pt8b.h>
#include <Fonts/FreeSerifBold24pt8b.h>
#include <Fonts/FreeSerifBoldItalic24pt8b.h>
// #include <Fonts/FreeSerifBoldOblique24pt8b.h>
// #include <Fonts/FreeSerifItalic24pt8b.h>
// #include <Fonts/FreeSerifOblique24pt8b.h>

#pragma endregion

// Type alias for readability
using PanelT = GxEPD2_310_GDEQ031T10;
using DisplayT = GxEPD2_BW<PanelT, PanelT::HEIGHT>;

// E-ink display
extern DisplayT display;

extern void einkHandler(void* parameter);

// ===================== EINK CLASS =====================
class PocketmageEink {
 public:
  explicit PocketmageEink(DisplayT& display) : display_(display) {}

  // Wire up external buffers/state used to read from globals
  void setLineSpacing(uint8_t lineSpacing) {
    lineSpacing_ = lineSpacing;
  };  // reference to lineSpacing (default 6)
  void setFullRefreshAfter(uint8_t fullRefreshAfter) {
    fullRefreshAfter_ = fullRefreshAfter;
  };  // reference to FULL_REFRESH_AFTER (default 5)
  void setCurrentFont(const GFXfont* font) {
    if (currentFont_ == font)
      return;
    currentFont_ = font;
  };  // reference to currentFont

  // Main display functions
  void refresh();
  void multiPassRefresh(int passes);
  void setFastFullRefresh(bool setting);
  void statusBar(const String& input, bool fullWindow = false);
  void drawStatusBar(const String& input);
  void computeFontMetrics_();
  void setTXTFont(const GFXfont* font);
  void einkTextDynamic(bool doFull, bool noRefresh = false);
  void resetDisplay(bool clearScreen = true, uint16_t color = GxEPD_WHITE);
  int countLines(const String& input, size_t maxLineLength = 29);
  uint16_t getEinkTextWidth(const String& s);

  // getters To Do: migrate definitions here from pocketmage_eink.cpp
  uint8_t maxCharsPerLine() const;
  uint8_t maxLines() const;
  const GFXfont* getCurrentFont();
  uint8_t getFontHeight() {
    return fontHeight_;
  };
  uint8_t getLineSpacing() {
    return lineSpacing_;
  };
  DisplayT& getDisplay() {
    return display_;
  };
  void forceSlowFullUpdate(bool force);

 private:
  DisplayT& display_;  // class reference to hardware display object
  bool forceSlowFullUpdate_ = false;
  uint8_t partialCounter_ = 0;
  const GFXfont* currentFont_ = nullptr;
  uint8_t fullRefreshAfter_ = FULL_REFRESH_AFTER;

  // font metrics
  uint8_t lineSpacing_ = 6;
  uint8_t maxCharsPerLine_ = 0;
  uint8_t maxLines_ = 0;
  uint8_t fontHeight_ = 0;
};

void setupEink();

PocketmageEink& EINK();