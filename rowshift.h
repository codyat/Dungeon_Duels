///////////////////////////////////////////////////////////////////////////
//I did not write this code, it was found online and utilized for the
//purpose of completing this project.
///////////////////////////////////////////////////////////////////////////

#ifndef columnShift_H_
#define columnShift_H_

#include <avr/io.h>
#include <util/delay.h>

#define column_PORT   PORTB
#define column_DDR    DDRB
#define column_DS_POS PB0      //Data pin (DS) pin location
#define column_SH_CP_POS PB2      //Shift Clock (SH_CP) pin location
#define column_ST_CP_POS PB1      //Store Clock (ST_CP) pin location

#define row_PORT   PORTC
#define row_DDR    DDRC
#define row_DS_POS PC4      //Data pin (DS) pin location
#define row_SH_CP_POS PC3      //Shift Clock (SH_CP) pin location
#define row_ST_CP_POS PC5      //Store Clock (ST_CP) pin location

#define seg7_PORT   PORTC
#define seg7_DDR    DDRC
#define seg7_DS_POS PC0      //Data pin (DS) pin location
#define seg7_SH_CP_POS PC2      //Shift Clock (SH_CP) pin location
#define seg7_ST_CP_POS PC1      //Store Clock (ST_CP) pin location

/***************************************
Configure Connections ***ENDS***
****************************************/

//Initialize column System

void columnInit()
{
   //Make the Data(DS), Shift clock (SH_CP), Store Clock (ST_CP) lines output
   column_DDR|=((1<<column_SH_CP_POS)|(1<<column_ST_CP_POS)|(1<<column_DS_POS));
}
void rowInit()
{
   //Make the Data(DS), Shift clock (SH_CP), Store Clock (ST_CP) lines output
   row_DDR|=((1<<row_SH_CP_POS)|(1<<row_ST_CP_POS)|(1<<row_DS_POS));
}
void seg7Init()
{
   //Make the Data(DS), Shift clock (SH_CP), Store Clock (ST_CP) lines output
   seg7_DDR|=((1<<seg7_SH_CP_POS)|(1<<seg7_ST_CP_POS)|(1<<seg7_DS_POS));
}

//Low level macros to change data (DS)lines
#define columnDataHigh() (column_PORT|=(1<<column_DS_POS))
#define columnDataLow() (column_PORT&=(~(1<<column_DS_POS)))

#define rowDataHigh() (row_PORT|=(1<<row_DS_POS))
#define rowDataLow() (row_PORT&=(~(1<<row_DS_POS)))

#define seg7DataHigh() (seg7_PORT|=(1<<seg7_DS_POS))
#define seg7DataLow() (seg7_PORT&=(~(1<<seg7_DS_POS)))
//Sends a clock pulse on SH_CP line
void columnPulse()
{
   //Pulse the Shift Clock

   column_PORT|=(1<<column_SH_CP_POS);//HIGH

   column_PORT&=(~(1<<column_SH_CP_POS));//LOW

}
void rowPulse()
{
   //Pulse the Shift Clock

   row_PORT|=(1<<row_SH_CP_POS);//HIGH

   row_PORT&=(~(1<<row_SH_CP_POS));//LOW

}
void seg7Pulse()
{
   //Pulse the Shift Clock

   seg7_PORT|=(1<<seg7_SH_CP_POS);//HIGH

   seg7_PORT&=(~(1<<seg7_SH_CP_POS));//LOW

}
//Sends a clock pulse on ST_CP line
void columnLatch()
{
   //Pulse the Store Clock

   column_PORT|=(1<<column_ST_CP_POS);//HIGH
   _delay_loop_1(1);

   column_PORT&=(~(1<<column_ST_CP_POS));//LOW
   _delay_loop_1(1);
}
void rowLatch()
{
   //Pulse the Store Clock

   row_PORT|=(1<<row_ST_CP_POS);//HIGH
   _delay_loop_1(1);

   row_PORT&=(~(1<<row_ST_CP_POS));//LOW
   _delay_loop_1(1);
}
void seg7Latch()
{
   //Pulse the Store Clock

   seg7_PORT|=(1<<seg7_ST_CP_POS);//HIGH
   _delay_loop_1(1);

   seg7_PORT&=(~(1<<seg7_ST_CP_POS));//LOW
   _delay_loop_1(1);
}

/*

Main High level function to write a single byte to
Output shift register 74column.

Arguments:
   single byte to write to the 74column IC

Returns:
   NONE

Description:
   The byte is serially transfecolumn to 74column
   and then latched. The byte is then available on
   output line Q0 to Q7 of the column IC.

*/
void columnWrite(uint8_t data)
{
   //Send each 8 bits serially
   //Order is MSB first
   for(uint8_t i=0;i<8;i++)
   {
      //Output the data on DS line according to the
      //Value of MSB
      if(data & 0b10000000)
      {
         //MSB is 1 so output low
		 //Column is common cathode
         columnDataLow();
      }
      else
      {
         //MSB is 0 so output high
		 //Column is common cathode
         columnDataHigh();
      }

      columnPulse();  //Pulse the Clock line
      data=data<<1;  //Now bring next bit at MSB position

   }

   //Now all 8 bits have been transfercolumn to shift register
   //Move them to output latch at one
   columnLatch();
}
void rowWrite(uint8_t data)
{
   //Send each 8 bits serially
   //Order is MSB first
   for(uint8_t i=0;i<8;i++)
   {
      //Output the data on DS line according to the
      //Value of MSB
      if(data & 0b10000000)
      {
         //MSB is 1 so output low
         //Column is common cathode
         rowDataLow();
      }
      else
      {
         //MSB is 0 so output high
         //Column is common cathode
         rowDataHigh();
      }

      rowPulse();  //Pulse the Clock line
      data=data<<1;  //Now bring next bit at MSB position

   }

   //Now all 8 bits have been transfercolumn to shift register
   //Move them to output latch at one
   rowLatch();
}
void seg7Write(uint8_t data)
{
   //Send each 8 bits serially
   //Order is MSB first
   for(uint8_t i=0;i<8;i++)
   {
      //Output the data on DS line according to the
      //Value of MSB
      if(data & 0b10000000)
      {
         //MSB is 1 so output low
         //Column is common cathode
         seg7DataLow();
      }
      else
      {
         //MSB is 0 so output high
         //Column is common cathode
         seg7DataHigh();
      }

      seg7Pulse();  //Pulse the Clock line
      data=data<<1;  //Now bring next bit at MSB position

   }

   //Now all 8 bits have been transfercolumn to shift register
   //Move them to output latch at one
   seg7Latch();
}

/*

Simple Delay function approx 0.5 seconds

*/

void columnWait()
{
   for(uint8_t i=0;i<45;i++)
   {
      _delay_loop_2(0);
   }
}
void rowWait()
{
   for(uint8_t i=0;i<45;i++)
   {
      _delay_loop_2(0);
   }
}
void seg7Wait()
{
   for(uint8_t i=0;i<45;i++)
   {
      _delay_loop_2(0);
   }
}
#endif
