#ifndef MXPSQL_MPARC_CWRAP_H
#define MXPSQL_MPARC_CWRAP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mparc_status;
typedef struct mparc_status mparc_status;

struct mparc;
typedef struct mparc mparc_t;

bool mparc_new(mparc_t** obj);
void mparc_destroy(mparc_t* obj);


#ifdef __cplusplus
}
#endif

#endif
