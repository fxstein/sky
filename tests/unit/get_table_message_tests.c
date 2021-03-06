#include <stdio.h>
#include <stdlib.h>

#include <get_table_message.h>
#include <mem.h>
#include <dbg.h>

#include "../minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Serialization
//--------------------------------------

int test_sky_get_table_message_pack() {
    cleantmp();
    sky_get_table_message *message = sky_get_table_message_create();
    message->name = bfromcstr("foo");
    
    FILE *file = fopen("tmp/message", "w");
    mu_assert_bool(sky_get_table_message_pack(message, file) == 0);
    fclose(file);
    mu_assert_file("tmp/message", "tests/fixtures/get_table_message/0/message");
    sky_get_table_message_free(message);
    return 0;
}

int test_sky_get_table_message_unpack() {
    FILE *file = fopen("tests/fixtures/get_table_message/0/message", "r");
    sky_get_table_message *message = sky_get_table_message_create();
    mu_assert_bool(sky_get_table_message_unpack(message, file) == 0);
    fclose(file);

    mu_assert_bstring(message->name, "foo");
    sky_get_table_message_free(message);
    return 0;
}


//--------------------------------------
// Processing
//--------------------------------------

int test_sky_get_table_message_process_with_existing_table() {
    cleantmp();
    sky_server *server = sky_server_create(NULL);
    server->path = bfromcstr("tmp");
    sky_message_header *header = sky_message_header_create();

    // Create, open & close table.
    sky_table *table = sky_table_create();
    table->path = bfromcstr("tmp/foo");
    sky_table_open(table);
    sky_table_free(table);

    FILE *input = fopen("tests/fixtures/get_table_message/1/input", "r");
    FILE *output = fopen("tmp/output", "w");
    int rc = sky_get_table_message_process(server, header, NULL, input, output);
    mu_assert_int_equals(rc, 0);
    
    mu_assert_file("tmp/output", "tests/fixtures/get_table_message/1/output");

    sky_message_header_free(header);
    sky_server_stop(server);
    sky_server_free(server);
    return 0;
}

int test_sky_get_table_message_process_with_missing_table() {
    cleantmp();
    sky_server *server = sky_server_create(NULL);
    server->path = bfromcstr("tmp");
    sky_message_header *header = sky_message_header_create();

    FILE *input = fopen("tests/fixtures/get_table_message/2/input", "r");
    FILE *output = fopen("tmp/output", "w");
    int rc = sky_get_table_message_process(server, header, NULL, input, output);
    mu_assert_int_equals(rc, 0);
    
    mu_assert_file("tmp/output", "tests/fixtures/get_table_message/2/output");

    sky_message_header_free(header);
    sky_server_free(server);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_get_table_message_pack);
    mu_run_test(test_sky_get_table_message_unpack);
    mu_run_test(test_sky_get_table_message_process_with_existing_table);
    mu_run_test(test_sky_get_table_message_process_with_missing_table);
    return 0;
}

RUN_TESTS()