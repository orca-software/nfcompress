#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <file.h>

class FileTest : public CppUnit::TestCase
{
public:
  CPPUNIT_TEST_SUITE(FileTest);
  CPPUNIT_TEST_SUITE_END();
};


int main()
{
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(FileTest::suite());
  if (runner.run()) {
    return 0;
  } else {
    return 1;
  }
}
