void kernel twoDimDct(global float* image, int w, local float* block)
{
	#define s0 (0.353553385f)
	#define s1 (0.254897773f)
	#define s2 (0.270598054f)
	#define s3 (0.300672442f)
	#define s4 (0.353553385f)
	#define s5 (0.449988097f)
	#define s6 (0.653281510f)
	#define s7 (1.28145778f)

	#define a1 (0.707106769f)
	#define a2 (0.541196108f)
	#define a3 a1
	#define a4 (1.30656302f)
	#define a5 (0.382683426f)
	
	#define BLOCK_SIZE (8)

    int x_block = get_group_id(0);
    int y_block = get_group_id(1);

	int local_idx = get_local_id(0);

	int block_start = mad24(x_block, BLOCK_SIZE, y_block * w * BLOCK_SIZE);

	int image_row_start = mad24(local_idx, w, block_start);
	int image_col_start = local_idx + block_start;

	int block_row_start = BLOCK_SIZE * local_idx;
	int block_col_start = local_idx;

	float stripe[8];
	
	stripe[0] = image[image_row_start + 0] + image[image_row_start + 7];
	stripe[1] = image[image_row_start + 1] + image[image_row_start + 6];
	stripe[2] = image[image_row_start + 2] + image[image_row_start + 5];
	stripe[3] = image[image_row_start + 3] + image[image_row_start + 4];
	stripe[4] = image[image_row_start + 3] - image[image_row_start + 4];
	stripe[5] = image[image_row_start + 2] - image[image_row_start + 5];
	stripe[6] = image[image_row_start + 1] - image[image_row_start + 6];
	stripe[7] = image[image_row_start + 0] - image[image_row_start + 7];

	float temp0 = stripe[0];
	float temp1 = stripe[1];
	stripe[0] =  stripe[0] + stripe[3];
	stripe[1] =  stripe[1] + stripe[2];
	stripe[2] =  temp1    - stripe[2];
	stripe[3] =  temp0    - stripe[3];
	stripe[4] = -stripe[4] - stripe[5];
	stripe[5] =  stripe[5] + stripe[6];
	stripe[6] =  stripe[6] + stripe[7];
	
	temp0 = stripe[0];
	stripe[0] = stripe[0] + stripe[1];
	stripe[1] = temp0          - stripe[1];
	stripe[2] = stripe[2] + stripe[3];

	float tempMinus6plus4 = -(stripe[4] + stripe[6]);
	stripe[2] *= a1;
	stripe[4] = mad(-stripe[4], a2, tempMinus6plus4 * a5);
	stripe[5] *= a3;
	stripe[6] = mad(stripe[6], a4, tempMinus6plus4 * a5);

	float temp2 = stripe[2];
	stripe[2] += stripe[3];
	stripe[3] -= temp2;
	temp2 = stripe[5];
	stripe[5] += stripe[7];
	stripe[7] -= temp2;

	block[                     block_col_start ] = stripe[0] * s0;
	block[mad24(4, BLOCK_SIZE, block_col_start)] = stripe[1] * s4;
	block[mad24(2, BLOCK_SIZE, block_col_start)] = stripe[2] * s2;
	block[mad24(6, BLOCK_SIZE, block_col_start)] = stripe[3] * s6;
	block[mad24(5, BLOCK_SIZE, block_col_start)] = (stripe[4] + stripe[7]) * s5;
	block[mad24(1, BLOCK_SIZE, block_col_start)] = (stripe[5] + stripe[6]) * s1;
	block[mad24(7, BLOCK_SIZE, block_col_start)] = (stripe[5] - stripe[6]) * s7;
	block[mad24(3, BLOCK_SIZE, block_col_start)] = (stripe[7] - stripe[4]) * s3;

	barrier(CLK_LOCAL_MEM_FENCE);

	stripe[0] = block[block_row_start + 0] + block[block_row_start + 7];
	stripe[1] = block[block_row_start + 1] + block[block_row_start + 6];
	stripe[2] = block[block_row_start + 2] + block[block_row_start + 5];
	stripe[3] = block[block_row_start + 3] + block[block_row_start + 4];
	stripe[4] = block[block_row_start + 3] - block[block_row_start + 4];
	stripe[5] = block[block_row_start + 2] - block[block_row_start + 5];
	stripe[6] = block[block_row_start + 1] - block[block_row_start + 6];
	stripe[7] = block[block_row_start + 0] - block[block_row_start + 7];

	temp0 = stripe[0];
	temp1 = stripe[1];
	stripe[0] =  stripe[0] + stripe[3];
	stripe[1] =  stripe[1] + stripe[2];
	stripe[2] =  temp1    - stripe[2];
	stripe[3] =  temp0    - stripe[3];
	stripe[4] = -stripe[4] - stripe[5];
	stripe[5] =  stripe[5] + stripe[6];
	stripe[6] =  stripe[6] + stripe[7];
	
	temp0 = stripe[0];
	stripe[0] = stripe[0] + stripe[1];
	stripe[1] = temp0          - stripe[1];
	stripe[2] = stripe[2] + stripe[3];

	tempMinus6plus4 = -(stripe[4] + stripe[6]);
	stripe[2] *= a1;
	stripe[4] = mad(-stripe[4], a2, tempMinus6plus4 * a5);
	stripe[5] *= a3;
	stripe[6] = mad(stripe[6], a4, tempMinus6plus4 * a5);

	temp2 = stripe[2];
	stripe[2] += stripe[3];
	stripe[3] -= temp2;
	temp2 = stripe[5];
	stripe[5] += stripe[7];
	stripe[7] -= temp2;

	if (local_idx == 0)
		image[        image_col_start ] = stripe[0] * s0 - 1024;
	else
		image[        image_col_start ] = stripe[0] * s0;
	image[mad24(4, w, image_col_start)] = stripe[1] * s4;
	image[mad24(2, w, image_col_start)] = stripe[2] * s2;
	image[mad24(6, w, image_col_start)] = stripe[3] * s6;
	image[mad24(5, w, image_col_start)] = (stripe[4] + stripe[7]) * s5;
	image[mad24(1, w, image_col_start)] = (stripe[5] + stripe[6]) * s1;
	image[mad24(7, w, image_col_start)] = (stripe[5] - stripe[6]) * s7;
	image[mad24(3, w, image_col_start)] = (stripe[7] - stripe[4]) * s3;
}
