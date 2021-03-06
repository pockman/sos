#include "vfs.h"
#include "string.h"
#include "ramfs.h"
#include "vm.h"
#include "common.h"
#include "errno.h"
#include <sos/limits.h>

#define NR_INITRD_FILES 64
#define RAMFS_ENTRY_MAGIC 0xBF

typedef struct initrd_entry_header
{
   unsigned char magic; // The magic number is there to check for consistency.
   char name[NAMELEN+1];
   unsigned int offset; // Offset in the initrd the file starts.
   unsigned int length; // Length of the file.
} __attribute__((packed)) initrd_entry_header_t;

typedef struct initrd_header
{
    u32 nfiles;
    initrd_entry_header_t files[NR_INITRD_FILES];
} initrd_header_t;

void Ramfs::init(char* addr, size_t size, const char* cmdline)
{
    (void)size;

    int cmd_len = strlen(cmdline);
    _cmdline = (char*)vmm.kmalloc(cmd_len+1, 1);
    strcpy(_cmdline, cmdline);

    _sb = (initrd_header_t*)addr;
    //kprintf("files: %d\n", _sb->nfiles);

    u32 dev = DEVNO(RAMFS_MAJOR, 0);
    _iroot = vfs.alloc_inode(dev, 1);
    _iroot->ref = 1;
    _iroot->dev = dev;
    _iroot->type = FsNodeType::Dir;
    _iroot->ino = 1;
    _iroot->size = 0;
    _iroot->blksize = 1;
    _iroot->blocks = _iroot->size;
    _iroot->fs = this;

    _nr_nodes = _sb->nfiles;
}

ssize_t Ramfs::read(File * filp, char * buf, size_t count, off_t * offset)
{
    inode_t* ip = filp->inode();
    off_t off = filp->off();
    initrd_entry_header* ieh = &_sb->files[ip->ino-2];

    if (off >= ip->size) return 0;
    if (count + off >= ip->size) count = ip->size - off;

    char* data = (char*)_sb + ieh->offset + off;
    memcpy(buf, data, count);

    filp->set_off(off + count);
    if (offset) *offset = filp->off();
    return count;
}

ssize_t Ramfs::write(File* filp, const char * buf, size_t count, off_t *offset)
{
    inode_t* ip = filp->inode();
    kassert(ip->type == FsNodeType::File);
    (void)buf;
    (void)count;
    (void)offset;
    return -EINVAL;

}
int Ramfs::readdir(File* filp, dentry_t * de, filldir_t)
{
    inode_t* ip = filp->inode();
    int id = filp->off() / sizeof(initrd_entry_header_t);

    if (ip->type != FsNodeType::Dir) return -EINVAL;
    if (id >= _nr_nodes) return 0;

    initrd_entry_header_t* ieh = &_sb->files[id];
    strcpy(de->name, ieh->name);

    de->ip = vfs.alloc_inode(_iroot->dev, id+2);
    if (de->ip->ref == 0) {
        de->ip->ref++;
        de->ip->ino = id+2;
        read_inode(de->ip);
    }

    filp->set_off((id+1)*sizeof(initrd_entry_header_t));
    return 1;
}

dentry_t * Ramfs::lookup(inode_t * dir, dentry_t *de)
{
    if (dir->type != FsNodeType::Dir) return NULL;

    int i = -1;
    for (i = 0; i < _nr_nodes; ++i) {
        initrd_entry_header* dp = &_sb->files[i];
        if (strncmp(de->name, dp->name, NAMELEN) == 0) {
            break;
        }
    }

    if (i >= _nr_nodes) return NULL;
    de->ip = vfs.alloc_inode(_iroot->dev, i+2);
    if (de->ip->ref == 0) {
        de->ip->ref++;
        de->ip->ino = i+2;
        read_inode(de->ip);
    }

    return de;
}

void Ramfs::read_inode(inode_t *ip)
{
    int i = ip->ino;
    if (i == 1) {
        memcpy(ip, _iroot, sizeof *ip);
    } else if (i > 1 && i <= _nr_nodes+1) {
        initrd_entry_header* de = &_sb->files[i-2];
        kassert(de->magic == RAMFS_ENTRY_MAGIC);
        ip->size = de->length;
        ip->blksize = 1;
        ip->blocks = ip->size;
        ip->dev = DEVNO(RAMFS_MAJOR, 0);
        ip->type = FsNodeType::File;
        ip->data = 0;
        ip->fs = this;
    }
}

FileSystem* create_ramfs(const void* data)
{
    auto* info = (ramfs_mod_info_t*)data;
    auto* ramfs = new Ramfs;
    ramfs->init(info->addr, info->size, info->cmdline);
    return ramfs;
}

