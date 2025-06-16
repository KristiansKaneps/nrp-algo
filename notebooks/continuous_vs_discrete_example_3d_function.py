import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d

def plot_simple_3d_function():
    # First plot: Paraboloid with sinusoidal modulation
    fig1 = plt.figure()
    ax1 = fig1.add_subplot(111, projection='3d')

    # Grid setup with zoomed-in range
    x_data = np.linspace(-3, 3, 200)
    y_data = np.linspace(-3, 3, 200)
    X, Y = np.meshgrid(x_data, y_data)

    # Simple paraboloid with sinusoidal modulation
    Z = X**2 + Y**2 + 5 * np.sin(np.sqrt(X**2 + Y**2))

    # Plot
    ax1.plot_surface(X, Y, Z, cmap='viridis', edgecolor='none')
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

    # Second plot: Discrete stepped surface
    fig2 = plt.figure()
    ax2 = fig2.add_subplot(111, projection='3d')

    # Same grid setup
    x_data = np.linspace(-3, 3, 200)
    y_data = np.linspace(-3, 3, 200)
    X, Y = np.meshgrid(x_data, y_data)

    # Discrete function using floor for stepped effect
    Z_discrete = np.floor(X**2 + Y**2 + 3 * np.sin(np.sqrt(X**2 + Y**2)))

    # Plot
    ax2.plot_surface(X, Y, Z_discrete, cmap='viridis', edgecolor='none')
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
    plot_simple_3d_function()