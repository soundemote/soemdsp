#ifdef __APPLE__
#include <stdint.h>   // for uint32_t
#include <limits.h>   // for CHAR_BIT

uint32_t _rotl (uint32_t n, unsigned int c)
{
  const unsigned int mask = (CHAR_BIT*sizeof(n)-1);
  
  assert ( (c<=mask) &&"rotate by type width or more");
  c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
  return (n<<c) | (n>>( (-c)&mask ));
}

uint32_t _rotr (uint32_t n, unsigned int c)
{
  const unsigned int mask = (CHAR_BIT*sizeof(n)-1);
  
  assert ( (c<=mask) &&"rotate by type width or more");
  c &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
  return (n>>c) | (n<<( (-c)&mask ));
}
#endif

#include "KeyGenerator.h"

namespace se {

//-------------------------------------------------------------------------------------------------
// construction/destruction:

KeyGenerator::KeyGenerator(int productIndex) : productIndex(productIndex)
{
  serialNumber           = 0;
  licenseeNameAndAddress = NULL;
}

KeyGenerator::~KeyGenerator()
{
  // free memory for the name:
  if( licenseeNameAndAddress != NULL )
  {
    delete[] licenseeNameAndAddress;
    licenseeNameAndAddress = NULL;
  }
}

//-------------------------------------------------------------------------------------------------
// setup:

void KeyGenerator::setProductIndex(productIndices newProductIndex)
{
  productIndex = (unsigned long int)newProductIndex;
}

void KeyGenerator::setSerialNumber(unsigned long newSerialNumber)
{
  serialNumber = newSerialNumber;
}

void KeyGenerator::setEncodedSerialNumber(char *newEncodedSerialNumber)
{
  int  i;
  char decodedCharArray[8];
  for(i=0; i<8; ++i)
    decodedCharArray[i] = newEncodedSerialNumber[i];
  decodeCharacterArray(decodedCharArray, 8);

  unsigned long accu = 0;
  unsigned long tmp;
  char          currentChar;
  for(i=0; i<8; ++i)
  {
    currentChar  = decodedCharArray[7-i];
    tmp          = (unsigned long) currentChar - 65;
    accu         = accu << 4;
    accu        += tmp;
  }
  serialNumber = descrambleBits(accu);
}

void KeyGenerator::setLicenseeNameAndEmail(char *newLicenseeNameAndAddress)
{
  // free old and allocate new memory for the name:
  if( licenseeNameAndAddress != NULL )
  {
    delete[] licenseeNameAndAddress;
    licenseeNameAndAddress = NULL;
  }
  if( newLicenseeNameAndAddress != NULL )
  {
    int newLength = (int) strlen(newLicenseeNameAndAddress);
    licenseeNameAndAddress  = new char[newLength+1];
    for(int c=0; c<=newLength; c++) // the <= is valid here, because we have one more cell allocated
      licenseeNameAndAddress[c] = newLicenseeNameAndAddress[c];
  }
}

//-------------------------------------------------------------------------------------------------
// inquiry:

unsigned long KeyGenerator::getSerialNumber()
{
  return serialNumber;
}

char* KeyGenerator::getKeyString(int numChars)
{
  char* keyString = new char[numChars+1];

  // initialize the (pseudo-) random number generator:
  unsigned long m = 32768;  // 32768=2^15
  unsigned long a = (36739 * productIndex)  % m;
  unsigned long c = (17389 * serialNumber)  % m;
  unsigned long s = serialNumber            % m;

  randomNumberGenerator01.setFactor(a);
  randomNumberGenerator01.setAdditiveConstant(c);
  randomNumberGenerator01.setStateFromString(licenseeNameAndAddress);
  randomNumberGenerator02.setState(s);

  // fill the char-array with random characters, derived from the generated random numbers:
  unsigned long currentLongInteger;
  unsigned long bitMask = 127;
  char          currentCharacter;
  for(int i = 0; i < numChars; ++i)
  {
    if( PrimeNumbers::isPrime(i) )
      currentLongInteger = randomNumberGenerator01.getRandomNumber();
    else
      currentLongInteger = randomNumberGenerator02.getRandomNumber();

    if( ((i+1) % 3) == 0 )
    {
      unsigned long tmp = randomNumberGenerator01.getRandomNumber();
      randomNumberGenerator02.setState(tmp);
      tmp = randomNumberGenerator02.getRandomNumber();
      randomNumberGenerator01.setState(tmp);
    }

    currentCharacter   = (char) (currentLongInteger & bitMask);
     // currentCharacter is in the range 0...127

    currentCharacter   = currentCharacter % 26;
     // currentCharacter is now in the range 0...25

    currentCharacter  += 65;
     // currentCharacter is now in the range 65...90, these are the ASCII-codes
     // for uppercase letters A-Z

    keyString[i]      = currentCharacter;
  }

  keyString[numChars] = '\0'; // zero termination
  return keyString;
}

bool KeyGenerator::testKeyString(char *keyToTest, int numChars)
{
  char* correctKey = getKeyString(numChars);
  bool keyIsValid = true;
  for(int i = 0; i < numChars; ++i)
  {
    if( keyToTest[i] != correctKey[i] )
      keyIsValid = false;
  }
  delete[] correctKey;
  return keyIsValid;
}

//-------------------------------------------------------------------------------------------------
// others:

//INLINE unsigned long bitReverse(unsigned long number, unsigned long numBits) // always_inline fails on linux
inline unsigned long bitReverse(unsigned long number, unsigned long numBits)
{
  unsigned long result = 0;
  for(unsigned long n = 0; n < numBits; n++)
  {
    // leftshift the previous result by one and accept the new LSB of the current number on the
    // right:
    result = (result << 1) + (number & 1);

    // rightshift the number to make the second bit from the right to the new LSB:
    number >>= 1;
  }
  return result;
}

void KeyGenerator::decodeCharacterArray(char *y, unsigned char n)
{
  int i;
  for(i=0; i<n; ++i)
    y[i] -= 65;
  char* yTmp = new char[n];
  for(i=0; i<n; ++i)
    yTmp[i] = y[i];

  unsigned char tmpState = (initialState*productIndex) % 16;
  y[0] = inverseFourBitMapping2( yTmp[0] ^ fourBitMapping1(tmpState) );
  for(i=1; i<n; ++i)
    y[i] = inverseFourBitMapping2( yTmp[i] ^ fourBitMapping1(yTmp[i-1]) );

  for(i=0; i<n; ++i)
    y[i] += 65;
  delete[] yTmp;
}

unsigned long KeyGenerator::descrambleBits(unsigned long y)
{
  unsigned long x = y;

  for(int i=1; i<27; ++i)
  {
    x = x ^ xorConstant2;
    x = _rotr(x, 7);
    x = bitReverse(x, (unsigned long) 32);
    x = x ^ xorConstant1;
  }

  return x;
}

unsigned char KeyGenerator::fourBitMapping1(unsigned char x)
{
	jassert(x >= 0 && x <= 15);

	const vector<int> ret{ 9, 3, 8, 14, 11, 7, 0, 12, 15, 2, 13, 4, 1, 6, 5, 10 };

	return (unsigned char)ret[x];
}

unsigned char KeyGenerator::inverseFourBitMapping1(unsigned char y)
{
	jassert(y >= 0 && y <= 15);

	const vector<int> ret{ 6, 12, 9, 1, 11, 14, 13, 5, 2, 0, 15, 4, 7, 10, 3, 8 };

	return (unsigned char)ret[y];
}

unsigned char KeyGenerator::inverseFourBitMapping2(unsigned char y)
{
	jassert(y >= 0 && y <= 15);

	const vector<int> ret{ 2, 7, 10, 6, 12, 9, 14, 1, 3, 13, 5, 15, 0, 11, 8, 4 };

	return (unsigned char)ret[y];
}

//=================================================================================================
// these had to be moved from the cpp-file to the .h file because gcc can't
// inline these functions - this is actually not good because if this code
// becomes part of the plugin binary, a cracker may have access to it to make
// a keygen - hmmm...

//-----------------------------------------------------------------------------------------------
// these functions should be inlined to make sure that their code does not appear in any product
// but the KeyGenerator to prevent crackers from extracting that part of code from a product:

char* KeyGenerator::getEncodedSerialNumber()
{
  char* encodedSerial = new char[9];
  unsigned long a = scrambleBits(serialNumber);
  unsigned long aTmp;
  char          currentChar;
  for(int i=0; i<8; ++i)
  {
    aTmp             = a & 0x0000000F;             // mask to retain only the 4 rightmost bits
    currentChar      = (char)(aTmp+65);            // map 4-bit pattern to a character
    encodedSerial[i] = currentChar;                // write character into string
    a                = a >> 4;                     // rightshift by 4 bits
  }
  encodeCharacterArray(encodedSerial, 8);
  encodedSerial[8] = '\0';                         // zero termination
  return encodedSerial;
}

//INLINE void KeyGenerator::encodeCharacterArray(char* x, unsigned char n) // fails on linux
void KeyGenerator::encodeCharacterArray(char* x, unsigned char n)
{
  int i;
  for(i=0; i<n; ++i)
    x[i] -= 65;

  unsigned char tmpState = (initialState*productIndex) % 16;
  x[0] = (fourBitMapping1(tmpState) ^ fourBitMapping2(x[0]));
  for(i=1; i<n; ++i)
    x[i] = (fourBitMapping1(x[i-1]) ^ fourBitMapping2(x[i]));

  for(i=0; i<n; ++i)
    x[i] += 65;
}

//INLINE unsigned long KeyGenerator::scrambleBits(unsigned long x) // fails on linux
inline unsigned long KeyGenerator::scrambleBits(unsigned long x)
{
  unsigned long y = x;

  for(int i=1; i<27; ++i)
  {
    y = y ^ xorConstant1;                   // bitwise XOR
    y = bitReverse(y, (unsigned long)32);   // permutation
    y = _rotl(y, 7);                        // permutation
    y = y ^ xorConstant2;                   // bitwise XOR
  }

  return y;
}

//INLINE unsigned char KeyGenerator::fourBitMapping2(unsigned char x) // fails on linux
inline unsigned char KeyGenerator::fourBitMapping2(unsigned char x)
{
	jassert(x >= 0 && x <= 15);

	const vector<int> ret{ 12, 7, 0, 8, 15, 10, 3, 1, 14, 5, 2, 13, 4, 9, 6, 11 };

	return (unsigned char)ret[x];
}

}
