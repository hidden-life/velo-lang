#include <gtest/gtest.h>

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/source/source_file.h"

using Velo::Diagnostic::DiagnosticEngine;
using Velo::Diagnostic::DiagnosticSeverity;
using Velo::Source::SourceFile;

TEST(DiagnosticEngineTest, StoresReportedError) {
    const SourceFile file("inline.velo", "hello");
    const auto range = file.range(0, 4);

    ASSERT_TRUE(range.has_value());

    DiagnosticEngine engine;
    engine.error("E0001", "test error", *range);

    ASSERT_EQ(engine.size(), 1U);
    ASSERT_TRUE(engine.hasErrors());

    const auto &diag = engine.diagnostics().front();
    EXPECT_EQ(diag.severity(), DiagnosticSeverity::Error);
    EXPECT_EQ(diag.code(), "E0001");
    EXPECT_EQ(diag.message(), "test error");
}

TEST(DiagnosticEngineTest, DoesNotReportErrorsWhenOnlyWarningsExist) {
    const SourceFile file("inline.velo", "hello");
    const auto range = file.range(0, 4);

    ASSERT_TRUE(range.has_value());

    DiagnosticEngine engine;
    engine.warning("W0001", "test warning", *range);

    ASSERT_EQ(engine.size(), 1U);
    ASSERT_FALSE(engine.hasErrors());
}