/// Copyright 2023 Daniel Perez-Morera
/**
  *  C++ class to encapsulate Unix shared memory intrinsic structures and system calls
  *  Author: Operating systems (Francisco Arroyo)
  *  Version: 2023/Mar/15
  *
  * Ref.: https://en.wikipedia.org/wiki/Shared_memory
  *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>

#include "MailBox.h"

#define MAXDATA 1024
#define KEY 0xC10596

/**
  *  Class constructor
  *
  *  Must call "msgget" to create a mailbox
  *
  *  MailBoxkey is your student id number: 0xA12345 (to represent as hexadecimal value)
  *  size = 1
  *  MailBoxflg: IPC_CREAT | 0600
  *
 **/
MailBox::MailBox() {
   int st;
   st = msgget( KEY, IPC_CREAT | 0600 );
   if ( -1 == st ) {
      perror( "MailBox::MailBox" );
      exit( -1 );
   }

   this->id = st;

}


/**
  *   Class destructor
  *
  *   Must call msgctl
  *
 **/
MailBox::~MailBox() {
   int st = -1;
   // call msgctl to destroy this message queue
   st = msgctl(this->id, IPC_RMID, nullptr);
   // check for errors
   if ( -1 == st && errno != EINVAL) {
      perror( "MailBox::~MailBox" );
      exit( -1 );
   }
}


/**
  *   Send method
  *
  *   Need to call msgsnd to receive a pointer to shared memory area
  *
  *   other fields must come as parameters, or build a specialized struct
  *
 **/
int MailBox::send( long type, void * buffer, int numBytes ) {
   int st = -1;

   // must declare a msgbuf variable and set all the fields
   struct msgbuf {
      long type;   // this field must exist at first place
      char data[ MAXDATA ];        // char array for simplicity
      // user can define other fields
   } m;
   memset(m.data, 0, MAXDATA);
   m.type = type;
   memcpy( (void * ) m.data, buffer, numBytes );
   // set other fields if necessary

   // use msgsnd system call to send message to a queue
   st = msgsnd(this->id, (void*)&m, sizeof(m.data), 0);
   if ( -1 == st && errno != EINVAL ) {
      std::cout << errno << std::endl;
      perror( "MailBox::send" );
      exit( -1 );
   }
   return st;
}


/**
  *   receive method
  *
  *   Need to call msgrecv to receive messages from queue
  *
  *   Remember rules to receive message, see documentation
  *
 **/
int MailBox::recv( long type, void * buffer, int capacity ) {
   int st = -1;

   // must declare a msgbuf variable 
   struct msgbuf {
      long type;  // this field must exist at first place
      char data[ MAXDATA ];  // char array for simplicity
      // user can define other fields
   } m;
   memset(m.data, 0, MAXDATA);
   // use msgrcv system call to receive a message from the queue
   st = msgrcv(this->id, (void*)&m, sizeof(m.data), type, 0);
   // copy data from m to parameter variables
   memset(buffer, 0, capacity);
   memcpy( buffer, (void * ) m.data, capacity);
   return st;  // Return number of byte of the messege
}

