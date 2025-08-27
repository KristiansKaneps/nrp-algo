import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl

# Use Times New Roman for all plot text
mpl.rcParams["font.family"] = "Times New Roman"
mpl.rcParams["font.serif"] = ["Times New Roman"]
mpl.rcParams["font.size"] = 18
mpl.rcParams["axes.titlesize"] = 24
mpl.rcParams["axes.labelsize"] = 18
mpl.rcParams["legend.fontsize"] = 16
mpl.rcParams["xtick.labelsize"] = 16
mpl.rcParams["ytick.labelsize"] = 16

def get_latest_file_in_env(env_dir: str, suffix: str) -> str:
    """Return the latest file within env's instance* subfolders by timestamped filename.

    Example suffix values:
    - "DLAS_score_statistics.csv"
    - "LAHC_score_statistics.csv"
    - "TABU_MOVE_score_statistics.csv"
    - "TABU_STATE_score_statistics.csv"
    - "SA_score_statistics.csv"
    """
    candidate_paths = []
    if not os.path.isdir(env_dir):
        raise FileNotFoundError(f"Environment directory not found: {env_dir}")

    for child_name in os.listdir(env_dir):
        child_path = os.path.join(env_dir, child_name)
        if not os.path.isdir(child_path):
            continue
        if not child_name.startswith("instance"):
            # skip folders like "prev" or others
            continue

        try:
            for filename in os.listdir(child_path):
                if filename.endswith('_' + suffix):
                    candidate_paths.append(os.path.join(child_path, filename))
        except FileNotFoundError:
            # Instance folder might disappear mid-scan; ignore
            continue

    if not candidate_paths:
        raise FileNotFoundError(f"No files ending with {suffix} found under {env_dir}/*.")

    # Filenames are of the form: YYYY-MM-DD_HH-MM-SS_ALGO_... — lexicographic sort works
    candidate_paths.sort(key=lambda p: os.path.basename(p))
    return candidate_paths[-1]

def get_latest_file_in_instance(env_dir: str, instance: str, suffix: str) -> str:
    """Return the latest file for a specific instance (e.g., instance2) within env_dir."""
    instance_dir = os.path.join(env_dir, instance)
    if not os.path.isdir(instance_dir):
        raise FileNotFoundError(f"Instance directory not found: {instance_dir}")
    files = [f for f in os.listdir(instance_dir) if f.endswith('_' + suffix)]
    if not files:
        raise FileNotFoundError(f"No files ending with {suffix} found in {instance_dir}")
    files.sort()
    return os.path.join(instance_dir, files[-1])

def get_latest_steps_file_in_instance(env_dir: str, instance: str, algo: str) -> str:
    """Return latest steps-per-second file for a specific instance and algorithm."""
    suffix = f"{algo}_steps_per_second.csv"
    return get_latest_file_in_instance(env_dir, instance, suffix)

LAHC = "LAHC"
DLAS = "DLAS"
TABU_MOVE = "TABU_MOVE"
TABU_STATE = "TABU_STATE"
SA = "SA"

# Directories
local_dir = "local"
do_dir    = "digitalocean"

instances = [
    "instance2",
    "instance7",
    "instance11",
]

env_display = {
    "local": "Pd",
    "digitalocean": "Ms",
}

# Get latest files across unified instance directories
latest = {
    "local": {
        "dlas":       get_latest_file_in_env(local_dir, "DLAS_score_statistics.csv"),
        "lahc":       get_latest_file_in_env(local_dir, "LAHC_score_statistics.csv"),
        "tabu_move":  get_latest_file_in_env(local_dir, "TABU_MOVE_score_statistics.csv"),
        # "tabu_state": get_latest_file_in_env(local_dir, "TABU_STATE_score_statistics.csv"),
        # "sa":         get_latest_file_in_env(local_dir, "SA_score_statistics.csv"),
    },
    "digitalocean": {
        "dlas":       get_latest_file_in_env(do_dir, "DLAS_score_statistics.csv"),
        "lahc":       get_latest_file_in_env(do_dir, "LAHC_score_statistics.csv"),
        "tabu_move":  get_latest_file_in_env(do_dir, "TABU_MOVE_score_statistics.csv"),
        # "tabu_state": get_latest_file_in_env(do_dir, "TABU_STATE_score_statistics.csv"),
        # "sa":         get_latest_file_in_env(do_dir, "SA_score_statistics.csv"),
    },
}

