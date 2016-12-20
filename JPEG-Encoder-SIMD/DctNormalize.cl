void kernel dctNormalize(global float* image, int w)
{
	#define BLOCK_SIZE (8)

	int x = get_global_id(0);
	int y = get_global_id(1);

	image[x * BLOCK_SIZE + y * BLOCK_SIZE * w] -= 1024.0f;
}