// I will write this function as an OpenCL kernel if time permits
// It's not part of the code algorithm but could stand to benefit 
// from the GPU
convertRBGtoYUV(image)

centroids = random_centroids(image, k)

do {
	enqueueKernel(clusterByCentroid(image, centroids))
}
while (check_convergence(image, centroids))


// Same note as for convertRGBtoYUV -- will parallelize if time permits
colorize(image, centroids)
convertYUVtoRGB(image)




kernel clusterByCentroid(image, centroids) {
	index = get_global_id(0)
	distance = -1
	centroid = -1

	// compute the minimum euclidean distance of this point compared to all centroids
	for (c in centroids) {
		d = sqrt(pow(image[index].x - c.x, 2) + pow(image[index].y - c.y, 2))
		if (distance == -1) {
			distance = d;
			centroid = c;
		} else if (d <= distance) {
			distance = d;
			centroid = c;
		}
	}

	// set the empty value of the original image to the cluster id
	image[index].w = c
}

// This is being done by the host
// Alternatively, it could be done in GPU by using atomic_inc to increment
// indicies and counts from within the kernel to a shared memory space.
// I wanted to avoid that initially due to the synchronization it will
// induce in the most mathematically intensive parts of the kernel, but 
// I may try to implement it anyway to see what the cost is compared with
// going back to the host to calculate the convergence. 
bool check_convergence(image, centroids) {
	mean_centroids;
	mean_centroids_count;

	for (i in image) {
		mean_centroids[image[i].w] += image[i]
		mean_centroids_count[image[i].w]++;		
	}

	for (i in mean_centroids) {
		if (mean_centroids[i] != centroids[i]) {
			return true;
		}
	}
	return false;
}
