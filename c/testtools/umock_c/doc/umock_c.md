umock_c


 
#Overview

umock_c is a C mocking library that exposes APIs to allow:
-	defining mock functions, 
-	recording expected calls 
-	comparing expected calls with actual calls. 
On top of the basic functionality, additional convenience features like modifiers on expected calls are provided.

#Simple example

A test written with umock_c looks like below:

Let’s assume unit A depends on unit B. unit B has a function called test_dependency_1_arg.

In unit B’s header one would write:

```c
MOCKABLE_FUNCTION(int, test_dependency_1_arg, int, a);
```

Let’s assume unit A has a function called function_under_test.

A test that checks that function_under_test calls its dependency and injects a return value, while ignoring all arguments on the call looks like this:

```c
TEST_FUNCTION(my_first_test)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(44)
        .IgnoreAllArguments();

    // act
    function_under_test();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}
```

#Exposed API (umock_c.h)

```c
DEFINE_ENUM(UMOCK_C_ERROR_CODE,
    UMOCK_C_ARG_INDEX_OUT_OF_RANGE);

typedef void(*ON_UMOCK_C_ERROR)(UMOCK_C_ERROR_CODE error_code);

#define MOCKABLE_FUNCTION(result, function, ...) \
	...

#define REGISTER_GLOBAL_MOCK_RETURN_HOOK(mock_function, mock_hook_function) \
    ...

#define REGISTER_GLOBAL_MOCK_RETURN(mock_function, return_value) \
    ...

#define REGISTER_GLOBAL_MOCK_FAIL_RETURN(mock_function, fail_return_value) \
    ...

#define REGISTER_GLOBAL_MOCK_RETURNS(mock_function, return_value, fail_return_value) \
    ...
    ...

#define STRICT_EXPECTED_CALL(call) \
	...

#define EXPECTED_CALL(call) \
	...

    extern int umock_c_init(ON_UMOCK_C_ERROR on_umock_c_error);
    extern void umock_c_deinit(void);
    extern int umock_c_reset_all_calls(void);
    extern const char* umock_c_get_actual_calls(void);
    extern const char* umock_c_get_expected_calls(void);
```

##Mock definitions API

###MOCKABLE_FUNCTION

```c
MOCKABLE_FUNCTION(result, function, ...)
```

MOCKABLE_FUNCTION shall be used to wrap function definition allow the user to declare a function that can be mocked.

The macro shall generate a function signature in case ENABLE_MOCKS is not defined.

