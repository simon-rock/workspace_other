/*
 * null engine
 *
 * IO engine that doesn't do any real IO transfers, it just pretends to.
 * The main purpose is to test fio itself.
 *
 * It also can act as external C++ engine - compiled with:
 *
 * g++ -O2 -g -shared -rdynamic -fPIC -o null.so null.c -DFIO_EXTERNAL_ENGINE
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "fio.h"
#include <optgroup.h>

#define __cplusplus

struct null_data {
	struct io_u **io_us;
	int queued;
	//int events;
};
// for async
struct fio_myio_iou {
	struct io_u *io_u;
	int io_complete;
};
// param of ioengine named myio
struct myio_options {
	void *pad;
	char* conf;
};

static struct io_u *fio_myio_event(struct thread_data *td, int event)
{
    printf("----fio_myio_event----\n");
	struct null_data *nd = (struct null_data *) td->io_ops_data;

	return nd->io_us[event];
}
// get all the io which is completed
static int fio_myio_getevents(struct thread_data *td, unsigned int min,
			      unsigned int max,
			      const struct timespec fio_unused *t)
{
	struct null_data *nd = (struct null_data *) td->io_ops_data;
	/*int ret = 0;
	
	if (min_events) {
		ret = nd->events;
		nd->events = 0;
	}

	return ret;*/
    unsigned int events = 0;
	struct io_u *io_u;
	int i;

	//dprint(FD_IO, "%s\n", __FUNCTION__);
	do {
		io_u_qiter(&td->io_u_all, io_u, i) {
			struct fio_myio_iou *io;

			if (!(io_u->flags & IO_U_F_FLIGHT))
				continue;

			io = io_u->engine_data;
			if (io->io_complete) {
				io->io_complete = 0;
				nd->io_us[events] = io_u;
				events++;

				if (events >= max)
					break;
			}

		}
		if (events < min)
			usleep(100);
		else
			break;

	} while (1);
    nd->queued -= events;
    printf("----fio_myio_getevents----in %d:%d current_queue:%d get:%d\n", min, max, nd->queued, events);
	return events;
}
// commit io in the queue to storage system
static int fio_myio_commit(struct thread_data *td)
{
    return 0;
    /*
	struct null_data *nd = (struct null_data *) td->io_ops_data;

	if (!nd->events) {
#ifndef FIO_EXTERNAL_ENGINE
		io_u_mark_submit(td, nd->queued);
#endif
		nd->events = nd->queued;
		nd->queued = 0;
	}
    */
	return 0;
}

static int fio_myio_queue(struct thread_data *td, struct io_u *io_u)
{
	struct null_data *nd = (struct null_data *) td->io_ops_data;

	fio_ro_check(td, io_u);
    printf("----fio_myio_queue----\n");
	if (td->io_ops->flags & FIO_SYNCIO)
		return FIO_Q_COMPLETED;
	//if (nd->events)
    if (nd->queued >= td->o.iodepth)
		return FIO_Q_BUSY;
	//nd->io_us[nd->queued++] = io_u;
    nd->queued++;
    struct fio_myio_iou *io = io_u->engine_data;
    io->io_complete = 1;
    printf("----fio_myio_queue---- %d\n", nd->queued);
	return FIO_Q_QUEUED;
}

// simon: remark fio_unused
static int fio_myio_open_file(struct thread_data fio_unused *td,
			 struct fio_file fio_unused *f)
{
    printf("----fio_myio_open_file----\n");
	return 0;
}
// simon: remark fio_unused
static int fio_myio_close_file(struct thread_data fio_unused *td,
                              struct fio_file fio_unused *f){
    printf("----fio_myio_close_file----\n");
    return 0;
}
static void fio_myio_cleanup(struct thread_data *td)
{
    printf("----fio_myio_cleanup----\n");
	struct null_data *nd = (struct null_data *) td->io_ops_data;

	if (nd) {
		free(nd->io_us);
		free(nd);
	}
}
static int fio_myio_setup(thread_data* td){
    printf("----fio_myio_setup----\n");
    return 0;
}
    
