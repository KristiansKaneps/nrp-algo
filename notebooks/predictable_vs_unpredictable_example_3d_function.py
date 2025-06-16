import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d

def plot_3d_functions():
    # First plot: Slightly unpredictable but still bowl-shaped discrete function
    fig1 = plt.figure()
    ax1 = fig1.add_subplot(111, projection='3d')

    # Grid setup with zoomed-in range
    x_data = np.linspace(-3, 3, 200)
    y_data = np.linspace(-3, 3, 200)
    X, Y = np.meshgrid(x_data, y_data)

    # Slightly unpredictable function: Paraboloid with increased sinusoidal modulation, made discrete
    Z_predictable = np.floor(0.5 * (X**2 + Y**2) + 3 * np.sin(2 * np.sqrt(X**2 + Y**2)) + 2 * np.cos(3 * X) * np.sin(3 * Y))

    # Plot
    ax1.plot_surface(X, Y, Z_predictable, cmap='viridis', edgecolor='none')
    ax1.set_xlabel(r"$x$")
    ax1.set_ylabel(r"$y$")
    ax1.set_zlabel(r"$z$")
    ax1.set_xticklabels([])
    ax1.set_yticklabels([])
    ax1.set_zticklabels([])

    # Remove padding
    plt.margins(0)
    fig1.subplots_adjust(left=0, right=1, top=1, bottom=0)

    plt.show()

    # Second plot: Unpredictable discrete function
    fig2 = plt.figure()
    ax2 = fig2.add_subplot(111, projection='3d')

    # Same grid setup
    x_data = np.linspace(-3, 3, 200)
    y_data = np.linspace(-3, 3, 200)
    X, Y = np.meshgrid(x_data, y_data)

    # Unpredictable function: High-frequency sinusoids + nonlinear terms, made discrete
    Z_unpredictable = np.floor(10 * np.sin(3 * X) * np.cos(3 * Y) +
                              5 * np.sin(5 * X + 1) * np.cos(5 * Y - 1) +
                              0.5 * np.abs(X * Y))

    # Plot
    ax2.plot_surface(X, Y, Z_unpredictable, cmap='viridis', edgecolor='none')
    ax2.set_xlabel(r"$x$")
    ax2.set_ylabel(r"$y$")
    ax2.set_zlabel(r"$z$")
    ax2.set_xticklabels([])
    ax2.set_yticklabels([])
    ax2.set_zticklabels([])

    # Remove padding
    plt.margins(0)
    fig2.subplots_adjust(left=0, right=1, top=1, bottom=0)

    plt.show()

if __name__ == '__main__':
    plot_3d_functions()