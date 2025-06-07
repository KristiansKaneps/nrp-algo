import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d

def plot_example_3d_function():
    ax = plt.axes(projection='3d')

    # Grid setup
    x_data = np.linspace(-5, 5, 200)
    y_data = np.linspace(-5, 5, 200)
    X, Y = np.meshgrid(x_data, y_data)

    # "Simplex-style" random terrain:
    # Combining multiple frequency sinusoids + random coefficients
    np.random.seed(42) # for reproducibility
    freqs = [1, 2, 3, 5]
    Z = np.zeros_like(X)

    for f in freqs:
        amp = np.random.uniform(-500000, 0)
        phase = np.random.uniform(0, 2 * np.pi)
        Z += amp * np.sin(f * X + phase) * np.cos(f * Y - phase) - 500000

    # Add some extra non-linearity and ridge effects
    Z += 0.3 * X + 0.2 * Y ** 2 - 0.1 * np.abs(X * Y)

    # Plot
    ax.plot_surface(X, Y, Z, cmap='viridis', edgecolor='none')
    # ax.set_title("Random Constraint-like Surface")
    ax.set_xlabel(r"$S_X$")
    ax.set_ylabel(r"$S_Y$")
    ax.set_zlabel(r"$C(S)$")
    # ax.set_xticks([])
    # ax.set_yticks([])
    ax.set_xticklabels([])
    ax.set_yticklabels([])

    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    plot_example_3d_function()