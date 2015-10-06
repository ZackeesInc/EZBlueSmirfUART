// API
// BT_BondPrivatelyWith(const char* addr1, const char* addr2):
//  This function bonds the two bluetooth devices together. One address MUST
//  match the bluetooth address of the attached unit and the other address matches the connect to
//  bluetooth. The order of the addresses does not matter. The rationale for
//  using two addresses is for error detect - it is a common use case to
//  put in the wrong address and having both address means that during the programming phase
//  if there is a mismatch between then the failure condition is printed rather
//  than silently fail.
// BT_Available():
//  This function/macro will return true if the bluetooth stream has characters to read, false
//  otherwise. It is functionally equivalent to Serial.available() usage.
// BT_Read():
//  This function/macro will return an int representing the character read, or -1 if the stream
//  not available. It is functionally equivalent to Serial.read() in usage.
// BT_Print(str):
//  This function/macro will print a character to the bluetooth stream and send it over the wire.
//  It is functionally equivalent to Serial.print() in usage.
// BT_IsConnected():
//  This function will check whether the current device is connected or not. Warning that this function is
//  very slow and should be used very sparingly or not at all.
// BT_NOVERBOSE:
//  When defined this will silence verbose output and the connection state as the device attempts to pair.

#include "Arduino.h"

#ifdef BT_NOVERBOSE
# define BT_DBG_PRINT(X)
# define BT_DBG_PRINTLN(X)
#else
# define BT_DBG_PRINT(X) Serial.print(X)
# define BT_DBG_PRINTLN(X) Serial.println(X)
#endif

#define BT_ERR_PRINT(X) Serial.print(X)
#define BT_ERR_PRINTLN(X) Serial.println(X)

struct CharStr {
  static const int n = 50;
  char data[n];
  CharStr() { memset(this->data, 0, n); }
};

#define BT_Available() Bluetooth.available()
#define BT_Read() Bluetooth.read()
#define BT_Print(X) Bluetooth.print(X)

// Functions contained in this utility collection.
void BT_Setup();
void BT_WaitUntil(const char* str, unsigned long time_ms);
void BT_EnterCmdModeAndPrintSettingsAndExit();
void BT_FlushPrint(unsigned long time_wait_ms = 200ul);

bool BT_IsConnected();

void BT_Send(const char* msg);
void BT_CmdResponse(const char* cmd, const char* resp, unsigned long time_ms = 4000ul);

bool BT_ReceiveResponse(CharStr* cmd, unsigned long timeout_ms = 4000ul);

bool BT_BondPrivatelyWith(const char* bt_addr1, const char* bt_addr2);
void BT_Echo();



//------------ IMPL ---------------//


bool BT_IsConnected() {
  BT_CmdResponse("$$$", "CMD", 1000ul);
  BT_Send("GK\n");
  
  CharStr conn;
  BT_ReceiveResponse(&conn);
  bool is_connected = (0 == stricmp(conn.data, "1,0,0"));
  
  BT_CmdResponse("---\n", "END");
  return is_connected;
}

void BT_Setup() {
  delay(1000);
  
  // Bluetooth setup.
  Bluetooth.begin(115200);  // Default speed  
}

void BT_Send(const char* msg) {
  BT_FlushPrint();
  BT_DBG_PRINT("> "); BT_DBG_PRINT(msg); Bluetooth.print(msg); BT_DBG_PRINTLN();
}

void BT_Echo() {
  
  if(BT_Available()) {  // If the bluetooth sent any characters
    // Send any characters the bluetooth prints to the serial monitor
    char c = (char)Bluetooth.read();
    
    BT_DBG_PRINT((char)c);  
  }
  if(Serial.available()) {  // If stuff was typed in the serial monitor
    BT_Print((char)Serial.read());
  }
}

bool BT_ReceiveResponse(CharStr* dest, unsigned long timeout_ms) {
  unsigned long start_ms = millis();
  int pos = 0;
  
  while (true) {
    if (pos >= dest->n) {
      BT_ERR_PRINT(__FUNCTION__); BT_ERR_PRINT(": Error - overflow");
      return false;
    }
    
    unsigned long delta_ms = millis() - start_ms;
    // Exit subroutine for timeout.
    if (delta_ms > timeout_ms) {
      BT_ERR_PRINT(__FUNCTION__); BT_ERR_PRINTLN(": Error - timeout waiting for response");
      return false;
    }
    
    // Not ready yet so defer.
    if (!Bluetooth.available()) { continue; }
    
    const char c = (char)Bluetooth.read();
    // If terminating character.
    if (('\n' == c) || ('\r' == c)) {
      dest->data[pos] = '\0';
      BT_DBG_PRINTLN(dest->data);
      return true;
    } else {
      // Copy otherwise and continue loop.
      dest->data[pos] = c;
      pos++;
    }
  }
}

