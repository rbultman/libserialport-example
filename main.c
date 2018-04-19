/*
 * An example program that makes use of libserialport for serial port access.
 *
 * Tested on Raspberry Pi.
 *
 * To test, connect an Arduino or other board that appears as a serial port
 * to the Pi via USB.  The Arduino sketch should just periodically print
 * something via Serial.println.
 *
 * Author: Rob Bultman
 * License: MIT
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libserialport.h>
#include <unistd.h>

// Configuration
static const char portNameBase[] = "/dev/ttyACM";
static const int baudRate = 9600;
static const int bits = 8;
static const int stopBits = 1;
static const int parity = SP_PARITY_NONE;

/*
 * Wait for a port to appear.  Return a pointer to the port.
 * Illustrates getting a list of ports.
 */
struct sp_port * GetPort(void) {
   struct sp_port **portList;
   int retval;
   struct sp_port *port = NULL;
   int i;
   char * nameMatch; 
   char * portName;

   port = NULL;

   do {
      retval = sp_list_ports(&portList);

      if (retval == SP_OK) {
         nameMatch = NULL;
         for(i=0; portList[i] != NULL; i++) {
            portName = sp_get_port_name(portList[i]);
            nameMatch = strstr(portName, portNameBase);
            if (nameMatch != NULL) {
               break;
            }
         }
         if (nameMatch != NULL) {
            sp_copy_port(portList[i], &port);
         } else {
            puts("Waiting for a serial port to appear.");
            sleep(1);
         }
      }

      sp_free_port_list(portList);
   } while (port == NULL);

   return port;
}

/*
 * Configure the serial port.
 */
int ConfigureSerialPort(struct sp_port *port) {
   int retval = 0;

   if (SP_OK != sp_set_baudrate(port, baudRate))
   {
      puts("Unable to set port baudrate.");
      retval = -1;
   } else if(SP_OK != sp_set_bits(port, bits)) {
      puts("Unable to set port width.");
      retval = -1;
   } else if (SP_OK !=  sp_set_parity(port, parity)) {
      puts("Unable to set port parity.");
      retval = -1;
   } else if (SP_OK != sp_set_stopbits(port, stopBits)) {
      puts("Unable to set port stop bits.");
      retval = -1;
   } else {
      puts("Port configured.");
   }

   return retval;
}

/* 
 * Wait for an event on the serial port.
 * Illustrates use of sp_wait.
 */
int WaitForEventOnPort(struct sp_port *port) {
   int retval;
   struct sp_event_set *eventSet = NULL;

   retval = sp_new_event_set(&eventSet);
   if (retval == SP_OK) {
      retval = sp_add_port_events(eventSet, port, SP_EVENT_RX_READY | SP_EVENT_ERROR);
      if (retval == SP_OK) {
         retval = sp_wait(eventSet, 10000);
      } else {
         puts("Unable to add events to port.");
         retval = -1;
      }
   } else {
      puts("Unable to create new event set.");
      retval = -1;
   }
   sp_free_event_set(eventSet);

   return retval;
}

/*
 * Read data from the port.
 */
int ReadFromPort(struct sp_port *port) {
   int count = 0;
   int bytesWaiting;
   char buf[256];
   int retval;
   int i;

   sp_flush(port, SP_BUF_INPUT);

   while (count < 4) {
      WaitForEventOnPort(port);
      bytesWaiting = sp_input_waiting(port);
      if (bytesWaiting > 0) {
         memset(buf, 0, sizeof(buf));
         retval = sp_blocking_read(port, buf, sizeof(buf)-1, 10);
         if (retval < 0) {
            printf("Error reading from serial port: %d\r\n", retval);
            retval = -1;
            break;
         } else {
            for(i=0; i<retval; i++) {
               printf("%c", buf[i]);
               if (buf[i] == 13) {
                  count++;
               }
            }
         }
      } else if (bytesWaiting < 0) {
         printf("Error getting bytes available from serial port: %d\r\n", bytesWaiting);
         retval = -1;
         break;
      }
      retval = 0;
   }
   return retval;
}

int main(void) {
   int retval;
   int error = 0;
   struct sp_port *port;

   do {
      port = GetPort();

      if (port == NULL) {
         puts("Did not find a suitable port.");
      } else {
         printf("Using %s\r\n", sp_get_port_name(port));
         retval = sp_open(port, SP_MODE_READ | SP_MODE_WRITE);
         if (retval == SP_OK) {
            puts("Serial port successfully opened.");

            if (ConfigureSerialPort(port) == 0) {
               error = ReadFromPort(port);
            }

            puts("Closing serial port.");
            retval = sp_close(port);
            if(retval == SP_OK) {
               puts("Serial port closed.");
            } else {
               puts("Unable to close serial port.");
            }

         } else {
            puts("Error opening serial port.");
         }
      }

      if (port != NULL) {
         sp_free_port(port);
      }
   } while (error != 0);

   return 0;
}
