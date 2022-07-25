//---------------------------------------------------------------------------
// Copyright (C) 2002-2003 Dallas Semiconductor Corporation, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Dallas Semiconductor
// shall not be used except as stated in the Dallas Semiconductor
// Branding Policy.
// ---------------------------------------------------------------------------
//
//  an192.C - Application Note 192 example implementation using Win32.
//


#include <windows.h>
#include <stdio.h>

// definitions
#define FALSE                          0
#define TRUE                           1

#define WRITE_FUNCTION                 0
#define READ_FUNCTION                  1

// Mode Commands
#define MODE_DATA                      0xE1
#define MODE_COMMAND                   0xE3
#define MODE_STOP_PULSE                0xF1

// Return byte value
#define RB_CHIPID_MASK                 0x1C
#define RB_RESET_MASK                  0x03
#define RB_1WIRESHORT                  0x00
#define RB_PRESENCE                    0x01
#define RB_ALARMPRESENCE               0x02
#define RB_NOPRESENCE                  0x03

#define RB_BIT_MASK                    0x03
#define RB_BIT_ONE                     0x03
#define RB_BIT_ZERO                    0x00

// Masks for all bit ranges
#define CMD_MASK                       0x80
#define FUNCTSEL_MASK                  0x60
#define BITPOL_MASK                    0x10
#define SPEEDSEL_MASK                  0x0C
#define MODSEL_MASK                    0x02
#define PARMSEL_MASK                   0x70
#define PARMSET_MASK                   0x0E

// Command or config bit
#define CMD_COMM                       0x81
#define CMD_CONFIG                     0x01

// Function select bits
#define FUNCTSEL_BIT                   0x00
#define FUNCTSEL_SEARCHON              0x30
#define FUNCTSEL_SEARCHOFF             0x20
#define FUNCTSEL_RESET                 0x40
#define FUNCTSEL_CHMOD                 0x60

// Bit polarity/Pulse voltage bits
#define BITPOL_ONE                     0x10
#define BITPOL_ZERO                    0x00
#define BITPOL_5V                      0x00
#define BITPOL_12V                     0x10

// One Wire speed bits
#define SPEEDSEL_STD                   0x00
#define SPEEDSEL_FLEX                  0x04
#define SPEEDSEL_OD                    0x08
#define SPEEDSEL_PULSE                 0x0C

// Data/Command mode select bits
#define MODSEL_DATA                    0x00
#define MODSEL_COMMAND                 0x02

// 5V Follow Pulse select bits 
#define PRIME5V_TRUE                   0x02
#define PRIME5V_FALSE                  0x00

// Parameter select bits
#define PARMSEL_PARMREAD               0x00
#define PARMSEL_SLEW                   0x10
#define PARMSEL_12VPULSE               0x20
#define PARMSEL_5VPULSE                0x30
#define PARMSEL_WRITE1LOW              0x40
#define PARMSEL_SAMPLEOFFSET           0x50
#define PARMSEL_ACTIVEPULLUPTIME       0x60
#define PARMSEL_BAUDRATE               0x70

// Pull down slew rate.
#define PARMSET_Slew15Vus              0x00
#define PARMSET_Slew2p2Vus             0x02
#define PARMSET_Slew1p65Vus            0x04
#define PARMSET_Slew1p37Vus            0x06
#define PARMSET_Slew1p1Vus             0x08
#define PARMSET_Slew0p83Vus            0x0A
#define PARMSET_Slew0p7Vus             0x0C
#define PARMSET_Slew0p55Vus            0x0E

// 12V programming pulse time table
#define PARMSET_32us                   0x00
#define PARMSET_64us                   0x02
#define PARMSET_128us                  0x04
#define PARMSET_256us                  0x06
#define PARMSET_512us                  0x08
#define PARMSET_1024us                 0x0A
#define PARMSET_2048us                 0x0C
#define PARMSET_infinite               0x0E

// 5V strong pull up pulse time table
#define PARMSET_16p4ms                 0x00
#define PARMSET_65p5ms                 0x02
#define PARMSET_131ms                  0x04
#define PARMSET_262ms                  0x06
#define PARMSET_524ms                  0x08
#define PARMSET_1p05s                  0x0A
#define PARMSET_dynamic                0x0C
#define PARMSET_infinite               0x0E

// Write 1 low time
#define PARMSET_Write8us               0x00
#define PARMSET_Write9us               0x02
#define PARMSET_Write10us              0x04
#define PARMSET_Write11us              0x06
#define PARMSET_Write12us              0x08
#define PARMSET_Write13us              0x0A
#define PARMSET_Write14us              0x0C
#define PARMSET_Write15us              0x0E

// Data sample offset and Write 0 recovery time
#define PARMSET_SampOff3us             0x00
#define PARMSET_SampOff4us             0x02
#define PARMSET_SampOff5us             0x04
#define PARMSET_SampOff6us             0x06
#define PARMSET_SampOff7us             0x08
#define PARMSET_SampOff8us             0x0A
#define PARMSET_SampOff9us             0x0C
#define PARMSET_SampOff10us            0x0E

// Active pull up on time
#define PARMSET_PullUp0p0us            0x00
#define PARMSET_PullUp0p5us            0x02
#define PARMSET_PullUp1p0us            0x04
#define PARMSET_PullUp1p5us            0x06
#define PARMSET_PullUp2p0us            0x08
#define PARMSET_PullUp2p5us            0x0A
#define PARMSET_PullUp3p0us            0x0C
#define PARMSET_PullUp3p5us            0x0E

// Baud rate bits
#define PARMSET_9600                   0x00
#define PARMSET_19200                  0x02
#define PARMSET_57600                  0x04
#define PARMSET_115200                 0x06

// DS2480B program voltage available
#define DS2480BPROG_MASK                0x20

// mode bit flags
#define MODE_NORMAL                    0x00
#define MODE_OVERDRIVE                 0x01
#define MODE_STRONG5                   0x02
#define MODE_PROGRAM                   0x04
#define MODE_BREAK                     0x08

#define MAX_BAUD                       PARMSET_115200

// Basic 1-Wire functions
int  OWReset();
unsigned char OWTouchBit(unsigned char sendbit);
unsigned char OWTouchByte(unsigned char sendbyte);
void OWWriteByte(unsigned char byte_value);
void OWWriteBit(unsigned char bit_value);
unsigned char OWReadBit();
unsigned char OWReadByte();
int OWBlock(unsigned char *tran_buf, int tran_len);
int  OWSearch();
int  OWFirst();
int  OWNext();
int  OWVerify();
void OWTargetSetup(unsigned char family_code);
void OWFamilySkipSetup();