def plot_score_statistics(env: str, instance: str):
    # Read CSVs
    data = {}
    env_dir = local_dir if env == "local" else do_dir
    suffixes = {
        "DLAS": "DLAS_score_statistics.csv",
        "LAHC": "LAHC_score_statistics.csv",
        "TABU": "TABU_MOVE_score_statistics.csv",
        # "TABU (State)": "TABU_STATE_score_statistics.csv",
        # "SA": "SA_score_statistics.csv",
    }
    for algo, suffix in suffixes.items():
        try:
            path = get_latest_file_in_instance(env_dir, instance, suffix)
            data[algo] = pd.read_csv(path, sep=";")
        except FileNotFoundError:
            continue
    if not data:
        return

    # Define colors per algorithm
    algo_colors = {
        "DLAS": "tab:blue",
        "LAHC": "tab:orange",
        "TABU": "tab:green",
        # "TABU (Move)": "tab:green",
        # "TABU (State)": "tab:red",
        # "SA": "tab:purple",
    }

    # Define line styles per score type
    score_styles = {
        "Strict": "--",
        "Hard": "-",
        "Soft": ":",
    }

    # Localized display names for score types
    score_display = {
        "Strict": "Strict - Striktais",
        "Hard": "Hard - Vidējais",
        "Soft": "Soft - Mīkstais",
    }

    plt.figure(figsize=(12, 7))

    # Plot all
    for algo, df in data.items():
        for score_type, style in score_styles.items():
            plt.plot(
                df["Time"] / 1000.0,
                df[score_type],
                label=f"{algo} - {score_display.get(score_type, score_type)}",
                color=algo_colors[algo],
                linestyle=style,
                linewidth=2
            )

    # Formatting
    plt.xscale("log")
    plt.xlabel("Laiks (sekundes, logaritmisks mērogs)")
    plt.ylabel("Novērtējums (punkti)")
    title_font = {"fontname": "Times New Roman"}
    instance_index = {"instance2": 1, "instance7": 2, "instance11": 3}
    inst_num = instance_index.get(instance, ''.join([c for c in instance if c.isdigit()]) or instance)
    inst_disp = f"{inst_num}. eksemplārs"
    plt.title(f"Labākā stāvokļa novērtējums pēc laika ({env_display[env]}, {inst_disp})", fontdict=title_font)
    plt.legend()
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()

def plot_score_statistics_env_compare(instance: str):
    # Prepare data for both environments
    envs = ["local", "digitalocean"]
    suffixes = {
        "DLAS": "DLAS_score_statistics.csv",
        "LAHC": "LAHC_score_statistics.csv",
        "TABU": "TABU_MOVE_score_statistics.csv",
    }

    env_algo_to_df = {}
    for env in envs:
        env_dir = local_dir if env == "local" else do_dir
        for algo, suffix in suffixes.items():
            try:
                path = get_latest_file_in_instance(env_dir, instance, suffix)
                env_algo_to_df[(env, algo)] = pd.read_csv(path, sep=";")
            except FileNotFoundError:
                continue

    if not env_algo_to_df:
        return

    algo_colors = {
        "DLAS": "tab:blue",
        "LAHC": "tab:orange",
        "TABU": "tab:green",
    }
    # Distinct line styles per environment for comparison plots
    env_styles = {
        "local": "-",
        "digitalocean": "--",
    }
    score_display = {
        "Strict": "Strict - Striktais",
        "Hard": "Hard - Vidējais",
        "Soft": "Soft - Mīkstais",
    }

    plt.figure(figsize=(12, 7))

    for (env, algo), df in env_algo_to_df.items():
        marker = None
        markevery = None
        markersize = None
        plt.plot(
            df["Time"] / 1000.0,
            df["Hard"],
            label=f"{algo} ({env_display[env]})",
            color=algo_colors[algo],
            linestyle=env_styles.get(env, "-"),
            linewidth=2,
            marker=marker,
            markersize=markersize,
            markevery=markevery,
        )

    plt.xscale("log")
    plt.xlabel("Laiks (sekundes, logaritmisks mērogs)")
    plt.ylabel("Novērtējums (punkti)")
    title_font = {"fontname": "Times New Roman"}
    instance_index = {"instance2": 1, "instance7": 2, "instance11": 3}
    inst_num = instance_index.get(instance, ''.join([c for c in instance if c.isdigit()]) or instance)
    inst_disp = f"{inst_num}. eksemplārs"
    plt.title(f"Labākā stāvokļa novērtējums pēc laika (vides salīdzinājums, {inst_disp})", fontdict=title_font)
    plt.legend()
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()

