#ifndef _minunit_h
#define _minunit_h

#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <bstring.h>
#include <file.h>

#include <importer.h>

//==============================================================================
//
// Minunit
//
//==============================================================================

//--------------------------------------
// Core
//--------------------------------------

#define mu_fail(MSG, ...) do {\
    fprintf(stderr, "%s:%d: " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
    return 1;\
} while(0)

#define mu_assert_with_msg(TEST, MSG, ...) do {\
    if (!(TEST)) {\
        fprintf(stderr, "%s:%d: %s " MSG "\n", __FILE__, __LINE__, #TEST, ##__VA_ARGS__);\
        return 1;\
    }\
} while (0)

#define mu_assert(TEST, MSG, ...) do {\
    if (!(TEST)) {\
        fprintf(stderr, "%s:%d: %s " MSG "\n", __FILE__, __LINE__, #TEST, ##__VA_ARGS__);\
        return 1;\
    }\
} while (0)

#define mu_run_test(TEST) do {\
    fprintf(stderr, "%s\n", #TEST);\
    int rc = TEST();\
    tests_run++; \
    if (rc) {\
        fprintf(stderr, "\n  Test Failure: %s()\n", #TEST);\
        return rc;\
    }\
} while (0)

#define RUN_TESTS() int main() {\
   fprintf(stderr, "== %s ==\n", __FILE__);\
   int rc = all_tests();\
   fprintf(stderr, "\n");\
   return rc;\
}

int tests_run;


//--------------------------------------
// Typed asserts
//--------------------------------------

#define mu_assert_bool(ACTUAL) mu_assert_with_msg(ACTUAL, "Expected value to be true but was %d", ACTUAL)
#define mu_assert_int_equals(ACTUAL, EXPECTED) mu_assert_with_msg(ACTUAL == EXPECTED, "Expected: %d; Received: %d", EXPECTED, ACTUAL)
#define mu_assert_long_equals(ACTUAL, EXPECTED) mu_assert_with_msg(ACTUAL == EXPECTED, "Expected: %ld; Received: %ld", EXPECTED, ACTUAL)
#define mu_assert_int64_equals(ACTUAL, EXPECTED) mu_assert_with_msg(ACTUAL == EXPECTED, "Expected: %lld; Received: %lld", (long long int)EXPECTED, (long long int)ACTUAL)

#define mu_assert_bstring(ACTUAL, EXPECTED) do {\
    bstring expected = bfromcstr(EXPECTED); \
    if(blength(ACTUAL) != blength(expected)) { \
        mu_fail("String length doesn\'t not match. exp:%d, recv:%d", blength(expected), blength(ACTUAL)); \
    } \
    int32_t _i; \
    for(_i=0; _i<blength(ACTUAL); _i++) { \
        if(bchar(ACTUAL, _i) != bchar(expected, _i)) { \
            mu_fail("Unexpected byte at %d. exp:\\x%02x, recv:\\x%02x", _i, bchar(expected, _i), bchar(ACTUAL, _i)); \
        } \
    } \
    bdestroy(expected); \
} while(0)



//==============================================================================
//
// Constants
//
//==============================================================================

// The temporary directory.
#define TMPDIR "tmp"
struct tagbstring BSTMPDIR = bsStatic(TMPDIR);

// The temporary file used for file operations in the test suite.
#define TEMPFILE "/tmp/skytemp"

// The path where memory dumps are directed to.
#define MEMDUMPFILE "/tmp/memdump"

#define TEST_PORT 10001


//==============================================================================
//
// Helpers
//
//==============================================================================

// Empties the tmp directory.
#define cleantmp() do {\
    mu_assert_with_msg(sky_file_rm_r(&BSTMPDIR) == 0, "Unable to clean tmp directory"); \
    mu_assert_with_msg(mkdir(TMPDIR, S_IRWXU | S_IRWXG | S_IRWXO) == 0, "Unable to create tmp directory"); \
} while(0)
    
// Loads the tmp directory with the contents of another directory.
#define loadtmp(PATH) do {\
    if(strcmp(PATH, "") == 0) { \
        cleantmp(); \
    } \
    else { \
        struct tagbstring _srcpath = bsStatic(PATH); \
        cleantmp(); \
        mu_assert_with_msg(sky_file_cp_r(&_srcpath, &BSTMPDIR) == 0, "Unable to copy to tmp directory"); \
    } \
} while(0)

