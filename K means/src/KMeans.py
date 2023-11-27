import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import cv2


def read_image(filepath='../data/ustc-cow.png'):
    image = cv2.imread(filepath)  # Replace with the actual path to your image
    # Convert the image from BGR to RGB
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    return image


class KMeans:
    def __init__(self, k=4, max_iter=10):
        self.k = k
        self.max_iter = max_iter

    # Randomly initialize the centers
    def initialize_centers(self, points):
        """
        points: (n_samples, n_dims, )
        """
        n, d = points.shape

        centers = np.zeros((self.k, d))
        for k in range(self.k):
            # use more random points to initialize centers, make kmeans more stable
            random_index = np.random.choice(n, size=10, replace=False)
            centers[k] = points[random_index].mean(axis=0)

        return centers

    # Assign each point to the closest center
    @staticmethod
    def assign_points(centers, points):
        """
        centers: (n_clusters, n_dims, )
        points: (n_samples, n_dims, )
        return labels: (n_samples, )
        """
        n_samples, n_dims = points.shape
        labels = np.zeros(n_samples)

        for i in range(n_samples):
            distances = np.linalg.norm(points[i] - centers, axis=1)
            labels[i] = np.argmin(distances)

        return labels.astype(np.uint8)

    # Update the centers based on the new assignment of points
    def update_centers(self, centers, labels, points):
        """
        centers: (n_clusters, n_dims, )
        labels: (n_samples, )
        points: (n_samples, n_dims, )
        return centers: (n_clusters, n_dims, )
        """
        new_centers = np.copy(centers)
        for k in range(self.k):
            cluster_points = points[labels == k]
            if len(cluster_points) > 0:
                new_centers[k] = np.mean(cluster_points, axis=0)
        return new_centers

    # k-means clustering
    def fit(self, points):
        """
        points: (n_samples, n_dims, )
        return centers: (n_clusters, n_dims, )
        """

        centers = self.initialize_centers(points)

        for i in range(self.max_iter):
            labels = self.assign_points(centers, points)
            new_centers = self.update_centers(centers, labels, points)

            if np.array_equal(centers, new_centers):
                break

            centers = new_centers

        return centers

    def compress(self, image):
        """
        img: (width, height, 3)
        return compressed img: (width, height, 3)
        """
        # flatten the image pixels
        points = image.reshape((-1, image.shape[-1]))

        centers = self.fit(points)
        labels = self.assign_points(centers, points)

        return centers[labels].reshape(image.shape)


if __name__ == '__main__':
    matplotlib.use('Agg')
    img = read_image(filepath='../data/ustc-cow.png')
    for k in [2, 4, 8, 16, 32]:
        kmeans = KMeans(k=k, max_iter=10)
        compressed_img = kmeans.compress(img).round().astype(np.uint8)

        plt.figure(figsize=(10, 10))
        plt.imshow(compressed_img)
        plt.title('Compressed Image')
        plt.axis('off')
        plt.savefig(f'../output/compressed_image_k_{k}.png')
