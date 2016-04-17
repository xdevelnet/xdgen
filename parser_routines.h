char *check_round_bracket_before_above_word (char *search, char *edge) {
	while (forever) {
		if (search < edge) return NULL;
		if (*search == '=') return NULL;
		if (*search == ROUND_BRACKET_CHAR) return search;
		search--;
	}
}

char *find_function_name_behind_round_bracket (char *ptr) {
	ptr--;
	while (*ptr == SPACE_CHAR or *ptr == LINEBREAK_CHAR or *ptr == TAB_CHAR) {
		if (*ptr == '=') return NULL;
		ptr--;
	}
	while (*ptr != SPACE_CHAR and *ptr != LINEBREAK_CHAR and *ptr != TAB_CHAR and *ptr != ASTERISK_CHAR) ptr--;
	return ++ptr;
}

char *flush_normalized_function_type (char *function_name_ptr, char *top_break) {
	char *flying_pointer = function_name_ptr;
	while (flying_pointer != top_break and *flying_pointer != '>' and *flying_pointer != ';' and *flying_pointer != '}'
		   and *flying_pointer != '#') flying_pointer--;
	if (*flying_pointer == '#') flying_pointer = strchr(flying_pointer, LINEBREAK_CHAR);

	char space_flag = 1;
	while (++flying_pointer != function_name_ptr) {
		if (flying_pointer >= function_name_ptr) return NULL;
		if (*flying_pointer == ASTERISK_CHAR) break; //let's handle asterisks inside separated loop
		if (*flying_pointer != SPACE_CHAR and *flying_pointer != LINEBREAK_CHAR and *flying_pointer != TAB_CHAR) {
			put_to_mem(*flying_pointer); space_flag = 0;
		} else {
			if (space_flag == 0) {
				put_to_mem(SPACE_CHAR);
				space_flag = 1;
			}
		}
	}

	if (*flying_pointer == ASTERISK_CHAR) {
		if (space_flag == 0) put_to_mem(SPACE_CHAR); //put space char if we haven't one
		put_to_mem(*flying_pointer);
		while (++flying_pointer != function_name_ptr) {
			if (*flying_pointer == SPACE_CHAR or *flying_pointer == LINEBREAK_CHAR or *flying_pointer == TAB_CHAR) continue;
			if (*flying_pointer != ASTERISK_CHAR) break; //garbage between funtion name and round bracket? whatever!
			//TODO: it may be 'const' keyword... Should I handle this? Why people use constant pointers? Screw it, i'll do that later
			//TODO: Oh god, it also may be 'restrict'. WHY?! Because I can! http://goo.gl/uXVkkg
			put_to_mem(*flying_pointer);
		}
	}
	if (flying_pointer > function_name_ptr) return NULL; //make sure we're not totally screwed
	else {
		char *cstorage = storage + storage_current - 1;
		if (*cstorage == SPACE_CHAR) *cstorage = 0; else put_to_mem(0); //trim last possible space
		return function_name_ptr;
	}
}

void flush_function_name (char *pointer_to_function_name) {
	while (*pointer_to_function_name != SPACE_CHAR and *pointer_to_function_name != ROUND_BRACKET_CHAR) {
		put_to_mem(*pointer_to_function_name);
		pointer_to_function_name++;
	}
	put_to_mem(0);
}

void flush_null_separated_args (char *round_bracket, size_t *argc) {
	char we_already_counted = 0;
	char space_flag = 1;
	char *flying_ptr = round_bracket;

	while (*++flying_ptr != ROUND_2ND_BR_CHAR) {
		if (*flying_ptr == SPACE_CHAR or *flying_ptr == TAB_CHAR or *flying_ptr == LINEBREAK_CHAR) {
			if (space_flag == 0) {
				put_to_mem(SPACE_CHAR);
				space_flag = 1;
			}
			continue;
		}
		if (we_already_counted == 0) {
			++*argc;
			we_already_counted = 1;
		}
		if (*flying_ptr == COMMA_CHAR) {
			put_to_mem(0);
			we_already_counted = 0;
			space_flag = 1;
			continue;
		}
		space_flag = 0;
		put_to_mem(*flying_ptr);
	}
	put_to_mem(0);
}


char *find_doc_block_ending (char *start) {
	char *ending;
	char *strstr_result;
	while (forever) {
		ending = strchr(start+1, LINEBREAK_CHAR); if (ending == NULL) break;
		strstr_result = strstr(start, double_slash);
		if (strstr_result > ending or strstr_result == NULL) {
			break;
		} else {
			start = ending;
		}
	}
	return start;
}



