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
#include "routines.h"
#include "parser_routines.h"


int main(int argc, char **argv) {
	storage = get_new_memory(storage_space, sizeof(char));
	unsigned int files_count = parse_cli_and_prepare_file_list(&argc, argv);
	if (files_count == 0) {
		fprintf(stderr, "Nothing to parse. Exiting...");
		return EXIT_SUCCESS; //"Nothing to parse" is still result, not critical error. Should I use EXIT_FAILURE?
	}
	doc_references = get_new_memory(doc_space, sizeof(function_seek));

	struct cache_def {
		int fd;
		char *fname;
		size_t filesize;
		char *fileptr;
		char *search_from;

		char *above_word;
		char *round_bracket;
		char *function_name;
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
		cache.search_from = cache.fileptr;

		while (forever) {
			cache.above_word = strstr(cache.search_from, search_above_string);
			if (cache.above_word == NULL or cache.above_word > cache.fileptr + cache.filesize) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			cache.round_bracket = strchr_backward(cache.above_word, ROUND_BRACKET_CHAR, cache.search_from);
			if (cache.round_bracket <= cache.search_from) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			cache.function_name = find_function_name_behind_round_bracket(cache.round_bracket);
			if (cache.function_name <= cache.search_from) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			doc_cache.arg_count = 0;
			doc_cache.desc = 0;
			doc_cache.infinity_arg = 0;

			doc_cache.return_type = storage_current;
			if (flush_normalized_function_type(cache.function_name, cache.search_from) == NULL) {
				erase_area(&doc_cache, sizeof(doc_cache));
				break;
			}

			doc_cache.filename = cache.fname - storage; //I already have saved filenames, no reason to overcopy

			doc_cache.function_name = storage_current;
			flush_function_name(cache.function_name);

			doc_cache.arg_values = storage_current;
			flush_null_separated_args(cache.round_bracket, &(doc_cache.arg_count));
			cache.search_from = find_doc_block_ending(strchr(cache.above_word + sizeof(search_above_string) - 1, '\n'));

			put_to_doc(&doc_cache);
			erase_area(&doc_cache, sizeof(doc_cache));
		}

		iterator++;
	}

	size_t i = 0;
		while (i < doc_current) {
			printf("Filename: %s Function name: %s Return type: %s\n", storage+(doc_references+i)->filename,
				   storage+(doc_references+i)->function_name, storage+(doc_references+i)->return_type );
			i++;
		}

	return EXIT_SUCCESS;
}
