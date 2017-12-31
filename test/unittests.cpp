#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <file.h>
#include <compress.h>

const char *test_data_dir = NULL;

class FileTest : public CppUnit::TestCase
{
  void test_file_open() {
    std::string filename = test_data_dir;
    filename += "/";
    filename += "nfcapd.test1";

    nf_file_t *file = file_load(filename.c_str(), NULL);
    CPPUNIT_ASSERT(file);
    file_free(&file);
  }
  void test_decompress_lzo() {
    std::string filename = test_data_dir; 
    filename += "/"; 
    filename += "nfcapd.testlzo";

    nf_file_t *file = file_load(filename.c_str(), &decompressor);
    CPPUNIT_ASSERT(file);
    CPPUNIT_ASSERT(file->blocks[0]->file_compression == compressed_lzo);
    CPPUNIT_ASSERT(file->blocks[0]->compression == compressed_none);
    file_free(&file);
  }
public:
  CPPUNIT_TEST_SUITE(FileTest);
  CPPUNIT_TEST(test_file_open);
  CPPUNIT_TEST(test_decompress_lzo);
  CPPUNIT_TEST_SUITE_END();
};


int main(int argc, char *argv[])
{

  if (argc > 1) {
    test_data_dir = argv[1];
  }
  else {
    test_data_dir = ".";
  }
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(FileTest::suite());
  if (runner.run()) {
    return 0;
  } else {
    return 1;
  }
}
