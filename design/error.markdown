#Error Handling
##Write Failures

Here are errors may occur after writing. We del errors that are not considered by BehaviorDB.

1. [EBADF]            The d argument is not a valid descriptor open forwriting.

2. [EFBIG]            An attempt was made to write a file that exceeds the process’s file size limit or the maximum file size.

3. [EFAULT]           Part of iov or data to be written to the file points outside the process’s allocated address space.

4. [EINVAL]           The pointer associated with d was negative. 

5. [ENOSPC]           There is no free space remaining on the file system containing the file.

6. [EDQUOT]           The user’s quota of disk blocks on the file system containing the file has been exhausted.

7. [EIO]              An I/O error occurred while reading from or writing to the file system.

8. [EINTR]            A signal interrupted the write before it could be completed.

9. [EINVAL]           The value nbytes is greater than INT_MAX.

<del>[EPIPE]            An attempt is made to write to a pipe that is not open for reading by any process.</del>

<del>[EPIPE]            An attempt is made to write to a socket of type SOCK_STREAM that is not connected to a peer socket.</del>

<del>[EAGAIN]           The file was marked for non‐blocking I/O, and no data could be written immediately.</del>

<del>[EROFS]            An attempt was made to write over a disk label area at the beginning of a slice.  Use disklabel(8) -W to enable writing on the disk label area.<del>