If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate all the boilerplate code needed by the macros in umock API to function to record the calls. Note: a lot of code (including function definitions and bodies, global variables (both static and extern).

Example:

```c
MOCKABLE_FUNCTION(int, test_function, int, arg1)
```

should generate for production code:

```c
int test_function(int arg1);
```

###ENABLE_MOCKS

If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate the declaration of the function and code for the mocked function, thus allowing setting up of expectations in test functions. If ENABLE_MOCKS is not defined, MOCKABLE_FUNCTION shall only generate a declaration for the function.

##umock init/deinit

###umock_c_init

```c
int umock_c_init(ON_UMOCK_C_ERROR on_umock_c_error);
```

umock_c_init is needed before performing any action related to umock_c calls (or registering any types).
umock_c_init shall initialize umock_c. umock_c_init called if already initialized shall fail and return a non-zero value. 
umock_c_init shall initialize the umock supported types.
on_umock_c_error can be NULL.
If on_umock_c_error is non-NULL it shall be saved for later use (to be invoked whenever an umock_c error needs to be signaled to the user).
umock_c_deinit
void umock_c_deinit(void);
umock_c_deinit shall free all umock_c used resources. If umock_c was not initialized, umock_c_deinit shall do nothing.
Expected calls recording API
STRICT_EXPECTED_CALL
STRICT_EXPECTED_CALL(call)
STRICT_EXPECTED_CALL shall record that a certain call is expected. For each argument the argument value shall be stored for later comparison with actual calls. The call argument shall be the complete function invocation.
Example:
STRICT_EXPECTED_CALL(test_dependency_1_arg(42));
EXPECTED_CALL
EXPECTED_CALL(call)
EXPECTED_CALL shall record that a certain call is expected. No arguments shall be saved by default, unless other modifiers state it. The call argument shall be the complete function invocation.
Example:
EXPECTED_CALL(test_dependency_1_arg(42));
Call comparison API
umock_c_reset_all_calls
int umock_c_reset_all_calls(void);
umock_c_reset_all_calls shall reset all calls (actual and expected). On success, umock_c_reset_all_calls shall return 0. In case of any error, umock_c_reset_all_calls shall return a non-zero value.
umock_c_get_expected_calls
const char* umock_c_get_expected_calls(void);
umock_c_get_expected_calls shall return all the calls that were expected, but were not fulfilled. For each call, the format shall be "functionName(argument 1 value, …)". Each call shall be enclosed in "[]".

Example:
For a call with the signature:
int test_dependency_2_args(int a, int b)
if an expected call was recorded:
STRICT_EXPECTED_CALL(test_dependency_2_args(42, 1));
umock_c_get_expected_calls would return:
"[test_dependency_2_args(42,1)]"

umock_c_get_actual_calls
const char* umock_c_get_actual_calls(void);
umock_c_get_actual_calls shall return all the actual calls that were not matched to expected calls. For each call, the format shall be "functionName(argument 1 value, …)". Each call shall be enclosed in "[]". A call to umock_c_get_actual_calls shall not modify the actual calls that were recorded. 

Example:
For a call with the signature:
int test_dependency_2_args(int a, int b)
if an actual call was recorded:
test_dependency_2_args(42, 2);
umock_c_get_actual_calls would return:
"[test_dependency_2_args(42,2)]"

Supported types
Out of thebox
Out of the box umock_c shall support the following types:
-	char
-	unsigned char
-	short
-	unsigned short
-	int
-	unsigned int
-	long
-	unsigned long
-	long long
-	unsigned long long
-	float
-	double
-	long double
-	size_t
-	clock_t
-	time_t
-	struct tm
Custom types
Custom types, like structures shall be supported by allowing the user to define a set of functions that can be used by umock_c to operate with these types. Five functions shall be provided to umock_c:
-	A stringify function. This function shall return the string representation of a value of the given type.
-	An are_equal function. This function shall compare 2 values of the given type and return an int indicating whether they are equal (1 means equal, 0 means different).
-	A copy function. This function shall make a copy of a value for the given type.
-	A free function. This function shall free a copied value.
umockvalue_stringify_{type}
char* umockvalue_stringify_{type}(const {type}* value)
A stringify function shall allocate using malloc a char* and fill it with a string representation of value.
If any error is encountered during building the string representation, umockvalue_stringify_{type} shall return NULL.
Example:
umockvalue_are_equal_{type}
int umockvalue_are_equal_{type}(const {type}* left, const {type}* right)
The umockvalue_are_equal_{type} function shall return 1 if the 2 values are equal and 0 if they are not. If both left and right are NULL, umockvalue_are_equal_{type} shall return 1. If only one of left and right is NULL, umockvalue_are_equal_{type} shall return 0.
umockvalue_copy_{type}
int umockvalue_copy_{type}({type}* destination, const {type}* source)
The umockvalue_copy_{type} function shall copy the valuefrom source to destination. On success umockvalue_copy_{type} shall return 0. If any of the arguments is NULL, umockvalue_copy_{type} shall return a non-zero value. If any error occurs during copying the value, umockvalue_copy_{type} shall return a non-zero value.
umockvalue_free_{type}
void umockvalue_free_{type}({type}* value)
The umockvalue_free_{type} function shall free a value previously copied using umockvalue_copy_{type}. If value is NULL, no free shall be performed.

REGISTER_UMOCK_VALUE_TYPE
REGISTER_UMOCK_VALUE_TYPE(value_type, stringify_func, are_equal_func, copy_func, free_func)
REGISTER_UMOCK_VALUE_TYPE shall register the type identified by value_type to be usable by umock_c for argument and return types and instruct umock_c which functions to use for getting the stringify, are_equal, copy and free.
Example:
REGISTER_UMOCK_VALUE_TYPE(TEST_STRUCT*, umockvalue_stringify_TEST_STRUCT_ptr, umockvalue_are_equal_TEST_STRUCT_ptr, umockvalue_copy_TEST_STRUCT_ptr, umockvalue_free_TEST_STRUCT_ptr);

If only the value_type is specified in the macro invocation then the stringify, are_equal, copy and free function names shall be automatically derived from the type as: umockvalue_stringify_{value_type}, umockvalue_are_equal_{value_type}, umockvalue_copy_{value_type}, umockvalue_free_{value_type}.
Example:
REGISTER_UMOCK_VALUE_TYPE(TEST_STRUCT);

Extra optional C types
umockvalue_charptr
char* and const char* shall be supported out of the box through a separate header, umockvalue_charptr.h.
In order to enable the usage of char*, the function umockvalue_charptr_register_types can be used in the test suite init. The signature shall be:
Int umockvalue_charptr_register_types(void);
umockvalue_charptr_register_types shall return 0 on success and non-zero on failure.
umockvalue_stdint
The types in stdint.h shall be supported out of the box by including umockvalue_stdint.h.
In order to enable the usage of stdint types, the function umockvalue_stdint_register_types shall be used in the test suite init.
int umockvalue_stdint_register_types(void);
umockvalue_stdint_register_types shall return 0 on success and non-zero on failure.
Call modifiers
When an expected call is recorded a call modifier interface in the form of a structure containing function pointers shall be returned to the caller.
That allows constructs like:
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(44)
        .IgnoreAllArguments();

Note that each modifier function shall return a full modifier structure that allows chaining further call modifiers.
The last modifier in a chain overrides previous modifiers if any collision occurs. Example: A ValidateAllArguments after a previous IgnoreAllArgument will still validate all arguments.
IgnoreAllArguments(void)
The IgnoreAllArguments call modifier shall record that for that specific call all arguments will be ignored for that specific call.
ValidateAllArguments(void)
The ValidateAllArguments call modifier shall record that for that specific call all arguments will be validated.
IgnoreArgument_{arg_name}(void)
The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.
ValidateArgument_{arg_name}(void)
The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.
IgnoreArgument(size_t index)
The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.
If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.
ValidateArgument(size_t index)
The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.
If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.
SetReturn(return_type result)
The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.
SetFailReturn(return_type result)
The SetFailReturn call modifier shall record a fail return value. The fail return value can be recorded for more advanced features that would require failing or succeeding certain calls based on decisions made at runtime.
CopyOutArgumentBuffer(size_t index, const unsigned char* bytes, size_t length)
The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function. The memory shall be copied. If several calls to CopyOutArgumentBuffer are made, only the last buffer shall be kept. The buffers for the previous calls shall be freed.
CopyOutArgumentBuffer shall only be applicable to pointer types.
If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.
If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.
CopyOutArgument (arg_type value)
The CopyOutArgument call modifier shall copy an argument value to be injected as an out argument value when the code under test calls the mock function.
CopyOutArgument shall only be applicable to pointer types.
ValidateArgumentBuffer(size_t index, const unsigned char* bytes, size_t length)
The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function. If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer do not match then this should be treated as any other mismatch in argument comparison for that argument. ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.
ValidateArgumentBuffer shall only be applicable to pointer types.
If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.
If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.
IgnoreAllCalls(void)
The IgnoreAllCalls call modifier shall record that all calls matching the expected call shall be ignored. If no matching call occurs no missing call shall be reported. If multiple matching actual calls occur no unexpected calls shall be reported. The call matching shall be done taking into account arguments and call modifiers referring to arguments.
Global mock modifiers
REGISTER_GLOBAL_MOCK_RETURN_HOOK
REGISTER_GLOBAL_MOCK_RETURN_HOOK(mock_function, mock_hook_function)
The REGISTER_GLOBAL_MOCK_RETURN_HOOK shall register a mock hook to be called every time the mocked function is called by production code. The hook’s result shall be returned by the mock to the production code. The signature for the hook shall be assumed to have exactly the same arguments and return as the mocked function.
If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN_HOOK, the last one shall take effect over the previous ones.
REGISTER_GLOBAL_MOCK_RETURN
REGISTER_GLOBAL_MOCK_RETURN(mock_function, return_value)
The REGISTER_GLOBAL_MOCK_RETURN shall register a return value to always be returned by a mock function.
If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN, the last one shall take effect over the previous ones.
If no REGISTER_GLOBAL_MOCK_RETURN is performed for a mocked function, the mock will return a value declared as static of the same type as the functions return type.
REGISTER_GLOBAL_MOCK_FAIL_RETURN
REGISTER_GLOBAL_MOCK_FAIL_RETURN(mock_function, fail_return_value)
The REGISTER_GLOBAL_MOCK_FAIL_RETURN shall register a fail return value to be returned by a mock function when marked as failed in the expected calls.
If there are multiple invocations of REGISTER_GLOBAL_FAIL_MOCK_RETURN, the last one shall take effect over the previous ones.
REGISTER_GLOBAL_MOCK_RETURNS
REGISTER_GLOBAL_MOCK_RETURNS(mock_function, return_value, fail_return_value)
The REGISTER_GLOBAL_MOCK_RETURNS shall register both a success and a fail return value associated with a mock function.
If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURNS, the last one shall take effect over the previous ones.