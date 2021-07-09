
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <pwd.h>
#include <grp.h>
#include <errno.h>

#include "text_list.h"
#include "create_tar.h"
#include "tar_header.h"
#include "extract_tar.h"
#include "tar_options.h"


bool options_parse(char *argv, setup *options, int argc)
{
    size_t length = strlen(argv);
    for (size_t i = 0; i < length; i++) {
        if (argv[i] == 'c') {
            if (options->create_tar || options->extract_tar || argc < 4) {
                return fprintf(stderr, "[ERROR] Option conflict -- \"%c\".\n", argv[i]), false;
            }
            options->create_tar = true;
        } else if (argv[i] == 'x') {
            if (options->extract_tar || options->create_tar) {
                return fprintf(stderr, "[ERROR] Option conflict -- \"%c\".\n", argv[i]), false;
            }
            options->extract_tar = true;
        } else if (argv[i] == 'l') {
            if (options->print_header) {
                return fprintf(stderr, "[ERROR] Option conflict -- \"%c\".\n", argv[i]), false;
            }
            options->print_header = true;
        } else if (argv[i] == 'v') {
            if (options->report_tar) {
                return fprintf(stderr, "[ERROR] Option conflict -- \"%c\".\n", argv[i]), false;
            }
            options->report_tar = true;
        } else {
            return fprintf(stderr, "[ERROR] Option unknown \"%c\".\n", argv[i]), false;
        }
    }
    return true;
}

bool arg_parse(int argc, char **argv, setup *options, list *files_names)
{
    if (argc < 3) {
        return fprintf(stderr, "[ERROR] Entered command has few arguments.\n"), false;
    }
    if (!options_parse(argv[1], options, argc)) {
        return false;
    }
    options->tar_name = argv[2];
    
    if (options->create_tar) {
        for (int i = 3; i < argc; i++) {
            if (!add_to_list(files_names, argv[i])) {
                return false;
            }
        }
    }
    return true;
}


int main(int argc, char **argv)
{   
    setup options = {
        .create_tar = false,
        .extract_tar = false,
        .report_tar = false,
        .print_header = false,
        .return_value = EXIT_SUCCESS,
        .tar_name = NULL,
        .tar_fd = 0,
        .files_names = NULL
    };
    list *files_list = create_list();
    if (files_list == NULL) {
        fprintf(stderr, "[ERROR] Allocation has failed\n");
        return EXIT_FAILURE;
    }
    if (!arg_parse(argc, argv, &options, files_list)) {
        delete_list(files_list);
        return EXIT_FAILURE;
    }

    if (options.extract_tar) {
        delete_list(files_list);
        if ((options.tar_fd = open(options.tar_name, O_RDONLY)) == -1) {
            fprintf(stderr, "[ERROR] Problem with openning file.\n");
            return EXIT_FAILURE;
        }
        if (!extract_tar(options.tar_fd, options.report_tar)) {
            close(options.tar_fd);
            return EXIT_FAILURE;
        }
        close(options.tar_fd);
        return EXIT_SUCCESS;

    } else if (options.create_tar) {

        if ((options.tar_fd = open(options.tar_name, O_WRONLY | O_CREAT, 0666)) == -1) {
            delete_list(files_list);
            fprintf(stderr, "[ERROR] Problem with openning file.\n");
            return EXIT_FAILURE;
        }

        if (!make_tar(&options, files_list, "")) {
            close(options.tar_fd);
            return EXIT_FAILURE;
        }
        
        char end_buffer[1024] = { 0 };
        if (write(options.tar_fd, end_buffer, 1024) < 1024) {
            perror("[ERROR] writing into file");
            return EXIT_FAILURE;
        }
        close(options.tar_fd);
        return options.return_value;
    }
    return EXIT_FAILURE;
}
