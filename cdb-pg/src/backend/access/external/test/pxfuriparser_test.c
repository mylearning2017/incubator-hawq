#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

#include "c.h"
#include "../pxfuriparser.c"


/*
 * Test parsing of valid uri as given in LOCATION in a PXF external table.
 */
void 
test__parseGPHDUri__ValidURI(void **state)
{
	char* uri = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=SomeFragmenter&ACCESSOR=SomeAccessor&RESOLVER=SomeResolver&ANALYZER=SomeAnalyzer";
	List* options = NIL;
	ListCell* cell = NULL;
	OptionData* option = NULL;

	GPHDUri* parsed = parseGPHDUri(uri, GPHDURI_DONT_WARN);

	assert_true(parsed != NULL);
	assert_string_equal(parsed->uri, uri);

	assert_string_equal(parsed->protocol, "pxf");
	assert_string_equal(parsed->host, "1.2.3.4");
	assert_string_equal(parsed->port, "5678");
	assert_string_equal(parsed->data, "some/path/and/table.tbl");

	options = parsed->options;
	assert_int_equal(list_length(options), 4);

	cell = list_nth_cell(options, 0);
	option = lfirst(cell);
	assert_string_equal(option->key, "FRAGMENTER");
	assert_string_equal(option->value, "SomeFragmenter");

	cell = list_nth_cell(options, 1);
	option = lfirst(cell);
	assert_string_equal(option->key, "ACCESSOR");
	assert_string_equal(option->value, "SomeAccessor");

	cell = list_nth_cell(options, 2);
	option = lfirst(cell);
	assert_string_equal(option->key, "RESOLVER");
	assert_string_equal(option->value, "SomeResolver");

	cell = list_nth_cell(options, 3);
	option = lfirst(cell);
	assert_string_equal(option->key, "ANALYZER");
	assert_string_equal(option->value, "SomeAnalyzer");

	assert_true(parsed->fragments == NULL);

	freeGPHDUri(parsed);
}

/*
 * Negative test: parsing of uri without protocol delimiter "://"
 */
void
test__parseGPHDUri__NegativeTestNoProtocol(void **state)
{
	char* uri_no_protocol = "pxf:/1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_no_protocol, GPHDURI_DONT_WARN);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf:/1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of uri without options part
 */
void
test__parseGPHDUri__NegativeTestNoOptions(void **state)
{
	char* uri_no_options = "pxf://1.2.3.4:5678/some/path/and/table.tbl";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_no_options, GPHDURI_DONT_WARN);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl: missing options section");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with a missing equal
 */
void
test__parseGPHDUri__NegativeTestMissingEqual(void **state)
{
	char* uri_missing_equal = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_missing_equal, GPHDURI_DONT_WARN);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER: option 'FRAGMENTER' missing '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with duplicate equals
 */
void
test__parseGPHDUri__NegativeTestDuplicateEquals(void **state)
{
	char* uri_duplicate_equals = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter=DuplicateFragmenter";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_duplicate_equals, GPHDURI_DONT_WARN);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter=DuplicateFragmenter: option 'FRAGMENTER=HdfsDataFragmenter=DuplicateFragmenter' contains duplicate '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with a missing key
 */
void
test__parseGPHDUri__NegativeTestMissingKey(void **state)
{
	char* uri_missing_key = "pxf://1.2.3.4:5678/some/path/and/table.tbl?=HdfsDataFragmenter";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_missing_key, GPHDURI_DONT_WARN);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?=HdfsDataFragmenter: option '=HdfsDataFragmenter' missing key before '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with a missing value
 */
void
test__parseGPHDUri__NegativeTestMissingValue(void **state)
{
	char* uri_missing_value = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_missing_value, GPHDURI_DONT_WARN);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=: option 'FRAGMENTER=' missing value after '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Test GPHDUri_verify_no_duplicate_options: valid uri
 */
void
test__GPHDUri_verify_no_duplicate_options__ValidURI(void **state)
{
	char* valid_uri = "pxf://1.2.3.4:5678/some/path/and/table.tbl?Profile=a&Analyzer=b";

	/* Setting the test -- code omitted -- */
	GPHDUri* parsed = parseGPHDUri(valid_uri, GPHDURI_DONT_WARN);
	GPHDUri_verify_no_duplicate_options(parsed);
	freeGPHDUri(parsed);
}

/*
 * Negative test of GPHDUri_verify_no_duplicate_options: parsing of a uri with duplicate options
 */
void
test__GPHDUri_verify_no_duplicate_options__NegativeTestDuplicateOpts(void **state)
{
	char* uri_duplicate_opts = "pxf://1.2.3.4:5678/some/path/and/table.tbl?Profile=a&Analyzer=b&PROFILE=c";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		GPHDUri* parsed = parseGPHDUri(uri_duplicate_opts, GPHDURI_DONT_WARN);
		/* This will throw a ereport(ERROR).*/
		GPHDUri_verify_no_duplicate_options(parsed);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?Profile=a&Analyzer=b&PROFILE=c: Duplicate option(s): PROFILE");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Test GPHDUri_verify_core_options_exist with a valid uri
 */
void
test__GPHDUri_verify_core_options_exist__ValidURI(void **state)
{
	char* valid_uri = "pxf://1.2.3.4:5678/some/path/and/table.tbl?Fragmenter=1&Accessor=2&Resolver=3";

	/* Setting the test -- code omitted -- */
	GPHDUri* parsed = parseGPHDUri(valid_uri, GPHDURI_DONT_WARN);
	List *coreOptions = list_make3("FRAGMENTER", "ACCESSOR", "RESOLVER");
	GPHDUri_verify_core_options_exist(parsed, coreOptions);
	freeGPHDUri(parsed);
	list_free(coreOptions);
}

/*
 * Negative test of GPHDUri_verify_core_options_exist: Missing core options
 */
void
test__GPHDUri_verify_core_options_exist__NegativeTestMissingCoreOpts(void **state)
{
	char* missing_core_opts = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=a";
	List *coreOptions;
	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		GPHDUri* parsed = parseGPHDUri(missing_core_opts, GPHDURI_DONT_WARN);
		coreOptions = list_make3("FRAGMENTER", "ACCESSOR", "RESOLVER");
		/* This will throw a ereport(ERROR).*/
		GPHDUri_verify_core_options_exist(parsed, coreOptions);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=a: PROFILE or ACCESSOR and RESOLVER option(s) missing");
		list_free(coreOptions);
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

void run_parseGPHDUri_and_verify_key_value(char* uri, char* key, char* value)
{
	List* options = NIL;
	ListCell* cell = NULL;
	OptionData* option = NULL;

	GPHDUri* parsed = parseGPHDUri(uri, GPHDURI_DONT_WARN);

	assert_true(parsed != NULL);
	assert_string_equal(parsed->uri, uri);

	assert_string_equal(parsed->protocol, "pxf");
	assert_string_equal(parsed->host, "1.2.3.4");
	assert_string_equal(parsed->port, "5678");
	assert_string_equal(parsed->data, "some/path");

	options = parsed->options;
	assert_int_equal(list_length(options), 1);

	cell = list_nth_cell(options, 0);
	option = lfirst(cell);
	assert_string_equal(option->key, key);
	assert_string_equal(option->value, value);

	assert_true(parsed->fragments == NULL);

	freeGPHDUri(parsed);
}

void
test__parseGPHDUri__DeprecatedFragmenter(void **state)
{
	static const char *url_pattern = "pxf://1.2.3.4:5678/some/path?FRAGMENTER=%s";
	const char *cases[][2] = 
	{
		{ "HdfsDataFragmenter", "com.pivotal.pxf.plugins.hdfs.HdfsDataFragmenter" },
		{ "HiveDataFragmenter", "com.pivotal.pxf.plugins.hive.HiveDataFragmenter" },
		{ "HBaseDataFragmenter", "com.pivotal.pxf.plugins.hbase.HBaseDataFragmenter" },
		{ "UntouchedFragmenter", "UntouchedFragmenter" }
	};

	for (int i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), url_pattern, cases[i][0]);
		run_parseGPHDUri_and_verify_key_value(buf, "FRAGMENTER", cases[i][1]);
	}
}

void
test__parseGPHDUri__DeprecatedAccessor(void **state)
{
	static const char *url_pattern = "pxf://1.2.3.4:5678/some/path?ACCESSOR=%s";
	const char *cases[][2] = 
	{
		{ "TextFileAccessor", "com.pivotal.pxf.plugins.hdfs.LineBreakAccessor" },
		{ "LineBreakAccessor", "com.pivotal.pxf.plugins.hdfs.LineBreakAccessor" },
		{ "HiveAccessor", "com.pivotal.pxf.plugins.hive.HiveAccessor" },
		{ "AvroFileAccessor", "com.pivotal.pxf.plugins.hdfs.AvroFileAccessor" },
		{ "UntouchedAccessor", "UntouchedAccessor" }
	};

	for (int i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), url_pattern, cases[i][0]);
		run_parseGPHDUri_and_verify_key_value(buf, "ACCESSOR", cases[i][1]);
	}
}