// Extended 1-Wire functions
int OWSpeed(int new_speed);
int OWLevel(int level);
int OWProgramPulse(void);
int OWWriteBytePower(int sendbyte);
int OWReadBitPower(int applyPowerResponse);

// DS2480B utility functions
int DS2480B_Detect(void);
int DS2480B_ChangeBaud(unsigned char newbaud);

// UART connectivity functions (Win32 implementation)
void FlushCOM(void);
int  WriteCOM(int outlen, unsigned char *outbuf);
int  ReadCOM(int inlen, unsigned char *inbuf);
void BreakCOM(void);
void SetBaudCOM(unsigned char new_baud);
void msDelay(int len);
int  OpenCOM(char *port_zstr);
void CloseCOM(void);

// misc utility functions
unsigned char docrc8(unsigned char value);
int bitacc(int op, int state, int loc, unsigned char *buf);

// search state
unsigned char ROM_NO[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
unsigned char crc8;

// DS2480B state
int ULevel; // 1-Wire level
int UBaud;  // baud rate
int UMode;  // command or data mode state
int USpeed; // 1-Wire communication speed
int ALARM_RESET_COMPLIANCE = FALSE; // flag for DS1994/DS2404 'special' reset  

// Win32 serial globals
HANDLE ComID;
OVERLAPPED osRead,osWrite;

// debug mode
int dodebug=FALSE;

//--------------------------------------------------------------------------
// TEST BUILD MAIN
//
int main(short argc, char **argv)
{
   int rslt,i,cnt;
   unsigned char sendpacket[10];
   int sendlen=0;

   // WIN32 SETUP
   // setup the port
   if (OpenCOM("COM1") != TRUE)
   // alternate PORT naming: 
   //      if (OpenCOM("\\\\.\\COM1") != TRUE)
   {
      printf("Failed to open port\n");
      exit(0);
   }
   // END WIN32 SETUP

   // enable debug if anything on command line
   if (argc > 1)
      dodebug = TRUE; 

   // verify the DS2480B is present
   if (!DS2480B_Detect())
   {
      printf("Failed to find and setup DS2480B\n");
      exit(0);
   }

   // find ALL devices
   printf("\nFIND ALL\n");
   cnt = 0;
   rslt = OWFirst();
   while (rslt)
   {
      // print device found
      for (i = 7; i >= 0; i--)
         printf("%02X", ROM_NO[i]);
      printf("  %d\n",++cnt);

      rslt = OWNext();
   }

   // find only 0x1A
   printf("\nFIND ONLY 0x1A\n");
   cnt = 0;
   OWTargetSetup(0x1A);
   while (OWNext())
   {
      // check for incorrect type
      if (ROM_NO[0] != 0x1A)
         break;
      
      // print device found
      for (i = 7; i >= 0; i--)
         printf("%02X", ROM_NO[i]);
      printf("  %d\n",++cnt);
   }

   // find all but 0x04, 0x1A, 0x23, and 0x01
   printf("\nFIND ALL EXCEPT 0x10, 0x04, 0x0A, 0x1A, 0x23, 0x01\n");
   cnt = 0;
   rslt = OWFirst();
   while (rslt)
   {
      // check for incorrect type
      if ((ROM_NO[0] == 0x04) || (ROM_NO[0] == 0x1A) || 
          (ROM_NO[0] == 0x01) || (ROM_NO[0] == 0x23) ||
          (ROM_NO[0] == 0x0A) || (ROM_NO[0] == 0x10))
         OWFamilySkipSetup();
      else
      {
         // print device found
         for (i = 7; i >= 0; i--)
            printf("%02X", ROM_NO[i]);
         printf("  %d\n",++cnt);
      }

      rslt = OWNext();
   }

   // find a DS1920 
   printf("\nFind DS1920/DS1820 and do a conversion\n");
   OWTargetSetup(0x10);
   if (OWNext())
   {
      // verify correct type
      if (ROM_NO[0] == 0x10)
      {
         // print device found
         for (i = 7; i >= 0; i--)
            printf("%02X", ROM_NO[i]);
         printf("\n");

         // device already selected from search
         // send the convert command
         if (!OWWriteBytePower(0x44))
            printf("Fail convert command\n");

         // sleep for 1 second
         msDelay(1000);

         // turn off the 1-Wire Net strong pull-up
         OWLevel(MODE_NORMAL); 

         // verify complete
         if (OWReadByte() != 0xFF)
            printf("ERROR, temperature conversion was not complete\n");

         // select the device
         sendpacket[0] = 0x55; // match command
         for (i = 0; i < 8; i++)
            sendpacket[i+1] = ROM_NO[i];

         // Reset 1-Wire 
         if (OWReset())
         {
            // MATCH ROM sequence
            OWBlock(sendpacket,9);

            // Read Scratch pad
            sendlen = 0;
            sendpacket[sendlen++] = 0xBE;
            for (i = 0; i < 9; i++)
               sendpacket[sendlen++] = 0xFF;

            if (OWBlock(sendpacket,sendlen))
            {
               printf("Scatchpad result = ");
               for (i = 0; i < sendlen; i++)
                  printf("%02X",sendpacket[i]);
               printf("\n");
            }
            else
               printf("ERROR\n");
         }
         else
            printf("NO RESET\n");
      }
   }

   // WIN32 CLEANUP
   // release the port
   CloseCOM();
   // WIN32 CLEANUP
}

//---------------------------------------------------------------------------
//-------- Basic 1-Wire functions
//---------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Reset all of the devices on the 1-Wire Net and return the result.
//
// Returns: TRUE(1):  presense pulse(s) detected, device(s) reset
//          FALSE(0): no presense pulses detected
//
// WARNING: Without setting the above global (FAMILY_CODE_04_ALARM_TOUCHRESET_COMPLIANCE)
//          to TRUE, this routine will not function correctly on some
//          Alarm reset types of the DS1994/DS1427/DS2404 with
//          Rev 1,2, and 3 of the DS2480/DS2480B. 
//          
//
int OWReset(void)
{
   unsigned char readbuffer[10],sendpacket[10];
   unsigned char sendlen=0;

   // make sure normal level
   OWLevel(MODE_NORMAL);

   // check for correct mode
   if (UMode != MODSEL_COMMAND)
   {
      UMode = MODSEL_COMMAND;
      sendpacket[sendlen++] = MODE_COMMAND;
   }

   // construct the command
   sendpacket[sendlen++] = (unsigned char)(CMD_COMM | FUNCTSEL_RESET | USpeed);

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the 1 byte response
      if (ReadCOM(1,readbuffer) == 1)
      {
         // check for special reset 
         if(ALARM_RESET_COMPLIANCE)
         {
            msDelay(5); // delay 5 ms to give DS1994 enough time
            FlushCOM();
            return TRUE;
        }

         // make sure this byte looks like a reset byte
         if (((readbuffer[0] & RB_RESET_MASK) == RB_PRESENCE) ||
             ((readbuffer[0] & RB_RESET_MASK) == RB_ALARMPRESENCE))
            return TRUE;
      }
   }

   // an error occured so re-sync with DS2480B
   DS2480B_Detect();

   return FALSE;
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net.
// The parameter 'sendbit' least significant bit is used.
//
// 'sendbit' - 1 bit to send (least significant byte)
//
void OWWriteBit(unsigned char sendbit)
{
   OWTouchBit(sendbit);
}

//--------------------------------------------------------------------------
// Send 8 bits of read communication to the 1-Wire Net and and return the
// result 8 bits read from the 1-Wire Net.
//
// Returns:  8 bits read from 1-Wire Net
//
unsigned char OWReadBit(void)
{
   return OWTouchBit(0x01);
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and return the
// result 1 bit read from the 1-Wire Net.  The parameter 'sendbit'
// least significant bit is used and the least significant bit
// of the result is the return bit.
//
// 'sendbit' - the least significant bit is the bit to send
//
// Returns: 0:   0 bit read from sendbit
//          1:   1 bit read from sendbit
//
unsigned char OWTouchBit(unsigned char sendbit)
{
   unsigned char readbuffer[10],sendpacket[10];
   unsigned char sendlen=0;

   // make sure normal level
   OWLevel(MODE_NORMAL);

   // check for correct mode
   if (UMode != MODSEL_COMMAND)
   {
      UMode = MODSEL_COMMAND;
      sendpacket[sendlen++] = MODE_COMMAND;
   }

   // construct the command
   sendpacket[sendlen] = (sendbit != 0) ? BITPOL_ONE : BITPOL_ZERO;
   sendpacket[sendlen++] |= CMD_COMM | FUNCTSEL_BIT | USpeed;

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the response
      if (ReadCOM(1,readbuffer) == 1)
      {
         // interpret the response
         if (((readbuffer[0] & 0xE0) == 0x80) &&
             ((readbuffer[0] & RB_BIT_MASK) == RB_BIT_ONE))
            return 1;
         else
            return 0;
      }
   }

   // an error occured so re-sync with DS2480B
   DS2480B_Detect();

   return 0;
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net is the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  TRUE: bytes written and echo was the same
//           FALSE: echo was not the same
//
void OWWriteByte(unsigned char sendbyte)
{
   OWTouchByte(sendbyte);
}

//--------------------------------------------------------------------------
// Send 8 bits of read communication to the 1-Wire Net and and return the
// result 8 bits read from the 1-Wire Net.
//
// Returns:  8 bits read from 1-Wire Net
//
unsigned char OWReadByte(void)
{
   return OWTouchByte(0xFF);
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.  The parameter 'sendbyte'
// least significant 8 bits are used and the least significant 8 bits
// of the result is the return byte.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  8 bits read from sendbyte
//
unsigned char OWTouchByte(unsigned char sendbyte)
{
   unsigned char readbuffer[10],sendpacket[10];
   unsigned char sendlen=0;

   // make sure normal level
   OWLevel(MODE_NORMAL);

   // check for correct mode
   if (UMode != MODSEL_DATA)
   {
      UMode = MODSEL_DATA;
      sendpacket[sendlen++] = MODE_DATA;
   }

   // add the byte to send
   sendpacket[sendlen++] = (unsigned char)sendbyte;

   // check for duplication of data that looks like COMMAND mode
   if (sendbyte ==(int)MODE_COMMAND)
      sendpacket[sendlen++] = (unsigned char)sendbyte;

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the 1 byte response
      if (ReadCOM(1,readbuffer) == 1)
      {
          // return the response
          return readbuffer[0];
      }
   }

   // an error occured so re-sync with DS2480B
   DS2480B_Detect();

   return 0;
}

//--------------------------------------------------------------------------
// The 'OWBlock' transfers a block of data to and from the
// 1-Wire Net. The result is returned in the same buffer.
//
// 'tran_buf' - pointer to a block of unsigned
//              chars of length 'tran_len' that will be sent
//              to the 1-Wire Net
// 'tran_len' - length in bytes to transfer
//
// Returns:   TRUE (1) : If the buffer transfer was succesful.
//            FALSE (0): If an error occured.
//
//  The maximum tran_length is (160)
//
int OWBlock(unsigned char *tran_buf, int tran_len)
{
   unsigned char sendpacket[320];
   unsigned char sendlen=0,pos,i;

   // check for a block too big
   if (tran_len > 160)
      return FALSE;

   // construct the packet to send to the DS2480B
   // check for correct mode
   if (UMode != MODSEL_DATA)
   {
      UMode = MODSEL_DATA;
      sendpacket[sendlen++] = MODE_DATA;
   }

   // add the bytes to send
   pos = sendlen;
   for (i = 0; i < tran_len; i++)
   {
      sendpacket[sendlen++] = tran_buf[i];

      // duplicate data that looks like COMMAND mode
      if (tran_buf[i] == MODE_COMMAND)
         sendpacket[sendlen++] = tran_buf[i];
   }

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the response
      if (ReadCOM(tran_len,tran_buf) == tran_len)
         return TRUE;
   }

   // an error occured so re-sync with DS2480B
   DS2480B_Detect();

   return FALSE;
}

//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : no device present
//
int OWFirst()
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = FALSE;
   LastFamilyDiscrepancy = 0;

   return OWSearch();
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
int OWNext()
{
   // leave the search state alone
   return OWSearch();
}

//--------------------------------------------------------------------------
// Verify the device with the ROM number in ROM_NO buffer is present.
// Return TRUE  : device verified present
//        FALSE : device not present
//
int OWVerify()
{
   unsigned char rom_backup[8];
   int i,rslt,ld_backup,ldf_backup,lfd_backup;

   // keep a backup copy of the current state
   for (i = 0; i < 8; i++)
      rom_backup[i] = ROM_NO[i];
   ld_backup = LastDiscrepancy;
   ldf_backup = LastDeviceFlag;
   lfd_backup = LastFamilyDiscrepancy;

   // set search to find the same device
   LastDiscrepancy = 64;
   LastDeviceFlag = FALSE;

   if (OWSearch())
   {
      // check if same device found
      rslt = TRUE;
      for (i = 0; i < 8; i++)
      {
         if (rom_backup[i] != ROM_NO[i])
         {
            rslt = FALSE;
            break;
         }
      }
   }
   else
     rslt = FALSE;

   // restore the search state 
   for (i = 0; i < 8; i++)
      ROM_NO[i] = rom_backup[i];
   LastDiscrepancy = ld_backup;
   LastDeviceFlag = ldf_backup;
   LastFamilyDiscrepancy = lfd_backup;

   // return the result of the verify
   return rslt;
}

//--------------------------------------------------------------------------
// Setup the search to find the device type 'family_code' on the next call
// to OWNext() if it is present.
//
void OWTargetSetup(unsigned char family_code)
{
   int i;

   // set the search state to find SearchFamily type devices
   ROM_NO[0] = family_code;
   for (i = 1; i < 8; i++)
      ROM_NO[i] = 0;
   LastDiscrepancy = 64;
   LastFamilyDiscrepancy = 0;
   LastDeviceFlag = FALSE;
}

//--------------------------------------------------------------------------
// Setup the search to skip the current device type on the next call
// to OWNext().
//
void OWFamilySkipSetup()
{
   // set the Last discrepancy to last family discrepancy
   LastDiscrepancy = LastFamilyDiscrepancy;

   // clear the last family discrpepancy
   LastFamilyDiscrepancy = 0;

   // check for end of list
   if (LastDiscrepancy == 0) 
      LastDeviceFlag = TRUE;

}

//--------------------------------------------------------------------------
// The 'OWSearch' function does a general search.  This function
// continues from the previos search state. The search state
// can be reset by using the 'OWFirst' function.
// This function contains one parameter 'alarm_only'.
// When 'alarm_only' is TRUE (1) the find alarm command
// 0xEC is sent instead of the normal search command 0xF0.
// Using the find alarm command 0xEC will limit the search to only
// 1-Wire devices that are in an 'alarm' state.
//
// Returns:   TRUE (1) : when a 1-Wire device was found and it's
//                       Serial Number placed in the global ROM 
//            FALSE (0): when no new device was found.  Either the
//                       last search was the last device or there
//                       are no devices on the 1-Wire Net.
//
int OWSearch(void)
{
   unsigned char last_zero,pos;
   unsigned char tmp_rom[8];
   unsigned char readbuffer[20],sendpacket[40];
   unsigned char i,sendlen=0;

   // if the last call was the last one
   if (LastDeviceFlag)
   {
      // reset the search
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      return FALSE;
   }

   // reset the 1-wire
   // if there are no parts on 1-wire, return FALSE
   if (!OWReset())
   {
      // reset the search
      LastDiscrepancy = 0;
      LastFamilyDiscrepancy = 0;
      return FALSE;
   }

   // build the command stream
   // call a function that may add the change mode command to the buff
   // check for correct mode
   if (UMode != MODSEL_DATA)
   {
      UMode = MODSEL_DATA;
      sendpacket[sendlen++] = MODE_DATA;
   }

   // search command
   sendpacket[sendlen++] = 0xF0;
   
   // change back to command mode
   UMode = MODSEL_COMMAND;
   sendpacket[sendlen++] = MODE_COMMAND;

   // search mode on
   sendpacket[sendlen++] = (unsigned char)(CMD_COMM | FUNCTSEL_SEARCHON | USpeed);

   // change back to data mode
   UMode = MODSEL_DATA;
   sendpacket[sendlen++] = MODE_DATA;

   // set the temp Last Discrepancy to 0
   last_zero = 0;
   
   // add the 16 bytes of the search
   pos = sendlen;
   for (i = 0; i < 16; i++)
      sendpacket[sendlen++] = 0;

   // only modify bits if not the first search
   if (LastDiscrepancy != 0)
   {
      // set the bits in the added buffer
      for (i = 0; i < 64; i++)
      {
         // before last discrepancy
         if (i < (LastDiscrepancy - 1))
               bitacc(WRITE_FUNCTION,
                   bitacc(READ_FUNCTION,0,i,&ROM_NO[0]),
                   (short)(i * 2 + 1),
                   &sendpacket[pos]);
         // at last discrepancy
         else if (i == (LastDiscrepancy - 1))
                bitacc(WRITE_FUNCTION,1,(short)(i * 2 + 1),
                   &sendpacket[pos]);
         // after last discrepancy so leave zeros
      }
   }

   // change back to command mode
   UMode = MODSEL_COMMAND;
   sendpacket[sendlen++] = MODE_COMMAND;

   // search OFF command
   sendpacket[sendlen++] = (unsigned char)(CMD_COMM | FUNCTSEL_SEARCHOFF | USpeed);

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the 1 byte response
      if (ReadCOM(17,readbuffer) == 17)
      {
         // interpret the bit stream
         for (i = 0; i < 64; i++)
         {
            // get the ROM bit
            bitacc(WRITE_FUNCTION,
                   bitacc(READ_FUNCTION,0,(short)(i * 2 + 1),&readbuffer[1]),i,
                   &tmp_rom[0]);
            // check LastDiscrepancy
            if ((bitacc(READ_FUNCTION,0,(short)(i * 2),&readbuffer[1]) == 1) &&
                (bitacc(READ_FUNCTION,0,(short)(i * 2 + 1),&readbuffer[1]) == 0))
            {
               last_zero = i + 1;
               // check LastFamilyDiscrepancy
               if (i < 8)
                  LastFamilyDiscrepancy = i + 1;
            }
         }

         // do dowcrc
         crc8 = 0;
         for (i = 0; i < 8; i++)
            docrc8(tmp_rom[i]);

         // check results
         if ((crc8 != 0) || (LastDiscrepancy == 63) || (tmp_rom[0] == 0))
         {
            // error during search
            // reset the search
            LastDiscrepancy = 0;
            LastDeviceFlag = FALSE;
            LastFamilyDiscrepancy = 0;
            return FALSE;
         }
         // successful search
         else
         {
            // set the last discrepancy
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)
               LastDeviceFlag = TRUE;

            // copy the ROM to the buffer
            for (i = 0; i < 8; i++)
               ROM_NO[i] = tmp_rom[i];

            return TRUE;
         }
      }
   }
   
   // an error occured so re-sync with DS2480B
   DS2480B_Detect();

   // reset the search
   LastDiscrepancy = 0;
   LastDeviceFlag = FALSE;
   LastFamilyDiscrepancy = 0;

   return FALSE;
}

