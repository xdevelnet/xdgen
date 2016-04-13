char *find_function_name_behind_round_bracket (char *ptr) {
	ptr--;
	while (*ptr == SPACE_CHAR or *ptr == LINEBREAK_CHAR or *ptr == TAB_CHAR) ptr--;
	while (*ptr != SPACE_CHAR and *ptr != LINEBREAK_CHAR and *ptr != TAB_CHAR and *ptr != ASTERISK_CHAR) ptr--;
	return ++ptr;
}

char *flush_normalized_function_type (char *function_name_ptr, char *top_break) {
	char *flying_pointer = function_name_ptr;
	while (*flying_pointer != '>' and *flying_pointer != ';' and *flying_pointer != '}' and flying_pointer != top_break) flying_pointer--;
	//TODO: I should check if someone put pure preprocessor statement behind funtion
	char space_flag = 1;
	while (++flying_pointer != function_name_ptr) {
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
			argc++;
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
