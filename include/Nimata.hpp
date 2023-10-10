/*---author-----------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Nimata

-----licence----------------------------------------------------------------------------------------
 
MIT License

Copyright (c) 2023 Justin Asselin (juste-injuste)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 
-----versions---------------------------------------------------------------------------------------

-----description------------------------------------------------------------------------------------

-----inclusion guard------------------------------------------------------------------------------*/
#ifndef NIMATA_HPP
#define NIMATA_HPP
// --necessary standard libraries-------------------------------------------------------------------
#include <thread>
#include <ostream>
#include <iostream>
// --Nimata library---------------------------------------------------------------------------------
namespace Nimata
{
  namespace Version
  {
    const long MAJOR = 000;
    const long MINOR = 001;
    const long PATCH = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
// --Nimata library: frontend forward declarations--------------------------------------------------
  inline namespace Frontend
  {
    // output ostream
    std::ostream out_ostream{std::cout.rdbuf()};
    // error ostream
    std::ostream err_ostream{std::cerr.rdbuf()};
    // warning ostream
    std::ostream wrn_ostream{std::cerr.rdbuf()};
  }
// --Nimata library: frontend struct and class definitions------------------------------------------
  inline namespace Frontend
  {
    
  }
// --Nimata library: backend forward declaration----------------------------------------------------
  namespace Backend
  {

  }
// --Nimata library: frontend definitions-----------------------------------------------------------
  inline namespace Frontend
  {
    
  }
// --Nimata library: backend definitions------------------------------------------------------------
  namespace Backend
  {

  }
}
#endif
