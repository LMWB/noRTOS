#!/usr/bin/env python3
"""
Simple GUI + CLI CSV Plotter for temperature logs.
Uses pandas + matplotlib to plot timestamped data.

📦 Requirements (Ubuntu/Debian):
    sudo apt install python3-tk python3-pil python3-pil.imagetk
    pip install pandas matplotlib

💡 Description:
    - Opens a GUI window with a "Browse" button to select a CSV file (no args)
    - OR: can be run from CLI with a CSV file path as argument
      Example:
          python3 csv_plotter_gui.py my_log.csv
    - Expects a CSV with a "Timestamp" column and any number of numeric columns.
    - Automatically parses timestamps and plots all numeric data vs. time.
    - The plot opens in an interactive Matplotlib window.
    
🕓 Example CSV format:
    Timestamp,T1,T2,T3
    2025-10-22 14:11:07.372,17.19,15.65,63.29
    2025-10-22 14:11:08.806,17.22,15.68,63.35
    ...
"""

import sys
import tkinter as tk
from tkinter import filedialog, messagebox
import pandas as pd
import matplotlib
matplotlib.use("TkAgg")  # ensure Matplotlib uses Tkinter GUI backend
import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter, AutoDateLocator


def plot_csv(csv_file):
    """Load CSV file and plot all numeric columns vs Timestamp."""
    try:
        df = pd.read_csv(
            csv_file,
            sep=",",
            decimal=".",
            parse_dates=["Timestamp"],
            dayfirst=False,
            encoding="utf-8",
            skip_blank_lines=True
        )
    except Exception as e:
        messagebox.showerror("Read Error", f"Failed to read file:\n{e}")
        return

    if "Timestamp" not in df.columns:
        messagebox.showerror("Invalid CSV", "No 'Timestamp' column found.")
        return

    df = df.dropna(subset=["Timestamp"])

    plt.figure(figsize=(12, 6))
    for col in df.columns[1:]:
        if pd.api.types.is_numeric_dtype(df[col]):
            plt.plot(df["Timestamp"], df[col], label=col, linewidth=1.2)

    plt.title(f"CSV Data: {csv_file}")
    plt.xlabel("Time")
    plt.ylabel("Value")
    plt.grid(True, linestyle="--", alpha=0.6)
    plt.legend(loc="upper left", ncol=2)

    ax = plt.gca()
    ax.xaxis.set_major_locator(AutoDateLocator())
    ax.xaxis.set_major_formatter(DateFormatter("%d.%m.%y-%H:%M:%S"))
    plt.xticks(rotation=30, ha="right")

    plt.tight_layout()
    plt.show()


class CSVPlotterGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("CSV Plotter")

        # --- File selection ---
        frame = tk.Frame(root, padx=10, pady=10)
        frame.pack(fill="x")

        tk.Label(frame, text="Select CSV File:").pack(side="left")
        self.file_entry = tk.Entry(frame, width=60)
        self.file_entry.pack(side="left", padx=5)
        tk.Button(frame, text="Browse", command=self.browse_file).pack(side="left")

        # --- Plot button ---
        tk.Button(root, text="Plot CSV Data", command=self.plot_csv).pack(pady=10)

    def browse_file(self):
        """Open file dialog to select CSV."""
        file_path = filedialog.askopenfilename(
            title="Select CSV file",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")]
        )
        if file_path:
            self.file_entry.delete(0, tk.END)
            self.file_entry.insert(0, file_path)

    def plot_csv(self):
        """Plot selected CSV."""
        csv_file = self.file_entry.get().strip()
        if not csv_file:
            messagebox.showwarning("No file", "Please select a CSV file first.")
            return
        plot_csv(csv_file)


if __name__ == "__main__":
    if len(sys.argv) > 1:
        # --- CLI mode ---
        csv_file = sys.argv[1]
        plot_csv(csv_file)
    else:
        # --- GUI mode ---
        root = tk.Tk()
        app = CSVPlotterGUI(root)
        root.mainloop()