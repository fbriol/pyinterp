// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/period.hpp"

#include <gtest/gtest.h>

namespace pyinterp {

TEST(PeriodTest, ConstructionAndProperties) {
  // [10, 20)
  Period p1(10, 20, false);
  EXPECT_EQ(p1.begin, 10);
  EXPECT_EQ(p1.last, 19);
  EXPECT_EQ(p1.end(), 20);
  EXPECT_EQ(p1.duration(), 10);
  EXPECT_FALSE(p1.is_null());

  // [10, 19]
  Period p2(10, 19, true);
  EXPECT_EQ(p2.begin, 10);
  EXPECT_EQ(p2.last, 19);
  EXPECT_EQ(p2.end(), 20);
  EXPECT_EQ(p1, p2);

  // Null period
  Period null_p(20, 10, false);
  EXPECT_TRUE(null_p.is_null());
}

TEST(PeriodTest, ContainsAndPosition) {
  Period p(10, 20);  // [10, 20] -> [10, 21)

  // Point containment
  EXPECT_TRUE(p.contains(10));
  EXPECT_TRUE(p.contains(20));
  EXPECT_FALSE(p.contains(9));
  EXPECT_FALSE(p.contains(21));

  // Period containment
  EXPECT_TRUE(p.contains(Period(12, 18)));
  EXPECT_TRUE(p.contains(p));
  EXPECT_FALSE(p.contains(Period(9, 15)));

  // Relative position
  EXPECT_TRUE(p.is_after(9));
  EXPECT_FALSE(p.is_after(10));
  EXPECT_TRUE(p.is_before(21));
  EXPECT_FALSE(p.is_before(20));
}

TEST(PeriodTest, IntersectionAndMerge) {
  Period p1(10, 20);  // [10, 20]
  Period p2(15, 25);  // [15, 25]
  Period p3(21, 30);  // [21, 30] (Adjacent to p1)
  Period p4(30, 40);  // [30, 40] (Disjoint)

  // Intersects
  EXPECT_TRUE(p1.intersects(p2));
  EXPECT_FALSE(p1.intersects(p3));
  EXPECT_FALSE(p1.intersects(p4));

  // Intersection
  Period i12 = p1.intersection(p2);
  EXPECT_EQ(i12.begin, 15);
  EXPECT_EQ(i12.last, 20);

  EXPECT_TRUE(p1.intersection(p3).is_null());

  // Adjacent
  EXPECT_TRUE(p1.is_adjacent(p3));
  EXPECT_FALSE(p1.is_adjacent(p2));  // Overlapping, not just adjacent

  // Merge
  Period m12 = p1.merge(p2);  // [10, 25]
  EXPECT_EQ(m12.begin, 10);
  EXPECT_EQ(m12.last, 25);

  Period m13 = p1.merge(p3);  // [10, 30]
  EXPECT_EQ(m13.begin, 10);
  EXPECT_EQ(m13.last, 30);

  EXPECT_TRUE(p1.merge(p4).is_null());
}

TEST(PeriodTest, ExtendAndShift) {
  Period p(10, 20);  // [10, 20]

  // Extend with point inside - no change
  Period e1 = p.extend(15);
  EXPECT_EQ(e1.begin, 10);
  EXPECT_EQ(e1.last, 20);

  // Extend with point before - expands begin
  Period e2 = p.extend(5);
  EXPECT_EQ(e2.begin, 5);
  EXPECT_EQ(e2.last, 20);

  // Extend with point after - expands last
  Period e3 = p.extend(25);
  EXPECT_EQ(e3.begin, 10);
  EXPECT_EQ(e3.last, 25);

  // Shift forward
  Period s1 = p.shift(10);
  EXPECT_EQ(s1.begin, 20);
  EXPECT_EQ(s1.last, 30);
  EXPECT_EQ(s1.duration(), p.duration());

  // Shift backward
  Period s2 = p.shift(-5);
  EXPECT_EQ(s2.begin, 5);
  EXPECT_EQ(s2.last, 15);
  EXPECT_EQ(s2.duration(), p.duration());

  // Shift by zero
  Period s3 = p.shift(0);
  EXPECT_EQ(s3.begin, p.begin);
  EXPECT_EQ(s3.last, p.last);
}

TEST(PeriodTest, IsClose) {
  Period p(100, 200);  // [100, 200]

  // Exact boundaries
  EXPECT_TRUE(p.is_close(100, 0));
  EXPECT_TRUE(p.is_close(200, 0));

  // Inside period
  EXPECT_TRUE(p.is_close(150, 0));

  // Within tolerance before begin
  EXPECT_TRUE(p.is_close(90, 15));
  EXPECT_FALSE(p.is_close(90, 5));

  // Within tolerance after last
  EXPECT_TRUE(p.is_close(210, 15));
  EXPECT_FALSE(p.is_close(210, 5));

  // Outside tolerance
  EXPECT_FALSE(p.is_close(50, 10));
  EXPECT_FALSE(p.is_close(250, 10));
}

TEST(PeriodTest, NullPeriodBehavior) {
  Period null_p;
  EXPECT_TRUE(null_p.is_null());
  EXPECT_EQ(null_p.begin, 0);
  EXPECT_EQ(null_p.last, -1);
  EXPECT_EQ(null_p.end(), 0);
  EXPECT_EQ(null_p.duration(), 0);  // end() - begin = 0 - 0 = 0

  Period valid_p(10, 20);

  // Null period operations
  EXPECT_FALSE(null_p.contains(0));
  EXPECT_FALSE(null_p.contains(valid_p));
  EXPECT_FALSE(null_p.is_after(5));
  EXPECT_FALSE(null_p.is_before(5));
  EXPECT_FALSE(null_p.intersects(valid_p));
  EXPECT_TRUE(null_p.intersection(valid_p).is_null());
  EXPECT_TRUE(null_p.merge(valid_p).is_null());
}

TEST(PeriodTest, NegativeValues) {
  // Period with negative values
  Period p(-100, -50);  // [-100, -50]
  EXPECT_EQ(p.begin, -100);
  EXPECT_EQ(p.last, -50);
  EXPECT_EQ(p.end(), -49);
  EXPECT_EQ(p.duration(), 51);
  EXPECT_FALSE(p.is_null());

  // Contains negative point
  EXPECT_TRUE(p.contains(-75));
  EXPECT_FALSE(p.contains(-25));

  // Operations with negative periods
  Period p2(-75, -25);
  EXPECT_TRUE(p.intersects(p2));
  Period i = p.intersection(p2);
  EXPECT_EQ(i.begin, -75);
  EXPECT_EQ(i.last, -50);
}

TEST(PeriodTest, ComparisonOperators) {
  Period p1(10, 20);
  Period p2(10, 20);
  Period p3(10, 25);
  Period p4(15, 20);

  // Equality
  EXPECT_EQ(p1, p2);
  EXPECT_NE(p1, p3);
  EXPECT_NE(p1, p4);

  // Ordering (lexicographic on begin, then last)
  EXPECT_LT(p1, p3);
  EXPECT_LT(p1, p4);
  EXPECT_GT(p3, p1);
  EXPECT_LE(p1, p2);
  EXPECT_GE(p1, p2);
}

TEST(PeriodTest, EdgeCases) {
  // Single point period [5, 5]
  Period single(5, 5);
  EXPECT_EQ(single.begin, 5);
  EXPECT_EQ(single.last, 5);
  EXPECT_EQ(single.duration(), 1);
  EXPECT_TRUE(single.contains(5));
  EXPECT_FALSE(single.contains(6));

  // Adjacent periods
  Period p1(0, 10);   // [0, 10]
  Period p2(11, 20);  // [11, 20]
  EXPECT_TRUE(p1.is_adjacent(p2));
  EXPECT_TRUE(p2.is_adjacent(p1));
  EXPECT_FALSE(p1.intersects(p2));

  // Merge adjacent creates continuous period
  Period merged = p1.merge(p2);
  EXPECT_EQ(merged.begin, 0);
  EXPECT_EQ(merged.last, 20);
  EXPECT_FALSE(merged.is_null());
}

TEST(PeriodListTest, StorageAndSorting) {
  PeriodList list;
  list.push_back(Period(10, 15));
  list.push_back(Period(1, 5));
  list.push_back(Period(20, 25));

  EXPECT_EQ(list.size(), 3);
  EXPECT_FALSE(list.is_sorted_and_disjoint());

  list.sort();
  EXPECT_EQ(list[0].begin, 1);
  EXPECT_EQ(list[1].begin, 10);
  EXPECT_EQ(list[2].begin, 20);
  EXPECT_TRUE(list.is_sorted_and_disjoint());

  list.clear();
  EXPECT_EQ(list.size(), 0);
}

TEST(PeriodListTest, FindAndClose) {
  PeriodList list;  // ns default
  list.push_back(Period(100, 200));
  list.push_back(Period(300, 400));
  list.sort();

  // Find containing
  EXPECT_EQ(list.find_containing_index(150), 0);
  EXPECT_EQ(list.find_containing_index(300), 1);
  EXPECT_EQ(list.find_containing_index(250), -1);

  // Is close
  EXPECT_TRUE(list.is_close(90, 15));  // 90 is within 15 of 100
  EXPECT_FALSE(list.is_close(90, 5));
}

TEST(PeriodListTest, TotalDuration) {
  PeriodList list;
  EXPECT_EQ(list.total_duration(), 0);  // Empty list

  list.push_back(Period(0, 10));   // duration = 11
  list.push_back(Period(20, 30));  // duration = 11
  list.push_back(Period(40, 45));  // duration = 6

  EXPECT_EQ(list.total_duration(), 28);  // 11 + 11 + 6
}

TEST(PeriodListTest, OverlappingPeriods) {
  PeriodList list;
  list.push_back(Period(10, 20));
  list.push_back(Period(15, 25));  // Overlaps with first

  EXPECT_FALSE(list.is_sorted_and_disjoint());

  // After sorting, still overlapping
  list.sort();
  EXPECT_FALSE(list.is_sorted_and_disjoint());
}

TEST(PeriodListTest, FindContainingEdgeCases) {
  PeriodList list;
  list.push_back(Period(10, 20));
  list.push_back(Period(30, 40));
  list.push_back(Period(50, 60));
  list.sort();

  // Find at boundaries
  EXPECT_EQ(list.find_containing_index(10), 0);   // Begin of first
  EXPECT_EQ(list.find_containing_index(20), 0);   // Last of first
  EXPECT_EQ(list.find_containing_index(21), -1);  // Gap
  EXPECT_EQ(list.find_containing_index(60), 2);   // Last of last

  // Find in gaps
  EXPECT_EQ(list.find_containing_index(25), -1);
  EXPECT_EQ(list.find_containing_index(45), -1);

  // Find before all periods
  EXPECT_EQ(list.find_containing_index(5), -1);

  // Find after all periods
  EXPECT_EQ(list.find_containing_index(65), -1);

  // Verify find_containing returns nullptr for non-existent
  EXPECT_EQ(list.find_containing(25), nullptr);
  EXPECT_NE(list.find_containing(30), nullptr);
}

TEST(PeriodListTest, EmptyList) {
  PeriodList list;

  EXPECT_TRUE(list.is_sorted_and_disjoint());  // Empty is trivially sorted
  EXPECT_EQ(list.total_duration(), 0);
  EXPECT_EQ(list.find_containing_index(100), -1);
  EXPECT_EQ(list.find_containing(100), nullptr);
  EXPECT_FALSE(list.is_close(100, 10));
}

TEST(PeriodListTest, SingleElement) {
  PeriodList list;
  list.push_back(Period(10, 20));

  EXPECT_TRUE(list.is_sorted_and_disjoint());
  EXPECT_EQ(list.total_duration(), 11);
  EXPECT_EQ(list.find_containing_index(15), 0);
  EXPECT_EQ(list.find_containing_index(5), -1);
}

}  // namespace pyinterp
