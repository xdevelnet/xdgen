#include <dirent.h>
#include <string.h>
#include <strings.h>

int compare_string_ending_equality (char *string, char *ending) {
	size_t stringlen = strlen(string); //TODO: Optimize function performance. Yea, we can avoid iteration over strings twice.
	size_t endinglen = strlen(ending);
	if (stringlen < endinglen) return -2; //non-zero integer, just like strcmp()
	string += stringlen - endinglen;
	return strcmp(string, ending);
}

char strpartcmp(char *str, char *part) {
	while (forever) {
		if (*part == 0) return 0;
		if (*(str++) != *(part++)) return 1;
	}
}

char *strchr_backward (char *position, const char c, const char *s) {
	//above let's test comments here. HUEHUEHUE
	// returns a pointer to the       first occurrence of the character c before *position in the string *s,
	//or NULL pointer if no character found
	//1 - pointer to starting character
	//2 - search character
	//3 - pointer to first character of string
	while (forever) {
		if (position < s) return NULL;
		if (*position == c) return position;
		position--;
	}
}

char *strchr_double_backward (char *position, const char c, const char z, const char *s) {
	//above
	//returns a pointer to the first occurrence of the character c before *position in the string *s, or NULL pointer if no character found
	//1 - pointer to starting character
	//2 - search character
	//3 - pointer to first character of string
	while (forever) {
		if (position < s) return NULL;
		if (*position == c or *position == z) return position;
		position--;
	}
}

size_t get_file_size(int fd) {
	struct stat sb;
	if (fstat(fd, &sb) < 0) {
		perror("");
		exit(EXIT_FAILURE);
	}
	return (size_t) sb.st_size; //idk why I should use off_t type
}

char *try_readonly_mmap(int file_descriptor, size_t file_size) {
	if (file_descriptor < 0) return NULL;
	void* mmap_result = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
	if (mmap_result == MAP_FAILED) {
		perror("mmap()");
		exit(EXIT_FAILURE);
	}
	return (char *) mmap_result;
}

void flush_string_to_mem_storage (const char *string) {
	while (*string != 0) {
		put_to_mem(*string);
		string++;
	}
	put_to_mem(0);
}

unsigned int parse_cli_and_prepare_file_list(int *argc, char **argv) {
	//ok, I know about "Single responsibility principle". Now look at that function and imagine what will happens
	//if we try to split it to parse_cli() and prepare_file_list()
	//
	//feel it? Inside every single IF/ELSE statements I need to prepare corresponding data like "is_file = 0;" or
	//"is_directory = 1;" and CHECK IT WITH IF/ELSE STATEMENTS AGAIN!
	//P.S. I also want to notice, that this principle is related to OOP, not PP
	char *dir_addr;
	struct stat statbuf;
	if (*argc > 1) {
		if (stat(argv[1], &statbuf) != 0 ) {
			fprintf(stderr,"%s %s\n", argv[1], "not found!");
			exit(EXIT_FAILURE);
		}
		if (S_ISREG(statbuf.st_mode) != 0) {
			flush_string_to_mem_storage(argv[1]);
			return 1;
		}
		if (S_ISDIR(statbuf.st_mode) != 0) {
			dir_addr = argv[1];
		} else {
			fprintf(stderr, "%s %s\n", argv[1], "is not file or directory!");
			exit(EXIT_FAILURE);
		}
	} else {
		dir_addr = getenv("PWD");
	}
	DIR *dir_stream;
	struct dirent *file_entry;
	if (chdir(dir_addr) != 0) {
		perror(dir_addr);
		exit(1);
	}
	dir_stream = opendir(dir_addr);
	if (!dir_stream) {
		fprintf(stderr, "%s %s\n", dir_addr, "not available!");
		exit(EXIT_FAILURE);
	}
	unsigned int file_count = 0;
	while ((file_entry = readdir(dir_stream)) != NULL) {
		char *dname = file_entry->d_name; //avoid multiple dereferencing
		if (strcmp(dname, "..") == 0 or strcmp(dname, ".") == 0) continue;
		if (compare_string_ending_equality(dname, ".c") != 0 and compare_string_ending_equality(dname, ".h") != 0 and
			compare_string_ending_equality(dname, ".cpp") != 0) continue;
		flush_string_to_mem_storage(dname);
		file_count++;
	}
	return file_count;
}

void erase_area(void *addr, size_t size) {
	memset(addr, 0, size);
	AVOID_MEMSET_OPTIMIZE(addr);
}

int slay_them_all (const void *a, const void *b) {
	const function_seek *first_elem = a;
	const function_seek *second_elem = b;
	int first_compare = strcasecmp(storage+first_elem->filename, storage+second_elem->filename);
	if (first_compare != 0) return first_compare;
	return strcasecmp(storage+first_elem->function_name, storage+second_elem->function_name);
}
