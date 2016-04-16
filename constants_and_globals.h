#define forever 1
#define FILE_IS_NOT_BIG_ENOUGH_FOR_PARSING 25

#define SPACE_CHAR ' '
#define TAB_CHAR '	'
#define LINEBREAK_CHAR '\n'
#define ASTERISK_CHAR '*'
#define ROUND_BRACKET_CHAR '('
#define ROUND_2ND_BR_CHAR ')'
#define COMMA_CHAR ','

#define AVOID_MEMSET_OPTIMIZE(addr) *(volatile char *) addr = *(volatile char *) addr;
//i'm not protecting this macro from weird usage

struct function_seek_struct {
    size_t filename;
    size_t function_name;
    size_t return_type;
    size_t arg_count;
    size_t arg_values;
    size_t arg_desc;
    size_t desc;
    size_t infinity_arg;
};
typedef struct function_seek_struct function_seek;


//
// PREPARE HTML
//

const char search_above_string[] = "//above";
const char double_slash[] = "//";
const char no_desc_provided[] = "No description provided!";
const char args_desc_separator[] = " [ NEXT ] ";
