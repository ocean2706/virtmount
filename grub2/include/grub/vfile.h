/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2007  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_VFILE_HEADER
#define GRUB_VFILE_HEADER	1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/device.h>
#include <grub/fs.h>
#include <grub/file.h>
#include <grub/disk.h>

/* File description.  */
struct grub_vfile
{

	grub_file_t file;

};
typedef struct grub_vfile *grub_vfile_t;

extern grub_disk_read_hook_t EXPORT_VAR(grub_vfile_progress_hook);

/* Filters with lower ID are executed first.  */
typedef enum grub_vfile_filter_id
  {
    GRUB_FILE_FILTER_PUBKEY,
    GRUB_FILE_FILTER_GZIO,
    GRUB_FILE_FILTER_XZIO,
    GRUB_FILE_FILTER_LZOPIO,
    GRUB_FILE_FILTER_MAX,
    GRUB_FILE_FILTER_COMPRESSION_FIRST = GRUB_FILE_FILTER_GZIO,
    GRUB_FILE_FILTER_COMPRESSION_LAST = GRUB_FILE_FILTER_LZOPIO,
  } grub_vfile_filter_id_t;

typedef enum grub_vfile_driver_id{

		RAW,
		COW,
		QCOW,
		QCOW2,
		VHDX,
		VDI,
		VMDK,
		VPC

} grub_vfile_driver_id_t;

typedef grub_vfile_t (*grub_vfile_filter_t) (grub_vfile_t in, const char *filename);

extern grub_vfile_filter_t EXPORT_VAR(grub_vfile_filters_all)[GRUB_FILE_FILTER_MAX];
extern grub_vfile_filter_t EXPORT_VAR(grub_vfile_filters_enabled)[GRUB_FILE_FILTER_MAX];

static inline void
grub_vfile_filter_register (grub_vfile_filter_id_t id, grub_vfile_filter_t filter)
{
  grub_file_filters_all[id] = filter;
  grub_file_filters_enabled[id] = filter;
}

static inline void
grub_vfile_filter_unregister (grub_vfile_filter_id_t id)
{
  grub_file_filters_all[id] = 0;
  grub_file_filters_enabled[id] = 0;
}

static inline void
grub_vfile_filter_disable (grub_file_filter_id_t id)
{
  grub_file_filters_enabled[id] = 0;
}

static inline void
grub_vfile_filter_disable_compression (void)
{
  grub_file_filter_id_t id;

  for (id = GRUB_FILE_FILTER_COMPRESSION_FIRST;
       id <= GRUB_FILE_FILTER_COMPRESSION_LAST; id++)
    grub_file_filters_enabled[id] = 0;
}

static inline void
grub_vfile_filter_disable_all (void)
{
  grub_file_filter_id_t id;

  for (id = 0;
       id < GRUB_FILE_FILTER_MAX; id++)
    grub_file_filters_enabled[id] = 0;
}

static inline void
grub_vfile_filter_disable_pubkey (void)
{
  grub_vfile_filters_enabled[GRUB_FILE_FILTER_PUBKEY] = 0;
}

/* Get a device name from NAME.  */
char *EXPORT_FUNC(grub_vfile_get_device_name) (const char *name);

grub_vfile_t EXPORT_FUNC(grub_vfile_open) (const char *name);
grub_ssize_t EXPORT_FUNC(grub_vfile_read) (grub_vfile_t file, void *buf,
					  grub_size_t len);
grub_off_t EXPORT_FUNC(grub_vfile_seek) (grub_vfile_t file, grub_off_t offset);
grub_err_t EXPORT_FUNC(grub_vfile_close) (grub_vfile_t file);

/* Return value of grub_file_size() in case file size is unknown. */
#define GRUB_FILE_SIZE_UNKNOWN	 0xffffffffffffffffULL

static inline grub_off_t
grub_vfile_size (const grub_file_t file)
{
  return file->size;
}

static inline grub_off_t
grub_vfile_tell (const grub_vfile_t file)
{
  return file->file->offset;
}

static inline int
grub_vfile_seekable (const grub_vfile_t file)
{
  return !(file->file->not_easily_seekable);
}

grub_vfile_t
grub_vfile_offset_open (grub_vfile_t parent, grub_off_t start,
		       grub_off_t size);
void
grub_vfile_offset_close (grub_vfile_t file);

#endif /* ! GRUB_FILE_HEADER */
