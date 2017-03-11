__kernel void assign_cluster(
		__global float2* image_data,
		__global float2* centroids,
		__global float* centroid_assignments,
	    uint k)
{
    const int x     = get_global_id(0);
    const int y     = get_global_id(1);
    const int width = get_global_size(0);
    const int id = y * width + x;

	int min_dist = -1;
	int cluster = 0;
	float2 pixel = image_data[id];

	for (int i = 0; i < k; i++) {
		float res = distance(centroids[i], pixel);
		if (min_dist == -1 || res < min_dist) {
			min_dist = res;
			cluster = i;
		}
	}

	centroid_assignments[id] = cluster;
}

