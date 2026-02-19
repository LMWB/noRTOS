import tkinter as tk
from tkinter import ttk, scrolledtext

import serial.tools.list_ports

import datetime

import json

from serial_interface import SerialInterface

from data_logger import DataLogger


class SerialGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Serial Monitor")

        self.serial = None
        self.logging_active = False
        self.log_file = None
        self.csv_writer = None

        self.temp_vars = {}
        self.temp_min = {}
        self.temp_max = {}

        self._build_ui()

    # ───────────────────────────── UI SETUP ─────────────────────────────
    def _build_ui(self):
        # --- Config frame ---
        config_frame = tk.Frame(self.root)
        config_frame.pack(padx=10, pady=(10, 0), fill=tk.X)

        tk.Label(config_frame, text="Port:").pack(side=tk.LEFT)
        self.port_var = tk.StringVar()
        self.port_dropdown = ttk.Combobox(config_frame, textvariable=self.port_var, width=15, state='readonly')
        self.port_dropdown['values'] = self._get_serial_ports()
        self.port_dropdown.pack(side=tk.LEFT, padx=5)

        tk.Label(config_frame, text="Baudrate:").pack(side=tk.LEFT)
        self.baud_var = tk.StringVar(value="115200")
        self.baud_dropdown = ttk.Combobox(config_frame, textvariable=self.baud_var, width=10, state='readonly')
        self.baud_dropdown['values'] = ["9600", "19200", "38400", "57600", "115200"]
        self.baud_dropdown.pack(side=tk.LEFT, padx=5)

        self.connect_btn = tk.Button(config_frame, text="Connect", command=self.connect_serial)
        self.connect_btn.pack(side=tk.LEFT, padx=5)

        self.disconnect_btn = tk.Button(config_frame, text="Disconnect", command=self.disconnect_serial, state='disabled')
        self.disconnect_btn.pack(side=tk.LEFT, padx=5)

        # --- Logging and Clear buttons ---
        log_frame = tk.Frame(self.root)
        log_frame.pack(padx=10, pady=(10, 0), fill=tk.X)

        tk.Label(log_frame, text="CSV File:").pack(side=tk.LEFT)
        self.log_filename = tk.Entry(log_frame, width=30)
        self.log_filename.insert(0, "serial_log.csv")
        self.log_filename.pack(side=tk.LEFT, padx=5)

        self.log_btn = tk.Button(log_frame, text="Start Logging", command=self.toggle_logging, state='disabled')
        self.log_btn.pack(side=tk.LEFT, padx=5)

        self.clear_btn = tk.Button(log_frame, text="Clear Window", command=self.clear_window)
        self.clear_btn.pack(side=tk.RIGHT, padx=5)

        # --- Live Temperature Frame ---
        temps_frame = tk.LabelFrame(self.root, text="Live Temperatures")
        temps_frame.pack(padx=10, pady=(10, 0), fill=tk.X)


        self.temp_vars = {}
        self.temp_min = {}
        self.temp_max = {}
        self.temp_min_labels = {}
        self.temp_max_labels = {}

        for i in range(1, 9):
            # Temperature label (T1 ... T8)
            label = tk.Label(temps_frame, text=f"T{i}:", width=5, anchor="e")
            label.grid(row=0, column=(i - 1) * 2, padx=(5, 0))

            # Temperature display entry
            val = tk.StringVar(value="—")
            entry = tk.Entry(
                temps_frame,
                textvariable=val,
                width=8,
                state='readonly',
                justify="center",
                readonlybackground="#E0E0E0"  # light gray before data
            )
            entry.grid(row=0, column=(i - 1) * 2 + 1, padx=(0, 5))

            # Min/Max labels below each entry
            min_label = tk.Label(temps_frame, text="Min: —", font=("TkDefaultFont", 8))
            min_label.grid(row=1, column=(i - 1) * 2, columnspan=2)
            max_label = tk.Label(temps_frame, text="Max: —", font=("TkDefaultFont", 8))
            max_label.grid(row=2, column=(i - 1) * 2, columnspan=2)

            # Store references
            self.temp_vars[f"T{i}"] = val
            self.temp_min[f"T{i}"] = None
            self.temp_max[f"T{i}"] = None
            self.temp_min_labels[f"T{i}"] = min_label
            self.temp_max_labels[f"T{i}"] = max_label


        # --- Message display ---
        self.text_area = scrolledtext.ScrolledText(self.root, wrap=tk.WORD, width=100, height=18, state='disabled')
        self.text_area.pack(padx=10, pady=10)

        # --- Input frame ---
        input_frame = tk.Frame(self.root)
        input_frame.pack(padx=10, pady=(0, 10), fill=tk.X)

        self.entry = tk.Entry(input_frame, width=50, state='disabled')
        self.entry.pack(side=tk.LEFT, expand=True, fill=tk.X)

        self.send_btn = tk.Button(input_frame, text="Send", command=self.send_message, state='disabled')
        self.send_btn.pack(side=tk.RIGHT)

        self.entry.bind("<Return>", lambda event: self.send_message())

        # ───────────────────────────── HELPER SERIAL HANDLING ─────────────────────────────

    def _update_temperature_display(self, key, value):
        """Update temperature value, min/max, and color display."""
        if key not in self.temp_vars:
            return

        # Update current value
        self.temp_vars[key].set(f"{value:.2f}")

        # Update min/max
        if self.temp_min[key] is None or value < self.temp_min[key]:
            self.temp_min[key] = value
        if self.temp_max[key] is None or value > self.temp_max[key]:
            self.temp_max[key] = value

        # Update labels
        self.temp_min_labels[key].config(text=f"Min: {self.temp_min[key]:.2f}")
        self.temp_max_labels[key].config(text=f"Max: {self.temp_max[key]:.2f}")

        # Determine background color based on range
        if value < 20:
            color = "#8EC9FF"   # blue
        elif value <= 60:
            color = "#B4F8C8"   # green
        else:
            color = "#FF9999"   # red

        # Find entry widget by traversing temp_vars' corresponding Entry
        entry_widget = None
        for widget in self.root.winfo_children():
            for sub in widget.winfo_children():
                if isinstance(sub, tk.Entry) and sub.cget("textvariable") == str(self.temp_vars[key]):
                    entry_widget = sub
                    break
            if entry_widget:
                break

        if entry_widget:
            entry_widget.config(readonlybackground=color)


    # ───────────────────────────── SERIAL HANDLING ─────────────────────────────
    def _get_serial_ports(self):
        return [port.device for port in serial.tools.list_ports.comports()]

    def connect_serial(self):
        port = self.port_var.get()
        try:
            baud = int(self.baud_var.get())
        except ValueError:
            self._append_message("Invalid baudrate")
            return

        if not port:
            self._append_message("Please select a port.")
            return

        try:
            self.serial = SerialInterface(port, baud, timeout=0.1)
            self.serial.add_listener(self.on_serial_message)
            self.serial.start()

            self._append_message(f"[INFO] Connected to {port} at {baud} baud.")
            self.connect_btn.config(state='disabled')
            self.disconnect_btn.config(state='normal')
            self.entry.config(state='normal')
            self.send_btn.config(state='normal')
            self.log_btn.config(state='normal')
        except Exception as e:
            self._append_message(f"[ERROR] {e}")

    def disconnect_serial(self):
        if self.serial:
            self.serial.stop()
            self.serial = None
        if self.logging_active:
            self._stop_logging()
        self._append_message("[INFO] Disconnected.")
        self.entry.config(state='disabled')
        self.send_btn.config(state='disabled')
        self.log_btn.config(state='disabled')
        self.connect_btn.config(state='normal')
        self.disconnect_btn.config(state='disabled')

    def send_message(self):
        if not self.serial:
            return
        message = self.entry.get().strip()
        if message:
            self.serial.send(message)
            self.entry.delete(0, tk.END)

    # ───────────────────────────── MESSAGE HANDLING ─────────────────────────────
    def on_serial_message(self, message):
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        formatted = f"{timestamp}  |  {message}"
        try:
            data = json.loads(message)
            if isinstance(data, dict):
                for key, val in data.items():
                    if key in self.temp_vars and isinstance(val, (int, float)):
                        self.root.after(0, self._update_temperature_display, key, val)
        except Exception:
            pass
        
        self.root.after(0, self._append_message, formatted)

        if self.logging_active and self.logger:
            self.logger.log(message)

    def _append_message(self, message):
        self.text_area.config(state='normal')
        self.text_area.insert(tk.END, message + '\n')
        self.text_area.see(tk.END)
        self.text_area.config(state='disabled')

    def clear_window(self):
        self.text_area.config(state='normal')
        self.text_area.delete('1.0', tk.END)
        self.text_area.config(state='disabled')

    # ───────────────────────────── LOGGING ─────────────────────────────
    def toggle_logging(self):
        if self.logging_active:
            self._stop_logging()
        else:
            self._start_logging()

    def _start_logging(self):
        filename = self.log_filename.get().strip()
        if not filename:
            self._append_message("[WARN] No log filename specified.")
            return

        try:
            self.logger = DataLogger(self.log_filename.get())
            self.logger.start()
            self.logging_active = True
            self.log_btn.config(text="Stop Logging")
            self._append_message(f"[INFO] Logging started → {filename}")
        except Exception as e:
            self._append_message(f"[ERROR] Could not open log file: {e}")

    def _stop_logging(self):
        if self.logging_active:
            self.logging_active = False
            self.log_btn.config(text="Start Logging")
            self.logger.stop()
            self._append_message("[INFO] Logging stopped.")
    # ───────────────────────────── EXIT HANDLING ─────────────────────────────
    def on_close(self):
        if self.serial:
            self.serial.stop()
        if self.logging_active:
            self._stop_logging()
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = SerialGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
