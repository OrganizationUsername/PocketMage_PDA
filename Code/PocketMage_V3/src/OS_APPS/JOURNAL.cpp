// AUDIT 1

#include <globals.h>
#if !OTA_APP // POCKETMAGE_OS
enum JournalState {J_MENU, J_TXT};
JournalState CurrentJournalState = J_MENU;

String currentJournal = "";
String bufferEditingFile = PM_SDAUTO().getEditingFile();

void JOURNAL_INIT() {
  CurrentAppState = JOURNAL;
  CurrentJournalState = J_MENU;
  EINK().forceSlowFullUpdate(true);
  newState = true;
  KB().setKeyboardState(NORMAL);
  bufferEditingFile = PM_SDAUTO().getEditingFile();
}

// File Operations
void loadJournal() {
  PM_SDAUTO().setEditingFile(currentJournal);
  PM_SDAUTO().loadFile();
}

void saveJournal() {
  PM_SDAUTO().setEditingFile(currentJournal);
  PM_SDAUTO().saveFile();
}

String getCurrentJournal() {return currentJournal;}

// Functions
bool isLeapYear(int year) {
  return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

void drawJMENU() {
  SDActive = true;
  pocketmage::setCpuSpeed(240);
  delay(50);

  // Display background
  display.setTextColor(GxEPD_BLACK);
  EINK().drawStatusBar("Enter Date or (Tab)->Today");
  display.drawBitmap(0, 0, _journal, 320, 218, GxEPD_BLACK);

  // Update current progress graph
  DateTime now = CLOCK().nowDT();
  String year = String(now.year());

  // 1-indexed matrix to store which days have journal entries
  bool hasJournal[13][32] = {false}; 

  // Read the directory exactly ONCE to prevent SD Card / VFS thrashing
  File dir = global_fs->open("/journal");
  if (dir && dir.isDirectory()) {
    File entry;
    while ((entry = dir.openNextFile())) {
      if (!entry.isDirectory()) {
        String name = entry.name();
        
        // Handle potential leading slash from filesystem driver
        if (name.startsWith("/")) name = name.substring(1);
        
        // Ensure format is exactly YYYYMMDD.txt (12 chars)
        if (name.length() == 12 && name.endsWith(".txt")) {
          if (name.substring(0, 4) == year) {
            int m = name.substring(4, 6).toInt();
            int d = name.substring(6, 8).toInt();
            // Bound checking before writing to array
            if (m >= 1 && m <= 12 && d >= 1 && d <= 31) {
              hasJournal[m][d] = true;
            }
          }
        }
      }
      entry.close();
    }
    dir.close();
  }

  // Pre-calculated Y offsets for each month row
  int yOffsets[12] = {50, 59, 68, 77, 86, 95, 104, 113, 122, 131, 140, 149};
  int daysInMonths[12] = {
    31, isLeapYear(now.year()) ? 29 : 28, 31, 30, 31, 30, 
    31, 31, 30, 31, 30, 31
  };

  // Plot the graph from the matrix
  for (int m = 1; m <= 12; m++) {
    for (int d = 1; d <= daysInMonths[m - 1]; d++) {
      if (hasJournal[m][d]) {
        display.fillRect(91 + (7 * (d - 1)), yOffsets[m - 1], 4, 4, GxEPD_BLACK);
      }
    }
  }

  if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
  SDActive = false;
}

// Loops
void processKB_JOURNAL() {
  // We only want to trigger the prompt AFTER the e-ink screen has finished 
  // drawing the background graph, which is indicated by newState becoming false.
  if (CurrentJournalState == J_MENU && !newState) {
    
    // Instantly launch the date prompt! Blocks here until Tab or Enter is pressed.
    String uiDate = datePrompt("");

    // Safety check: If the user hit the physical HOME button to exit the app while 
    // the prompt was open, CurrentAppState will have changed globally. Abort.
    if (CurrentAppState != JOURNAL) return;

    if (uiDate.length() == 10) {
      SDActive = true;
      pocketmage::setCpuSpeed(240);
      delay(50);

      // Convert DD/MM/YYYY from datePrompt into YYYYMMDD for the file system
      String d = uiDate.substring(0, 2);
      String m = uiDate.substring(3, 5);
      String y = uiDate.substring(6, 10);
      String fileName = "/journal/" + y + m + d + ".txt";

      // If file doesn't exist, create it
      if (!global_fs->exists(fileName)) {
        File f = global_fs->open(fileName, FILE_WRITE);
        if (f) f.close();
      }

      currentJournal = fileName;

      if (SAVE_POWER) pocketmage::setCpuSpeed(POWER_SAVE_FREQ);
      SDActive = false;
      
      // Pass execution to the text editor
      TXT_INIT_JournalMode();
    }
  }
}

void einkHandler_JOURNAL() {
  switch (CurrentJournalState) {
    case J_MENU:
      if (newState) {
        newState = false;

        EINK().resetDisplay(); // Clear the uninitialized buffer correctly
        drawJMENU();

        EINK().multiPassRefresh(2);
      }
      break;
    default:
      CurrentJournalState = J_MENU;
      break;
  } 
}
#endif