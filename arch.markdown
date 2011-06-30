#BehaviorDB Architecture Future Document

##Address

##Pool
###Conditions/Assumptions
####Pool Class
CTOR of the pool class should hold conditions as follows:
1. file_  for storing chunks has ben initiated
2. addr_eval<AddrType> has been initiated
3. idPool_ for managing chunk id has been initiated
4. headerPool_ for accessing headers has been initiated
####Pool Member Functions
Preconditions:
1. Data source, file_(s) or in-mem chars is accessible.
2. Size of data source does not greater than chunk size of destination pool.
Postconditions and error handling are described separatly in each function.

###Configuration

A pool has options of configuration as follows:

1. Directory Identification (dirID)
2. Working Directory (work_dir)
3. Address Evaluator (addrEval)

###External Data Format
####Pool file

Non-buffering file I/O

####Parameters:
 
 - Address size

 - Chunk size

 - Work directory (optional)

 - Log directory (optional)

####Description:

 // to be added

###Deconstructor

 // to be added

###Error Handling

####Construction Error
Error occurs during construction causes program call exit(1) immediately, some error message will be written to stderr.

####Runtime Error
When error happens during process of methods of a pool object, the object records error code and source line. These 
information will be logged by BehaviorDB in upper layer. For more information about the error code, please refer to 
error.hpp.

###Methods

####1. write

<table>
	<thead>
		<tr>
			<th> Brief <th> Parameters <th> Description
		</tr>
	</thead>
	<tbody>
		<tr>
			<td> Put 
			<td> data, size
			<td> Put data to a new chunk with new address
		</tr>
		<tr>
			<td> Append/Insert 
			<td> data, size, address [, off=-1, *header = 0]
			<td> Append data to an indicated address. When the chunk 
			refered by the address is not allocated, this method 
			functions as put with specified address.
		</tr>
		<tr>
			<td> Vectorized Put
			<td> variant IO vector
			<td> [to be added]
		</tr>
	</tbody>
</table>

####2. read

<table>
	<thead>
		<tr>
			<th> Brief <th> Parameters <th> Description
		</tr>
	</thead>
	<tbody>
		<tr>
			<td> Read front
			<td> address, buffer, size
			<td> Read 'size' bytes data of a chunk
		</tr>
		<tr>
			<td> Read segment
			<td> address, offset, buffer, size, [*header = 0]
			<td> Read 'size' bytes data according to the address and offset
		</tr>
	</tbody>
</table>

####3. move

<table>
	<thead>
		<tr>
			<th> Brief <th> Parameters <th> Description
		</tr>
	</thead>
	<tbody>
		<tr>
			<td> move
			<td> address, *dest_pool
			<td> Move all data in this pool to dest pool
		</tr>
		<tr>
			<td> merge_move
			<td> data, size, address, offset, *dest_pool, dest_addr [*header = 0]
			<td> Merge data within specified offset and move merged data to a pool
		</tr>
	</tbody>
</table>

####4. erase 

<table>
	<thead>
		<tr>
			<th> Brief <th> Parameters <th> Description
		</tr>
	</thead>
	<tbody>
		<tr>
			<td> erase
			<td> address
			<td> Mark an address as free
		</tr>
		<tr>
			<td> erase segment
			<td> address, offset, size
			<td> Move data after the segment forward to keep
			data in continuous.
		</tr>

	</tabody>
</table>

####Miscellaneous 

1. __head__ Take an address as parameter and get its chunk header. 
This method implies a seek-to-chunk operation.

2. __pine__ Make an address is invisible to client.

3. __unpine__ Make an address is visible to client.

3. __tell2addr-off__ Translate seek head to address-offset pair.

4. __addr-off2tell__ Reverse version of tell2addr-off.

5. __lock__

 - __acquire__ Prevent all operations to a pool.

 - __release__ Allow operations to a pool.