void
test__parseGPHDUri__DeprecatedResolver(void **state)
{
	static const char *url_pattern = "pxf://1.2.3.4:5678/some/path?RESOLVER=%s";
	const char *cases[][2] = 
	{
		{ "TextResolver", "com.pivotal.pxf.plugins.hdfs.StringPassResolver" },
		{ "StringPassResolver", "com.pivotal.pxf.plugins.hdfs.StringPassResolver" },
		{ "HiveResolver", "com.pivotal.pxf.plugins.hive.HiveResolver" },
		{ "UntouchedResolver", "UntouchedResolver" }
	};

	for (int i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i)
	{
		char buf[1024];
		snprintf(buf, sizeof(buf), url_pattern, cases[i][0]);
		run_parseGPHDUri_and_verify_key_value(buf, "RESOLVER", cases[i][1]);
	}
}

int 
main(int argc, char* argv[]) 
{
	cmockery_parse_arguments(argc, argv);

	const UnitTest tests[] = {
			unit_test(test__parseGPHDUri__ValidURI),
			unit_test(test__parseGPHDUri__DeprecatedFragmenter),
			unit_test(test__parseGPHDUri__DeprecatedAccessor),
			unit_test(test__parseGPHDUri__DeprecatedResolver),
			unit_test(test__parseGPHDUri__NegativeTestNoProtocol),
			unit_test(test__parseGPHDUri__NegativeTestNoOptions),
			unit_test(test__parseGPHDUri__NegativeTestMissingEqual),
			unit_test(test__parseGPHDUri__NegativeTestDuplicateEquals),
			unit_test(test__parseGPHDUri__NegativeTestMissingKey),
			unit_test(test__parseGPHDUri__NegativeTestMissingValue),
			unit_test(test__GPHDUri_verify_no_duplicate_options__ValidURI),
			unit_test(test__GPHDUri_verify_no_duplicate_options__NegativeTestDuplicateOpts),
			unit_test(test__GPHDUri_verify_core_options_exist__ValidURI),
			unit_test(test__GPHDUri_verify_core_options_exist__NegativeTestMissingCoreOpts)
	};
	return run_tests(tests);
}
