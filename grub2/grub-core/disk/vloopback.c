/* vloopback.c - command to add vloopback devices.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2006,2007  Free Software Foundation, Inc.
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

#include <grub/dl.h>
#include <grub/misc.h>
#include <grub/file.h>
#include <grub/vfile.h>
#include <grub/disk.h>
#include <grub/vdisk.h>
#include <grub/mm.h>
#include <grub/extcmd.h>
#include <grub/i18n.h>

GRUB_MOD_LICENSE ("GPLv3+");

struct grub_vloopback
{
  char *devname;
  grub_vfile_t file;
  struct grub_vloopback *next;
  unsigned long id;
};

static struct grub_vloopback *vloopback_list;
static unsigned long last_id = 0;

static const struct grub_arg_option options[] =
  {
    /* TRANSLATORS: The disk is simply removed from the list of available ones,
       not wiped, avoid to scare user.  */
    {"delete", 'd', 0, N_("Delete the specified vloopback drive."), 0, 0},
    {0, 0, 0, 0, 0, 0}
  };

/* Delete the vloopback device NAME.  */
static grub_err_t
delete_vloopback (const char *name)
{
  struct grub_vloopback *dev;
  struct grub_vloopback **prev;

  /* Search for the device.  */
  for (dev = vloopback_list, prev = &vloopback_list;
       dev;
       prev = &dev->next, dev = dev->next)
    if (grub_strcmp (dev->devname, name) == 0)
      break;

  if (! dev)
    return grub_error (GRUB_ERR_BAD_DEVICE, "device not found");

  /* Remove the device from the list.  */
  *prev = dev->next;

  grub_free (dev->devname);
  grub_file_close (dev->file);
  grub_free (dev);

  return 0;
}

/* The command to add and remove vloopback devices.  */
static grub_err_t
grub_cmd_vloopback (grub_extcmd_context_t ctxt, int argc, char **args)
{
  struct grub_arg_list *state = ctxt->state;
  grub_vfile_t file;
  struct grub_vloopback *newdev;
  grub_err_t ret;

  if (argc < 1)
    return grub_error (GRUB_ERR_BAD_ARGUMENT, "device name required");

  /* Check if `-d' was used.  */
  if (state[0].set)
      return delete_vloopback (args[0]);

  if (argc < 2)
    return grub_error (GRUB_ERR_BAD_ARGUMENT, N_("filename expected"));

  file = grub_vfile_open (args[1]);
  if (! file)
    return grub_errno;

  /* First try to replace the old device.  */
  for (newdev = vloopback_list; newdev; newdev = newdev->next)
    if (grub_strcmp (newdev->devname, args[0]) == 0)
      break;

  if (newdev)
    {
      grub_vfile_close (newdev->file);
      newdev->file = file;

      return 0;
    }

  /* Unable to replace it, make a new entry.  */
  newdev = grub_malloc (sizeof (struct grub_vloopback));
  if (! newdev)
    goto fail;

  newdev->devname = grub_strdup (args[0]);
  if (! newdev->devname)
    {
      grub_free (newdev);
      goto fail;
    }

  newdev->file = file;
  newdev->id = last_id++;

  /* Add the new entry to the list.  */
  newdev->next = vloopback_list;
  vloopback_list = newdev;

  return 0;

fail:
  ret = grub_errno;
  grub_vfile_close (file);
  return ret;
}


static int
grub_vloopback_iterate (grub_vdisk_dev_iterate_hook_t hook, void *hook_data,
		       grub_disk_pull_t pull)
{
  struct grub_vloopback *d;
  if (pull != GRUB_DISK_PULL_NONE)
    return 0;
  for (d = vloopback_list; d; d = d->next)
    {
      if (hook (d->devname, hook_data))
	return 1;
    }
  return 0;
}

static grub_err_t
grub_vloopback_open (const char *name, grub_vdisk_t disk)
{
  struct grub_vloopback *dev;

  for (dev = vloopback_list; dev; dev = dev->next)
    if (grub_strcmp (dev->devname, name) == 0)
      break;

  if (! dev)
    return grub_error (GRUB_ERR_UNKNOWN_DEVICE, "can't open device");

  /* Use the filesize for the disk size, round up to a complete sector.  */
  if (dev->file->file->size != GRUB_FILE_SIZE_UNKNOWN)
    disk->grub_disk->total_sectors = ((dev->file->file->size + GRUB_DISK_SECTOR_SIZE - 1)
			   / GRUB_DISK_SECTOR_SIZE);
  else
    disk->grub_disk->total_sectors = GRUB_DISK_SIZE_UNKNOWN;
  /* Avoid reading more than 512M.  */
  disk->grub_disk->max_agglomerate = 1 << (29 - GRUB_DISK_SECTOR_BITS
				- GRUB_DISK_CACHE_BITS);

  disk->grub_disk->id = dev->id;

  disk->grub_disk->data = dev;

  return 0;
}

static grub_err_t
grub_vloopback_read (grub_vdisk_t disk, grub_disk_addr_t sector,
		    grub_size_t size, char *buf)
{
  grub_vfile_t file = (grub_vfile_t)( ((struct grub_vloopback *) disk->grub_disk->data)->file);
  grub_off_t pos;

  grub_vfile_seek (file, sector << GRUB_DISK_SECTOR_BITS);

  grub_vfile_read (file, buf, size << GRUB_DISK_SECTOR_BITS);
  if (grub_errno)
    return grub_errno;

  /* In case there is more data read than there is available, in case
     of files that are not a multiple of GRUB_DISK_SECTOR_SIZE, fill
     the rest with zeros.  */
  pos = (sector + size) << GRUB_DISK_SECTOR_BITS;
  if (pos > file->file->size)
    {
      grub_size_t amount = pos - file->file->size;
      grub_memset (buf + (size << GRUB_DISK_SECTOR_BITS) - amount, 0, amount);
    }

  return 0;
}

static grub_err_t
grub_vloopback_write (grub_vdisk_t disk __attribute ((unused)),
		     grub_disk_addr_t sector __attribute ((unused)),
		     grub_size_t size __attribute ((unused)),
		     const char *buf __attribute ((unused)))
{
  return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET,
		     "vloopback write is not supported");
}

static struct grub_disk_dev grub_vloopback_dev =
  {
    .name = "vloopback",
    .id = GRUB_DISK_DEVICE_LOOPBACK_ID,
    .iterate = grub_vloopback_iterate,
    .open = grub_vloopback_open,
    .read = grub_vloopback_read,
    .write = grub_vloopback_write,
    .next = 0
  };

static grub_extcmd_t cmd;

GRUB_MOD_INIT(vloopback)
{
  cmd = grub_register_extcmd ("vloopback", grub_cmd_vloopback, 0,
			      N_("[-d] DEVICENAME FILE."),
			      /* TRANSLATORS: The file itself is not destroyed
				 or transformed into drive.  */
			      N_("Make a virtual drive from a file."), options);
  grub_disk_dev_register (&grub_vloopback_dev);
}

GRUB_MOD_FINI(vloopback)
{
  grub_unregister_extcmd (cmd);
  grub_disk_dev_unregister (&grub_vloopback_dev);
}
