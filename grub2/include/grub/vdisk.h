/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_VDISK_HEADER
#define GRUB_VDISK_HEADER	1

#include <config.h>

#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/types.h>
#include <grub/device.h>
/* For NULL.  */
#include <grub/mm.h>
#include <grub/disk.h>

/* These are used to set a device id. When you add a new disk device,
   you must define a new id for it here.  */


struct grub_vdisk;
#ifdef GRUB_VUTIL
struct grub_vdisk_memberlist;
#endif
typedef int (*grub_vdisk_dev_iterate_hook_t) (const char *name, void *data);

/* Disk device.  */

struct grub_vdisk_dev
{
  /* The device name.  */

	grub_disk_dev_t grub_disk_dev; // ref to real grub disk in disk.h
	const char *name;

  /* The device id used by the cache manager.  */
  enum grub_disk_dev_id id; //maybe use vdisk id ?

  /* Call HOOK with each device name, until HOOK returns non-zero.  */
  int (*iterate) (grub_vdisk_dev_iterate_hook_t hook, void *hook_data,
		  grub_disk_pull_t pull);

  /* Open the device named NAME, and set up DISK.  */
  grub_err_t (*open) (const char *name, struct grub_vdisk *disk);

  /* Close the disk DISK.  */
  void (*close) (struct grub_vdisk *disk);

  /* Read SIZE sectors from the sector SECTOR of the disk DISK into BUF.  */
  grub_err_t (*read) (struct grub_vdisk *disk, grub_disk_addr_t sector,
		      grub_size_t size, char *buf);

  /* Write SIZE sectors from BUF into the sector SECTOR of the disk DISK.  */
  grub_err_t (*write) (struct grub_vdisk *disk, grub_disk_addr_t sector,
		       grub_size_t size, const char *buf);

#ifdef GRUB_VUTIL
  struct grub_vdisk_memberlist *(*memberlist) (struct grub_vdisk *disk);
  const char * (*raidname) (struct grub_vdisk *disk);
#endif

  /* The next disk device.  */
  struct grub_vdisk_dev *next;
} ;


typedef struct grub_vdisk_dev *grub_vdisk_dev_t;

extern grub_vdisk_dev_t EXPORT_VAR (grub_vdisk_dev_list);

/*struct grub_partition;

typedef void (*grub_disk_read_hook_t) (grub_disk_addr_t sector,
				       unsigned offset, unsigned length,
				       void *data);
*/
/* Disk.  */
struct grub_vdisk
{

  grub_disk_t grub_disk; // real grub disk info


  /* The underlying disk device.  */
  grub_vdisk_dev_t dev;


};
typedef struct grub_vdisk *grub_vdisk_t;

#ifdef GRUB_VUTIL
struct grub_vdisk_memberlist
{
  grub_vdisk_t disk;
  struct grub_vdisk_memberlist *next;
};
typedef struct grub_vdisk_memberlist *grub_vdisk_memberlist_t;
#endif



void EXPORT_FUNC(grub_vdisk_dev_register) (grub_vdisk_dev_t dev);
void EXPORT_FUNC(grub_vdisk_dev_unregister) (grub_vdisk_dev_t dev);
static inline int
grub_vdisk_dev_iterate (grub_vdisk_dev_iterate_hook_t hook, void *hook_data)
{
  grub_disk_dev_t p;
  grub_disk_pull_t pull;

  for (pull = 0; pull < GRUB_DISK_PULL_MAX; pull++)
    for (p = grub_disk_dev_list; p; p = p->next)
      if (p->iterate && (p->iterate) (hook, hook_data, pull))
	return 1;

  return 0;
}

grub_vdisk_t EXPORT_FUNC(grub_vdisk_open) (const char *name);
void EXPORT_FUNC(grub_vdisk_close) (grub_disk_t disk);
grub_err_t EXPORT_FUNC(grub_vdisk_read) (grub_vdisk_t disk,
					grub_disk_addr_t sector,
					grub_off_t offset,
					grub_size_t size,
					void *buf);
grub_err_t grub_vdisk_write (grub_vdisk_t disk,
			    grub_disk_addr_t sector,
			    grub_off_t offset,
			    grub_size_t size,
			    const void *buf);
extern grub_err_t (*EXPORT_VAR(grub_vdisk_write_weak)) (grub_vdisk_t disk,
						       grub_disk_addr_t sector,
						       grub_off_t offset,
						       grub_size_t size,
						       const void *buf);


grub_uint64_t EXPORT_FUNC(grub_vdisk_get_size) (grub_vdisk_t disk);



/* Disk cache.  */




#endif /* ! GRUB_DISK_HEADER */
