#BehaviorDB Architecture Document

##Pool
###Interfaces
1. insert<br/>
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
			<td> address, data, size 
			<td> Append data to an indicated address. When the chunk 
			refered by the address is not allocated, this method 
			functions as put with specified address.
		</tr>
		<tr>
			<td> Insert
			<td> address, offset, data, size
			<td> Insert data to spcified offset from the address
		</tr>
	</tbody>
</table>

2. get
3. move
4. del