static int fio_myio_init(struct thread_data *td)
{
    struct myio_options *o = td->eo;
    printf("----fio_myio_init----\n");
    //printf("----fio_myio_init----(%s)\n", o->conf);
	struct null_data *nd = (struct null_data *) malloc(sizeof(*nd));

	memset(nd, 0, sizeof(*nd));

	if (td->o.iodepth != 1) {
		nd->io_us = (struct io_u **) malloc(td->o.iodepth * sizeof(struct io_u *));
		memset(nd->io_us, 0, td->o.iodepth * sizeof(struct io_u *));
	} else
		td->io_ops->flags |= FIO_SYNCIO;

	td->io_ops_data = nd;
	return 0;
}

static int fio_myio_io_u_init(struct thread_data *td, struct io_u *io_u){
    //if (!io_u->engine_data) {
		struct fio_myio_iou *io;
        
		io = malloc(sizeof(struct fio_myio_iou));
        printf("----fio_myio_io_u_init----io_u:%p io:%p\n", io_u, io);
		if (!io) {
			td_verror(td, errno, "malloc");
			return 1;
		}
		io->io_complete = 0;
		io->io_u = io_u;
		io_u->engine_data = io;
        //}
	return 0;
}

static void fio_myio_io_u_free(struct thread_data *td, struct io_u *io_u)
{
    
	struct fio_myio_iou *io = io_u->engine_data;
    printf("----fio_myio_io_u_free----io_u:%p io%p\n", io_u, io_u->engine_data);
	if (io) {
		if (io->io_complete)
			log_err("incomplete IO found.\n");
		io_u->engine_data = NULL;
		free(io);
	}
}

#ifndef __cplusplus
static struct ioengine_ops ioengine = {
	.name		= "null",
	.version	= FIO_IOOPS_VERSION,
	.queue		= fio_null_queue,
	.commit		= fio_null_commit,
	.getevents	= fio_null_getevents,
	.event		= fio_null_event,
	.init		= fio_null_init,
	.cleanup	= fio_null_cleanup,
	.open_file	= fio_null_open,
	.flags		= FIO_DISKLESSIO | FIO_FAKEIO,
};

static void fio_init fio_null_register(void)
{
	register_ioengine(&ioengine);
}

static void fio_exit fio_null_unregister(void)
{
	unregister_ioengine(&ioengine);
}

#else

#ifdef FIO_EXTERNAL_ENGINE
extern "C" {
    /*static struct fio_option options[] = {
	{
		.name	= "conf",
		.lname	= "myio conf",
		.type	= FIO_OPT_STR_SET,
		.off1	= offsetof(struct myio_options, conf),
		.help	= "conf file, a simple test",
		.category = FIO_OPT_C_ENGINE,
		.group	= FIO_OPT_G_RBD,
	},
	{
		.name	= NULL,
	},
    };*/
static struct fio_option options[2];
static struct ioengine_ops ioengine;
void get_ioengine(struct ioengine_ops **ioengine_ptr)
{
    // i don't know why the conf name cann't been load???, you cann't add conf in myio.fio!!!!
    options[0].name	= "conf";
    options[0].lname = "myio conf";
    options[0].type	= FIO_OPT_STR_SET;
    options[0].off1	= offsetof(struct myio_options, conf);
    options[0].help	= "conf file, a simple test";
    options[0].category = FIO_OPT_C_ENGINE;
    options[0].group	= FIO_OPT_G_RBD;
    options[1].name	= NULL;
    
	*ioengine_ptr = &ioengine;
	ioengine.name           = "cpp_myio";
	ioengine.version        = FIO_IOOPS_VERSION;
    ioengine.setup          = fio_myio_setup;
    ioengine.init           = fio_myio_init;
    ioengine.open_file      = fio_myio_open_file,
    ioengine.close_file     = fio_myio_close_file,
        
    ioengine.queue          = fio_myio_queue;
	ioengine.commit         = fio_myio_commit;
	ioengine.getevents      = fio_myio_getevents;
	ioengine.event          = fio_myio_event;
    
    ioengine.io_u_init      = fio_myio_io_u_init,
	ioengine.io_u_free      = fio_myio_io_u_free,
	ioengine.cleanup        = fio_myio_cleanup;
	ioengine.flags          = FIO_DISKLESSIO | FIO_FAKEIO;
    ioengine.options		= options;
	ioengine.option_struct_size	= sizeof(struct myio_options);
}
}
#endif /* FIO_EXTERNAL_ENGINE */

#endif /* __cplusplus */