//---------------------------------------------------------------------------
//-------- Extended 1-Wire functions
//---------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Set the 1-Wire Net communucation speed.
//
// 'new_speed' - new speed defined as
//                MODE_NORMAL     0x00
//                MODE_OVERDRIVE  0x01
//
// Returns:  current 1-Wire Net speed
//
int OWSpeed(int new_speed)
{
   unsigned char sendpacket[5];
   unsigned char sendlen=0;
   unsigned char rt = FALSE;

   // check if change from current mode
   if (((new_speed == MODE_OVERDRIVE) &&
        (USpeed != SPEEDSEL_OD)) ||
       ((new_speed == MODE_NORMAL) &&
        (USpeed != SPEEDSEL_FLEX)))
   {
      if (new_speed == MODE_OVERDRIVE)
      {
         // if overdrive then switch to 115200 baud
         if (DS2480B_ChangeBaud(MAX_BAUD) == MAX_BAUD)
         {
            USpeed = SPEEDSEL_OD;
            rt = TRUE;
         }

      }
      else if (new_speed == MODE_NORMAL)
      {
         // else normal so set to 9600 baud
         if (DS2480B_ChangeBaud(PARMSET_9600) == PARMSET_9600)
         {
            USpeed = SPEEDSEL_FLEX;
            rt = TRUE;
         }

      }

      // if baud rate is set correctly then change DS2480B speed
      if (rt)
      {
         // check for correct mode
         if (UMode != MODSEL_COMMAND)
         {
            UMode = MODSEL_COMMAND;
            sendpacket[sendlen++] = MODE_COMMAND;
         }

         // proceed to set the DS2480B communication speed
         sendpacket[sendlen++] = CMD_COMM | FUNCTSEL_SEARCHOFF | USpeed;

         // send the packet
         if (!WriteCOM(sendlen,sendpacket))
         {
            rt = FALSE;
            // lost communication with DS2480B then reset
            DS2480B_Detect();
         }
      }

   }

   // return the current speed
   return (USpeed == SPEEDSEL_OD) ? MODE_OVERDRIVE : MODE_NORMAL;
}

