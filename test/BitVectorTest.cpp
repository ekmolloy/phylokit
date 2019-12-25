#include "catch2.hpp"
#include "BitVector.hpp"

TEST_CASE("BitVector created with size has all bits zero") {
  size_t sz = 5000;
  BitVectorFixed bvf(sz);
  REQUIRE(bvf.size == sz);
  for (size_t i = 0; i < sz; i++) {
    REQUIRE(bvf.get(i) == 0);
  }
}

TEST_CASE("Bitvector set and get work") {
  size_t sz = 5000;
  BitVectorFixed bvf(sz);
  for (size_t i = 0; i < sz; i++) {
    REQUIRE(bvf.get(i) == 0);
    bvf.set(i);
    REQUIRE(bvf.get(i) == 1);
  }
}


TEST_CASE("Bitvector unset") {
  size_t sz = 5000;
  BitVectorFixed bvf(sz);
  for (size_t i = 0; i < sz; i++) {
    bvf.set(i);
  }
  for (size_t i = 0; i < sz; i++) {
    bvf.unset(i);
    REQUIRE(bvf.get(i) == 0);
  }
}

TEST_CASE("Bitvector copy constructor") {
  size_t sz = 5000;
  BitVectorFixed bvf(sz);
  bvf.set(4);
  bvf.set(75);
  bvf.set(4998);
  BitVectorFixed copy(bvf);
  REQUIRE(bvf == copy);
}

TEST_CASE("Bitvector ffs") {
  size_t sz = 5000;
  BitVectorFixed bvf(sz);
  REQUIRE(bvf.ffs() == -1);
  for (int i = 4999; i >= 0; i--) {
    bvf.set(i);
    REQUIRE(bvf.ffs() == i);
  }
}

TEST_CASE("BitVector popcount") {
  size_t sz = 5000;
  BitVectorFixed bvf(sz);
  REQUIRE(bvf.popcount() == 0);
  for (size_t i = 0; i < sz; i++) {
    bvf.set(i);
    REQUIRE(bvf.popcount() == i + 1);
  }
}

TEST_CASE("BitVector ")