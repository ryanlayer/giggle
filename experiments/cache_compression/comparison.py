from math import ceil
import matplotlib.pyplot as plt

colors = {
    'uncompressed': 'g',
    'fastlz': 'b',
    'zlib': 'r',
}
data = {
    'indexing': {
        'algo': {
            'uncompressed': [[0], [70.49318158458]],
            'fastlz': [[1, 2], [89.33131507, 155.041745]],
            'zlib': [[0, 1, 3, 6, 9], [88.74085122, 107.4996464, 116.3000386, 122.0017465, 226.4432716]],
        },
        'y_label': 'Time (s)'
    },
    'space': {
        'algo': {
            'uncompressed': [[0], [1.452596008]],
            'fastlz': [[1, 2], [0.681222584, 0.68060137]],
            'zlib': [[0, 1, 3, 6, 9], [1.459414745, 0.433347471, 0.428881416, 0.405332483, 0.40376754]],
        },
        'y_label': 'Space (GB)'
    },
    'search1': {
        'algo': {
            'uncompressed': [[0], [45.5873693]],
            'fastlz': [[1, 2], [59.18466978, 56.41575026]],
            'zlib': [[0, 1, 3, 6, 9], [54.34283227, 74.10076984, 73.39287504, 66.19744482, 67.62356376]],
        },
        'y_label': 'Time (ms)'
    },
    'search2': {
        'algo': {
            'uncompressed': [[0], [9.189936953]],
            'fastlz': [[1, 2], [12.98639579, 12.19613769]],
            'zlib': [[0, 1, 3, 6, 9], [11.95358581, 12.96278894, 12.6532173, 12.20321933, 12.62452388]],
        },
        'y_label': 'Time (ms)'
    }
}

for comparison_type in data:
    comparison = data[comparison_type]
    plt.figure()
    comparison_algo = comparison['algo']
    for algo in comparison_algo:
        color = colors[algo]
        algo_values = comparison_algo[algo]

        plt.plot(*algo_values, label=algo, color=color)
        plt.scatter(*algo_values, marker='v', color=color)

    plt.xlabel('Compression Level')
    plt.ylabel(comparison['y_label'])
    plt.xticks(range(10))
    plt.gca().set_ylim(bottom=0)
    plt.title(f'Compression Performance- {comparison_type}')
    plt.legend()

    plt.savefig(f'compression-{comparison_type}.png')
