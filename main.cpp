#include <iostream>
#include <string>
#include <stdio.h>
#include "include/incbin.h"
#include "include/timing.cpp"

#include "pharovm/pharo.h"
#include "pharovm/imageAccess.h"
#include "include/pharoClient.h"

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX g_

INCBIN(PharoImage, "data.txt");
#define ImageFileDefaultName   "Pharo.image"
// Macro to embed the default efficiently Pharo Image file
// data in the binary (using incbin.h, by Dale Weiler).
// This macro invocation will declare the following three variables
//     const unsigned char        gPharoImageData[];  // a pointer to the embedded data
//     const unsigned char *const gPharoImageEnd;     // a marker to the end
//     const unsigned int         gPharoImageSize;    // the size of the embedded file
// Note that this does not work in Microsoft Visual Studio.
#define MEMORY_STREAM(label) \
  memstream( (char*) ((g_ ## label ## _data)),              \
             (char*) ((g_ ##label ## _data) + (g_ ## label ##_size)))

using namespace std;


struct membuf: std::streambuf {
  membuf(char const* base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
  virtual ~membuf() = default;
};

struct memstream: virtual membuf, std::istream {

  memstream(char const* base, char* const end)
    : membuf(base, reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(base) )
    , std::istream(static_cast<std::streambuf*>(this)) { }

  memstream(char const* base, size_t size)
    : membuf(base, size)
    , std::istream(static_cast<std::streambuf*>(this)) { }
};

// Read all lines from some input stream and print on stdout.
void print_lines(std::istream& is)
{
  std::string line;
  int  n = 0;
  while(std::getline(is, line) && n < 25)
    std::cout << " line[" << n++ << "] = " << line << std::endl;
}

int main(int argc, char *argv[])
{
  double t0 = now();

  std::puts("\n Read from memory stream -------");
  auto is = MEMORY_STREAM(PharoImage);
  print_lines(is);

  int exitCode = vm_main(argc, argv);

  double t1 = now();
  printf("Time to read: %g s.\n", t1 - t0 );
  return exitCode;
}