def plot_steps_statistics(env: str, instance: str):
    # Read CSVs per algorithm (DLAS, LAHC, TABU)
    data = {}
    env_dir = local_dir if env == "local" else do_dir
    algos = ["DLAS", "LAHC", "TABU"]
    algo_file_names = {"DLAS": "DLAS", "LAHC": "LAHC", "TABU": "TABU_MOVE"}
    for algo in algos:
        try:
            file_algo = algo_file_names.get(algo, algo)
            path = get_latest_steps_file_in_instance(env_dir, instance, file_algo)
            df = pd.read_csv(path, sep=";")
            data[algo] = df
        except FileNotFoundError:
            continue
    if not data:
        return

    algo_colors = {
        "DLAS": "tab:blue",
        "LAHC": "tab:orange",
        "TABU": "tab:green",
    }
    series_styles = {
        "StepsPerSecond": ":",
        "AverageStepsPerSecond": "-",
    }
    series_display = {
        "StepsPerSecond": "",
        "AverageStepsPerSecond": "vidējais",
    }

    plt.figure(figsize=(12, 7))

    for algo, df in data.items():
        for series, style in series_styles.items():
            if series not in df.columns:
                continue
            # Build label: omit suffix for instantaneous, keep "vidējais" for average
            label_text = f"{algo}" if series == "StepsPerSecond" else f"{algo} – {series_display.get(series, 'vidējais')}"
            plt.plot(
                df["Time"] / 1000.0,
                df[series],
                label=label_text,
                color=algo_colors[algo],
                linestyle=style,
                linewidth=2,
                alpha=0.25 if series == "StepsPerSecond" else 1.0,
            )

    plt.xlabel("Laiks (sekundes)")
    plt.ylabel("Soļi sekundē")
    title_font = {"fontname": "Times New Roman"}
    instance_index = {"instance2": 1, "instance7": 2, "instance11": 3}
    inst_num = instance_index.get(instance, ''.join([c for c in instance if c.isdigit()]) or instance)
    inst_disp = f"{inst_num}. eksemplārs"
    plt.title(f"MH risināšanas ātrums ({env_display[env]}, {inst_disp})", fontdict=title_font)
    plt.subplots_adjust(bottom=0.25)
    plt.legend(loc="upper center", bbox_to_anchor=(0.5, -0.18), ncol=3, frameon=False)
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()

def plot_steps_statistics_env_compare(instance: str):
    envs = ["local", "digitalocean"]
    algos = ["DLAS", "LAHC", "TABU"]
    algo_file_names = {"DLAS": "DLAS", "LAHC": "LAHC", "TABU": "TABU_MOVE"}

    env_algo_to_df = {}
    for env in envs:
        env_dir = local_dir if env == "local" else do_dir
        for algo in algos:
            try:
                file_algo = algo_file_names.get(algo, algo)
                path = get_latest_steps_file_in_instance(env_dir, instance, file_algo)
                env_algo_to_df[(env, algo)] = pd.read_csv(path, sep=";")
            except FileNotFoundError:
                continue
    if not env_algo_to_df:
        return

    algo_colors = {
        "DLAS": "tab:blue",
        "LAHC": "tab:orange",
        "TABU": "tab:green",
    }
    env_styles = {
        "local": "-",
        "digitalocean": "--",
    }

    plt.figure(figsize=(12, 7))

    for (env, algo), df in env_algo_to_df.items():
        marker = None
        markevery = None
        markersize = None
        if "StepsPerSecond" not in df.columns:
            continue
        plt.plot(
            df["Time"] / 1000.0,
            df["StepsPerSecond"],
            label=f"{algo} ({env_display[env]})",
            color=algo_colors[algo],
            linestyle=env_styles.get(env, "-"),
            linewidth=2,
            marker=marker,
            markersize=markersize,
            markevery=markevery,
            alpha=0.25,
        )

        # Add average line at full opacity
        if "AverageStepsPerSecond" in df.columns:
            plt.plot(
                df["Time"] / 1000.0,
                df["AverageStepsPerSecond"],
                label=f"{algo} ({env_display[env]}) – vidējais",
                color=algo_colors[algo],
                linestyle=env_styles.get(env, "-"),
                linewidth=2,
                alpha=1.0,
            )

    plt.xlabel("Laiks (sekundes)")
    plt.ylabel("Soļi sekundē")
    title_font = {"fontname": "Times New Roman"}
    instance_index = {"instance2": 1, "instance7": 2, "instance11": 3}
    inst_num = instance_index.get(instance, ''.join([c for c in instance if c.isdigit()]) or instance)
    inst_disp = f"{inst_num}. eksemplārs"
    plt.title(f"MH risināšanas ātrums (vides salīdzinājums, {inst_disp})", fontdict=title_font)
    plt.subplots_adjust(bottom=0.35)
    plt.legend(loc="upper center", bbox_to_anchor=(0.5, -0.26), ncol=4, frameon=False)
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    for env in ["local", "digitalocean"]:
        for inst in instances:
            plot_score_statistics(env, inst)
            plot_steps_statistics(env, inst)
    for inst in instances:
        plot_score_statistics_env_compare(inst)
        plot_steps_statistics_env_compare(inst)