//--------------------------------------------------------------------------
// Set the 1-Wire Net line level.  The values for new_level are
// as follows:
//
// 'new_level' - new level defined as
//                MODE_NORMAL     0x00
//                MODE_STRONG5    0x02
//
// Returns:  current 1-Wire Net level
//
int OWLevel(int new_level)
{
   unsigned char sendpacket[10],readbuffer[10];
   unsigned char sendlen=0;
   unsigned char rt=FALSE,docheck=FALSE;

   // check if need to change level
   if (new_level != ULevel)
   {
      // check for correct mode
      if (UMode != MODSEL_COMMAND)
      {
         UMode = MODSEL_COMMAND;
         sendpacket[sendlen++] = MODE_COMMAND;
      }
      
      // check if just putting back to normal
      if (new_level == MODE_NORMAL)
      {
         // check for disable strong pullup step
         if (ULevel == MODE_STRONG5)
            docheck = TRUE;

         // stop pulse command
         sendpacket[sendlen++] = MODE_STOP_PULSE;

         // add the command to begin the pulse WITHOUT prime
         sendpacket[sendlen++] = CMD_COMM | FUNCTSEL_CHMOD | SPEEDSEL_PULSE | BITPOL_5V | PRIME5V_FALSE;

         // stop pulse command
         sendpacket[sendlen++] = MODE_STOP_PULSE;

         // flush the buffers
         FlushCOM();

         // send the packet
         if (WriteCOM(sendlen,sendpacket))
         {
            // read back the 1 byte response
            if (ReadCOM(2,readbuffer) == 2)
            {
               // check response byte
               if (((readbuffer[0] & 0xE0) == 0xE0) && ((readbuffer[1] & 0xE0) == 0xE0))
               {
                  rt = TRUE;
                  ULevel = MODE_NORMAL;

               }
            }
         }
      }
      // set new level
      else
      {
         // set the SPUD time value
         sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_5VPULSE | PARMSET_infinite;
         // add the command to begin the pulse
         sendpacket[sendlen++] = CMD_COMM | FUNCTSEL_CHMOD | SPEEDSEL_PULSE | BITPOL_5V;

         // flush the buffers
         FlushCOM();

         // send the packet
         if (WriteCOM(sendlen,sendpacket))
         {
            // read back the 1 byte response from setting time limit
            if (ReadCOM(1,readbuffer) == 1)
            {
               // check response byte
               if ((readbuffer[0] & 0x81) == 0)
               {
                  ULevel = new_level;
                  rt = TRUE;
               }
            }
         }
      }

      // if lost communication with DS2480B then reset
      if (rt != TRUE)
         DS2480B_Detect();
   }

   // return the current level
   return ULevel;
}

