#BehaviorDB Architecture Future Document

##Address

##Pool

###Constructor

####Parameters:
 
 - Address size

 - Chunk size

 - Work directory (optional)

 - Log directory (optional)

####Description:

 // to be added

###Deconstructor

 // to be added

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
			<td> Append/Put 
			<td> address, data, size, [*header = 0]
			<td> Append data to an indicated address. When the chunk 
			refered by the address is not allocated, this method 
			functions as put with specified address.
		</tr>
		<tr>
			<td> Insert
			<td> address, offset, data, size, [*header = 0]
			<td> Insert data to spcified offset from the address
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
			<td> address, offset, data, size, *dest_pool, dest_addr [*header = 0]
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

