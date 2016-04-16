/*
 * xdgen - fast and handy document generator
 *
 * Copyright (C) 2016  xdevelnet (xdevelnet at xdevelnet dot org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <iso646.h>
#include <unistd.h>

#include "constants_and_globals.h"
#include "mem_handling.h"
#include "routines.h"
#include "parser_routines.h"


int main(int argc, char **argv) {
	storage = get_new_memory(storage_space, sizeof(char));
	unsigned int files_count = parse_cli_and_prepare_file_list(&argc, argv);
	if (files_count == 0) {
		fprintf(stderr, "Nothing to parse. Exiting...\n");
		return EXIT_SUCCESS; //"Nothing to parse" is still result, not critical error. Should I use EXIT_FAILURE?
	}
	doc_references = get_new_memory(doc_space, sizeof(function_seek));

	struct cache_def {
		int fd;
		char *fname;
		size_t filesize;
		char *fileptr;

		char *above_word;
		char *after_above;
		char *round_bracket;
		char *function_name;
		char *doc_block_ending; //also used as next search pointer for 'above'
	} cache;
	function_seek doc_cache;

	unsigned int iterator = 0;
	size_t next_filename = 0;


	while (iterator < files_count) {
		cache.fname = storage + next_filename;
		next_filename += strlen(cache.fname) + 1;
		cache.fd = get_readonly_descriptor(cache.fname);
		cache.filesize = get_file_size(cache.fd);
		if (cache.filesize < FILE_IS_NOT_BIG_ENOUGH_FOR_PARSING) {
			iterator++;
			continue;
		}
		cache.fileptr = try_readonly_mmap(cache.fd, cache.filesize);
		cache.doc_block_ending = cache.fileptr;

		while (forever) {
			cache.above_word = strstr(cache.doc_block_ending, search_above_string);
			if (cache.above_word == NULL or cache.above_word > cache.fileptr + cache.filesize) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			cache.round_bracket = strchr_backward(cache.above_word, ROUND_BRACKET_CHAR, cache.doc_block_ending);
			if (cache.round_bracket <= cache.doc_block_ending) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			cache.function_name = find_function_name_behind_round_bracket(cache.round_bracket);
			if (cache.function_name <= cache.doc_block_ending) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			doc_cache.arg_count = 0;
			doc_cache.desc = 0;
			doc_cache.infinity_arg = 0;

			doc_cache.return_type = storage_current;
			if (flush_normalized_function_type(cache.function_name, cache.doc_block_ending) == NULL) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			doc_cache.filename = cache.fname - storage; //I already have saved filenames, no reason to overcopy

			doc_cache.function_name = storage_current;
			flush_function_name(cache.function_name);

			doc_cache.arg_values = storage_current;
			flush_null_separated_args(cache.round_bracket, &(doc_cache.arg_count));
			cache.after_above = strchr(cache.above_word + sizeof(search_above_string) - 1, '\n'); //yea, user can put
			//comments for documentation after 'above' word.
			cache.doc_block_ending = find_doc_block_ending(cache.after_above);

			doc_cache.desc = storage_current;
			flush_desc(cache.after_above, cache.doc_block_ending);
			doc_cache.arg_desc = storage_current;
			if (doc_cache.arg_count != 0) flush_arg_desc(cache.after_above, cache.doc_block_ending, doc_cache.arg_count); else {
				flush_string_to_mem_storage(no_desc_provided);
			}

			put_to_doc(&doc_cache);
			erase_area(&doc_cache, sizeof(doc_cache));
		}

		iterator++;
	}

	//Here we got all required data. Time to sort it by 1) filenames 2) function names (check out compare function)

	qsort(doc_references, doc_current, sizeof(function_seek), slay_them_all);

	//loop below will be replaced with html generation

	iterator = 0;
		while (iterator < doc_current) {
			printf("Filename: %s Return type: %s Function name: %s Args count: %lu\nDesc: %s\nArg desc: %s\n\n",
				   storage+(doc_references+iterator)->filename, storage+(doc_references+iterator)->return_type,
				   storage+(doc_references+iterator)->function_name, (doc_references+iterator)->arg_count,
				   storage+(doc_references+iterator)->desc, storage+(doc_references+iterator)->arg_desc );
			iterator++;
		}

	return EXIT_SUCCESS;
}