//--------------------------------------------------------------------------
// This procedure creates a fixed 480 microseconds 12 volt pulse
// on the 1-Wire Net for programming EPROM iButtons.
//
// Returns:  TRUE  successful
//           FALSE program voltage not available
//
int OWProgramPulse(void)
{
   unsigned char sendpacket[10],readbuffer[10];
   unsigned char sendlen=0;

   // make sure normal level
   OWLevel(MODE_NORMAL);

   // check for correct mode
   if (UMode != MODSEL_COMMAND)
   {
      UMode = MODSEL_COMMAND;
      sendpacket[sendlen++] = MODE_COMMAND;
   }

   // set the SPUD time value
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_12VPULSE | PARMSET_512us;

   // pulse command
   sendpacket[sendlen++] = CMD_COMM | FUNCTSEL_CHMOD | BITPOL_12V | SPEEDSEL_PULSE;

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the 2 byte response
      if (ReadCOM(2,readbuffer) == 2)
      {
         // check response byte
         if (((readbuffer[0] | CMD_CONFIG) ==
                (CMD_CONFIG | PARMSEL_12VPULSE | PARMSET_512us)) &&
             ((readbuffer[1] & 0xFC) ==
                (0xFC & (CMD_COMM | FUNCTSEL_CHMOD | BITPOL_12V | SPEEDSEL_PULSE))))
            return TRUE;
      }
   }

   // an error occured so re-sync with DS2480B
   DS2480B_Detect();

   return FALSE;
}

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net is the same (write operation).  
// The parameter 'sendbyte' least significant 8 bits are used.  After the
// 8 bits are sent change the level of the 1-Wire net.
//
// 'sendbyte' - 8 bits to send (least significant bit)
//
// Returns:  TRUE: bytes written and echo was the same, strong pullup now on
//           FALSE: echo was not the same 
//
int OWWriteBytePower(int sendbyte)
{
   unsigned char sendpacket[10],readbuffer[10];
   unsigned char sendlen=0;
   unsigned char rt=FALSE;
   unsigned char i, temp_byte;

   // check for correct mode
   if (UMode != MODSEL_COMMAND)
   {
      UMode = MODSEL_COMMAND;
      sendpacket[sendlen++] = MODE_COMMAND;
   }

   // set the SPUD time value
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_5VPULSE | PARMSET_infinite;

   // construct the stream to include 8 bit commands with the last one
   // enabling the strong-pullup
   temp_byte = sendbyte;
   for (i = 0; i < 8; i++)
   {
      sendpacket[sendlen++] = ((temp_byte & 0x01) ? BITPOL_ONE : BITPOL_ZERO)
                              | CMD_COMM | FUNCTSEL_BIT | USpeed |
                              ((i == 7) ? PRIME5V_TRUE : PRIME5V_FALSE);
      temp_byte >>= 1;
   }

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the 9 byte response from setting time limit
      if (ReadCOM(9,readbuffer) == 9)
      {
         // check response 
         if ((readbuffer[0] & 0x81) == 0)
         {
            // indicate the port is now at power delivery
            ULevel = MODE_STRONG5;

            // reconstruct the echo byte
            temp_byte = 0; 
            for (i = 0; i < 8; i++)
            {
               temp_byte >>= 1;
               temp_byte |= (readbuffer[i + 1] & 0x01) ? 0x80 : 0;
            }
            
            if (temp_byte == sendbyte)
               rt = TRUE;
         }
      }
   }

   // if lost communication with DS2480B then reset
   if (rt != TRUE)
      DS2480B_Detect();
   
   return rt;
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and verify that the
// response matches the 'applyPowerResponse' bit and apply power delivery
// to the 1-Wire net.  Note that some implementations may apply the power
// first and then turn it off if the response is incorrect.
//
// 'applyPowerResponse' - 1 bit response to check, if correct then start
//                        power delivery 
//
// Returns:  TRUE: bit written and response correct, strong pullup now on
//           FALSE: response incorrect
//
int OWReadBitPower(int applyPowerResponse)
{
   unsigned char sendpacket[3],readbuffer[3];
   unsigned char sendlen=0;
   unsigned char rt=FALSE;

   // check for correct mode
   if (UMode != MODSEL_COMMAND)
   {
      UMode = MODSEL_COMMAND;
      sendpacket[sendlen++] = MODE_COMMAND;
   }

   // set the SPUD time value
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_5VPULSE | PARMSET_infinite;

   // enabling the strong-pullup after bit
   sendpacket[sendlen++] = BITPOL_ONE 
                           | CMD_COMM | FUNCTSEL_BIT | USpeed |
                           PRIME5V_TRUE;
   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the 2 byte response from setting time limit
      if (ReadCOM(2,readbuffer) == 2)
      {
         // check response to duration set
         if ((readbuffer[0] & 0x81) == 0)
         {
            // indicate the port is now at power delivery
            ULevel = MODE_STRONG5;

            // check the response bit
            if ((readbuffer[1] & 0x01) == applyPowerResponse)
               rt = TRUE;
            else
               OWLevel(MODE_NORMAL);

            return rt;
         }
      }
   }

   // if lost communication with DS2480B then reset
   if (rt != TRUE)
      DS2480B_Detect();
   
   return rt;
}

