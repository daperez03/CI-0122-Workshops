// bitmap.h 
//	Data structures defining a bitmap -- an array of bits each of which
//	can be either on or off.
//
//	Represented as an array of unsigned integers, on which we do
//	modulo arithmetic to find the bit we are interested in.
//
//	The bitmap can be parameterized with with the number of bits being 
//	managed.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef BITMAP_H
#define BITMAP_H

#include "copyright.h"
#include "utility.h"
#include "openfile.h"

// Definitions helpful for representing a bitmap as an array of integers
#define BitsInByte 	8
#define BitsInWord 	32

// The following class defines a "bitmap" -- an array of bits,
// each of which can be independently set, cleared, and tested.
//
// Most useful for managing the allocation of the elements of an array --
// for instance, disk sectors, or main memory pages.
// Each bit represents whether the corresponding sector or page is
// in use or free.

class BitMap {
  public:
    /// @brief Initialize a bitmap, with "nitems" bits
    /// @param nitems Number of items disable
    /// @details Initially, all bits are cleared.
    BitMap(int nitems);
    
    /// @brief De-allocate bitmap
    ~BitMap();
    
    /// @brief // Set the "nth" bit
    /// @param which Which bit
    void Mark(int which);

    /// @brief Clear the "nth" bit
    /// @param which Which bit
    void Clear(int which);

    /// @brief Is the "nth" bit set?
    /// @param which Which bit
    /// @return true if is set, false if is unset
    bool Test(int which);

    /// @brief Find space between bit map
    /// @details effect, set the bit.
    /// @return # of a clear bit, and as a side, if no bits are clear, return -1
    int Find();
		
    /// @brief Return the number of clear bits
    /// @return number of clear bits
    int NumClear();

    /// @brief Print contents of bitmap
    void Print();
    
    // These aren't needed until FILESYS, when we will need to read and 
    // write the bitmap to a file

    /// @brief Fetch contents from disk
    /// @param file Contents
    void FetchFrom(OpenFile *file); 
    
    /// @brief Write contents to disk
    /// @param file Contents
    void WriteBack(OpenFile *file);

  private:
    ///@brief number of bits in the bitmap
    int numBits;

    /// @brief number of words of bitmap storage
    int numWords;
      // (rounded up if numBits is not a
      //  multiple of the number of bits in
      //  a word)
    
    /// @brief Bit storage
    unsigned int *map;
};

#endif // BITMAP_H
