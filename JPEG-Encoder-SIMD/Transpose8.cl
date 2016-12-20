void kernel transpose8( global float *image, int w, local float *block)
{
	#define BLOCK_SIZE (8)

    int x_block = get_group_id(0);
    int y_block = get_group_id(1);

	int x_local = get_local_id(0);
	
	int local_input_idx = x_local;
	int local_output_idx = x_local * BLOCK_SIZE;

	int gloabl_input_idx = y_block * BLOCK_SIZE * w + x_block * BLOCK_SIZE + x_local * BLOCK_SIZE;
	int gloabl_output_idx = gloabl_input_idx;

	block[local_input_idx] = image[gloabl_input_idx]; local_input_idx += BLOCK_SIZE; gloabl_input_idx++;
	block[local_input_idx] = image[gloabl_input_idx]; local_input_idx += BLOCK_SIZE; gloabl_input_idx++;
	block[local_input_idx] = image[gloabl_input_idx]; local_input_idx += BLOCK_SIZE; gloabl_input_idx++;
	block[local_input_idx] = image[gloabl_input_idx]; local_input_idx += BLOCK_SIZE; gloabl_input_idx++;
	block[local_input_idx] = image[gloabl_input_idx]; local_input_idx += BLOCK_SIZE; gloabl_input_idx++;
	block[local_input_idx] = image[gloabl_input_idx]; local_input_idx += BLOCK_SIZE; gloabl_input_idx++;
	block[local_input_idx] = image[gloabl_input_idx]; local_input_idx += BLOCK_SIZE; gloabl_input_idx++;
	block[local_input_idx] = image[gloabl_input_idx];

	barrier(CLK_LOCAL_MEM_FENCE);

	image[gloabl_output_idx] = block[local_output_idx]; local_output_idx++; gloabl_output_idx++;
	image[gloabl_output_idx] = block[local_output_idx]; local_output_idx++; gloabl_output_idx++;
	image[gloabl_output_idx] = block[local_output_idx]; local_output_idx++; gloabl_output_idx++;
	image[gloabl_output_idx] = block[local_output_idx]; local_output_idx++; gloabl_output_idx++;
	image[gloabl_output_idx] = block[local_output_idx]; local_output_idx++; gloabl_output_idx++;
	image[gloabl_output_idx] = block[local_output_idx]; local_output_idx++; gloabl_output_idx++;
	image[gloabl_output_idx] = block[local_output_idx]; local_output_idx++; gloabl_output_idx++;
	image[gloabl_output_idx] = block[local_output_idx];
}