size_t flush_desc (char *after_above, char *doc_block_ending) {
	size_t desc_bytes_count = 0;
	char space_flag = 1;
	char current_char; //avoid multiple dereferencing
	char *flying_pointer = after_above;

	while (flying_pointer < doc_block_ending) {
		flying_pointer = strstr(flying_pointer, double_slash) + sizeof(double_slash) - 1;
		if (flying_pointer >= doc_block_ending or flying_pointer == NULL) break;
		if (*flying_pointer == LINEBREAK_CHAR) { //empty comment line
			flying_pointer++;
			continue;
		}
		if (strpartcmp(flying_pointer+1, " -") == 0 and *flying_pointer >= '0' and *flying_pointer <= '9') continue;
		//we gonna handle such lines with separated funtion

		while (forever) {
			current_char = *flying_pointer;
			if (current_char == LINEBREAK_CHAR) {
				if (space_flag == 0) put_to_mem(SPACE_CHAR);
				space_flag = 1;
				break;
			}
			if ((current_char == SPACE_CHAR or current_char == TAB_CHAR) and space_flag == 1) {
				flying_pointer++; //let's drop&chomp redundant tabs or spaces
				continue;
			}
			space_flag = 0;
			if (current_char == SPACE_CHAR or current_char == TAB_CHAR) {
				current_char = SPACE_CHAR;
				space_flag = 1;
			}
			put_to_mem(current_char);

			flying_pointer++;
			desc_bytes_count++;
		}
	}
	if (desc_bytes_count == 0) {
		flush_string_to_mem_storage("No description provided!");
	} else {
		*(storage+storage_current-1) =  0;
	}
	return desc_bytes_count;
}

size_t flush_arg_desc (char *after_above, char *doc_block_ending, size_t args_count) {
	//AAAAA! COPYPASTING!!
	//Ok. There is huge piece of code which is similar as in flush_desc. But the thing is, that uniting
	//cause some bicycles, possible GOTO expressions (say hello to gotophobia), reducing readability of code.
	//Also, these functions can be much more different in future. If you have some suggestions - feel free
	//email me or create new issue on github.
	size_t desc_bytes_count = 0;
	char space_flag = 1;
	char current_char; //avoid multiple dereferencing
	char *flying_pointer = after_above;

	if (args_count == 0) return 0; //who knows what happens if someone use it without checking args_count?
	size_t real_args_count = 0; //let's check how much args dorumentation we really have


	while (flying_pointer < doc_block_ending) {
		flying_pointer = strstr(flying_pointer, double_slash) + sizeof(double_slash) - 1;
		if (flying_pointer >= doc_block_ending or flying_pointer == NULL) break;
		if (*flying_pointer == LINEBREAK_CHAR) { //empty comment line
			flying_pointer++;
			continue;
		}
		if (strpartcmp(flying_pointer+1, " -") != 0 or *flying_pointer < '0' or *flying_pointer > '9') continue;
		//TODO: statement above should be rewritten - we need to handle more than 9 args, lol.
		//...but don't use regular expressions, they are too fat for that purpose
		if (strtoul(flying_pointer, NULL, 10) - *flying_pointer <= args_count) continue;
		flying_pointer += 3; //3 means strlen of "0 -" or "1 -". If you (or me) gonna rewrite thing 2 lines above -
		//don't forget to rewrite this thing

		while (forever) {
			current_char = *flying_pointer;
			if (current_char == LINEBREAK_CHAR) {
				if (desc_bytes_count == 0) flush_string_to_mem_storage(no_desc_provided);
				flush_string_to_mem_storage(args_desc_separator); storage_current--;
				//Yes, I can separate with nulls. And then, this should be parsed and reformatted. So, I decided to put
				//html inside mem at this stage. If you don't like it - just change separator.
				space_flag = 1;
				break;
			}
			if ((current_char == SPACE_CHAR or current_char == TAB_CHAR) and space_flag == 1) {
				flying_pointer++; //let's drop&chomp redundant tabs or spaces
				continue;
			}
			space_flag = 0;
			if (current_char == SPACE_CHAR or current_char == TAB_CHAR) {
				current_char = SPACE_CHAR;
				space_flag = 1;
			}
			put_to_mem(current_char);

			flying_pointer++;
			desc_bytes_count++;
		}
		real_args_count++;
		if (real_args_count == args_count) break;
		desc_bytes_count = 0; //we don't actually need bytes count outside function. But it's still required!
	}
	size_t not_enough_args_desc;
	if (real_args_count < args_count) {

		not_enough_args_desc = args_count - real_args_count;
		while (not_enough_args_desc > 0) {
			flush_string_to_mem_storage(no_desc_provided); storage_current--;
			flush_string_to_mem_storage(args_desc_separator); storage_current--;
			not_enough_args_desc--;
		}
	}
	put_to_mem(0);


	return real_args_count;
}
