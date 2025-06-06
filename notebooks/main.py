import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def plot_score_statistics():
    # Read the CSV file
    df = pd.read_csv("statistics_output/2025-06-06_19-18-13_score_statistics.csv", sep=";")

    x = df["Time"]
    y1 = df["Strict"]
    y2 = df["Hard"]
    y3 = df["Soft"]

    plt.figure(figsize=(10, 6))

    # plt.plot(x, y1, marker='o', label="Strict")
    # plt.plot(x, y2, marker='x', label="Hard")
    # plt.plot(x, y3, marker='^', label="Soft")
    plt.plot(x, y1, label="Strict")
    plt.plot(x, y2, label="Hard")
    plt.plot(x, y3, label="Soft")

    plt.xscale("log")  # Logarithmic time axis
    plt.xlabel("Time (log scale)")
    plt.ylabel("Score")
    plt.title("Score Evolution Over Time")
    plt.legend()
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    plot_score_statistics()

