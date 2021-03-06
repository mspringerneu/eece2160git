/*
 *  Project: EECE 2160 Lab #03 Part 2
 *  Author:  Matthew Springer
 *  Date:    February 1, 2017
 */

#include <iostream> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/mman.h> 
#include <cmath>
using namespace std;

// Physical base address of GPIO 
const unsigned gpio_address = 0x400d0000; 

// Length of memory-mapped IO window 
const unsigned gpio_size = 0xff; 

const int gpio_led1_offset = 0x12C;  // Offset for LED1 
const int gpio_led2_offset = 0x130;  // Offset for LED2 
const int gpio_led3_offset = 0x134;  // Offset for LED3 
const int gpio_led4_offset = 0x138;  // Offset for LED4 
const int gpio_led5_offset = 0x13C;  // Offset for LED5 
const int gpio_led6_offset = 0x140;  // Offset for LED6 
const int gpio_led7_offset = 0x144;  // Offset for LED7 
const int gpio_led8_offset = 0x148;  // Offset for LED8 

const int gpio_sw1_offset = 0x14C;  // Offset for Switch 1 
const int gpio_sw2_offset = 0x150;  // Offset for Switch 2 
const int gpio_sw3_offset = 0x154;  // Offset for Switch 3 
const int gpio_sw4_offset = 0x158;  // Offset for Switch 4 
const int gpio_sw5_offset = 0x15C;  // Offset for Switch 5 
const int gpio_sw6_offset = 0x160;  // Offset for Switch 6 
const int gpio_sw7_offset = 0x164;  // Offset for Switch 7 
const int gpio_sw8_offset = 0x168;  // Offset for Switch 8 

const int gpio_pbtnl_offset = 0x16C;  // Offset for left push button 
const int gpio_pbtnr_offset = 0x170;  // Offset for right push button 
const int gpio_pbtnu_offset = 0x174;  // Offset for up push button 
const int gpio_pbtnd_offset = 0x178;  // Offset for down push button 
const int gpio_pbtnc_offset = 0x17C;  // Offset for center push button 

/** 
 * Write a 4-byte value at the specified general-purpose I/O location. 
 * 
 * @param  pBase  Base address returned by 'mmap'. 
 * @param  offset	Offset where device is mapped. 
 * @param  value	Value to be written. 
 */ 
void RegisterWrite(char *pBase, int offset, int value) 
{ 
	* (int *) (pBase + offset) = value; 
} 

/** 
 * Read a 4-byte value from the specified general-purpose I/O location. 
 * 
 * @param  pBase  Base address returned by 'mmap'. 
 * @param  offset	Offset where device is mapped. 
 * @return        Value read. 
 */ 
int RegisterRead(char *pBase, int offset) 
{ 
	return * (int *) (pBase + offset); 
} 

/** 
 * Initialize general-purpose I/O 
 *  - Opens access to physical memory /dev/mem 
 *  - Maps memory at offset 'gpio_address' into virtual address space 
 * 
 * @param  fd	File descriptor passed by reference, where the result 
 *				    of function 'open' will be stored. 
 * @return		Address to virtual memory which is mapped to physical, 
 *          	or MAP_FAILED on error. 
 */ 
char *Initialize(int *fd) 
{ 
	*fd = open( "/dev/mem", O_RDWR); 
	return (char *) mmap(NULL, gpio_size, PROT_READ | PROT_WRITE, MAP_SHARED, 
			*fd, gpio_address); 
} 

/** 
 * Close general-purpose I/O. 
 * 
 * @param  pBase Virtual address where I/O was mapped. 
 * @param  fd	   File descriptor previously returned by 'open'. 
 */ 
void Finalize(char *pBase, int fd) 
{ 
	munmap(pBase, gpio_size); 
	close(fd);
} 

/** Changes the state of an LED (ON or OFF)
 * @param	 pBase	base address of I/O
 * @param	 ledNum	LED number (0 to 7)
 * @param	 state
State to change to (ON or OFF)
*/
void Write1Led(char *pBase, int ledNum, int state) {
	int ledOffset = 0x12C + (ledNum * 0x004);
	RegisterWrite(pBase, ledOffset, state);
}

/** Reads the value of a switch
 * - Uses base address of I/O
 * @param	 pBase      base address of I/O
 * @param	 switchNum 	Switch number (0 to 7)
 * @return            switch value read
*/
int Read1Switch(char *pBase, int switchNum) {
	int switchOffset = 0x14C + (switchNum * 0x004);
	return RegisterRead(pBase, switchOffset);
}

/** Changes the state of an LED (ON or OFF) to represent a given int in binary
 * @param	 pBase	base address of I/O
 * @param	 numVal the number (0-255) to be represented in binary on the LEDs
 * @param	 state
State to change to (ON or OFF)
*/
void WriteAllLeds(char *pBase, int numVal) {
     int remainder = numVal;
     int length = 8;
     int * binaryRep = new int[length];
     bool converting = true;
     for (int i = length - 1; i >= 0; i--) {
         if (converting) {
             int place = pow(2, i);
             if (remainder >= place) {
                // cout << "Led #" << i << ": 1" << endl;
                binaryRep[i] = 1;
                remainder = remainder - place;
                if (remainder == 0) {
                   converting = false;
                }
             }
             else {
                  // cout << "Led #" << i << ": 0" << endl;
                  binaryRep[i] = 0;
             }
         }
         else {
              // cout << "Led #" << i << ": 0" << endl;
              binaryRep[i] = 0;
         }
     }
     for (int j = 0; j < length; j++) {
         Write1Led(pBase, j, binaryRep[j]);
     }
     delete [] binaryRep;
}

/** Reads all the switches and returns their value in a single integer. 
 * 
 * @param pBase Base address for general-purpose I/O 
 * @return A value that represents the value of the switches 
 */ 
 int ReadAllSwitches(char *pBase) {
     int sum = 0;
     int length = 8;
     bool converting = true;
     for (int i = 0; i < length; i++) {
         int place = pow(2, i);
         sum += place * Read1Switch(pBase, i);
     }
     cout << "The integer representation of the current switch configuration is: " << sum << endl;
}

/** Turns off all LEDs
 * - Uses base address of I/O
 * @param	pBase base address of I/O
*/
void TurnOffLEDs(char *pBase) {
     for (int i = 0; i < 8; i++) {
         Write1Led(pBase, i, 0);
     }
}

/** 
 * Main function to interact with I/O Interfaces
 */
int main() 
{ 
	// Initialize 
	int fd; 
	char *pBase = Initialize(&fd); 

	// Check error 
	if (pBase == MAP_FAILED) 
	{ 
		cerr << "Mapping I/O memory failed - Did you run with 'sudo'?\n";
		exit(1); // Returns 1 to the operating system;	
	}	 
	 
	// ************** Put your code here **********************
	int ledNum, state, switchNum, binaryNum, exitNum;
  cout << "Please enter an LED number between 0-7: ";
  cin >> ledNum;
  cout << "Please enter a state for LED #" << ledNum << " (0 or 1): ";
  cin >> state;
  Write1Led(pBase, ledNum, state);
  cout << "Please enter a switch number between 0-7: ";
  cin >> switchNum;
  cout << "The value of switch #" << switchNum << " is: " << Read1Switch(pBase, switchNum) << endl;
  cout << "Please enter an integer between 0-255: ";
  cin >> binaryNum;
  WriteAllLeds(pBase, binaryNum);
  ReadAllSwitches(pBase);
	// Done 
  // WriteAllLeds(pBase, 0);	
	Finalize(pBase, fd);
 
  return 0;
}
