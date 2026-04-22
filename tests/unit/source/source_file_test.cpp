#include <gtest/gtest.h>

#include <velo/source/source_file.h>

using Velo::Source::SourceFile;

TEST(SourceFileTest, ReturnsCorrectLocationForSingleLineContent) {
    const SourceFile file("inline.velo", "hello");
    const auto location = file.locationAt(1);

    ASSERT_TRUE(location.has_value());
    EXPECT_EQ(location->offset(), 1U);
    EXPECT_EQ(location->line(), 1U);
    EXPECT_EQ(location->column(), 2U);
}

TEST(SoiurceFileTest, ReturnsCorrectLocationForMultiLineContent) {
    const SourceFile file("inline.velo", "abc\ndef");
    const auto location = file.locationAt(4);

    ASSERT_TRUE(location.has_value());
    EXPECT_EQ(location->offset(), 4U);
    EXPECT_EQ(location->line(), 2U);
    EXPECT_EQ(location->column(), 1U);
}

TEST(SourceFileTest, BuildsValidRange) {
    const SourceFile file("inline.velo", "hello\nworld");
    const auto range = file.range(0, 4);

    ASSERT_TRUE(range.has_value());
    EXPECT_TRUE(range->isValid());
    EXPECT_EQ(range->begin().line(), 1U);
    EXPECT_EQ(range->begin().column(), 1U);
    EXPECT_EQ(range->end().line(), 1U);
    EXPECT_EQ(range->end().column(), 5U);
}