//---------------------------------------------------------------------------
//-------- DS2480B functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Attempt to resyc and detect a DS2480B and set the FLEX parameters
//
// Returns:  TRUE  - DS2480B detected successfully
//           FALSE - Could not detect DS2480B
//
int DS2480B_Detect(void)
{
   unsigned char sendpacket[10],readbuffer[10];
   unsigned char sendlen=0;

   // reset modes
   UMode = MODSEL_COMMAND;
   UBaud = PARMSET_9600;
   USpeed = SPEEDSEL_FLEX;

   // set the baud rate to 9600
   SetBaudCOM((unsigned char)UBaud);

   // send a break to reset the DS2480B
   BreakCOM();

   // delay to let line settle
   msDelay(2);

   // flush the buffers
   FlushCOM();

   // send the timing byte
   sendpacket[0] = 0xC1;
   if (WriteCOM(1,sendpacket) != 1)
      return FALSE;

   // delay to let line settle
   msDelay(2);

   // set the FLEX configuration parameters
   // default PDSRC = 1.37Vus
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_SLEW | PARMSET_Slew1p37Vus;
   // default W1LT = 10us
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_WRITE1LOW | PARMSET_Write10us;
   // default DSO/WORT = 8us
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_SAMPLEOFFSET | PARMSET_SampOff8us;

   // construct the command to read the baud rate (to test command block)
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_PARMREAD | (PARMSEL_BAUDRATE >> 3);

   // also do 1 bit operation (to test 1-Wire block)
   sendpacket[sendlen++] = CMD_COMM | FUNCTSEL_BIT | UBaud | BITPOL_ONE;

   // flush the buffers
   FlushCOM();

   // send the packet
   if (WriteCOM(sendlen,sendpacket))
   {
      // read back the response
      if (ReadCOM(5,readbuffer) == 5)
      {
         // look at the baud rate and bit operation
         // to see if the response makes sense
         if (((readbuffer[3] & 0xF1) == 0x00) &&
             ((readbuffer[3] & 0x0E) == UBaud) &&
             ((readbuffer[4] & 0xF0) == 0x90) &&
             ((readbuffer[4] & 0x0C) == UBaud))
            return TRUE;
      }
   }

   return FALSE;
}

