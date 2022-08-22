from sklearn.datasets import make_classification
from sklearn.datasets import make_blobs
import pandas as pd
import numpy as np
if __name__ == "__main__":
    X, _ = make_classification(n_samples=10000, n_features=3, n_informative=3,
                               n_redundant=0, n_repeated=0, n_classes=3, n_clusters_per_class=2,
                               class_sep=1.5,
                               flip_y=0, weights=[0.5, 0.5, 0.5])
    X = pd.DataFrame(X)
    np.savetxt('./a_3d.txt', X.values, fmt='%.3f')

    X, _ = make_blobs(n_samples=10000, centers=4, n_features=3, random_state=17)
    X = pd.DataFrame(X)
    np.savetxt('./b_3d.txt', X.values, fmt='%.3f')

    X, _ = make_blobs(n_samples=10000, centers=2, n_features=3, random_state=10)
    X = pd.DataFrame(X)
    np.savetxt('./c_3d.txt', X.values, fmt='%.3f')

    X, _ = make_blobs(n_samples=100, centers=3, n_features=3, random_state=5)
    X = pd.DataFrame(X)
    np.savetxt('./d_3d.txt', X.values, fmt='%.3f')
