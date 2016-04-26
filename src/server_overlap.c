#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <err.h>

#include "util.h"
#include "giggle_index.h"
#include "file_read.h"
#include "ll.h"

#define PORT 8080

struct region
{
    char *chrm;
    uint32_t start, end;
    int success;
};


struct args {
    struct giggle_index *gi;
    char **track_names;
    char *header;
};

int scan_url_vals(void *cls,
                  enum MHD_ValueKind kind,
                  const char *key,
                  const char *value)
{
    struct region *r = (struct region *)cls;
    r->success = 0;

    if ((key != NULL) && (strncmp("region", key, 6) == 0)) {
        if (value != NULL) {
            if (parse_region((char *)value,
                             &(r->chrm), 
                             &(r->start),
                             &(r->end)) == 0) {
                r->success = 1;
                return MHD_NO;
            } else {
                return MHD_YES;
            }
        } 
    }

    return MHD_YES;
}

int answer_to_connection(void *cls,
                         struct MHD_Connection *connection, 
                         const char *url, 
                         const char *method, const char *version, 
                         const char *upload_data, 
                         size_t *upload_data_size, void **con_cls)
{
    struct MHD_Response *response;
    int ret = 0;

    struct region r;
    r.chrm = NULL;

    int num_vals = MHD_get_connection_values(connection,
                                             MHD_GET_ARGUMENT_KIND,
                                             scan_url_vals,
                                             &r);

    if (r.success == 1) {
        char *page = NULL, *tmp_page = NULL;;
        asprintf(&page,
                 "myJsonMethod({%s\"files\":[",
                 ((struct args*)cls)->header);
        fprintf(stderr, "%s %u %u\n", r.chrm, r.start, r.end);
        struct giggle_index *gi = ((struct args*)cls)->gi;
        char **track_names = ((struct args*)cls)->track_names;

        struct giggle_query_result *gqr = giggle_query(gi,
                                                      r.chrm,
                                                      r.start,
                                                      r.end,
                                                      NULL);
        uint32_t i;
        for(i = 0; i < gqr->num_files; i++) {
            if (i != 0) {
                asprintf(&tmp_page,"%s,", page);
                free(page);
                page = tmp_page;
            }
            struct file_data *fd = 
                (struct file_data *)unordered_list_get(gi->file_index, i); 
            asprintf(&tmp_page,
                     "%s"
                     "{"
                     "\"info\":{%s},"
                     "\"size\":\"%u\","
                     "\"overlaps\":\"%u\""
                     "}",
                     page,
                     track_names[i],
                     fd->num_intervals,
                     giggle_get_query_len(gqr, i));
            free(page);
            page = tmp_page;
        }
        asprintf(&tmp_page,"%s]});", page);
        free(page);
        page = tmp_page;

        response = MHD_create_response_from_buffer(
                strlen (page),
                (void*) page, MHD_RESPMEM_MUST_FREE);

        ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
        MHD_destroy_response (response);
        giggle_query_result_destroy(&gqr);
        free(r.chrm);
        r.chrm = NULL;
    }

    if (r.chrm != NULL)
        free(r.chrm);

    return ret;
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr,
                "usage:\t%s <num threads> <index dir> <track names> <header>\n",
                argv[0]);
        return 0;
    }

    uint32_t NUMBER_OF_THREADS = atoi(argv[1]);
    char *index_dir_name = argv[2]; 
    char *track_name_file = argv[3]; 
    char *header_file_name = argv[4]; 

    FILE *fp = fopen(header_file_name, "r");
    if (!fp)
        err(1, "Could not open header file '%s'", header_file_name);
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *header = (char *)malloc((fsize + 1)*sizeof(char));
    fread(header, fsize, 1, fp);
    fclose(fp);


    struct args *arg = (struct args *)malloc(sizeof(struct args));

    arg->gi = giggle_load(index_dir_name,
                          uint32_t_ll_giggle_set_data_handler);

    arg->track_names = (char **)calloc(arg->gi->file_index->num, sizeof(char*));
    arg->header = header;
    size_t linecap = 0;
    ssize_t linelen;
    fp = fopen(track_name_file, "r");
    if (!fp)
        err(1, "Could not open track names file '%s'", track_name_file);

    uint32_t i = 0;
    while ((linelen = getline(&(arg->track_names[i]), &linecap, fp)) > 0) {
        arg->track_names[i][strlen(arg->track_names[i])-1] = '\0';
        i+=1;
    }
    uint32_t num_tracks = i;

    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                              PORT,
                              NULL,
                              NULL, 
                              &answer_to_connection,
                              arg,
                              MHD_OPTION_THREAD_POOL_SIZE,
                              (unsigned int) NUMBER_OF_THREADS,
                              MHD_OPTION_END);

    if (NULL == daemon) return 1;
    getchar (); 

    giggle_index_destroy(&(arg->gi));
    cache.destroy();
    for(i = 0; i < num_tracks; ++i)
        free(arg->track_names[i]);
    free(arg->track_names);
 
    MHD_stop_daemon (daemon);
    return 0;
}