//---------------------------------------------------------------------------
// Change the DS2480B from the current baud rate to the new baud rate.
//
// 'newbaud' - the new baud rate to change to, defined as:
//               PARMSET_9600     0x00
//               PARMSET_19200    0x02
//               PARMSET_57600    0x04
//               PARMSET_115200   0x06
//
// Returns:  current DS2480B baud rate.
//
int DS2480B_ChangeBaud(unsigned char newbaud)
{
   unsigned char rt=FALSE;
   unsigned char readbuffer[5],sendpacket[5],sendpacket2[5];
   unsigned char sendlen=0,sendlen2=0;

   // see if diffenent then current baud rate
   if (UBaud == newbaud)
      return UBaud;
   else
   {
      // build the command packet
      // check for correct mode
      if (UMode != MODSEL_COMMAND)
      {
         UMode = MODSEL_COMMAND;
         sendpacket[sendlen++] = MODE_COMMAND;
      }
      // build the command
      sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_BAUDRATE | newbaud;

      // flush the buffers
      FlushCOM();

      // send the packet
      if (!WriteCOM(sendlen,sendpacket))
         rt = FALSE;
      else
      {
         // make sure buffer is flushed
         msDelay(5);

         // change our baud rate
         SetBaudCOM(newbaud);
         UBaud = newbaud;

         // wait for things to settle
         msDelay(5);

         // build a command packet to read back baud rate
         sendpacket2[sendlen2++] = CMD_CONFIG | PARMSEL_PARMREAD | (PARMSEL_BAUDRATE >> 3);

         // flush the buffers
         FlushCOM();

         // send the packet
         if (WriteCOM(sendlen2,sendpacket2))
         {
            // read back the 1 byte response
            if (ReadCOM(1,readbuffer) == 1)
            {
               // verify correct baud
               if (((readbuffer[0] & 0x0E) == (sendpacket[sendlen-1] & 0x0E)))
                  rt = TRUE;
            }
         }
      }
   }

   // if lost communication with DS2480B then reset
   if (rt != TRUE)
      DS2480B_Detect();

   return UBaud;
}


//---------------------------------------------------------------------------
//-------- WIN32 COM functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Attempt to open a com port.  Keep the handle in ComID.
// Set the starting baud rate to 9600.
//
// 'port_zstr' - zero terminate port name.  For this platform
//               use format COMX where X is the port number.
//
//
// Returns: TRUE(1)  - success, COM port opened
//          FALSE(0) - failure, could not open specified port
//
int OpenCOM(char *port_zstr)
{
   char tempstr[80];
   short fRetVal;
   COMMTIMEOUTS CommTimeOuts;
   DCB dcb;

   // open COMM device
   if ((ComID =
      CreateFile( port_zstr, GENERIC_READ | GENERIC_WRITE,
                  0,
                  NULL,                 // no security attrs
                  OPEN_EXISTING,
                  FILE_FLAG_OVERLAPPED, // overlapped I/O
                  NULL )) == (HANDLE) -1 )
   {
      ComID = 0;
      return FALSE;
   }
   else
   {
      // create events for detection of reading and write to com port
      sprintf(tempstr,"COMM_READ_OVERLAPPED_EVENT_FOR_%s",port_zstr);
      osRead.hEvent = CreateEvent(NULL,TRUE,FALSE,tempstr);
      sprintf(tempstr,"COMM_WRITE_OVERLAPPED_EVENT_FOR_%s",port_zstr);
      osWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,tempstr);

      // get any early notifications
      SetCommMask(ComID, EV_RXCHAR | EV_TXEMPTY | EV_ERR | EV_BREAK);

      // setup device buffers
      SetupComm(ComID, 2048, 2048);

      // purge any information in the buffer
      PurgeComm(ComID, PURGE_TXABORT | PURGE_RXABORT |
                           PURGE_TXCLEAR | PURGE_RXCLEAR );

      // set up for overlapped non-blocking I/O
      CommTimeOuts.ReadIntervalTimeout = 0;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 20;
      CommTimeOuts.ReadTotalTimeoutConstant = 40;
      CommTimeOuts.WriteTotalTimeoutMultiplier = 20;
      CommTimeOuts.WriteTotalTimeoutConstant = 40;
      SetCommTimeouts(ComID, &CommTimeOuts);

      // setup the com port
      GetCommState(ComID, &dcb);

      dcb.BaudRate = CBR_9600;               // current baud rate
      dcb.fBinary = TRUE;                    // binary mode, no EOF check
      dcb.fParity = FALSE;                   // enable parity checking
      dcb.fOutxCtsFlow = FALSE;              // CTS output flow control
      dcb.fOutxDsrFlow = FALSE;              // DSR output flow control
      dcb.fDtrControl = DTR_CONTROL_ENABLE;  // DTR flow control type
      dcb.fDsrSensitivity = FALSE;           // DSR sensitivity
      dcb.fTXContinueOnXoff = TRUE;          // XOFF continues Tx
      dcb.fOutX = FALSE;                     // XON/XOFF out flow control
      dcb.fInX = FALSE;                      // XON/XOFF in flow control
      dcb.fErrorChar = FALSE;                // enable error replacement
      dcb.fNull = FALSE;                     // enable null stripping
      dcb.fRtsControl = RTS_CONTROL_ENABLE;  // RTS flow control
      dcb.fAbortOnError = FALSE;             // abort reads/writes on error
      dcb.XonLim = 0;                        // transmit XON threshold
      dcb.XoffLim = 0;                       // transmit XOFF threshold
      dcb.ByteSize = 8;                      // number of bits/byte, 4-8
      dcb.Parity = NOPARITY;                 // 0-4=no,odd,even,mark,space
      dcb.StopBits = ONESTOPBIT;             // 0,1,2 = 1, 1.5, 2
      dcb.XonChar = 0;                       // Tx and Rx XON character
      dcb.XoffChar = 1;                      // Tx and Rx XOFF character
      dcb.ErrorChar = 0;                     // error replacement character
      dcb.EofChar = 0;                       // end of input character
      dcb.EvtChar = 0;                       // received event character

      fRetVal = SetCommState(ComID, &dcb);
   }

   // check if successfull
   if (!fRetVal)
   {
      CloseHandle(ComID);
      CloseHandle(osRead.hEvent);
      CloseHandle(osWrite.hEvent);
      ComID = 0;
   }

   return fRetVal;
}

