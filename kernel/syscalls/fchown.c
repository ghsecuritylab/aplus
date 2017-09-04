#include <aplus.h>
#include <aplus/vfs.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <aplus/syscall.h>
#include <libc.h>

SYSCALL(86, fchown,
int sys_fchown(int fd, uid_t owner, gid_t group) {
    inode_t* inode = current_task->fd[fd].inode;
    if(unlikely(!inode)) {
        errno = EBADF;
        return -1;
    }

    if(!((current_task->uid == inode->uid) || (current_task->uid == TASK_ROOT_UID))) {
        errno = EPERM;
        return -1;
    }

    return vfs_chown(inode, owner, group);
});