// Uses an import file to create a table in the given directory.
#define importtmp_n(PATH, TABLET_COUNT) do {\
    cleantmp(); \
    sky_importer *importer = sky_importer_create(); \
    importer->path = bfromcstr(TMPDIR); \
    importer->tablet_count = TABLET_COUNT; \
    FILE *file = fopen(PATH, "r"); \
    mu_assert_with_msg(file != NULL, "Unable to open import file: " PATH); \
    int rc = sky_importer_import(importer, file); \
    mu_assert_int_equals(rc, 0); \
    fclose(file); \
    sky_importer_free(importer); \
} while(0)
    
// Uses an import file to create a table with a single tablet in a given directory.
#define importtmp(PATH) importtmp_n(PATH, 1)

// Asserts property data.
#define mu_assert_property(INDEX, ID, NAME) \
    mu_assert(table->properties[INDEX]->id == ID, "Expected property #" #INDEX " id to be: " #ID); \
    mu_assert(biseqcstr(table->properties[INDEX]->name, NAME) == 1, "Expected property #" #INDEX " name to be: " #NAME);

// Asserts the contents of the temp file.
#define mu_assert_file(FILENAME1, FILENAME2) do {\
    FILE *file1 = fopen(FILENAME1, "r"); \
    if(file1 == NULL) mu_fail("Cannot open file 1: %s", FILENAME1); \
    FILE *file2 = fopen(FILENAME2, "r"); \
    if(file2 == NULL) mu_fail("Cannot open file 2: %s", FILENAME2); \
    bstring content1 = bread((bNread)fread, file1); \
    bstring content2 = bread((bNread)fread, file2); \
    if(blength(content1) != blength(content2)) { \
        mu_fail("File length doesn\'t not match. exp:%d (%s), recv:%d (%s)", blength(content1), FILENAME1, blength(content2), FILENAME2); \
    } \
    int32_t _i; \
    for(_i=0; _i<blength(content1); _i++) { \
        if(bchar(content1, _i) != bchar(content2, _i)) { \
            mu_fail("Unexpected byte at %d. exp:\\x%02x, recv:\\x%02x (%s, %s)", _i, bchar(content1, _i), bchar(content2, _i), FILENAME1, FILENAME2); \
        } \
    } \
    bdestroy(content1); \
    bdestroy(content2); \
    fclose(file1); \
    fclose(file2); \
} while(0)

// Asserts the contents of the temp file.
#define mu_assert_tempfile(EXP_FILENAME) \
    mu_assert_file(TEMPFILE, EXP_FILENAME)

// Asserts the contents of memory. If an error occurs then the memory is
// dumped to file.
#define mu_assert_mem(ACTUAL, EXPECTED, N) do {\
    if(memcmp(ACTUAL, EXPECTED, N) != 0) { \
        FILE *file = fopen(MEMDUMPFILE, "w"); \
        fwrite(ACTUAL, N, sizeof(char), file); \
        fclose(file); \
        mu_fail("Memory contents do not match. Memory dumped to: " MEMDUMPFILE); \
    } \
} while(0)

// Redirects STDERR to a log.
#define mu_begin_log() do { \
    bool opened = freopen("tmp/stderr", "w", stderr); \
    mu_assert_with_msg(opened, "Unable to redirect STDERR to log"); \
} while(0)

// Redirects STDERR back to the console output.
#define mu_end_log() do { \
    fflush(stderr); \
    dup2(1, 2); \
} while(0)

// Asserts that a given string is in the log.
#define mu_assert_log(STRING) do { \
    FILE *log_file = fopen("tmp/stderr", "r"); \
    mu_assert_with_msg(log_file != NULL, "Unable to open log file"); \
    bstring log_content = bread((bNread)fread, log_file); \
    struct tagbstring search_str = bsStatic(STRING); \
    mu_assert_with_msg(binstr(log_content, 0, &search_str) != BSTR_ERR, "Text not found in log: " STRING); \
    bdestroy(log_content); \
} while(0)

#endif