#include <boost/test/unit_test.hpp>

#include <iostream>
#include <set>
#include <aq/AQMatrix.h>

BOOST_AUTO_TEST_SUITE(AQMatrix)

BOOST_AUTO_TEST_CASE(test_temporary_table)
{
  uint32_t status = 0;
  uint32_t pos = 0;
  uint64_t group = 0;
  uint64_t invalid = 0;
  uint64_t nb_row = 0;

  FILE * fd = fopen("B001T0001P000000000000.TMP", "rb");
  fread(&status, sizeof(uint32_t), 1, fd);

  std::set<uint32_t> positions;
  while (!feof(fd))
  {
    fread(&group, sizeof(uint64_t), 1, fd);
    if (feof(fd)) break;
    fread(&invalid, sizeof(uint64_t), 1, fd);
    fread(&pos, sizeof(uint32_t), 1, fd);
    BOOST_CHECK(positions.find(pos) == positions.end());
    positions.insert(pos);
    std::cout << "[ " << pos << " ; " << group << " ; " << invalid << " ] " << std::endl;
    nb_row ++;
  }

  std::cout << nb_row << " rows parsed" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
