#Error Handling
##Write Failures

According to write(2) man pages. Errors may occur after writing are listed as follows
(deleted errors are notconsidered by BehaviorDB).

<table>
<thead>
	<tr>
		<th>Error</th>
		<th>Desc</th>
		<th>Handling</th>
	</tr>
</thead>
<tbody>
<tr>
	<td>[EBADF]</td>
	<td>The d argument is not a valid descriptor open forwriting.</td>
	<td>Check fopen immediately. Throw std::runtime_error if fopen is failed.</td>
</tr>

<tr>
	<td>[EFBIG]</td>
	<td>An attempt was made to write a file that exceeds the process’s file size limit or the maximum file size.</td>
	<td></td>
</tr>

<tr>
	<td>[EFAULT]</td>
	<td>Part of iov or data to be written to the file points outside the process’s allocated address space.</td>
	<td></td>
</tr>

<tr>
	<td>[EINVAL]</td>
	<td>The pointer associated with d was negative. </td>
	<td></td>
</tr>

<tr>
	<td>[ENOSPC]</td>
	<td>There is no free space remaining on the file system containing the file.</td>
	<td></td>
</tr>

<tr>
	<td>[EDQUOT]</td>
	<td>The user’s quota of disk blocks on the file system containing the file has been exhausted.</td>
	<td></td>
</tr>

<tr>
	<td>[EIO]</td>
	<td>An I/O error occurred while reading from or writing to the file system.</td>
	<td></td>
</tr>

<tr>
	<td>[EINTR]</td>
	<td>A signal interrupted the write before it could be completed.</td>
	<td></td>
</tr>

<tr>
	<td>[EINVAL]</td>
	<td>The value nbytes is greater than INT_MAX.</td>
	<td></td>
</tr>
</tbody>

</table>

<del>[EPIPE]            An attempt is made to write to a pipe that is not open for reading by any process.</del>
<del>[EPIPE]            An attempt is made to write to a socket of type SOCK_STREAM that is not connected to a peer socket.</del>
<del>[EAGAIN]           The file was marked for non‐blocking I/O, and no data could be written immediately.</del>
<del>[EROFS]            An attempt was made to write over a disk label area at the beginning of a slice.  
Use disklabel(8) -W to enable writing on the disk label area.<del>