void BT_CmdResponse(const char* cmd, const char* resp, unsigned long time_ms) {
  BT_Send(cmd);
  BT_WaitUntil(resp, time_ms);
}

void BT_WaitUntil(const char* str, unsigned long time_ms) {
  unsigned long start_ms = millis();
  int n = strlen(str);
  
  const char* start = str;
  const char* end = str + n;
  
  for (const char* it = start; it != end;) {
    unsigned long delta_ms = millis() - start_ms;
    if (delta_ms > time_ms) {
      BT_ERR_PRINT("Error, timeout in bluetooth while waiting for "); BT_ERR_PRINTLN(str);
      return;
    }
    
    if (!Bluetooth.available()) { continue; }
    
    const char in = (char)Bluetooth.read();
    if (in == *it) {
      it++;
    }
  }
  
  BT_DBG_PRINT(str);
  BT_FlushPrint();
}

void BT_FlushPrint(unsigned long time_wait_ms) {
  unsigned long start_ms = millis();
  
  int count = 0;
  char last_char = 0;
  while (true) {
    // keep reseting timer when there is more data to consume.
    if (Bluetooth.available()) { start_ms = millis(); }
    unsigned long delta_ms = millis() - start_ms;
    
    if (delta_ms > time_wait_ms) { break; }
    
    if (Bluetooth.available()) {
      ++count;
      char c = (char)Bluetooth.read();
      last_char = c;
      BT_DBG_PRINT(c);
    }
  }
  
  // Add new line if not present.
  if ((count > 0) && (last_char != '\n')) {
    BT_DBG_PRINTLN();
  }
}

void BT_EnterCmdModeAndPrintSettingsAndExit() {
  BT_Send("$$$");  // Print three times individually
  BT_WaitUntil("CMD", 1000ul);      // Expected Response.

  
  BT_FlushPrint();

  BT_Send("D\n");  // Settings
  BT_FlushPrint();
  BT_DBG_PRINTLN("");  // \n printed
  BT_Send("E\n");  // Advanced settings.
  BT_FlushPrint();
  
  // Get out of cmd mode.
  BT_Send("---\n");  // Print three times individually
  BT_WaitUntil("END", 1000ul);
  
  BT_DBG_PRINTLN("\nFinished Bluetooth settings print and successfully exited");
}

bool BT_BondPrivatelyWith(const char* bt_addr1, const char* bt_addr2) {
  BT_Setup();
  BT_CmdResponse("$$$", "CMD", 1000ul);
  BT_CmdResponse("SF,1\n", "AOK", 5000ul);
  //BT_CmdResponse("SO,%\n", "AOK", 5000ul);
  BT_CmdResponse("SA,4\n", "AOK");
  BT_CmdResponse("SE,1\n", "AOK");
  
  BT_Send("GB\n");
  CharStr rec_addr;
  BT_ReceiveResponse(&rec_addr);

  
  const char* connect_to_addr = NULL;
  if (0 == stricmp(rec_addr.data, bt_addr1)) {
    connect_to_addr = bt_addr2;
  } else if(0 == stricmp(rec_addr.data, bt_addr2)) {
    connect_to_addr = bt_addr1;
  } else {
    BT_DBG_PRINT(__FUNCTION__); BT_DBG_PRINTLN(": Error Bluetooth bonding address is incorrect");
    BT_DBG_PRINT("\trec_addr: \""); BT_DBG_PRINT(rec_addr.data); BT_DBG_PRINTLN("\"");
    BT_DBG_PRINT("\tbt_addr1: \""); BT_DBG_PRINT(bt_addr1); BT_DBG_PRINTLN("\"");
    BT_DBG_PRINT("\tbt_addr2: \""); BT_DBG_PRINT(bt_addr2); BT_DBG_PRINTLN("\"");
    return false;
  }
  
  
  char buf[50];
  sprintf(buf, "SR,%s\n", connect_to_addr);
  BT_CmdResponse(buf, "AOK");
  delay(1000);
  BT_CmdResponse("SM,6\n", "AOK");
  BT_CmdResponse("SP,c0de\n", "AOK");
  BT_CmdResponse("R,1\n", "Reboot!", 6000ul);
  delay(1000);

  BT_CmdResponse("$$$", "CMD", 2000ul);


  BT_Send("D\n");
  BT_FlushPrint(1000ul);
  BT_CmdResponse("---\n", "END");
  BT_FlushPrint();

  return true;
}