//---------------------------------------------------------------------------
// Closes the connection to the port.
//
void CloseCOM(void)
{
   // disable event notification and wait for thread
   // to halt
   SetCommMask(ComID, 0);

   // drop DTR
   EscapeCommFunction(ComID, CLRDTR);

   // purge any outstanding reads/writes and close device handle
   PurgeComm(ComID, PURGE_TXABORT | PURGE_RXABORT |
                    PURGE_TXCLEAR | PURGE_RXCLEAR );
   CloseHandle(ComID);
   CloseHandle(osRead.hEvent);
   CloseHandle(osWrite.hEvent);
   ComID = 0;
}

//---------------------------------------------------------------------------
// Flush the rx and tx buffers
//
void FlushCOM(void)
{
   // purge any information in the buffer
   PurgeComm(ComID, PURGE_TXABORT | PURGE_RXABORT |
                    PURGE_TXCLEAR | PURGE_RXCLEAR );

   // debug
   if (dodebug)
      printf("__Flush__\n");
}

//--------------------------------------------------------------------------
// Write an array of bytes to the COM port, verify that it was
// sent out.  Assume that baud rate has been set.
//
// 'outlen'   - number of bytes to write to COM port
// 'outbuf'   - pointer ot an array of bytes to write
//
// Returns:  TRUE(1)  - success
//           FALSE(0) - failure
//
int WriteCOM(int outlen, unsigned char *outbuf)
{
   BOOL fWriteStat;
   DWORD dwBytesWritten=0;
   DWORD ler=0,to;
   int i;

   // debug
   if (dodebug)
   {
      for (i = 0; i < outlen; i++)
         printf(">%02X",outbuf[i]);
      printf("\n");
   }

   // calculate a timeout
   to = 20 * outlen + 60;

   // reset the write event
   ResetEvent(osWrite.hEvent);

   // write the byte
   fWriteStat = WriteFile(ComID, (LPSTR) &outbuf[0],
                outlen, &dwBytesWritten, &osWrite );

   // check for an error
   if (!fWriteStat)
      ler = GetLastError();

   // if not done writting then wait
   if (!fWriteStat && ler == ERROR_IO_PENDING)
   {
      WaitForSingleObject(osWrite.hEvent,to);

      // verify all is written correctly
      fWriteStat = GetOverlappedResult(ComID, &osWrite,
                   &dwBytesWritten, FALSE);

   }

   // check results of write
   if (!fWriteStat || (dwBytesWritten != (DWORD)outlen))
      return 0;
   else
      return 1;
}

//--------------------------------------------------------------------------
// Read an array of bytes to the COM port, verify that it was
// sent out.  Assume that baud rate has been set.
//
// 'inlen'     - number of bytes to read from COM port
// 'inbuf'     - pointer to a buffer to hold the incomming bytes
//
// Returns: number of characters read
//
int ReadCOM(int inlen, unsigned char *inbuf)
{
   DWORD dwLength=0;
   BOOL fReadStat;
   DWORD ler=0,to,i;

   // calculate a timeout
   to = 20 * inlen + 60;

   // reset the read event
   ResetEvent(osRead.hEvent);

   // read
   fReadStat = ReadFile(ComID, (LPSTR) &inbuf[0],
                      inlen, &dwLength, &osRead) ;

   // check for an error
   if (!fReadStat)
      ler = GetLastError();

   // if not done writing then wait
   if (!fReadStat && ler == ERROR_IO_PENDING)
   {
      // wait until everything is read
      WaitForSingleObject(osRead.hEvent,to);

      // verify all is read correctly
      fReadStat = GetOverlappedResult(ComID, &osRead,
                   &dwLength, FALSE);
   }

   // check results
   if (fReadStat)
   {
      // debug
      if (dodebug)
      {
         for (i = 0; i < dwLength; i++)
            printf("<%02X",inbuf[i]);
         printf("\n");
      }

      return dwLength;
   }
   else
      return 0;
}

//--------------------------------------------------------------------------
// Send a break on the com port for at least 2 ms
//
void BreakCOM(void)
{
   // start the reset pulse
   SetCommBreak(ComID);

   // sleep
   Sleep(2);

   // clear the break
   ClearCommBreak(ComID);

   // debug
   if (dodebug)
      printf("__Break__\n");
}

//--------------------------------------------------------------------------
// Set the baud rate on the com port.
//
// 'new_baud'  - new baud rate defined as
//                PARMSET_9600     0x00
//                PARMSET_19200    0x02
//                PARMSET_57600    0x04
//                PARMSET_115200   0x06
//
void SetBaudCOM(unsigned char new_baud)
{
   DCB dcb;

   // get the current com port state
   GetCommState(ComID, &dcb);

   // change just the baud rate
   switch (new_baud)
   {
      case PARMSET_115200:
         dcb.BaudRate = CBR_115200;
         break;
      case PARMSET_57600:
         dcb.BaudRate = CBR_57600;
         break;
      case PARMSET_19200:
         dcb.BaudRate = CBR_19200;
         break;
      case PARMSET_9600:
      default:
         dcb.BaudRate = CBR_9600;
         break;
   }

   // restore to set the new baud rate
   SetCommState(ComID, &dcb);

   // debug
   if (dodebug)
      printf("__SetBaudCOM %d__\n",dcb.BaudRate);
}

//--------------------------------------------------------------------------
//  Description:
//     Delay for at least 'len' ms
//
void msDelay(int len)
{
   // debug
   if (dodebug)
      printf("__msDelay %d__\n",len);

   DelayMs(len);
}

//---------------------------------------------------------------------------
//-------- Utility functions
//---------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Bit utility to read and write a bit in the buffer 'buf'.
//
// 'op'    - operation (1) to set and (0) to read
// 'state' - set (1) or clear (0) if operation is write (1)
// 'loc'   - bit number location to read or write
// 'buf'   - pointer to array of bytes that contains the bit
//           to read or write
//
// Returns: 1   if operation is set (1)
//          0/1 state of bit number 'loc' if operation is reading
//
int bitacc(int op, int state, int loc, unsigned char *buf)
{
   int nbyt,nbit;

   nbyt = (loc / 8);
   nbit = loc - (nbyt * 8);

   if (op == WRITE_FUNCTION)
   {
      if (state)
         buf[nbyt] |= (0x01 << nbit);
      else
         buf[nbyt] &= ~(0x01 << nbit);

      return 1;
   }
   else
      return ((buf[nbyt] >> nbit) & 0x01);
}

// TEST BUILD
static unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current 
// global 'crc8' value. 
// Returns current global crc8 value
//
unsigned char docrc8(unsigned char value)
{
   // See Application Note 27
   
   // TEST BUILD
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}



