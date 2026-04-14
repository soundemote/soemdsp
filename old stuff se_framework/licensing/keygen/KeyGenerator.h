#pragma once

#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __forceinline
#else
//#define INLINE inline
#define INLINE __attribute__((always_inline))
#endif
#endif // !INLINE

#include <stdlib.h>

#include "PrimeNumbers.h"
#include "RandomNumberGenerator01.h"
#include "RandomNumberGenerator02.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace se {

/** This class generates key-strings for the commercial product-line.

\todo make a KeyValidator-class which does only generate certain parts of the key */

class KeyGenerator
{

public:
	/** This is an enumeration of the products for which keys can be generated. */
	enum productIndices
	{
		invalid, // can't be 0, because it's used as index in the combobox
		TESTPLUGIN,
		PRETTYSCOPE,
		CHAOSFLY,
		MUSHROOMGENERATOR,
		NYQUISTGENERATOR,
		SPIRALGENERATOR,
		TORUSGENERATOR,
		RADARGENERATOR,
		SPHEREGENERATOR,
		FMD,
		CHAOSARP,
	};

	vector<string> productStrings{ 
		"invalid", 
		"TestPlugin", 
		"PrettyScope", 
		"Chaosfly", 
		"MushroomGenerator", 
		"NyquistGenerator", 
		"Spiral_Generator",
		"Torus_Generator",
		"RadarGenerator",
		"SphereGenerator",
		"FMD",
		"ChaosArp"
	};

  //---------------------------------------------------------------------------------------------
  // construction/destruction:

	KeyGenerator(int productIndex = 0);
  /**< Constructor */

  ~KeyGenerator();
  /**< Destructor */

  //---------------------------------------------------------------------------------------------
  // setup:

  void setProductIndex(productIndices newProductIndex);
  /**< Sets the product index (see enum above) for which the key has to be
  generated. */

  void setSerialNumber(unsigned long newSerialNumber);
  /**< Sets the serial number for which the key has to be generated. */

  void setEncodedSerialNumber(char* newEncodedSerialNumber);
  /**< Sets the serial number for which the key has to be generated in encoded form. The
  character array must be of length 8 plus the terminating zero. */

  void setLicenseeNameAndEmail(char* newLicenseeNameAndAddress);
  /**< Sets the name of the licensee for whom the key is to be generated. */

  //---------------------------------------------------------------------------------------------
  // inquiry:

  unsigned long getSerialNumber();
  /**< Returns the serial number as a clear integer number. */

  char* getEncodedSerialNumber();
  /**< Returns a pointer to a zero terminated string which represents the serial number in
  encoded form. The encoded form will consist of 8 characters, additionaly there is the
  teminating zero. The calling function must take care of deleting the array when it is not
  needed anymore. */

  char* getKeyString(int numChars);
  /**< Generates the key according to the settings of productIndex and serialNumber. The calling
  function must take care of deleting the array when it is not needed anymore. The key will
  consist of numChars characters plus the terminating zero. */

  bool testKeyString(char* keyToTest, int numChars);
  /**< Tests, if the the key is valid (fits to the settings of productIndex and serialNumber).
  If it is valid, it returns true, otherwise false. */

  //---------------------------------------------------------------------------------------------
  // de/encoding

  void encodeCharacterArray(char* x, unsigned char n);
  /**< Encodes a character array of length n. */

  void decodeCharacterArray(char* y, unsigned char n);
  /**< Decodes a character array of length n. */

  //=============================================================================================

	String name;
	String email;

protected:

  INLINE unsigned long scrambleBits(unsigned long x);
  /**< Scrambles the bits of an unsigned long number. */

  unsigned long descrambleBits(unsigned long y);
  /**< De-scrambles the bits of an unsigned long number. */

  unsigned char fourBitMapping1(unsigned char x);
  /**< Maps a 4-bit word (expected to be contained in the 4 least significant bits) to another
  4 bit word. */

  unsigned char inverseFourBitMapping1(unsigned char y);
  /**< Inverts the mapping of fourBitMapping1. */

  INLINE unsigned char fourBitMapping2(unsigned char x);
  /**< Maps a 4-bit word (expected to be contained in the 4 least significant bits) to another
  4 bit word. */

  unsigned char inverseFourBitMapping2(unsigned char y);
  /**< Inverts the mapping of fourBitMapping1. */

  static const unsigned long xorConstant1 = 0x55555555;
  static const unsigned long xorConstant2 = 0x95468725;
  static const unsigned long initialState = 13;

  unsigned long productIndex;
  unsigned long serialNumber;

  //unsigned char licenseeName[9];
  char* licenseeNameAndAddress;

  // we have an embedded RandomNumberGenerator object, which will be set up
  // according to productIndex and serialNumber:
  RandomNumberGenerator01 randomNumberGenerator01;
  RandomNumberGenerator02 randomNumberGenerator02;
};

}
